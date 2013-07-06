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
#ifndef OS_FIREWALL_H
#define OS_FIREWALL_H

#if defined (__cplusplus)
extern "C" {
#endif

#include <os_defs.h>
#include <netinet/in.h>

/* include OS specific header file                              */
#include <include/os_stdlib.h>
#include <os_if.h>

#ifdef OSPL_BUILD_OS
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define srcAddr char*

typedef enum {
    OPEN,
    CLOSED
} os_portState;

OS_API os_result
os_setPort(char *srcHost, int port, os_portState state, int prot);

struct sockaddr_in
resolveHost(char *hostStr, int port);

char*
getRule(char **fw, struct sockaddr_in *host, int prot);

#endif /* OS_FIREWALL_H */
