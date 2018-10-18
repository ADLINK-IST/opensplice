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

#include <ioctl.h>

typedef uint16_t        in_port_t;

#if defined ( VXWORKS_55 ) || defined ( VXWORKS_54 )

typedef size_t          socklen_t;
#define       AF_INET6 26

struct in6_addr {
        union {
                /*
                 * Note: Static initalizers of "union" type assume
                 * the constant on the RHS is the type of the first member
                 * of union.
                 * To make static initializers (and efficient usage) work,
                 * the order of members exposed to user and kernel view of
                 * this data structure is different.
                 * User environment sees specified uint8_t type as first
                 * member whereas kernel sees most efficient type as
                 * first member.
                 */
#ifdef _KERNEL
                uint32_t        _S6_u32[4];     /* IPv6 address */
                uint8_t         _S6_u8[16];     /* IPv6 address */
#else
                uint8_t         _S6_u8[16];     /* IPv6 address */
                uint32_t        _S6_u32[4];     /* IPv6 address */
#endif
                uint32_t        __S6_align;     /* Align on 32 bit boundary */
        } _S6_un;
};

typedef unsigned short  sa_family_t;

struct sockaddr_in6 {
        sa_family_t     sin6_family;
        in_port_t       sin6_port;
        uint32_t        sin6_flowinfo;
        struct in6_addr sin6_addr;
        uint32_t        sin6_scope_id;  /* Depends on scope of sin6_addr */
        uint32_t        __sin6_src_id;  /* Impl. specific - UDP replies */
};

#endif
