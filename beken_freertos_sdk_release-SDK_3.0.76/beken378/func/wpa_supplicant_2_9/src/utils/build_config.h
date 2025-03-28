/*
 * wpa_supplicant/hostapd - Build time configuration defines
 * Copyright (c) 2005-2006, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 *
 * This header file can be used to define configuration defines that were
 * originally defined in Makefile. This is mainly meant for IDE use or for
 * systems that do not have suitable 'make' tool. In these cases, it may be
 * easier to have a single place for defining all the needed C pre-processor
 * defines.
 */

#ifndef BUILD_CONFIG_H
#define BUILD_CONFIG_H

#include "sys_config.h"
#include "rwnx_config.h"
/* Insert configuration defines, e.g., #define EAP_MD5, here, if needed. */

#define BK_SUPPLICANT  1

//#define BK_SUPPLICANT_OPTIMIZE_CONNECTION          1

#define NEED_AP_MLME
#define CONFIG_NO_CONFIG_WRITE
#define OS_NO_C_LIB_DEFINES
#define HOSTAPD
#define CONFIG_NO_CONFIG_BLOBS

#if CFG_IEEE80211N
/* IEEE 802.11n (High Throughput) support */
#define CONFIG_IEEE80211N                          1
#endif

/* Driver interface for beken driver */
#define CONFIG_DRIVER_BEKEN                        1

/* ocv verify operating channel should enable the MFP */
#if CFG_WIFI_OCV
#define CONFIG_OCV
#endif

/* WPA2/IEEE 802.11i RSN pre-authentication */
//#define CONFIG_RSN_PREAUTH   1

/* PeerKey handshake for Station to Station Link (IEEE 802.11e DLS) */
//#define CONFIG_PEERKEY      1

/* IEEE 802.11w (management frame protection) */
//#define CONFIG_IEEE80211W   1

/* Integrated EAP server */
//#define CONFIG_EAP    1
/* EAP Re-authentication Protocol (ERP) in integrated EAP server */
//#define CONFIG_ERP    1
/* EAP-MD5 for the integrated EAP server */
#define CONFIG_EAP_MD5   1
/* EAP-TLS for the integrated EAP server */
#define CONFIG_EAP_TLS   1
/* EAP-MSCHAPv2 for the integrated EAP server */
//#define CONFIG_EAP_MSCHAPV2   1
/* EAP-PEAP for the integrated EAP server */
//#define CONFIG_EAP_PEAP    1
/* EAP-GTC for the integrated EAP server */
//#define CONFIG_EAP_GTC    1
/* EAP-TTLS for the integrated EAP server */
//#define CONFIG_EAP_TTLS   1
/* EAP-SIM for the integrated EAP server */
//#define CONFIG_EAP_SIM   1
/* EAP-AKA for the integrated EAP server */
//#define CONFIG_EAP_AKA    1
/* EAP-AKA' for the integrated EAP server.
    This requires CONFIG_EAP_AKA to be enabled, too. */
//#define CONFIG_EAP_AKA_PRIME  1
/* EAP-PAX for the integrated EAP server */
//#define CONFIG_EAP_PAX    1
/* EAP-PSK for the integrated EAP server (this is _not_ needed for WPA-PSK) */
//#define CONFIG_EAP_PSK    1
/* EAP-pwd for the integrated EAP server (secure authentication with a password) */
//#define CONFIG_EAP_PWD    1
/* EAP-SAKE for the integrated EAP server */
//#define CONFIG_EAP_SAKE     1

/* EAP-GPSK for the integrated EAP server */
//#define CONFIG_EAP_GPSK   1
/* Include support for optional SHA256 cipher suite in EAP-GPSK */
//#define CONFIG_EAP_GPSK_SHA256  1

/* EAP-FAST for the integrated EAP server
 *  Note: If OpenSSL is used as the TLS library, OpenSSL 1.0 or newer is needed
 *  for EAP-FAST support. Older OpenSSL releases would need to be patched, e.g.,
 *  with openssl-0.9.8x-tls-extensions.patch, to add the needed functions. */
//#define CONFIG_EAP_FAST 1

/* Wi-Fi Protected Setup (WPS) */
//#define CONFIG_WPS 1
/* Enable UPnP support for external WPS Registrars */
//#define CONFIG_WPS_UPNP  1
/* Enable WPS support with NFC config method */
//#define CONFIG_WPS_NFC   1

