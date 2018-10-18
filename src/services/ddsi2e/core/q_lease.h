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
struct lease *lease_new (nn_etime_t texpire, os_int64 tdur, struct entity_common *e);
void lease_register (struct lease *l);
void lease_free (struct lease *l);
void lease_renew (struct lease *l, nn_etime_t tnow);
void lease_set_expiry (struct lease *l, nn_etime_t when);
void check_and_handle_lease_expiration (struct thread_state1 *self, nn_etime_t tnow);

void handle_PMD (const struct receiver_state *rst, nn_wctime_t timestamp, unsigned statusinfo, const void *vdata, unsigned len);

#if defined (__cplusplus)
}
#endif

#endif /* Q_LEASE_H */
