#include "include.h"
#include "arm_arch.h"
#include "uart_pub.h"
#include "uart.h"
#include "drv_model_pub.h"
#include "sys_ctrl_pub.h"
#include "mem_pub.h"
#include "icu_pub.h"
#include "gpio_pub.h"
#include <stdio.h>
#include "mem_pub.h"
#include "intc_pub.h"

#if CFG_USE_STA_PS
#include "power_save_pub.h"
#endif
#include "mcu_ps_pub.h"

#if (CFG_SUPPORT_RTT)
#include <rtthread.h>
#include <rthw.h>
#endif

#if CFG_ENABLE_DEMO_TEST
#include "uart1_tcp_server_demo.h"
#endif

#if (CFG_SOC_NAME == SOC_BK7231N) || (CFG_SOC_NAME == SOC_BK7238) || (CFG_SOC_NAME == SOC_BK7252N && CFG_SOC_NAME_VARIANT == 0)
int uart_print_port = UART1_PORT;
#else
int uart_print_port = UART2_PORT;
#endif

#ifndef FALSE
#define FALSE    0
#endif

#ifndef TRUE
#define TRUE    1
#endif

#if AT_SERVICE_CFG
int print_log_enable = FALSE;
#endif

static struct uart_callback_des uart_receive_callback[3] = {{NULL}, {NULL}, {NULL}};
static struct uart_callback_des uart_txfifo_needwr_callback[3] = {{NULL}, {NULL}, {NULL}};
static struct uart_callback_des uart_tx_end_callback[3] = {{NULL}, {NULL}, {NULL}};

UART_S uart[3] =
{
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0}
};

static DD_OPERATIONS uart1_op =
{
    uart1_open,
    uart1_close,
    uart1_read,
    uart1_write,
    uart1_ctrl
};

static DD_OPERATIONS uart2_op =
{
    uart2_open,
    uart2_close,
    uart2_read,
    uart2_write,
    uart2_ctrl
};

#if (CFG_SOC_NAME == SOC_BK7252N)
static DD_OPERATIONS uart3_op =
{
    uart3_open,
    uart3_close,
    uart3_read,
    uart3_write,
    uart3_ctrl
};
#endif

#if AT_SERVICE_CFG
int log_enable(void)
{
    print_log_enable = TRUE;
    return print_log_enable;
}

int log_disable(void)
{
    print_log_enable = FALSE;
    return print_log_enable;
}
#endif

UINT8 uart_is_tx_fifo_empty(UINT8 uport)
{
	UINT32 param;

    if(UART1_PORT == uport)
        param = REG_READ(REG_UART1_FIFO_STATUS);
#if (CFG_SOC_NAME == SOC_BK7252N)
    else if(UART3_PORT == uport)
        param = REG_READ(REG_UART3_FIFO_STATUS);
#endif
    else
        param = REG_READ(REG_UART2_FIFO_STATUS);

    return (param & TX_FIFO_EMPTY) != 0 ? 1 : 0;
}

UINT8 uart_is_tx_fifo_full(UINT8 uport)
{
	UINT32 param;

    if(UART1_PORT == uport)
        param = REG_READ(REG_UART1_FIFO_STATUS);
 #if (CFG_SOC_NAME == SOC_BK7252N)
    else if(UART3_PORT == uport)
        param = REG_READ(REG_UART3_FIFO_STATUS);
#endif
    else
        param = REG_READ(REG_UART2_FIFO_STATUS);

    return (param & TX_FIFO_FULL) != 0 ? 1 : 0;
}

void bk_send_byte(UINT8 uport, UINT8 data)
{
    if(UART1_PORT == uport)
        while(!UART1_TX_WRITE_READY);
#if (CFG_SOC_NAME == SOC_BK7252N)
    else if(UART3_PORT == uport)
        while(!UART3_TX_WRITE_READY);
#endif
    else
        while(!UART2_TX_WRITE_READY);

    UART_WRITE_BYTE(uport, data);
}

void bk_send_string(UINT8 uport, const char *string)
{
	const char *p = string;
    while(*string)
    {
		if (*string == '\n') {
			if (p == string || *(string - 1) != '\r')
				bk_send_byte(uport, '\r');	// append '\r'
		}
        bk_send_byte(uport, *string++);
    }
}

/*uart2 as deubg port*/
void bk_printf(const char *fmt, ...)
{
    #if AT_SERVICE_CFG
    if(print_log_enable == FALSE) return;
    #endif
#if (CFG_SUPPORT_RTT)
    va_list args;
    rt_size_t length;
    static char rt_log_buf[RT_CONSOLEBUF_SIZE];

    va_start(args, fmt);
    /* the return value of vsnprintf is the number of bytes that would be
     * written to buffer had if the size of the buffer been sufficiently
     * large excluding the terminating null byte. If the output string
     * would be larger than the rt_log_buf, we have to adjust the output
     * length. */
    length = rt_vsnprintf(rt_log_buf, sizeof(rt_log_buf) - 1, fmt, args);
    if (length > RT_CONSOLEBUF_SIZE - 1)
        length = RT_CONSOLEBUF_SIZE - 1;
    rt_kprintf("%s", rt_log_buf);
    va_end(args);
#else
    va_list ap;
    char string[128];

    va_start(ap, fmt);
    vsnprintf(string, sizeof(string) - 1, fmt, ap);
    string[127] = 0;
    bk_send_string(uart_print_port, string);
    va_end(ap);
#endif

}

void print_hex_dump(const char *prefix, void *buf, int len)
{
	int i;
	u8 *b = buf;

	if (prefix)
		os_printf("%s", prefix);
	for (i = 0; i < len; i++)
		os_printf("%02X ", b[i]);
	os_printf("\n");
}

void fatal_print(const char *fmt, ...)
{
    os_printf(fmt);

    DEAD_WHILE();
}

