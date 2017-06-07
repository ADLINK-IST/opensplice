/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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

/* SHA1 not available (unoffical build.) */
