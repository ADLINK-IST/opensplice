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
#ifndef NN_MISC_H
#define NN_MISC_H

#include "q_protocol.h"

#if defined (__cplusplus)
extern "C" {
#endif

struct v_gid_s;
struct nn_guid;

int vendor_is_lite (nn_vendorid_t vendor);
int vendor_is_opensplice (nn_vendorid_t vid);
int vendor_is_rti (nn_vendorid_t vendor);
int vendor_is_twinoaks (nn_vendorid_t vendor);
int vendor_is_prismtech (nn_vendorid_t vendor);
int vendor_is_cloud (nn_vendorid_t vendor);
int is_own_vendor (nn_vendorid_t vendor);
unsigned char normalize_data_datafrag_flags (const SubmessageHeader_t *smhdr, int datafrag_as_data);
#if !LITE
int version_info_is_6_4_1 (const char *internals);
#endif

os_int64 fromSN (const nn_sequence_number_t sn);
nn_sequence_number_t toSN (os_int64);

#ifdef DDSI_INCLUDE_NETWORK_PARTITIONS
int WildcardOverlap(char * p1, char * p2);
#endif

int ddsi2_patmatch (const char *pat, const char *str);

#if !LITE
void nn_guid_to_ospl_gid (struct v_gid_s *gid, const struct nn_guid *guid, int guid_has_systemid);
int gid_is_fake (const struct v_gid_s *gid);
#endif

#if LITE
os_uint32 crc32_calc (const void *buf, os_uint32 length);
#endif

#if defined (__cplusplus)
}
#endif

#endif /* NN_MISC_H */
