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
#ifndef NN_PLIST_H
#define NN_PLIST_H

#include "q_feature_check.h"
#include "q_xqos.h"

#include "c_base.h"
#include "kernelModule.h"

#if defined (__cplusplus)
extern "C" {
#endif

typedef struct nn_property {
  char *name;
  char *value;
} nn_property_t;

typedef struct nn_property_set {
  int length;
  nn_property_t *properties;
} nn_propertyset_t;

typedef struct nn_original_writer_info {
  nn_guid_t original_writer_guid;
  nn_sequence_number_t original_writer_sn;
  nn_xqos_t *original_writer_qos;
} original_writer_info_t;

#define PP_PROTOCOL_VERSION                     ((os_uint64)1 <<  0)
#define PP_VENDORID                             ((os_uint64)1 <<  1)
#define PP_UNICAST_LOCATOR                      ((os_uint64)1 <<  2)
#define PP_MULTICAST_LOCATOR                    ((os_uint64)1 <<  3)
#define PP_DEFAULT_UNICAST_LOCATOR              ((os_uint64)1 <<  4)
#define PP_DEFAULT_MULTICAST_LOCATOR            ((os_uint64)1 <<  5)
#define PP_METATRAFFIC_UNICAST_LOCATOR          ((os_uint64)1 <<  6)
#define PP_METATRAFFIC_MULTICAST_LOCATOR        ((os_uint64)1 <<  7)
#define PP_EXPECTS_INLINE_QOS                   ((os_uint64)1 <<  8)
#define PP_PARTICIPANT_MANUAL_LIVELINESS_COUNT  ((os_uint64)1 <<  9)
#define PP_PARTICIPANT_BUILTIN_ENDPOINTS        ((os_uint64)1 << 10)
#define PP_PARTICIPANT_LEASE_DURATION           ((os_uint64)1 << 11)
#define PP_CONTENT_FILTER_PROPERTY              ((os_uint64)1 << 12)
#define PP_PARTICIPANT_GUID                     ((os_uint64)1 << 13)
#define PP_PARTICIPANT_ENTITYID                 ((os_uint64)1 << 14)
#define PP_GROUP_GUID                           ((os_uint64)1 << 15)
#define PP_GROUP_ENTITYID                       ((os_uint64)1 << 16)
#define PP_BUILTIN_ENDPOINT_SET                 ((os_uint64)1 << 17)
#define PP_PROPERTIES                           ((os_uint64)1 << 18)
#define PP_TYPE_MAX_SIZE_SERIALIZED             ((os_uint64)1 << 19)
#define PP_ENTITY_NAME                          ((os_uint64)1 << 20)
#define PP_KEYHASH                              ((os_uint64)1 << 21)
#define PP_STATUSINFO                           ((os_uint64)1 << 22)
#define PP_ORIGINAL_WRITER_INFO                 ((os_uint64)1 << 23)
#define PP_ENDPOINT_GUID                        ((os_uint64)1 << 24)
#define PP_PRISMTECH_WRITER_INFO                ((os_uint64)1 << 25)
#define PP_PRISMTECH_PARTICIPANT_VERSION_INFO   ((os_uint64)1 << 26)
#define PP_PRISMTECH_NODE_NAME                  ((os_uint64)1 << 27)
#define PP_PRISMTECH_EXEC_NAME                  ((os_uint64)1 << 28)
#define PP_PRISMTECH_PROCESS_ID                 ((os_uint64)1 << 29)
#define PP_PRISMTECH_SERVICE_TYPE               ((os_uint64)1 << 30)
#define PP_PRISMTECH_WATCHDOG_SCHEDULING        ((os_uint64)1 << 31)
#define PP_PRISMTECH_LISTENER_SCHEDULING        ((os_uint64)1 << 32)
#define PP_PRISMTECH_BUILTIN_ENDPOINT_SET       ((os_uint64)1 << 33)
#define PP_PRISMTECH_TYPE_DESCRIPTION           ((os_uint64)1 << 34)
#define PP_PRISMTECH_ENDPOINT_GID               ((os_uint64)1 << 35)
#define PP_PRISMTECH_GROUP_GID                  ((os_uint64)1 << 36)
#define PP_COHERENT_SET                         ((os_uint64)1 << 37)
#define PP_PRISMTECH_EOTINFO                    ((os_uint64)1 << 38)
#define PP_RTI_TYPECODE                         ((os_uint64)1 << 40)
/* Set for unrecognized parameters that are in the reserved space or
   in our own vendor-specific space that have the
   PID_UNRECOGNIZED_INCOMPATIBLE_FLAG set (see DDSI 2.1 9.6.2.2.1) */
#define PP_INCOMPATIBLE                         ((os_uint64)1 << 63)

#define NN_PRISMTECH_PARTICIPANT_VERSION_INFO_FIXED_CDRSIZE (24)

#define NN_PRISMTECH_FL_KERNEL_SEQUENCE_NUMBER  (1u << 0)
#define NN_PRISMTECH_FL_DISCOVERY_INCLUDES_GID  (1u << 1)
#define NN_PRISMTECH_FL_PTBES_FIXED_0           (1u << 2)
#define NN_PRISMTECH_FL_DDSI2_PARTICIPANT_FLAG  (1u << 3)
#define NN_PRISMTECH_FL_PARTICIPANT_IS_DDSI2    (1u << 4)
#define NN_PRISMTECH_FL_MINIMAL_BES_MODE        (1u << 5)

/* For locators one could patch the received message data to create
   singly-linked lists (parameter header -> offset of next entry in
   list relative to current), allowing aliasing of the data. But that
   requires modifying the data. For string sequences the length does
   the same thing. */
struct nn_locators_one {
  struct nn_locators_one *next;
  nn_locator_t loc;
};

typedef struct nn_locators {
  int n;
  struct nn_locators_one *first;
  struct nn_locators_one *last;
} nn_locators_t;

typedef unsigned nn_ipv4address_t;

typedef unsigned nn_port_t;

typedef struct nn_keyhash {
  unsigned char value[16];
} nn_keyhash_t;

typedef struct nn_prismtech_writer_info_old
{
  os_uint32 transactionId;
  v_gid writerGID;
  v_gid writerInstanceGID;
} nn_prismtech_writer_info_old_t;

typedef struct nn_prismtech_writer_info
{
  os_uint32 transactionId;
  v_gid writerGID;
  v_gid writerInstanceGID;
  os_uint32 sequenceNumber;
} nn_prismtech_writer_info_t;


typedef struct nn_prismtech_participant_version_info
{
  unsigned version;
  unsigned flags;
  unsigned unused[3];
  char *internals;
} nn_prismtech_participant_version_info_t;

typedef struct nn_prismtech_eotgroup_tid {
  nn_entityid_t writer_entityid;
  os_uint32 transactionId;
} nn_prismtech_eotgroup_tid_t;

typedef struct nn_prismtech_eotinfo {
  os_uint32 transactionId;
  os_uint32 n;
  nn_prismtech_eotgroup_tid_t *tids;
} nn_prismtech_eotinfo_t;

typedef struct nn_plist {
  os_uint64 present;
  os_uint64 aliased;
  int unalias_needs_bswap;

  nn_xqos_t qos;

  nn_protocol_version_t protocol_version;
  nn_vendorid_t vendorid;
  nn_locators_t unicast_locators;
  nn_locators_t multicast_locators;
  nn_locators_t default_unicast_locators;
  nn_locators_t default_multicast_locators;
  nn_locators_t metatraffic_unicast_locators;
  nn_locators_t metatraffic_multicast_locators;

  unsigned char expects_inline_qos;
  nn_count_t participant_manual_liveliness_count;
  unsigned participant_builtin_endpoints;
  nn_duration_t participant_lease_duration;
  /* nn_content_filter_property_t content_filter_property; */
  nn_guid_t participant_guid;
  nn_guid_t endpoint_guid;
  nn_guid_t group_guid;
#if 0 /* reserved, rather than NIY */
  nn_entityid_t participant_entityid;
  nn_entityid_t group_entityid;
#endif
  unsigned builtin_endpoint_set;
  unsigned prismtech_builtin_endpoint_set;
  /* nn_propertyset_t properties; */
  /* int type_max_size_serialized; */
  char *entity_name;
  nn_keyhash_t keyhash;
  unsigned statusinfo;
  nn_prismtech_writer_info_t prismtech_writer_info;
  nn_prismtech_participant_version_info_t prismtech_participant_version_info;
  char *node_name;
  char *exec_name;
  unsigned char is_service;
  unsigned service_type;
  unsigned process_id;
  v_gid endpoint_gid;
  v_gid group_gid;
  char *type_description;
  nn_sequence_number_t coherent_set_seqno;
  nn_prismtech_eotinfo_t eotinfo;
} nn_plist_t;


/***/


typedef struct nn_plist_src {
  nn_protocol_version_t protocol_version;
  nn_vendorid_t vendorid;
  int encoding;
  unsigned char *buf;
  size_t bufsz;
} nn_plist_src_t;

void nn_plist_init_empty (nn_plist_t *dest);
void nn_plist_mergein_missing (nn_plist_t *a, const nn_plist_t *b);
void nn_plist_copy (nn_plist_t *dst, const nn_plist_t *src);
nn_plist_t *nn_plist_dup (const nn_plist_t *src);
int nn_plist_init_frommsg (nn_plist_t *dest, char **nextafterplist, os_uint64 pwanted, os_uint64 qwanted, const nn_plist_src_t *src);
void nn_plist_fini (nn_plist_t *ps);
void nn_plist_addtomsg (struct nn_xmsg *m, const nn_plist_t *ps, os_uint64 pwanted, os_uint64 qwanted);
int nn_plist_init_default_participant (nn_plist_t *plist);

int validate_history_qospolicy (const nn_history_qospolicy_t *q);
int validate_durability_qospolicy (const nn_durability_qospolicy_t *q);
int validate_resource_limits_qospolicy (const nn_resource_limits_qospolicy_t *q);
int validate_history_and_resource_limits (const nn_history_qospolicy_t *qh, const nn_resource_limits_qospolicy_t *qr);
int validate_durability_service_qospolicy (const nn_durability_service_qospolicy_t *q);
int validate_liveliness_qospolicy (const nn_liveliness_qospolicy_t *q);
int validate_destination_order_qospolicy (const nn_destination_order_qospolicy_t *q);
int validate_ownership_qospolicy (const nn_ownership_qospolicy_t *q);
int validate_ownership_strength_qospolicy (const nn_ownership_strength_qospolicy_t *q);
int validate_presentation_qospolicy (const nn_presentation_qospolicy_t *q);
int validate_transport_priority_qospolicy (const nn_transport_priority_qospolicy_t *q);
int validate_reader_data_lifecycle (const nn_reader_data_lifecycle_qospolicy_t *q);
int validate_duration (const nn_duration_t *d);


struct nn_rmsg;
struct nn_rsample_info;
struct nn_rdata;

unsigned char *nn_plist_quickscan (struct nn_rsample_info *dest, const struct nn_rmsg *rmsg, const nn_plist_src_t *src);
void nn_plist_extract_wrinfo (nn_prismtech_writer_info_t *wri, const struct nn_rsample_info *sampleinfo, const struct nn_rdata *rdata);

#if defined (__cplusplus)
}
#endif

#endif /* NN_PLIST_H */

/* SHA1 not available (unoffical build.) */