void uart_hw_init(UINT8 uport)
{
    UINT32 reg, baud_div;
    UINT32 conf_reg_addr, fifo_conf_reg_addr;
    UINT32 flow_conf_reg_addr, wake_conf_reg_addr, intr_reg_addr;

#if !(CFG_SOC_NAME == SOC_BK7252N)
#if !((UART1_BAUD_RATE != UART_BAUD_RATE) && (UART2_BAUD_RATE != UART_BAUD_RATE))
    baud_div = UART_CLOCK / UART_BAUD_RATE;
#endif
#else
#if !((UART1_BAUD_RATE != UART_BAUD_RATE) && (UART2_BAUD_RATE != UART_BAUD_RATE) && (UART3_BAUD_RATE != UART_BAUD_RATE))
    baud_div = UART_CLOCK / UART_BAUD_RATE;
#endif
#endif

    if(UART1_PORT == uport)
    {
#if UART1_BAUD_RATE != UART_BAUD_RATE
    	baud_div = UART_CLOCK / UART1_BAUD_RATE;
#endif
        conf_reg_addr = REG_UART1_CONFIG;
        fifo_conf_reg_addr = REG_UART1_FIFO_CONFIG;
        flow_conf_reg_addr = REG_UART1_FLOW_CONFIG;
        wake_conf_reg_addr = REG_UART1_WAKE_CONFIG;
        intr_reg_addr = REG_UART1_INTR_ENABLE;
    }
#if (CFG_SOC_NAME == SOC_BK7252N)
    else if(UART3_PORT == uport)
    {
#if UART3_BAUD_RATE != UART_BAUD_RATE
        baud_div = UART_CLOCK / UART3_BAUD_RATE;
#endif
        conf_reg_addr = REG_UART3_CONFIG;
        fifo_conf_reg_addr = REG_UART3_FIFO_CONFIG;
        flow_conf_reg_addr = REG_UART3_FLOW_CONFIG;
        wake_conf_reg_addr = REG_UART3_WAKE_CONFIG;
        intr_reg_addr = REG_UART3_INTR_ENABLE;
    }
#endif
    else
    {
#if UART2_BAUD_RATE != UART_BAUD_RATE
        baud_div = UART_CLOCK / UART2_BAUD_RATE;
#endif
        conf_reg_addr = REG_UART2_CONFIG;
        fifo_conf_reg_addr = REG_UART2_FIFO_CONFIG;
        flow_conf_reg_addr = REG_UART2_FLOW_CONFIG;
        wake_conf_reg_addr = REG_UART2_WAKE_CONFIG;
        intr_reg_addr = REG_UART2_INTR_ENABLE;
    }

    baud_div = baud_div - 1;

    reg = UART_TX_ENABLE
          | (UART_RX_ENABLE & (~UART_IRDA))
          | (((DEF_DATA_LEN & UART_DATA_LEN_MASK) << UART_DATA_LEN_POSI)
          & (~UART_PAR_EN)
          & (~UART_PAR_ODD_MODE)
          & (~UART_STOP_LEN_2))
          | ((baud_div & UART_CLK_DIVID_MASK) << UART_CLK_DIVID_POSI);
    REG_WRITE(conf_reg_addr, reg);

    reg = ((TX_FIFO_THRD & TX_FIFO_THRESHOLD_MASK) << TX_FIFO_THRESHOLD_POSI)
          | ((RX_FIFO_THRD & RX_FIFO_THRESHOLD_MASK) << RX_FIFO_THRESHOLD_POSI)
          | ((RX_STOP_DETECT_TIME32 & RX_STOP_DETECT_TIME_MASK) << RX_STOP_DETECT_TIME_POSI);
    REG_WRITE(fifo_conf_reg_addr, reg);

    REG_WRITE(flow_conf_reg_addr, 0);
    REG_WRITE(wake_conf_reg_addr, 0);

    reg = RX_FIFO_NEED_READ_EN | UART_RX_STOP_END_EN;
    REG_WRITE(intr_reg_addr, reg);

    return;
}

void uart_hw_set_change(UINT8 uport, bk_uart_config_t *uart_config)
{
    UINT32 reg, baud_div, width;
    uart_parity_t 	     parity_en;
    uart_stop_bits_t	  stop_bits;
    uart_flow_control_t  flow_control;
    UINT8 parity_mode = 0;
    UINT32 intr_ena_reg_addr, conf_reg_addr, fifi_conf_reg_addr;
    UINT32 flow_conf_reg_addr, wake_reg_addr;

    if(UART1_PORT == uport)
    {
        intr_ena_reg_addr = REG_UART1_INTR_ENABLE;
        conf_reg_addr = REG_UART1_CONFIG;
        fifi_conf_reg_addr = REG_UART1_FIFO_CONFIG;
        flow_conf_reg_addr = REG_UART1_FLOW_CONFIG;
        wake_reg_addr = REG_UART1_WAKE_CONFIG;
    }
#if (CFG_SOC_NAME == SOC_BK7252N)
    else if(UART3_PORT == uport)
    {
        intr_ena_reg_addr = REG_UART3_INTR_ENABLE;
        conf_reg_addr = REG_UART3_CONFIG;
        fifi_conf_reg_addr = REG_UART3_FIFO_CONFIG;
        flow_conf_reg_addr = REG_UART3_FLOW_CONFIG;
        wake_reg_addr = REG_UART3_WAKE_CONFIG;
    }
#endif
    else
    {
        intr_ena_reg_addr = REG_UART2_INTR_ENABLE;
        conf_reg_addr = REG_UART2_CONFIG;
        fifi_conf_reg_addr = REG_UART2_FIFO_CONFIG;
        flow_conf_reg_addr = REG_UART2_FLOW_CONFIG;
        wake_reg_addr = REG_UART2_WAKE_CONFIG;
    }

    REG_WRITE(intr_ena_reg_addr, 0);//disable int

    baud_div = UART_CLOCK / uart_config->baud_rate;
    baud_div = baud_div - 1;
    width = uart_config->data_width;
    parity_en = uart_config->parity;
    stop_bits = uart_config->stop_bits;
    flow_control = uart_config->flow_control;
	(void)flow_control;

    if(parity_en)
    {
        if(parity_en == BK_PARITY_ODD)
            parity_mode = 0;  // Attention 0: odd,  1: even.
        else
            parity_mode = 1;
        parity_en = 1;
    }

    reg = UART_TX_ENABLE
          | (UART_RX_ENABLE & (~UART_IRDA))
          | ((width & UART_DATA_LEN_MASK) << UART_DATA_LEN_POSI)
          | (parity_en << 5)
          | (parity_mode << 6)
          | (stop_bits << 7)
          | ((baud_div & UART_CLK_DIVID_MASK) << UART_CLK_DIVID_POSI);

    width = ((width & UART_DATA_LEN_MASK) << UART_DATA_LEN_POSI);
    REG_WRITE(conf_reg_addr, reg);

    reg = ((TX_FIFO_THRD & TX_FIFO_THRESHOLD_MASK) << TX_FIFO_THRESHOLD_POSI)
          | ((RX_FIFO_THRD & RX_FIFO_THRESHOLD_MASK) << RX_FIFO_THRESHOLD_POSI)
          | ((RX_STOP_DETECT_TIME32 & RX_STOP_DETECT_TIME_MASK) << RX_STOP_DETECT_TIME_POSI);
    REG_WRITE(fifi_conf_reg_addr, reg);

    REG_WRITE(flow_conf_reg_addr, 0);
    REG_WRITE(wake_reg_addr, 0);

    reg = RX_FIFO_NEED_READ_EN | UART_RX_STOP_END_EN;
    if (parity_en) {
        reg |= UART_RX_PARITY_ERR_EN;
    }
    REG_WRITE(intr_ena_reg_addr, reg);
}

