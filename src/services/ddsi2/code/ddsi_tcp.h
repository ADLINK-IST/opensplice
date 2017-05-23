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
#ifndef _DDSI_TCP_H_
#define _DDSI_TCP_H_

#include "ddsi_tran.h"

#ifdef DDSI_INCLUDE_SSL

#include "ddsi_ssl.h"

struct ddsi_ssl_plugins
{
  void (*config) (void);
  c_bool (*init) (void);
  void (*fini) (void);
  void (*ssl_free) (SSL * ssl);
  void (*bio_vfree) (BIO * bio);
  os_ssize_t (*read) (SSL * ssl, void * buf, os_size_t len, int * err);
  os_ssize_t (*write) (SSL * ssl, const void * msg, os_size_t len, int * err);
  SSL * (*connect) (os_socket sock);
  BIO * (*listen) (os_socket sock);
  SSL * (*accept) (BIO * bio, os_socket * sock);
};

struct ddsi_ssl_plugins ddsi_tcp_ssl_plugin;

#endif

int ddsi_tcp_init (void);

#endif

/* SHA1 not available (unoffical build.) */
