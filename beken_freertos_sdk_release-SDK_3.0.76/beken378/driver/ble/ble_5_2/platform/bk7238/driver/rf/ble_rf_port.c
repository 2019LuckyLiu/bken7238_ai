#include "ble_rf_port.h"
#include "rwip_config.h"   // RW SW configuration
#include "rwip.h"
#include "rwip_int.h"
#include "architect.h"      // architectural platform definitions
#include <stdlib.h>    // standard lib functions
#include <stddef.h>    // standard definitions
#include <stdint.h>    // standard integer definition
#include <stdbool.h>   // boolean definition
#include <string.h>    // boolean definition
#include "rwip.h"      // RW SW initialization
#include "prf.h"       // RW SW initialization
#include "rwble.h"
#include "uart_pub.h"
#include "rtos_pub.h"
#include "ble.h"
#include "ble_pub.h"
#include "ble_api_5_x.h"
#include "sys_ctrl_pub.h"
#include "icu_pub.h"
#include "intc_pub.h"
#include "drv_model_pub.h"
#include "include.h"
#include "drv_model_pub.h"
#include "intc_pub.h"
#include "uart_pub.h"
#include "app_task.h"
#include "udebug.h"
#include "typedef.h"
#include "common_bt.h"
#include "arm_arch.h"
#include "power_save_pub.h"
#include "gapc_msg.h" 
#include "bk7011_cal_pub.h"

#if (CFG_LOW_VOLTAGE_PS)
#include "low_voltage_ps.h"
#endif

uint8_t ble_switch_mac_sleeped;
uint32_t rf_wifi_used = 0;
extern void bk7011_set_rx_hpf_bypass(UINT8 bypass);

uint8 is_rf_switch_to_ble(void)
{
	UINT32 param;

	sddev_control(SCTRL_DEV_NAME, CMD_BLE_RF_BIT_GET, &param);

	return (param > 0 ) ? 1 : 0;
}

void rf_wifi_used_set(void)
{
	GLOBAL_INT_DECLARATION();
	GLOBAL_INT_DISABLE();
	if (0 == rf_wifi_used) {
		rf_wifi_used = 1;
	}
	GLOBAL_INT_RESTORE();
}

void rf_wifi_used_clr(void)
{
	GLOBAL_INT_DECLARATION();
	GLOBAL_INT_DISABLE();
	if (1 == rf_wifi_used) {
		rf_wifi_used = 0;
	}
	GLOBAL_INT_RESTORE();
}

UINT32 if_rf_wifi_used(void)
{
	uint32_t value = 0;
	GLOBAL_INT_DECLARATION();
	GLOBAL_INT_DISABLE();
	value = rf_wifi_used;
	GLOBAL_INT_RESTORE();
	return value;
}

uint8_t if_ble_sleep(void)
{
	uint8_t ret;
	extern struct rwip_env_tag rwip_env;
	GLOBAL_INT_DIS();
	if (ble_ps_enabled()) {
		ret = (rwip_env.prevent_sleep & RW_DEEP_SLEEP);
	} else {
		ret = 0;
	}
	GLOBAL_INT_RES();

	return ret;
}

void ble_switch_rf_to_wifi(void)
{
	// if in ble dut mode, no need change back to wifi any more.
	// ble dut mode can not exit until power off
	// if (!is_rf_switch_to_ble() || power_save_if_rf_sleep())
	if (!is_rf_switch_to_ble())
		return;

	GLOBAL_INT_DECLARATION();
	GLOBAL_INT_DISABLE();

	sddev_control(SCTRL_DEV_NAME, CMD_BLE_RF_BIT_CLR, NULL);
	ble_switch_mac_sleeped = 0;
	//ble_cal_recover_txpwr();
	bk7011_set_rx_hpf_bypass(0);
	power_save_rf_hold_bit_clear(RF_HOLD_BY_BLE_BIT);
	//after swtich wifi check if can stop rf

	if (power_save_if_rf_sleep()) {
		GLOBAL_INT_RESTORE();
		return;
	} else {
#ifdef BLE_CHOOSE_WIFI_IDLE
		if (ble_switch_old_state != HW_IDLE && nxmac_current_state_getf() == HW_IDLE) {
			if(ke_state_get(TASK_MM) == MM_ACTIVE) {
				nxmac_next_state_setf(ble_switch_old_state);
				while (nxmac_current_state_getf() != ble_switch_old_state);
			}
		}
#endif

		power_save_rf_ps_wkup_semlist_set();

#ifdef BLE_CHOOSE_WIFI_IDLE
		//Re-enable MAC interrupts
		nxmac_enable_master_gen_int_en_setf(1);
		nxmac_enable_master_tx_rx_int_en_setf(1);
		ble_is_revert_all = 0;
		//PS_DEBUG_RF_UP_TRIGER;
#endif
	}

	GLOBAL_INT_RESTORE();
}