UINT32 uart_sw_init(UINT8 uport)
{
    uart[uport].rx = kfifo_alloc(RX_RB_LENGTH);
    uart[uport].tx = kfifo_alloc(TX_RB_LENGTH);

    if((!uart[uport].tx) || (!uart[uport].rx))
    {
        if(uart[uport].tx)
        {
            kfifo_free(uart[uport].tx);
        }

        if(uart[uport].rx)
        {
            kfifo_free(uart[uport].rx);
        }
        return UART_FAILURE;
    }

    return UART_SUCCESS;
}

UINT32 uart_sw_uninit(UINT8 uport)
{
    if(uart[uport].tx)
    {
        kfifo_free(uart[uport].tx);
    }

    if(uart[uport].rx)
    {
        kfifo_free(uart[uport].rx);
    }

    os_memset(&uart[uport], 0, sizeof(uart[uport]));

    return UART_SUCCESS;
}

void uart_fifo_flush(UINT8 uport)
{
    UINT32 val;
    UINT32 reg;
    UINT32 reg_addr;

    if(UART1_PORT == uport)
        reg_addr = REG_UART1_CONFIG;
#if (CFG_SOC_NAME == SOC_BK7252N)
    else if(UART3_PORT == uport)
        reg_addr = REG_UART3_CONFIG;
#endif
    else
        reg_addr = REG_UART2_CONFIG;

    val = REG_READ(reg_addr);

    reg = val & (~(UART_TX_ENABLE | UART_RX_ENABLE));

    REG_WRITE(reg_addr, reg);
    REG_WRITE(reg_addr, val);
}

void uart_hw_uninit(UINT8 uport)
{
    UINT32 i;
    UINT32 reg;
    UINT32 rx_count;
    UINT32 intr_ena_reg_addr, conf_reg_addr, fifostatus_reg_addr;
    if(UART1_PORT == uport)
    {
        intr_ena_reg_addr = REG_UART1_INTR_ENABLE;
        conf_reg_addr = REG_UART1_CONFIG;
        fifostatus_reg_addr = REG_UART1_FIFO_STATUS;
    }
#if (CFG_SOC_NAME == SOC_BK7252N)
    else if(UART3_PORT == uport)
    {
        intr_ena_reg_addr = REG_UART3_INTR_ENABLE;
        conf_reg_addr = REG_UART3_CONFIG;
        fifostatus_reg_addr = REG_UART3_FIFO_STATUS;
    }
#endif
    else
    {
        intr_ena_reg_addr = REG_UART2_INTR_ENABLE;
        conf_reg_addr = REG_UART2_CONFIG;
        fifostatus_reg_addr = REG_UART2_FIFO_STATUS;
    }

    /*disable rtx intr*/
    reg = REG_READ(intr_ena_reg_addr);
    reg &= (~(RX_FIFO_NEED_READ_EN | UART_RX_STOP_END_EN));
    REG_WRITE(intr_ena_reg_addr, reg);

    /* flush fifo*/
    uart_fifo_flush(uport);

    /* disable rtx*/
    reg = REG_READ(conf_reg_addr);
    reg = reg & (~(UART_TX_ENABLE | UART_RX_ENABLE));
    REG_WRITE(conf_reg_addr, reg);

    /* double discard fifo data*/
    reg = REG_READ(fifostatus_reg_addr);
    rx_count = (reg >> RX_FIFO_COUNT_POSI) & RX_FIFO_COUNT_MASK;
    for(i = 0; i < rx_count; i ++)
    {
        UART_READ_BYTE_DISCARD(uport);
    }
}

void uart_reset(UINT8 uport)
{
    if(UART1_PORT == uport)
    {
        uart1_exit();
        uart1_init();
    }
#if (CFG_SOC_NAME == SOC_BK7252N)
    else if(UART3_PORT == uport)
    {
        uart3_exit();
        uart3_init();
    }
#endif
    else
    {
        uart2_exit();
        uart2_init();
    }
}

void uart_send_backgroud(void)
{
    /* send the buf at backgroud context*/
    /* Todo maybe the uart_send_backgroud is UART3_PORT in bk7252n */
    uart_write_fifo_frame(UART2_PORT, uart[UART2_PORT].tx, DEBUG_PRT_MAX_CNT);
}

UINT32 uart_write_fifo_frame(UINT8 uport, KFIFO_PTR tx_ptr, UINT32 count)
{
    UINT32 len;
    UINT32 ret;
    UINT32 val;

    len = 0;

    while(1)
    {
        ret = kfifo_get(tx_ptr, (UINT8 *)&val, 1);
        if(0 == ret)
        {
            break;
        }


#if __CC_ARM
        uart_send_byte(uport, (UINT8)val);
#else
        bk_send_byte(uport, (UINT8)val);
#endif

        len += ret;
        if(len >= count)
        {
            break;
        }
    }

    return len;
}

UINT32 uart_read_fifo_frame(UINT8 uport, KFIFO_PTR rx_ptr)
{
    UINT32 val;
    UINT32 rx_count, fifo_status_reg;
    UINT32 unused = kfifo_unused(rx_ptr);

    if(UART1_PORT == uport)
        fifo_status_reg = REG_UART1_FIFO_STATUS;
#if (CFG_SOC_NAME == SOC_BK7252N)
    else if(UART3_PORT == uport)
        fifo_status_reg = REG_UART3_FIFO_STATUS;
#endif
    else
        fifo_status_reg = REG_UART2_FIFO_STATUS;

    rx_count = 0;
    while(REG_READ(fifo_status_reg) & FIFO_RD_READY)
    {
        UART_READ_BYTE(uport, val);
        if(unused > rx_count)
            rx_count += kfifo_put(rx_ptr, (UINT8 *)&val, 1);
    }

    if(unused <= rx_count)
        bk_printf("uart rx fifo full\r\n");

    return rx_count;
}

