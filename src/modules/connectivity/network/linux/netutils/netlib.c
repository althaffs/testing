/****************************************************************************
 * netutils/netlib/netlib.c
 *
 *   Copyright (C) 2007, 2009, 2011-2012 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Based heavily on portions of uIP:
 *
 *   Author: Adam Dunkels <adam@dunkels.com>
 *   Copyright (c) 2005, Swedish Institute of Computer Science
 *   All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 ****************************************************************************/

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <net/if.h>
#include <net/route.h>

#include <arpa/inet.h>

#include <sys/select.h>

#include "netlib.h"

int get_ipv4addr(const char *ifname, struct in_addr *addr)
{
	int ret = ERROR;

	if (ifname && addr) {
		int sockfd = socket(PF_INET, SOCK_DGRAM, 0);

		if (sockfd >= 0) {
			struct ifreq req;

			strncpy(req.ifr_name, ifname, IFNAMSIZ);
			ret = ioctl(sockfd, SIOCGIFADDR, (unsigned long)&req);

			if (!ret) {
				struct sockaddr_in *req_addr;

				req_addr = (struct sockaddr_in *)&req.ifr_addr;
				memcpy(addr, &req_addr->sin_addr,
					sizeof(struct in_addr));
			}

			close(sockfd);
		}
	}

	return ret;
}

int set_ipv4addr(const char *ifname, const struct in_addr *addr)
{
	int ret = ERROR;

	if (ifname && addr) {
		int sockfd = socket(PF_INET, SOCK_DGRAM, 0);

		if (sockfd >= 0) {
			struct sockaddr_in *inaddr;
			struct ifreq req;

			/* Add the device name to the request */
			strncpy(req.ifr_name, ifname, IFNAMSIZ);

			/* Add the INET address to the request */
			inaddr             = (struct sockaddr_in *)
						&req.ifr_addr;
			inaddr->sin_family = AF_INET;
			inaddr->sin_port   = 0;
			memcpy(&inaddr->sin_addr, addr, sizeof(struct in_addr));

			ret = ioctl(sockfd, SIOCSIFADDR, (unsigned long)&req);
			close(sockfd);
		}
	}

	return ret;
}

int get_dripv4addr(const char *ifname, struct in_addr *addr)
{
	int ret = ERROR;

	if (ifname && addr) {
		int sockfd = socket(PF_INET, SOCK_DGRAM, 0);

		if (sockfd >= 0) {
			struct ifreq req;

			strncpy(req.ifr_name, ifname, IFNAMSIZ);
			ret = ioctl(sockfd, SIOCGIFDSTADDR,
				(unsigned long)&req);

			if (!ret) {
				struct sockaddr_in *req_addr;

				req_addr = (struct sockaddr_in *)&req.ifr_addr;
				memcpy(addr, &req_addr->sin_addr,
					sizeof(struct in_addr));
			}

			close(sockfd);
		}
	}

	return ret;
}

int set_dripv4addr(const char *ifname, const struct in_addr *addr)
{
	int ret = ERROR;

	if (ifname && addr) {
		int sockfd = socket(PF_INET, SOCK_DGRAM, 0);

		if (sockfd >= 0) {
			struct sockaddr_in *inaddr;
			struct ifreq req;

			/* Add the device name to the request */
			strncpy(req.ifr_name, ifname, IFNAMSIZ);

			/* Add the INET address to the request */
			inaddr			= (struct sockaddr_in *)
							&req.ifr_addr;
			inaddr->sin_family	= AF_INET;
			inaddr->sin_port	= 0;
			memcpy(&inaddr->sin_addr, addr, sizeof(struct in_addr));

			ret = ioctl(sockfd, SIOCSIFDSTADDR,
				(unsigned long)&req);
			close(sockfd);
		}
	}

	return ret;
}

int get_ipv4netmask(const char *ifname, struct in_addr *addr)
{
	int ret = ERROR;

	if (ifname && addr) {
		int sockfd = socket(PF_INET, SOCK_DGRAM, 0);

		if (sockfd >= 0) {
			struct ifreq req;

			strncpy(req.ifr_name, ifname, IFNAMSIZ);
			ret = ioctl(sockfd, SIOCGIFNETMASK,
				(unsigned long)&req);

			if (!ret) {
				struct sockaddr_in *req_addr;

				req_addr = (struct sockaddr_in *)&req.ifr_addr;
				memcpy(addr, &req_addr->sin_addr,
					sizeof(struct in_addr));
			}

			close(sockfd);
		}
	}

	return ret;
}

