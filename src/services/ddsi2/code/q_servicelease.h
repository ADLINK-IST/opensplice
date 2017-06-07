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

/* SHA1 not available (unoffical build.) */
