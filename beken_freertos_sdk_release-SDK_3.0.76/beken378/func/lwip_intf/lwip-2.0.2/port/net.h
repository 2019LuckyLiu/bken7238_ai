#ifndef _NET_H_
#define _NET_H_

#include "lwip_netif_address.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void uap_ip_down(void);
extern void uap_ip_start(void);
extern void sta_ip_down(void);
extern void sta_ip_start(void);
extern void sta_ip_get_start_time(void);
extern uint32_t uap_ip_is_start(void);
extern uint32_t sta_ip_is_start(void);
extern void net_send_gratuitous_arp(void);
extern void *net_get_sta_handle(void);
extern void *net_get_uap_handle(void);
extern void net_wlan_remove_netif(void *mac);
extern int net_get_if_macaddr(void *macaddr, void *intrfc_handle);
extern int net_get_if_addr(struct wlan_ip_config *addr, void *intrfc_handle);
extern void ip_address_set(int iface, int dhcp, char *ip, char *mask, char*gw, char*dns);
#if CFG_WLAN_FAST_CONNECT_STATIC_IP || CFG_WLAN_SUPPORT_FAST_DHCP
extern void net_restart_dhcp(void);
#endif
#ifdef __cplusplus
}
#endif

#endif // _NET_H_
// eof