void uart_set_tx_fifo_needwr_int(UINT8 uport, UINT8 set)
{
	UINT32 reg;

	if(UART1_PORT == uport)
		reg = REG_READ(REG_UART1_INTR_ENABLE);
#if (CFG_SOC_NAME == SOC_BK7252N)
	else if(UART3_PORT == uport)
		reg = REG_READ(REG_UART3_INTR_ENABLE);
#endif
	else
		reg = REG_READ(REG_UART2_INTR_ENABLE);

	if(set == 1)
	{
		reg |= (TX_FIFO_NEED_WRITE_EN);
	}
	else
	{
		reg &= ~(TX_FIFO_NEED_WRITE_EN);
	}

	if(UART1_PORT == uport){
		REG_WRITE(REG_UART1_INTR_ENABLE, reg);
	}
#if BK7252N_UART_NEW_REG
	else if(UART3_PORT == uport)
		REG_WRITE(REG_UART3_INTR_ENABLE, reg);
#endif
	else
		REG_WRITE(REG_UART2_INTR_ENABLE, reg);

}

void uart_set_tx_stop_end_int(UINT8 uport, UINT8 set)
{
	UINT32 reg;

	if(UART1_PORT == uport)
		reg = REG_READ(REG_UART1_INTR_ENABLE);
#if (CFG_SOC_NAME == SOC_BK7252N)
	else if(UART3_PORT == uport)
		reg = REG_READ(REG_UART3_INTR_ENABLE);
#endif
	else
		reg = REG_READ(REG_UART2_INTR_ENABLE);

	if(set == 1)
	{
		reg |= (UART_TX_STOP_END_EN);
	}
	else
	{
		reg &= ~(UART_TX_STOP_END_EN);
	}

	if(UART1_PORT == uport)
		REG_WRITE(REG_UART1_INTR_ENABLE, reg);
#if (CFG_SOC_NAME == SOC_BK7252N)
	else if(UART3_PORT == uport)
		REG_WRITE(REG_UART3_INTR_ENABLE, reg);
#endif
	else
		REG_WRITE(REG_UART2_INTR_ENABLE, reg);
}

void uart_fast_init(void)
{
    UINT32 param;

    if (UART1_PORT == uart_print_port)
    {
        param = PWD_UART1_CLK_BIT;
    }
    else
    {
        param = PWD_UART2_CLK_BIT;
    }

    icu_ctrl(CMD_CLK_PWR_UP, &param);

    if (UART1_PORT == uart_print_port)
    {
        param = GFUNC_MODE_UART1;
    }
    else
    {
        param = GFUNC_MODE_UART2;
    }
    gpio_ctrl(CMD_GPIO_ENABLE_SECOND, &param);
    uart_hw_init(uart_print_port);
}

UINT32 conf_reg_value_bakeup = 0;
UINT32 intr_reg_value_bakeup = 0;
volatile UINT8 uart_print_port_bakeup_flag = 0;
void uart_print_port_suspend(void)
{
    UINT32 conf_reg_addr, intr_reg_addr;

    if (UART1_PORT == uart_print_port)
    {
        conf_reg_addr = REG_UART1_CONFIG;
        intr_reg_addr = REG_UART1_INTR_ENABLE;
    }
    else
    {
        conf_reg_addr = REG_UART2_CONFIG;
        intr_reg_addr = REG_UART2_INTR_ENABLE;
    }

    conf_reg_value_bakeup = REG_READ(conf_reg_addr);
    intr_reg_value_bakeup = REG_READ(intr_reg_addr);
    REG_WRITE(conf_reg_addr, 0);
    REG_WRITE(intr_reg_addr, 0);
    uart_print_port_bakeup_flag = 1;
}

void uart_print_port_recover(void)
{
    UINT32 conf_reg_addr, intr_reg_addr;

    if (UART1_PORT == uart_print_port)
    {
        conf_reg_addr = REG_UART1_CONFIG;
        intr_reg_addr = REG_UART1_INTR_ENABLE;
    }
    else
    {
        conf_reg_addr = REG_UART2_CONFIG;
        intr_reg_addr = REG_UART2_INTR_ENABLE;
    }

    if(uart_print_port_bakeup_flag)
    {
        REG_WRITE(conf_reg_addr, conf_reg_value_bakeup);
        REG_WRITE(intr_reg_addr, intr_reg_value_bakeup);
        uart_print_port_bakeup_flag = 0;
    }
}
/*******************************************************************/
void uart1_isr(void)
{

    UINT32 status;
    UINT32 intr_en;
    UINT32 intr_status;

    intr_en = REG_READ(REG_UART1_INTR_ENABLE);
    intr_status = REG_READ(REG_UART1_INTR_STATUS);
    REG_WRITE(REG_UART1_INTR_STATUS, intr_status);
    status = intr_status & intr_en;

    if(status & (RX_FIFO_NEED_READ_STA | UART_RX_STOP_END_STA))
    {
#if ((!CFG_SUPPORT_RTT) && (UART1_USE_FIFO_REC))
        uart_read_fifo_frame(UART1_PORT, uart[UART1_PORT].rx);
#endif

        if (uart_receive_callback[0].callback != 0)
        {
            void *param = uart_receive_callback[0].param;

            uart_receive_callback[0].callback(UART1_PORT, param);
        }
        else
        {
        	uart_read_byte(UART1_PORT); /*drop data for rtt*/
        }
    }

    if(status & TX_FIFO_NEED_WRITE_STA)
    {
        if (uart_txfifo_needwr_callback[0].callback != 0)
        {
            void *param = uart_txfifo_needwr_callback[0].param;

            uart_txfifo_needwr_callback[0].callback(UART1_PORT, param);
        }
    }

    if(status & RX_FIFO_OVER_FLOW_STA)
    {
    }

    if(status & UART_RX_PARITY_ERR_STA)
    {
        uart_fifo_flush(UART1_PORT);
    }

    if(status & UART_RX_STOP_ERR_STA)
    {
    }

    if(status & UART_TX_STOP_END_STA)
    {
        if (uart_tx_end_callback[0].callback != 0)
        {
            void *param = uart_tx_end_callback[0].param;

            uart_tx_end_callback[0].callback(UART1_PORT, param);
        }
    }

	if(status & UART_RXD_WAKEUP_STA)
    {
    }
}
void uart1_init(void)
{
    UINT32 ret;
    UINT32 param;
    UINT32 intr_status;

#if UART1_USE_FIFO_REC
    ret = uart_sw_init(UART1_PORT);
    ASSERT(UART_SUCCESS == ret);
#endif

    ddev_register_dev(UART1_DEV_NAME, &uart1_op);

    intc_service_register(IRQ_UART1, PRI_IRQ_UART1, uart1_isr);

    param = PWD_UART1_CLK_BIT;
    sddev_control(ICU_DEV_NAME, CMD_CLK_PWR_UP, &param);

    param = GFUNC_MODE_UART1;
    sddev_control(GPIO_DEV_NAME, CMD_GPIO_ENABLE_SECOND, &param);
    uart_hw_init(UART1_PORT);

    /*irq enable, Be careful: it is best that irq enable at open routine*/
    intr_status = REG_READ(REG_UART1_INTR_STATUS);
    REG_WRITE(REG_UART1_INTR_STATUS, intr_status);

    param = IRQ_UART1_BIT;
    sddev_control(ICU_DEV_NAME, CMD_ICU_INT_ENABLE, &param);
}