/* EAP-IKEv2 */
//#define CONFIG_EAP_IKEV2  1

/* Trusted Network Connect (EAP-TNC) */
//#define CONFIG_EAP_TNC  1

/* EAP-EKE for the integrated EAP server */
//#define CONFIG_EAP_EKE  1

/* PKCS#12 (PFX) support (used to read private key and certificate file from
 *  a file that usually has extension .p12 or .pfx) */
//#define CONFIG_PKCS12  1

/* RADIUS authentication server. This provides access to the integrated EAP
 *  server from external hosts using RADIUS. */
//#define CONFIG_RADIUS_SERVER  1

/* Build IPv6 support for RADIUS operations */
//#define CONFIG_IPV6  1

/* IEEE Std 802.11r-2008 (Fast BSS Transition) */
//#define CONFIG_IEEE80211R  1

/* Use the hostapd's IEEE 802.11 authentication (ACL), but without
 *  the IEEE 802.11 Management capability (e.g., FreeBSD/net80211) */
//#define CONFIG_DRIVER_RADIUS_ACL  1

/* Wireless Network Management (IEEE Std 802.11v-2011)
    Note: This is experimental and not complete implementation. */
//#define CONFIG_WNM   1

/* IEEE 802.11ac (Very High Throughput) support */
//#define  CONFIG_IEEE80211AC  1

/* Remove debugging code that is printing out debug messages to stdout.
 *  This can be used to reduce the size of the hostapd considerably if debugging
 *  code is not needed. */
#if !CFG_ENABLE_WPA_LOG
#define CONFIG_NO_STDOUT_DEBUG  	1
#define CONFIG_NO_HOSTAPD_LOGGER   	1
#define CONFIG_NO_WPA_MSG          	1
#endif

/* Add support for writing debug log to a file: -f /tmp/hostapd.log
 * Disabled by default. */
//#define CONFIG_DEBUG_FILE  1

/* Remove support for RADIUS accounting */
#define CONFIG_NO_ACCOUNTING 1

/* Remove support for RADIUS */
#define CONFIG_NO_RADIUS 1

/* Remove support for VLANs */
#define CONFIG_NO_VLAN  1

/* Enable support for fully dynamic VLANs. This enables hostapd to
 *  automatically create bridge and VLAN interfaces if necessary. */
//#define CONFIG_FULL_DYNAMIC_VLAN 1

/* Remove support for dumping internal state through control interface commands
 *  This can be used to reduce binary size at the cost of disabling a debugging option. */
#define CONFIG_NO_DUMP_STATE  1

/* Enable tracing code for developer debugging
 *  This tracks use of memory allocations and other registrations and reports
 *  incorrect use with a backtrace of call (or allocation) location. */
//#define CONFIG_WPA_TRACE  1

/* Use libbfd to get more details for developer debugging
 *  This enables use of libbfd to get more detailed symbols for the backtraces
 *  generated by CONFIG_WPA_TRACE. */
//#define CONFIG_WPA_TRACE_BFD  1

/* os_get_random() function is used to fetch random data when needed. */
#define CONFIG_NO_RANDOM_POOL 1

/* Select TLS implementation
 *  openssl = OpenSSL (default)
 *  gnutls = GnuTLS
 *  internal = Internal TLSv1 implementation (experimental)
 *  none = Empty template */
//#define CONFIG_TLS  openssl

/* TLS-based EAP methods require at least TLS v1.0. Newer version of TLS (v1.1)
 *  can be enabled to get a stronger construction of messages when block ciphers
 *  are used. */
//#define CONFIG_TLSV11  1

/* TLS-based EAP methods require at least TLS v1.0. Newer version of TLS (v1.2)
 *  can be enabled to enable use of stronger crypto algorithms. */
//#define CONFIG_TLSV12  1

/* If CONFIG_TLS=internal is used, additional library and include paths are
 *  needed for LibTomMath. Alternatively, an integrated, minimal version of
 *  LibTomMath can be used. See beginning of libtommath.c for details on benefits
 *  and drawbacks of this option. */
