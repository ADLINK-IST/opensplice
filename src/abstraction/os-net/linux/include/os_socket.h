/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

#ifndef OS_LINUX_SOCKET_H
#define OS_LINUX_SOCKET_H

#if defined (__cplusplus)
extern "C" {
#endif

#include <sys/socket.h>
#include <net/if.h>

#ifndef __USE_GNU
/* set the flag and mind to undef the flag later */
#define __USE_GNU
#define UNDEF_USE_GNU
#endif

#include <netinet/in.h>
#include <netinet/tcp.h>

/* only undef __USE_GNU if explicitly set before */
#ifdef UNDEF_USE_GNU
#undef __USE_GNU
#undef UNDEF_USE_GNU
#endif


#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <unistd.h>
#include <linux/socket.h>

#ifndef _GNU_SOURCE
#include <linux/if.h>
#endif


/* Keep defines before common header */
#define OS_IFNAMESIZE		IF_NAMESIZE
#define OS_NO_SIOCGLIFCONF
#define OS_SOCKET_HAS_SA_LEN	0
#define OS_SOCKET_HAS_IPV6      1

#include "../common/include/os_socket.h"

#if defined (__cplusplus)
}
#endif

#endif /* OS_LINUX_SOCKET_H */