void uart1_exit(void)
{
    UINT32 param;

    /*irq enable, Be careful: it is best that irq enable at close routine*/
    param = IRQ_UART1_BIT;
    sddev_control(ICU_DEV_NAME, CMD_ICU_INT_DISABLE, &param);

    uart_hw_uninit(UART1_PORT);

    ddev_unregister_dev(UART1_DEV_NAME);

    uart_sw_uninit(UART1_PORT);
}

UINT32 uart1_open(UINT32 op_flag)
{
    return UART_SUCCESS;
}

UINT32 uart1_close(void)
{
    return UART_SUCCESS;
}

UINT32 uart1_read(char *user_buf, UINT32 count, UINT32 op_flag)
{
#if UART1_USE_FIFO_REC
     return kfifo_get(uart[UART1_PORT].rx, (UINT8 *)user_buf, count);
#else
     return -1;
#endif
}

UINT32 uart1_write(char *user_buf, UINT32 count, UINT32 op_flag)
{
#if UART1_USE_FIFO_REC
     return kfifo_put(uart[UART1_PORT].tx, (UINT8 *)user_buf, count);
#else
     return -1;
#endif
}

UINT32 uart1_ctrl(UINT32 cmd, void *parm)
{
    UINT32 ret;
	int baud;
	UINT32 conf_reg_addr;
	UINT32 baud_div,reg;

    peri_busy_count_add();

    ret = UART_SUCCESS;
    switch(cmd)
    {
    case CMD_SEND_BACKGROUND:
        uart_send_backgroud();
        break;

    case CMD_UART_RESET:
        uart_reset(UART1_PORT);
        break;

    case CMD_RX_COUNT:
#if UART1_USE_FIFO_REC
        ret = kfifo_data_size(uart[UART1_PORT].rx);
#else
        ret = 0;
#endif
        break;

    case CMD_RX_PEEK:
    {
        UART_PEEK_RX_PTR peek;

        peek = (UART_PEEK_RX_PTR)parm;

        if(!((URX_PEEK_SIG != peek->sig)
                || (NULLPTR == peek->ptr)
                || (0 == peek->len)))
        {
            ret = kfifo_out_peek(uart[UART1_PORT].rx, peek->ptr, peek->len);
        }

        break;
    }
    case CMD_UART_INIT:
        uart_hw_set_change(UART1_PORT, parm);
        break;
    case CMD_UART_SET_RX_CALLBACK:
        if (parm)
        {
            struct uart_callback_des *uart_callback;

            uart_callback = (struct uart_callback_des *)parm;

            uart_rx_callback_set(UART1_PORT, uart_callback->callback, uart_callback->param);
        }
        else
        {
            uart_rx_callback_set(UART1_PORT, NULL, NULL);
        }
        break;
    case CMD_UART_SET_TX_CALLBACK:
        if (parm)
        {
            struct uart_callback_des *uart_callback;

            uart_callback = (struct uart_callback_des *)parm;

            uart_tx_end_callback_set(UART1_PORT, uart_callback->callback, uart_callback->param);
        }
        else
        {
            uart_tx_end_callback_set(UART1_PORT, NULL, NULL);
        }
        break;
    case CMD_UART_SET_TX_FIFO_NEEDWR_CALLBACK:
        if (parm)
        {
            struct uart_callback_des *uart_callback;

            uart_callback = (struct uart_callback_des *)parm;

            uart_tx_fifo_needwr_callback_set(UART1_PORT, uart_callback->callback, uart_callback->param);
        }
        else
        {
            uart_tx_fifo_needwr_callback_set(UART1_PORT, NULL, NULL);
        }
        break;
	case CMD_SET_STOP_END:
		uart_set_tx_stop_end_int(UART1_PORT, *(UINT8 *)parm);
		break;

	case CMD_SET_TX_FIFO_NEEDWR_INT:
		uart_set_tx_fifo_needwr_int(UART1_PORT, *(UINT8 *)parm);
		break;
	case CMD_SET_BAUT:

		baud =  *((int*)parm);
		baud_div = UART_CLOCK / baud;
		baud_div = baud_div - 1;
		conf_reg_addr = REG_UART1_CONFIG;//current uart
		reg = (REG_READ(conf_reg_addr))&(~(UART_CLK_DIVID_MASK<< UART_CLK_DIVID_POSI));
		reg = reg | ((baud_div & UART_CLK_DIVID_MASK) << UART_CLK_DIVID_POSI);
		REG_WRITE(conf_reg_addr, reg);

		break;
    default:
        break;
    }

    peri_busy_count_dec();

    return ret;
}