int set_ipv4netmask(const char *ifname, const struct in_addr *addr)
{
	int ret = ERROR;

	if (ifname && addr) {
		int sockfd = socket(PF_INET, SOCK_DGRAM, 0);

		if (sockfd >= 0) {
			struct sockaddr_in *inaddr;
			struct ifreq req;

			/* Add the device name to the request */
			strncpy(req.ifr_name, ifname, IFNAMSIZ);

			/* Add the INET address to the request */
			inaddr			= (struct sockaddr_in *)
							&req.ifr_addr;
			inaddr->sin_family	= AF_INET;
			inaddr->sin_port	= 0;
			memcpy(&inaddr->sin_addr, addr, sizeof(struct in_addr));

			ret = ioctl(sockfd, SIOCSIFNETMASK,
				(unsigned long)&req);
			close(sockfd);
		}
	}

	return ret;
}

int getmacaddr(const char *ifname, uint8_t  *macaddr)
{
	int ret = ERROR;

	if (ifname && macaddr) {

		/* Get a socket (only so that we get access to the INET
		 * subsystem)
		 */
		int sockfd = socket(PF_INET, SOCK_DGRAM, 0);

		if (sockfd >= 0) {
			struct ifreq req;

			memset(&req, 0, sizeof(struct ifreq));

			/* Put the driver name into the request */
			strncpy(req.ifr_name, ifname, IFNAMSIZ);

			/* Perform the ioctl to get the MAC address */
			ret = ioctl(sockfd, SIOCGIFHWADDR, (unsigned long)&req);
			if (!ret)
				/* Return the MAC address */
				memcpy(macaddr, &req.ifr_hwaddr.sa_data,
					IFHWADDRLEN);

			close(sockfd);
		}
	}

	return ret;
}

int set_defaultroute(const char *ifname, const struct in_addr *addr,
		bool gateway)
{
	int ret = ERROR;

	if (ifname && addr) {

		int sockfd = socket(PF_INET, SOCK_DGRAM, 0);

		if (sockfd >= 0) {
			struct sockaddr_in *inaddr;
			struct rtentry rt;

			/* Add the INET address to the gateway */
			inaddr			= (struct sockaddr_in *)
						&rt.rt_gateway;
			inaddr->sin_family	= AF_INET;
			inaddr->sin_addr.s_addr = addr->s_addr;

			/* Set the destination to 0.0.0.0 */
			inaddr			= (struct sockaddr_in *)
						&rt.rt_dst;
			inaddr->sin_family	= AF_INET;
			inaddr->sin_addr.s_addr = INADDR_ANY;

			/* Set the destination to 0.0.0.0 */
			inaddr			= (struct sockaddr_in *)
						&rt.rt_genmask;
			inaddr->sin_family	= AF_INET;
			inaddr->sin_addr.s_addr = INADDR_ANY;

			/* Route usable */
			rt.rt_flags = RTF_UP;

			/* If we have to define a route to gateway */
			if (gateway)
				rt.rt_flags |= RTF_GATEWAY;

			/* Set the name of device */
			rt.rt_dev = (char *)ifname;

			/* Set metric to zero */
			rt.rt_metric = 0;

			ret = ioctl(sockfd, SIOCADDRT, &rt);
			close(sockfd);
		}
	}

	return ret;
}

int del_allroutes_interface(const char *ifname)
{
	char buf[CMD_LENGTH];

	sprintf(buf, "ip route flush dev %s", ifname);

	return system(buf);
}

int dns_add_nameserver(const struct sockaddr *addr, socklen_t addrlen,
		bool append)
{
	FILE *stream;
	char addstr[DNS_MAX_ADDRSTR];
	int status;
	int ret;
	const char *flag = append ? "at" : "wt";

	stream = fopen(CONFIG_NETDB_RESOLVCONF_PATH, flag);
	if (stream == NULL) {
		log_err("Error: failed to open %s: %d",
			CONFIG_NETDB_RESOLVCONF_PATH, errno);
		return ERROR;
	}

	/* Check for an IPv4 address */
	if (addr->sa_family == AF_INET) {
		if (addrlen < sizeof(struct sockaddr_in)) {
			ret = ERROR;
			goto errout;
		} else {
			struct sockaddr_in *in4 = (struct sockaddr_in *)addr;

			if (!inet_ntop(AF_INET, &in4->sin_addr,
						addstr, DNS_MAX_ADDRSTR)) {
				ret = ERROR;
				log_err("Error: inet_ntop failed: %d", errno);
				goto errout;
			}
		}
	} else {
		log_err("Error: Unsupported family :%d", addr->sa_family);
		ret = ERROR;
		goto errout;
	}

	status = fprintf(stream, "%s %s\n", NETDB_DNS_KEYWORD, addstr);

	if (status < 0) {
		ret = ERROR;
		log_err("Error: fprintf failed : %d", errno);
		goto errout;
	}

	ret = OK;

errout:
	fclose(stream);
	return ret;
}

