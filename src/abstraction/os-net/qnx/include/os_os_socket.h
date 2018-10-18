/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#ifndef OS_QNX_SOCKET_H
#define OS_QNX_SOCKET_H

#include <net/if.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <netdb.h>

/* Keep defines before common header */
#define OS_NO_SIOCGLIFCONF
#define OS_NO_NETLINK
#define OS_IFNAMESIZE        	16
#define OS_SOCKET_HAS_SA_LEN 	1
#define OS_SOCKET_HAS_IPV6      1
#define DO_HOST_BY_NAME         1

#include "../common/include/os_socket.h"


#endif /* OS_QNX_SOCKET_H */