extern UINT32 txl_cntrl_pck_get(void);
extern uint32_t sch_arb_prog_check_role_and_index(void);

#define MAX_SKIP_CNT 5
uint32_t ble_switch_skip_cnt = 0;

static void ble_switch_rf_to_ble(void)
{
	uint32_t ble_infor = sch_arb_prog_check_role_and_index();
	uint8_t ble_skip = MAX_SKIP_CNT;

	if (ble_infor & (0x02 << 24)) {
		uint8_t ble_sync_lose = (ble_infor & 0xff);
		ble_skip = 4;
		if (ble_sync_lose > ble_skip) {
			ble_skip = 0;
		}
	}

	if (if_rf_wifi_used() && ble_switch_skip_cnt < ble_skip) {
		ble_switch_skip_cnt++;
		return;
	}

	GLOBAL_INT_DECLARATION();
	GLOBAL_INT_DISABLE();

	if (txl_cntrl_pck_get() && ble_switch_skip_cnt < ble_skip) {
		ble_switch_skip_cnt++;
		GLOBAL_INT_RESTORE();
		return;
	}

	if ((ps_get_sleep_prevent() & 0x35) && ble_switch_skip_cnt < ble_skip) {
		ble_switch_skip_cnt++;
		GLOBAL_INT_RESTORE();
		return;
	}

	if(REG_READ((ICU_BASE + 19 * 4))
		& (CO_BIT(FIQ_MAC_TX_RX_MISC)
		  | CO_BIT(FIQ_MAC_TX_RX_TIMER)
		  | CO_BIT(FIQ_MAC_RX_TRIGGER)
		  | CO_BIT(FIQ_MAC_TX_TRIGGER)
		  | CO_BIT(FIQ_MAC_GENERAL)
		  | CO_BIT(FIQ_MAC_PROT_TRIGGER)
		  | CO_BIT(FIQ_DPLL_UNLOCK)
		  )) {
		GLOBAL_INT_RESTORE();
		return;
	}

	ble_switch_skip_cnt = 0;

	power_save_rf_hold_bit_set(RF_HOLD_BY_BLE_BIT);
	bk7011_set_rx_hpf_bypass(1);
	#if (CFG_LOW_VOLTAGE_PS)
	if (LV_PS_ENABLED)
	{
		extern void power_save_ble_lv_cb(void);
		power_save_ble_lv_cb();
	}
	#endif
	//after swtich ble check if need start rf

#ifdef BLE_CHOOSE_WIFI_IDLE
	ble_switch_old_state = nxmac_current_state_getf();

	// Ask HW to go to IDLE
	if (ble_switch_old_state == HW_ACTIVE)
	{
		uint32_t i_tmp = 0, y_tmp = 0;
		uint32_t v_tmp;

		// Ask HW to go to IDLE
		if (nxmac_current_state_getf() != HW_IDLE) {
			nxmac_next_state_setf(HW_IDLE);

			while (1) {
				if (nxmac_status_idle_interrupt_getf() == 1) {
					break;
				} else {
					i_tmp++;
					if (i_tmp > 1000) {
						i_tmp = 0;
						v_tmp =nxmac_state_cntrl_get();
						PS_WPRT("s s:0x%x c:0x%x\r\n",v_tmp,power_save_get_sleep_count());
						y_tmp ++;
						if (y_tmp > 500) {
							y_tmp = 0;
							PS_WPRT("idle rec\r\n");
							bk_printf("idle rec\r\n");
							//long time wait idle interrupt,recover!
							hal_machw_disable_int();
							rwnxl_reset_handle(0);
							PS_WPRT("idle rec over\r\n");
							nxmac_next_state_setf(HW_IDLE);
						}
					}
				}
			}
			//while (nxmac_status_idle_interrupt_getf() != 1);
			nxmac_gen_int_ack_clear(NXMAC_IDLE_INTERRUPT_BIT);

			// reset modem.
			sctrl_modem_core_reset();

			// Disable MAC interrupts and clear all existing ones
			ble_switch_clear_mac_interrupts();
		}
	} else {
		ble_switch_old_state = HW_IDLE;
	}
	ble_is_revert_all = 1;
#endif
	sddev_control(SCTRL_DEV_NAME, CMD_BLE_RF_BIT_SET, NULL);

	//ble_cal_set_txpwr(tx_pwr_idx);

	ble_switch_mac_sleeped = 1;

	GLOBAL_INT_RESTORE();
	//PS_DEBUG_RF_UP_TRIGER;
}


