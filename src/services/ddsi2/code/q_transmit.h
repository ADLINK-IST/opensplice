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
#ifndef Q_TRANSMIT_H
#define Q_TRANSMIT_H

#include "os_defs.h"
#include "q_rtps.h" /* for nn_entityid_t */

#if defined (__cplusplus)
extern "C" {
#endif

struct nn_xpack;
struct nn_xmsg;
struct writer;
struct proxy_reader;
struct serdata;

/* Writing new data; serdata_twrite (serdata) is assumed to be really
   recentish; serdata is unref'd.  If xp == NULL, data is queued, else
   packed. */
int write_sample (struct nn_xpack *xp, struct writer *wr, struct serdata *serdata);
int write_sample_kernel_seq (struct nn_xpack *xp, struct writer *wr, struct serdata *serdata, int have_kernel_seq, os_uint32 kernel_seq);

void begin_coherent_set (struct writer *wr);
/* plist (if != NULL) gets "consumed" by end_coherent_set */
int end_coherent_set (struct nn_xpack *xp, struct writer *wr, struct nn_plist *plist, struct serdata *serdata, int have_kernel_seq, os_uint32 kernel_seq);

/* When calling the following functions, wr->lock must be held */
int create_fragment_message (struct writer *wr, os_int64 seq, const struct nn_plist *plist, struct serdata *serdata, unsigned fragnum, struct proxy_reader *prd,struct nn_xmsg **msg, int isnew);
int enqueue_sample_wrlock_held (struct writer *wr, os_int64 seq, const struct nn_plist *plist, struct serdata *serdata, struct proxy_reader *prd, int isnew);
void add_Heartbeat (struct nn_xmsg *msg, struct writer *wr, int hbansreq, nn_entityid_t dst, int issync);

#if defined (__cplusplus)
}
#endif

#endif /* Q_TRANSMIT_H */

/* SHA1 not available (unoffical build.) */
