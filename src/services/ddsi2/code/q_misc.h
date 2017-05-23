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
#ifndef NN_MISC_H
#define NN_MISC_H

#include "q_protocol.h"

#if defined (__cplusplus)
extern "C" {
#endif

struct v_gid_s;
struct nn_guid;

int vendor_is_opensplice (nn_vendorid_t vid);
int vendor_is_rti (nn_vendorid_t vendor);
int vendor_is_twinoaks (nn_vendorid_t vendor);
int vendor_is_prismtech (nn_vendorid_t vendor);
int vendor_is_cloud (nn_vendorid_t vendor);
int is_own_vendor (nn_vendorid_t vendor);
unsigned char normalize_data_datafrag_flags (const SubmessageHeader_t *smhdr, int datafrag_as_data);
int version_info_is_6_4_1 (const char *internals);

os_int64 fromSN (const nn_sequence_number_t sn);
nn_sequence_number_t toSN (os_int64);


int ddsi2_patmatch (const char *pat, const char *str);

void nn_guid_to_ospl_gid (struct v_gid_s *gid, const struct nn_guid *guid, int guid_has_systemid);
int gid_is_fake (const struct v_gid_s *gid);


#if defined (__cplusplus)
}
#endif

#endif /* NN_MISC_H */

/* SHA1 not available (unoffical build.) */
