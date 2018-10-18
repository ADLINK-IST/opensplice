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
#ifndef NN_SERVICELEASE_H
#define NN_SERVICELEASE_H

#if defined (__cplusplus)
extern "C" {
#endif

struct nn_servicelease;

struct nn_servicelease *nn_servicelease_new (void (*renew_cb) (void *arg), void *renew_arg);
int nn_servicelease_start_renewing (struct nn_servicelease *sl);
void nn_servicelease_free (struct nn_servicelease *sl);
void nn_servicelease_statechange_barrier (struct nn_servicelease *sl);

#if defined (__cplusplus)
}
#endif

#endif /* NN_SERVICELEASE_H */
