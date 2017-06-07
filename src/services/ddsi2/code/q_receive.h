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
#ifndef Q_RECEIVE_H
#define Q_RECEIVE_H

#if defined (__cplusplus)
extern "C" {
#endif

struct nn_rbufpool;
struct nn_rsample_info;
struct nn_rdata;
struct ddsi_tran_listener;

void *recv_thread (struct nn_rbufpool *rbpool);
void *listen_thread (struct ddsi_tran_listener * listener);
int user_dqueue_handler (const struct nn_rsample_info *sampleinfo, const struct nn_rdata *fragchain, const nn_guid_t *rdguid, void *qarg);

#if defined (__cplusplus)
}
#endif

#endif /* Q_RECEIVE_H */

/* SHA1 not available (unoffical build.) */
