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
#ifndef Q_PCAP_H
#define Q_PCAP_H

#include <stdio.h>
#include "q_time.h"

#if defined (__cplusplus)
extern "C" {
#endif

struct msghdr;

FILE * new_pcap_file (const char *name);

void write_pcap_received
(
  FILE * fp,
  nn_wctime_t tstamp,
  const os_sockaddr_storage * src,
  const os_sockaddr_storage * dst,
  unsigned char * buf,
  os_size_t sz
);

void write_pcap_sent
(
  FILE * fp,
  nn_wctime_t tstamp,
  const os_sockaddr_storage * src,
  const struct msghdr * hdr,
  os_size_t sz
);

#if defined (__cplusplus)
}
#endif

#endif /* Q_PCAP_H */

/* SHA1 not available (unoffical build.) */