void uart2_isr(void)
{
    UINT32 status;
    UINT32 intr_en;
    UINT32 intr_status;

    intr_en = REG_READ(REG_UART2_INTR_ENABLE);
    intr_status = REG_READ(REG_UART2_INTR_STATUS);
    REG_WRITE(REG_UART2_INTR_STATUS, intr_status);
    status = intr_status & intr_en;

    if(status & (RX_FIFO_NEED_READ_STA | UART_RX_STOP_END_STA))
    {
#if ((!CFG_SUPPORT_RTT) && (UART2_USE_FIFO_REC))
        uart_read_fifo_frame(UART2_PORT, uart[UART2_PORT].rx);
#endif

        if (uart_receive_callback[1].callback != 0)
        {
            void *param = uart_receive_callback[1].param;

            uart_receive_callback[1].callback(UART2_PORT, param);
        }
        else
        {
        	uart_read_byte(UART2_PORT); /*drop data for rtt*/
        }
    }

	if(status & TX_FIFO_NEED_WRITE_STA)
    {
        if (uart_txfifo_needwr_callback[1].callback != 0)
        {
            void *param = uart_txfifo_needwr_callback[1].param;

            uart_txfifo_needwr_callback[1].callback(UART2_PORT, param);
        }
    }

	if(status & RX_FIFO_OVER_FLOW_STA)
    {
    }

	if(status & UART_RX_PARITY_ERR_STA)
    {
        uart_fifo_flush(UART2_PORT);
    }

	if(status & UART_RX_STOP_ERR_STA)
    {
    }

	if(status & UART_TX_STOP_END_STA)
    {
        if (uart_tx_end_callback[1].callback != 0)
        {
            void *param = uart_tx_end_callback[1].param;

            uart_tx_end_callback[1].callback(UART2_PORT, param);
        }
    }

	if(status & UART_RXD_WAKEUP_STA)
    {
    }
}

void uart2_init(void)
{
    UINT32 ret;
    UINT32 param;
    UINT32 intr_status;

#if UART2_USE_FIFO_REC
    ret = uart_sw_init(UART2_PORT);
    ASSERT(UART_SUCCESS == ret);
#endif

    ddev_register_dev(UART2_DEV_NAME, &uart2_op);

    intc_service_register(IRQ_UART2, PRI_IRQ_UART2, uart2_isr);

    param = PWD_UART2_CLK_BIT;
    sddev_control(ICU_DEV_NAME, CMD_CLK_PWR_UP, &param);

    param = GFUNC_MODE_UART2;
    sddev_control(GPIO_DEV_NAME, CMD_GPIO_ENABLE_SECOND, &param);
    uart_hw_init(UART2_PORT);

    /*irq enable, Be careful: it is best that irq enable at open routine*/
    intr_status = REG_READ(REG_UART2_INTR_STATUS);
    REG_WRITE(REG_UART2_INTR_STATUS, intr_status);

    param = IRQ_UART2_BIT;
    sddev_control(ICU_DEV_NAME, CMD_ICU_INT_ENABLE, &param);
}

void uart2_exit(void)
{
    UINT32 param;

    /*irq enable, Be careful: it is best that irq enable at close routine*/
    param = IRQ_UART2_BIT;
    sddev_control(ICU_DEV_NAME, CMD_ICU_INT_DISABLE, &param);

    uart_hw_uninit(UART2_PORT);

    ddev_unregister_dev(UART2_DEV_NAME);

    uart_sw_uninit(UART2_PORT);
}

UINT32 uart2_open(UINT32 op_flag)
{
    return UART_SUCCESS;
}

UINT32 uart2_close(void)
{
    return UART_SUCCESS;
}

UINT32 uart2_read(char *user_buf, UINT32 count, UINT32 op_flag)
{
#if UART2_USE_FIFO_REC
    return kfifo_get(uart[UART2_PORT].rx, (UINT8 *)user_buf, count);
#else
    return -1;
#endif
}

UINT32 uart2_write(char *user_buf, UINT32 count, UINT32 op_flag)
{
#if UART2_USE_FIFO_REC
    return kfifo_put(uart[UART2_PORT].tx, (UINT8 *)user_buf, count);
#else
    return -1;
#endif
}

UINT32 uart2_ctrl(UINT32 cmd, void *parm)
{
    UINT32 ret;
	int baud;
	UINT32 conf_reg_addr;
	UINT32 baud_div,reg;
    peri_busy_count_add();

    ret = UART_SUCCESS;
    switch(cmd)
    {
    case CMD_SEND_BACKGROUND:
        uart_send_backgroud();
        break;

    case CMD_UART_RESET:
        uart_reset(UART2_PORT);
        break;

    case CMD_RX_COUNT:
#if UART2_USE_FIFO_REC
        ret = kfifo_data_size(uart[UART2_PORT].rx);
#else
        ret = 0;
#endif
        break;

    case CMD_RX_PEEK:
    {
        UART_PEEK_RX_PTR peek;

        peek = (UART_PEEK_RX_PTR)parm;

        if(!((URX_PEEK_SIG != peek->sig)
                || (NULLPTR == peek->ptr)
                || (0 == peek->len)))
        {
            ret = kfifo_out_peek(uart[UART2_PORT].rx, peek->ptr, peek->len);
        }

        break;
    }
    case CMD_UART_INIT:
        uart_hw_set_change(UART2_PORT, parm);
        break;
    case CMD_UART_SET_RX_CALLBACK:
        if (parm)
        {
            struct uart_callback_des *uart_callback;

            uart_callback = (struct uart_callback_des *)parm;

            uart_rx_callback_set(UART2_PORT, uart_callback->callback, uart_callback->param);
        }
        else
        {
            uart_rx_callback_set(UART2_PORT, NULL, NULL);
        }
        break;
    case CMD_UART_SET_TX_CALLBACK:
        if (parm)
        {
            struct uart_callback_des *uart_callback;

            uart_callback = (struct uart_callback_des *)parm;

            uart_tx_end_callback_set(UART2_PORT, uart_callback->callback, uart_callback->param);
        }
        else
        {
            uart_tx_end_callback_set(UART2_PORT, NULL, NULL);
        }
        break;
    case CMD_UART_SET_TX_FIFO_NEEDWR_CALLBACK:
        if (parm)
        {
            struct uart_callback_des *uart_callback;

            uart_callback = (struct uart_callback_des *)parm;

            uart_tx_fifo_needwr_callback_set(UART2_PORT, uart_callback->callback, uart_callback->param);
        }
        else
        {
            uart_tx_fifo_needwr_callback_set(UART2_PORT, NULL, NULL);
        }
        break;
	case CMD_SET_STOP_END:
		uart_set_tx_stop_end_int(UART2_PORT, *(UINT8 *)parm);
		break;

	case CMD_SET_TX_FIFO_NEEDWR_INT:
		uart_set_tx_fifo_needwr_int(UART2_PORT, *(UINT8 *)parm);
		break;
	case CMD_SET_BAUT:
		baud =  *((int*)parm);
		baud_div = UART_CLOCK / baud;
		baud_div = baud_div - 1;
		conf_reg_addr = REG_UART2_CONFIG;//current uart
		reg = (REG_READ(conf_reg_addr))&(~(UART_CLK_DIVID_MASK<< UART_CLK_DIVID_POSI));
		reg = reg | ((baud_div & UART_CLK_DIVID_MASK) << UART_CLK_DIVID_POSI);
		REG_WRITE(conf_reg_addr, reg);

		break;
    default:
        break;
    }

    peri_busy_count_dec();

    return ret;
}

