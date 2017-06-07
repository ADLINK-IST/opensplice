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
#ifndef Q_LEASE_H
#define Q_LEASE_H

#include "q_time.h"

#if defined (__cplusplus)
extern "C" {
#endif

struct receiver_state;
struct participant;
struct lease;
struct entity_common;
struct thread_state1;

void lease_management_init (void);
void lease_management_term (void);
struct lease *lease_new (os_int64 tdur, struct entity_common *e);
void lease_register (struct lease *l);
void lease_free (struct lease *l);
void lease_renew (struct lease *l, nn_etime_t tnow);
void check_and_handle_lease_expiration (struct thread_state1 *self, nn_etime_t tnow);

void handle_PMD (const struct receiver_state *rst, unsigned statusinfo, const void *vdata, unsigned len);

#if defined (__cplusplus)
}
#endif

#endif /* Q_LEASE_H */

/* SHA1 not available (unoffical build.) */