void ble_request_rf_by_isr(void)
{
	if (ble_get_sys_mode() == NORMAL_MODE && !ble_coex_pta_is_on()) {
		ble_switch_rf_to_ble();
	}
}

void ble_release_rf_by_isr(void)
{
	if (ble_get_sys_mode() == NORMAL_MODE && !ble_coex_pta_is_on()) {
		ble_switch_rf_to_wifi();
	}
}

#ifdef BLE_CHOOSE_WIFI_IDLE
static void ble_switch_rf_exit(void)
{
	GLOBAL_INT_DECLARATION();

	GLOBAL_INT_DISABLE();
	if (ble_is_revert_all)
	{
		if (ble_switch_old_state != HW_IDLE && nxmac_current_state_getf() == HW_IDLE) {
			if(ke_state_get(TASK_MM) == MM_ACTIVE) {
				nxmac_next_state_setf(ble_switch_old_state);
				while (nxmac_current_state_getf() != ble_switch_old_state);
			}
		}

		//Re-enable MAC interrupts
		nxmac_enable_master_gen_int_en_setf(1);
		nxmac_enable_master_tx_rx_int_en_setf(1);
		ble_delegate_ps_restore_mac = 0;
		ble_is_revert_all = 0;
	}
	GLOBAL_INT_RESTORE();
	power_save_rf_ps_wkup_semlist_set();

}
#endif

void ble_delegate_ps_restore_mac_flag_clear(void)
{
#ifdef BLE_CHOOSE_WIFI_IDLE
	GLOBAL_INT_DECLARATION();

	GLOBAL_INT_DISABLE();
	if((ble_delegate_ps_restore_mac) && (ble_active)
		&& (kernel_state_get(TASK_BLE_APP) > APPM_READY)
		&& (ble_is_revert_all != 0))
	{
		ble_delegate_ps_restore_mac = 0;
	}
	GLOBAL_INT_RESTORE();
#endif
}

void ble_stop_delegate_restore_mac_state(int flag)
{
#ifdef BLE_CHOOSE_WIFI_IDLE
	GLOBAL_INT_DECLARATION();

	GLOBAL_INT_DISABLE();
	if((ble_is_revert_all)
		&& (((kernel_state_get(TASK_BLE_APP) == APPM_READY)||(ble_active == 0))&&(!flag)))
	{
		ble_delegate_ps_restore_mac = 1;
	}
	GLOBAL_INT_RESTORE();
#endif
}

void ps_recover_ble_switch_mac_status(void)
{
#ifdef BLE_CHOOSE_WIFI_IDLE
	if( ble_is_revert_all )
	{
		ble_stop_delegate_restore_mac_state(0);
	}

	if(ble_delegate_ps_restore_mac)
	{
		ble_switch_rf_exit();
	}
#endif

}