#if (CFG_SOC_NAME == SOC_BK7252N)
void uart3_isr(void)
{
    UINT32 status;
    UINT32 intr_en;
    UINT32 intr_status;

    intr_en = REG_READ(REG_UART3_INTR_ENABLE);
    intr_status = REG_READ(REG_UART3_INTR_STATUS);
    REG_WRITE(REG_UART3_INTR_STATUS, intr_status);
    status = intr_status & intr_en;

    if(status & (RX_FIFO_NEED_READ_STA | UART_RX_STOP_END_STA))
    {
    #if ((!CFG_SUPPORT_RTT) && (UART3_USE_FIFO_REC))
        uart_read_fifo_frame(UART3_PORT, uart[UART3_PORT].rx);
    #endif

        if (uart_receive_callback[2].callback != 0)
        {
            void *param = uart_receive_callback[2].param;

            uart_receive_callback[2].callback(UART3_PORT, param);
        }
        else
        {
            uart_read_byte(UART3_PORT); /*drop data for rtt*/
        }
    }

    if(status & TX_FIFO_NEED_WRITE_STA)
    {
        if (uart_txfifo_needwr_callback[2].callback != 0)
        {
            void *param = uart_txfifo_needwr_callback[2].param;

            uart_txfifo_needwr_callback[2].callback(UART3_PORT, param);
        }
    }

    if(status & RX_FIFO_OVER_FLOW_STA)
    {
    }

    if(status & UART_RX_PARITY_ERR_STA)
    {
        uart_fifo_flush(UART3_PORT);
    }

    if(status & UART_RX_STOP_ERR_STA)
    {
    }

    if(status & UART_TX_STOP_END_STA)
    {
        if (uart_tx_end_callback[2].callback != 0)
        {
            void *param = uart_tx_end_callback[2].param;

            uart_tx_end_callback[2].callback(UART3_PORT, param);
        }
    }

    if(status & UART_RXD_WAKEUP_STA)
    {
    }
}

//Todo uart3's gpio and irq have not been adapted
void uart3_init(void)
{
    UINT32 ret;
    UINT32 param;
    UINT32 intr_status;

#if UART3_USE_FIFO_REC
    ret = uart_sw_init(UART3_PORT);
    ASSERT(UART_SUCCESS == ret);
#endif

    ddev_register_dev(UART3_DEV_NAME, &uart3_op);

    intc_service_register(IRQ_UART3, PRI_IRQ_UART3, uart3_isr);

    param = PWD_UART3_CLK_BIT;
    sddev_control(ICU_DEV_NAME, CMD_CLK_PWR_UP, &param);

    param = GFUNC_MODE_UART3;
    sddev_control(GPIO_DEV_NAME, CMD_GPIO_ENABLE_SECOND, &param);
    uart_hw_init(UART3_PORT);

    /*irq enable, Be careful: it is best that irq enable at open routine*/
    intr_status = REG_READ(REG_UART3_INTR_STATUS);
    REG_WRITE(REG_UART3_INTR_STATUS, intr_status);

    param = IRQ_UART3_BIT;
    sddev_control(ICU_DEV_NAME, CMD_ICU_INT_ENABLE, &param);
}

//Todo uart3's gpio and irq have not been adapted
void uart3_exit(void)
{
    UINT32 param;

    /*irq enable, Be careful: it is best that irq enable at close routine*/
    param = IRQ_UART3_BIT;
    sddev_control(ICU_DEV_NAME, CMD_ICU_INT_DISABLE, &param);

    uart_hw_uninit(UART3_PORT);

    ddev_unregister_dev(UART3_DEV_NAME);

    uart_sw_uninit(UART3_PORT);
}

UINT32 uart3_open(UINT32 op_flag)
{
    return UART_SUCCESS;
}

UINT32 uart3_close(void)
{
    return UART_SUCCESS;
}

UINT32 uart3_read(char *user_buf, UINT32 count, UINT32 op_flag)
{
#if UART3_USE_FIFO_REC
    return kfifo_get(uart[UART3_PORT].rx, (UINT8 *)user_buf, count);
#else
    return -1;
#endif
}

UINT32 uart3_write(char *user_buf, UINT32 count, UINT32 op_flag)
{
#if UART3_USE_FIFO_REC
    return kfifo_put(uart[UART3_PORT].tx, (UINT8 *)user_buf, count);
#else
    return -1;
#endif
}

UINT32 uart3_ctrl(UINT32 cmd, void *parm)
{
    UINT32 ret;
    int baud;
    UINT32 conf_reg_addr;
    UINT32 baud_div,reg;
    peri_busy_count_add();

    ret = UART_SUCCESS;
    switch(cmd)
    {
    case CMD_SEND_BACKGROUND:
        uart_send_backgroud();
        break;

    case CMD_UART_RESET:
        uart_reset(UART3_PORT);
        break;

    case CMD_RX_COUNT:
#if UART3_USE_FIFO_REC
        ret = kfifo_data_size(uart[UART3_PORT].rx);
#else
        ret = 0;
#endif
        break;

    case CMD_RX_PEEK:
    {
        UART_PEEK_RX_PTR peek;

        peek = (UART_PEEK_RX_PTR)parm;

        if(!((URX_PEEK_SIG != peek->sig)
                || (NULLPTR == peek->ptr)
                || (0 == peek->len)))
        {
            ret = kfifo_out_peek(uart[UART3_PORT].rx, peek->ptr, peek->len);
        }

        break;
    }
    case CMD_UART_INIT:
        uart_hw_set_change(UART3_PORT, parm);
        break;
    case CMD_UART_SET_RX_CALLBACK:
        if (parm)
        {
            struct uart_callback_des *uart_callback;

            uart_callback = (struct uart_callback_des *)parm;

            uart_rx_callback_set(UART3_PORT, uart_callback->callback, uart_callback->param);
        }
        else
        {
            uart_rx_callback_set(UART3_PORT, NULL, NULL);
        }
        break;
    case CMD_UART_SET_TX_CALLBACK:
        if (parm)
        {
            struct uart_callback_des *uart_callback;

            uart_callback = (struct uart_callback_des *)parm;

            uart_tx_end_callback_set(UART3_PORT, uart_callback->callback, uart_callback->param);
        }
        else
        {
            uart_tx_end_callback_set(UART3_PORT, NULL, NULL);
        }
        break;
    case CMD_UART_SET_TX_FIFO_NEEDWR_CALLBACK:
        if (parm)
        {
            struct uart_callback_des *uart_callback;

            uart_callback = (struct uart_callback_des *)parm;

            uart_tx_fifo_needwr_callback_set(UART3_PORT, uart_callback->callback, uart_callback->param);
        }
        else
        {
            uart_tx_fifo_needwr_callback_set(UART3_PORT, NULL, NULL);
        }
        break;
    case CMD_SET_STOP_END:
        uart_set_tx_stop_end_int(UART3_PORT, *(UINT8 *)parm);
        break;

    case CMD_SET_TX_FIFO_NEEDWR_INT:
        uart_set_tx_fifo_needwr_int(UART3_PORT, *(UINT8 *)parm);
        break;
    case CMD_SET_BAUT:
        baud =  *((int*)parm);
        baud_div = UART_CLOCK / baud;
        baud_div = baud_div - 1;
        conf_reg_addr = REG_UART3_CONFIG;//current uart
        reg = (REG_READ(conf_reg_addr))&(~(UART_CLK_DIVID_MASK<< UART_CLK_DIVID_POSI));
        reg = reg | ((baud_div & UART_CLK_DIVID_MASK) << UART_CLK_DIVID_POSI);
        REG_WRITE(conf_reg_addr, reg);

        break;
    default:
        break;
    }

    peri_busy_count_dec();

    return ret;
}
#endif

