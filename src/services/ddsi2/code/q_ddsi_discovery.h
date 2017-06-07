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
#ifndef NN_DDSI_DISCOVERY_H
#define NN_DDSI_DISCOVERY_H

#include "q_unused.h"

#if defined (__cplusplus)
extern "C" {
#endif

struct participant;
struct writer;
struct reader;
struct nn_rsample_info;
struct nn_rdata;
struct nn_plist;

int spdp_write (struct participant *pp);
int spdp_dispose_unregister (struct participant *pp);

int sedp_write_writer (struct writer *wr);
int sedp_write_reader (struct reader *rd);
int sedp_dispose_unregister_writer (struct writer *wr);
int sedp_dispose_unregister_reader (struct reader *rd);

int sedp_write_topic (struct participant *pp, const struct nn_plist *datap);
int sedp_write_cm_participant (struct participant *pp, int alive);
int sedp_write_cm_publisher (const struct nn_plist *datap, int alive);
int sedp_write_cm_subscriber (const struct nn_plist *datap, int alive);

int builtins_dqueue_handler (const struct nn_rsample_info *sampleinfo, const struct nn_rdata *fragchain, const nn_guid_t *rdguid, void *qarg);

#if defined (__cplusplus)
}
#endif

#endif /* NN_DDSI_DISCOVERY_H */

/* SHA1 not available (unoffical build.) */