int get_ipv4dnsaddr(struct in_addr *addr, int numServers)
{
	FILE *stream;
	int ret;

	if (!addr) {
		log_err("addr is NULL");
		return ERROR;
	}

	stream = fopen(CONFIG_NETDB_RESOLVCONF_PATH, "r");

	char buffer[CMD_LENGTH];
	char dnsServer[MAX_IP_ADDRESS_LEN];
	int i = 0;

	if (stream == NULL) {
		log_err("Error: failed to open %s: %d",
			CONFIG_NETDB_RESOLVCONF_PATH, errno);

		return ERROR;
	}

	while (!feof(stream)) {

		int b1, b2, b3, b4;

		if (fscanf(stream, "nameserver %d.%d.%d.%d", &b1, &b2, &b3, &b4)
						== 4 && i < numServers) {

			snprintf(dnsServer, MAX_IP_ADDRESS_LEN, "%d.%d.%d.%d",
				b1, b2, b3, b4);

			if (&addr[i]) {
				if (inet_aton(dnsServer, &addr[i]) == 0) {
					ret = ERROR;
					goto errout;
				}
			}
			i++;
		}
		fgets(buffer, CMD_LENGTH, stream);

	}

	ret = OK;

errout:
	fclose(stream);
	return ret;
}

int set_ipv4dnsaddr(const struct in_addr *inaddr, bool append)
{
	struct sockaddr_in addr;
	int ret = ERROR;

	if (inaddr) {
		/* Set the IPv4 DNS server address */
		addr.sin_family = AF_INET;
		addr.sin_port	= 0;
		memcpy(&addr.sin_addr, inaddr, sizeof(struct in_addr));

		ret = dns_add_nameserver((const struct sockaddr *)&addr,
			sizeof(struct sockaddr_in), append);
	}

	return ret;
}

int verify_ipv4addr_in_used(const struct in_addr *inaddr)
{
	int ret = ERROR;
	struct icmphdr icmp_hdr;
	struct sockaddr_in addr;
	int sequence = 0;

	if (system("sysctl -w net.ipv4.ping_group_range=\"0 0\" > NUL") < 0) {
		log_err("Error system");
		ret = ERROR;
		goto errout;
	}

	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);

	log_dbg("Ping IP address %d.%d.%d.%d\n",
		(inaddr->s_addr) & 0xff,
		(inaddr->s_addr >> 8) & 0xff,
		(inaddr->s_addr >> 16) & 0xff,
		(inaddr->s_addr >> 24) & 0xff);

	if (sock < 0) {
		log_err("Error sock : %d", errno);
		ret = ERROR;
		goto errout;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr = *inaddr;

	memset(&icmp_hdr, 0, sizeof(icmp_hdr));
	icmp_hdr.type = ICMP_ECHO;
	icmp_hdr.un.echo.id = 1234;

	for (;;) {

		unsigned char data[2048];
		int rc;
		struct timeval timeout = {2, 0};
		fd_set read_set;
		socklen_t slen;
		struct icmphdr rcv_hdr;

		icmp_hdr.un.echo.sequence = sequence++;

		memcpy(data, &icmp_hdr, sizeof(icmp_hdr));

		memcpy(data + sizeof(icmp_hdr), "hello", 5);

		rc = sendto(sock, data, sizeof(icmp_hdr) + 5,
			0, (struct sockaddr *)&addr, sizeof(addr));

		if (rc <= 0) {
			log_err("Error Sendto");
			ret = ERROR;
			goto errout;
		}

		log_dbg("Sent ICMP");

		memset(&read_set, 0, sizeof(read_set));
		FD_SET(sock, &read_set);

		//wait for a reply with a timeout
		rc = select(sock + 1, &read_set, NULL, NULL, &timeout);

		if (rc == 0) {
			log_dbg("Got no reply\n");
			ret = ERROR;
			goto errout;
		} else if (rc < 0) {
			log_err("Select");
			ret = ERROR;
			goto errout;
		}

		slen = 0;

		rc = recvfrom(sock, data, sizeof(data), 0, NULL, &slen);
		if (rc <= 0) {
			log_err("recvfrom");
			ret = ERROR;
			goto errout;
		} else if (rc < sizeof(rcv_hdr)) {
			log_dbg("Error, got short ICMP packet, %d bytes\n", rc);
			ret = ERROR;
			goto errout;
		}

		memcpy(&rcv_hdr, data, sizeof(rcv_hdr));

		if (rcv_hdr.type == ICMP_ECHOREPLY) {
			log_dbg("ICMP Reply, id=0x%x, sequence =  0x%x\n",
				icmp_hdr.un.echo.id, icmp_hdr.un.echo.sequence);
			ret = OK;
			goto errout;
		} else {
			log_dbg("Got ICMP packet with type 0x%x ?!?\n",
				rcv_hdr.type);
			ret = ERROR;
			goto errout;
		}

	}

errout:
	return ret;
}
