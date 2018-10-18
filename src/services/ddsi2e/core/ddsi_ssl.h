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
#ifndef _DDSI_SSL_H_
#define _DDSI_SSL_H_

#ifdef DDSI_INCLUDE_SSL

#ifdef _WIN32
/* WinSock2 must be included before openssl headers
   otherwise winsock will be used */
#include <WinSock2.h>
#endif

#include <openssl/ssl.h>

void ddsi_ssl_plugin (void);

#endif
#endif