UINT32 uart_wait_tx_over()
{
#if !(CFG_SOC_NAME == SOC_BK7252N)
    UINT32 uart_wait_us,baudrate1,baudrate2;
#else
    UINT32 uart_wait_us,baudrate1,baudrate2,baudrate3;
#endif
    baudrate1 = UART_CLOCK/((((REG_READ(REG_UART1_CONFIG))>>UART_CLK_DIVID_POSI)
                    & UART_CLK_DIVID_MASK) + 1);
    baudrate2 = UART_CLOCK/((((REG_READ(REG_UART2_CONFIG))>>UART_CLK_DIVID_POSI)
                    & UART_CLK_DIVID_MASK) + 1);
#if (CFG_SOC_NAME == SOC_BK7252N)
    baudrate3 = UART_CLOCK/((((REG_READ(REG_UART3_CONFIG))>>UART_CLK_DIVID_POSI)
                    & UART_CLK_DIVID_MASK) + 1);
#endif

#if !(CFG_SOC_NAME == SOC_BK7252N)
    uart_wait_us = 1000000 * UART2_TX_FIFO_COUNT * 10 / baudrate2
                + 1000000 * UART1_TX_FIFO_COUNT * 10 / baudrate1;
#else
    uart_wait_us = 1000000 * UART3_TX_FIFO_COUNT * 10 / baudrate3
                 + 1000000 * UART2_TX_FIFO_COUNT * 10 / baudrate2
                 + 1000000 * UART1_TX_FIFO_COUNT * 10 / baudrate1;
#endif

#if (CFG_SOC_NAME == SOC_BK7252N)
    while (UART3_TX_FIFO_EMPTY_GET() == 0)
    {
    }
#endif

    while (UART2_TX_FIFO_EMPTY_GET() == 0)
    {
    }

    while (UART1_TX_FIFO_EMPTY_GET() == 0)
    {
    }

    return uart_wait_us;
}

int uart_read_byte(int uport)
{
    int val = -1;
    UINT32 fifo_status_reg;

    if (UART1_PORT == uport)
        fifo_status_reg = REG_UART1_FIFO_STATUS;
#if (CFG_SOC_NAME == SOC_BK7252N)
    else if (UART3_PORT == uport)
        fifo_status_reg = REG_UART3_FIFO_STATUS;
#endif
    else
        fifo_status_reg = REG_UART2_FIFO_STATUS;

    if (REG_READ(fifo_status_reg) & FIFO_RD_READY)
        UART_READ_BYTE(uport, val);

    return val;
}

int uart_write_byte(int uport, char c)
{
    if (UART1_PORT == uport)
        while(!UART1_TX_WRITE_READY);
#if (CFG_SOC_NAME == SOC_BK7252N)
    else if (UART3_PORT == uport)
        while(!UART3_TX_WRITE_READY);
#endif
    else
        while(!UART2_TX_WRITE_READY);

    UART_WRITE_BYTE(uport, c);

    return (1);
}

int uart_rx_callback_set(int uport, uart_callback callback, void *param)
{
    if (uport == UART1_PORT)
    {
        uart_receive_callback[0].callback = callback;
        uart_receive_callback[0].param = param;
    }
    else if (uport == UART2_PORT)
    {
        uart_receive_callback[1].callback = callback;
        uart_receive_callback[1].param = param;
    }
#if (CFG_SOC_NAME == SOC_BK7252N)
    else if (uport == UART3_PORT)
    {
        uart_receive_callback[2].callback = callback;
        uart_receive_callback[2].param = param;
    }
#endif
    else
    {
        return -1;
    }

    return 0;
}

int uart_tx_fifo_needwr_callback_set(int uport, uart_callback callback, void *param)
{
    if (uport == UART1_PORT)
    {
        uart_txfifo_needwr_callback[0].callback = callback;
        uart_txfifo_needwr_callback[0].param = param;
    }
    else if (uport == UART2_PORT)
    {
        uart_txfifo_needwr_callback[1].callback = callback;
        uart_txfifo_needwr_callback[1].param = param;
    }
#if (CFG_SOC_NAME == SOC_BK7252N)
    else if (uport == UART3_PORT)
    {
        uart_txfifo_needwr_callback[2].callback = callback;
        uart_txfifo_needwr_callback[2].param = param;
    }
#endif
    else
    {
        return -1;
    }

    return 0;
}

int uart_tx_end_callback_set(int uport, uart_callback callback, void *param)
{
    if (uport == UART1_PORT)
    {
        uart_tx_end_callback[0].callback = callback;
        uart_tx_end_callback[0].param = param;
    }
    else if (uport == UART2_PORT)
    {
        uart_tx_end_callback[1].callback = callback;
        uart_tx_end_callback[1].param = param;
    }
#if (CFG_SOC_NAME == SOC_BK7252N)
    else if (uport == UART3_PORT)
    {
        uart_tx_end_callback[2].callback = callback;
        uart_tx_end_callback[2].param = param;
    }
#endif
    else
    {
        return -1;
    }

    return 0;
}

// EOF
