/****************************************************************************
 *  apps/include/netutils/netlib.h
 * Various non-standard APIs to support netutils.  All non-standard and
 * intended only for internal use.
 *
 * Copyright (C) 2007, 2009, 2011, 2015, 2017 Gregory Nutt. All rights reserved.
 *  Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Some of these APIs derive from uIP.  uIP also has a BSD style license:
 *
 *   Author: Adam Dunkels <adam@sics.se>
 *   Copyright (c) 2002, Adam Dunkels.
 *   All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

#ifndef __APPS_INCLUDE_NETUTILS_NETLIB_H
#define __APPS_INCLUDE_NETUTILS_NETLIB_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#include <netinet/in.h>

#include <artik_log.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define ERROR				-1
#define OK				0

#define IFHWADDRLEN			6

#define DNS_DEFAULT_PORT		53
#define CONFIG_NETDB_RESOLVCONF_PATH	"/etc/resolv.conf"
#define DNS_MAX_ADDRSTR			48
#define DNS_MAX_LINE			64
#define NETDB_DNS_KEYWORD		"nameserver"

#define CMD_LENGTH			256

#define MAX_IP_ADDRESS_LEN		24

/****************************************************************************
 * Public Data
 ****************************************************************************/

union dns_server_u {
	struct sockaddr		addr;
	struct sockaddr_in	ipv4;
};

#undef EXTERN
#if defined(__cplusplus)
extern "C"
{
#endif

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

int getmacaddr(const char *ifname, uint8_t *macacddr);
int get_ipv4addr(const char *ifname, struct in_addr *addr);
int set_ipv4addr(const char *ifname, const struct in_addr *addr);
int get_dripv4addr(const char *ifname, struct in_addr *addr);
int set_dripv4addr(const char *ifname, const struct in_addr *addr);
int get_ipv4netmask(const char *ifname, struct in_addr *addr);
int set_ipv4netmask(const char *ifname, const struct in_addr *addr);
int set_defaultroute(const char *ifname, const struct in_addr *addr,
		bool gateway);
int del_allroutes_interface(const char *ifname);
int dns_add_nameserver(const struct sockaddr *addr, socklen_t addrlen,
		bool append);
int get_ipv4dnsaddr(struct in_addr *addr, int numServers);
int set_ipv4dnsaddr(const struct in_addr *inaddr, bool append);
int verify_ipv4addr_in_used(const struct in_addr *inaddr);

#ifdef __cplusplus
}
#endif

#endif /* __APPS_INCLUDE_NETUTILS_NETLIB_H */