//#define CONFIG_INTERNAL_LIBTOMMATH  1
/* At the cost of about 4 kB of additional binary size, the internal LibTomMath
 *  can be configured to include faster routines for exptmod, sqr, and div to
 *  speed up DH and RSA calculation considerably */
//#define CONFIG_INTERNAL_LIBTOMMATH_FAST  1

/* Interworking (IEEE 802.11u)
 * This can be used to enable functionality to improve interworking with
 * external networks. */
//#define CONFIG_INTERWORKING  1

/* Hotspot 2.0 */
//#define CONFIG_HS20  1

/* Enable SQLite database support in hlr_auc_gw, EAP-SIM DB, and eap_user_file */
//#define CONFIG_SQLITE  1

/* Enable Fast Session Transfer (FST) */
//#define CONFIG_FST 1

/* Enable CLI commands for FST testing */
//#define CONFIG_FST_TEST  1

/* Testing options
 *  This can be used to enable some testing options (see also the example
 *  configuration file) that are really useful only for testing clients that
 *  connect to this hostapd. These options allow, for example, to drop a
 *  certain percentage of probe requests or auth/(re)assoc frames. */
//#define CONFIG_TESTING_OPTIONS  1

/* Automatic Channel Selection */
//#define CONFIG_ACS  1

/* caculate psk in advance, and store it in memory */
#define CONFIG_WPA_PSK_CACHE	   1

/* enable multiple PSK cache */
//#define CONFIG_WPA_PSK_CACHE_MULTI 1

#define CONFIG_SELECT_NETWORK_AFTER_PSK_SET   1

#define CONFIG_NO_ROAMING

#if CFG_STA_AUTO_RECONNECT
#define CONFIG_AUTO_RECONNECT
#endif

#if CFG_IEEE80211W
#define CONFIG_IEEE80211W
#endif

#if CFG_WPA3
//#define CFG_SOFTAP_WPA3 1
#define CONFIG_SAE
#define CONFIG_ECC
#define CONFIG_SAE_SMALL_STACK
#define CONFIG_SAE_OVERCLOCK
//#define CONFIG_SHA384

#if CFG_SME
#define CONFIG_SME
#endif

/* enable softap SAE */
#if CFG_SOFTAP_WPA3
#define CONFIG_SAE_AP
#define CONFIG_IEEE80211W_AP
#endif /* CFG_SOFTAP_WPA3 */

#if CFG_OWE
#define CONFIG_OWE 1
#endif

#ifndef CONFIG_SME
#define CONFIG_SAE_EXTERNAL
#endif
#endif

#if CFG_WPA_CTRL_IFACE
#define HOSTAP_THREAD_SAFE_WORKAROUND 0
#else
#define HOSTAP_THREAD_SAFE_WORKAROUND 1
#endif

/* fixed scan interval from previous connect request regardless of connection time */
//#define CONFIG_WPA_FIXED_SCAN_INTERVAL

#if CFG_WPA2_ENTERPRISE || CFG_WIFI_WPS
#define IEEE8021X_EAPOL
#define CONFIG_CRYPTO_INTERNAL
#define CONFIG_INTERNAL_LIBTOMMATH
#define LTM_FAST
#define CONFIG_DES
#define CONFIG_SHA256
#define CONFIG_INTERNAL_SHA384
#define CONFIG_INTERNAL_SHA512

#if CFG_WPA2_ENTERPRISE
#define EAP_TLS
#define CONFIG_TLS_INTERNAL_CLIENT
#endif

#if CFG_WPA3_ENTERPRISE
#define CONFIG_SUITEB
#define CONFIG_SUITEB192
#endif

/* the ca.pem and client private-key.pem is hard-coded */
// #define IEEE8021X_EAPOL_DEMO
#endif

#if CFG_WIFI_WPS
#define CONFIG_WPS
#define CONFIG_WSC
#define EAP_WSC
#define CONFIG_BK_WPS_WORKAROUND
#endif

#if CFG_WIFI_P2P
#define CONFIG_P2P
#define CONFIG_OFFCHANNEL
#define CONFIG_AP
#if CFG_WIFI_P2P_GO
#define CONFIG_WPS_AP
#define EAP_SERVER_WSC
#define EAP_SERVER_IDENTITY
#define CONFIG_EAP_SERVER
#endif
#endif

#endif /* BUILD_CONFIG_H */
