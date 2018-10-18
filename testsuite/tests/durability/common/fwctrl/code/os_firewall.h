/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
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
