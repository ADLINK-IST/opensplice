#ifndef NN_PLIST_H
#define NN_PLIST_H

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

#define PP_PROTOCOL_VERSION                     (1u <<  0)
#define PP_VENDORID                             (1u <<  1)
#define PP_UNICAST_LOCATOR                      (1u <<  2)
#define PP_MULTICAST_LOCATOR                    (1u <<  3)
#define PP_DEFAULT_UNICAST_LOCATOR              (1u <<  4)
#define PP_DEFAULT_MULTICAST_LOCATOR            (1u <<  5)
#define PP_METATRAFFIC_UNICAST_LOCATOR          (1u <<  6)
#define PP_METATRAFFIC_MULTICAST_LOCATOR        (1u <<  7)
#define PP_EXPECTS_INLINE_QOS                   (1u <<  8)
#define PP_PARTICIPANT_MANUAL_LIVELINESS_COUNT  (1u <<  9)
#define PP_PARTICIPANT_BUILTIN_ENDPOINTS        (1u << 10)
#define PP_PARTICIPANT_LEASE_DURATION           (1u << 11)
#define PP_CONTENT_FILTER_PROPERTY              (1u << 12)
#define PP_PARTICIPANT_GUID                     (1u << 13)
#define PP_PARTICIPANT_ENTITYID                 (1u << 14)
#define PP_GROUP_GUID                           (1u << 15)
#define PP_GROUP_ENTITYID                       (1u << 16)
#define PP_BUILTIN_ENDPOINT_SET                 (1u << 17)
#define PP_PROPERTIES                           (1u << 18)
#define PP_TYPE_MAX_SIZE_SERIALIZED             (1u << 19)
#define PP_ENTITY_NAME                          (1u << 20)
#define PP_KEYHASH                              (1u << 21)
#define PP_STATUSINFO                           (1u << 22)
#define PP_ORIGINAL_WRITER_INFO                 (1u << 23)
#define PP_ENDPOINT_GUID                        (1u << 24)
#define PP_PRISMTECH_WRITER_INFO                (1u << 25)
#define PP_PRISMTECH_PARTICIPANT_VERSION_INFO   (1u << 26)
/* Set for unrecognized parameters that are in the reserved space or
   in our own vendor-specific space that have the
   PID_UNRECOGNIZED_INCOMPATIBLE_FLAG set (see DDSI 2.1 9.6.2.2.1) */
#define PP_INCOMPATIBLE                         (1u << 31)

#define NN_PRISMTECH_PARTICIPANT_VERSION_INFO_FIXED_CDRSIZE (24)

#define NN_PRISMTECH_FL_KERNEL_SEQUENCE_NUMBER  (1u << 0)

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
  char value[16];
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

typedef struct nn_plist {
  unsigned present;
  unsigned aliased;
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

  char expects_inline_qos;
  nn_count_t participant_manual_liveliness_count;
  unsigned participant_builtin_endpoints;
  nn_duration_t participant_lease_duration;
  /* nn_content_filter_property_t content_filter_property; */
  nn_guid_t participant_guid;
  nn_guid_t endpoint_guid;
#if 0 /* reserved, rather than NIY */
  nn_entityid_t participant_entityid;
  nn_guid_t group_guid;
  nn_entityid_t group_entityid;
#endif
  unsigned builtin_endpoint_set;
  /* nn_propertyset_t properties; */
  /* int type_max_size_serialized; */
  char *entity_name;
  nn_keyhash_t keyhash;
  unsigned statusinfo;
  /* nn_original_writer_info_t original_writer_info; */
  nn_prismtech_writer_info_t prismtech_writer_info;
  nn_prismtech_participant_version_info_t prismtech_participant_version_info;
} nn_plist_t;


/***/


typedef struct nn_plist_src {
  nn_protocol_version_t protocol_version;
  nn_vendorid_t vendorid;
  int encoding;
  char *buf;
  int bufsz;
} nn_plist_src_t;

void nn_plist_init_empty (nn_plist_t *dest);
int nn_plist_init_frommsg (nn_plist_t *dest, char **nextafterplist, unsigned pwanted, unsigned qwanted, const nn_plist_src_t *src);
int nn_plist_unalias (nn_plist_t *ps);
void nn_plist_fini (nn_plist_t *ps);
int nn_plist_addtomsg (struct nn_xmsg *m, const nn_plist_t *ps, unsigned pwanted, unsigned qwanted);

struct nn_rmsg;
struct nn_rsample_info;
struct nn_rdata;

char *nn_plist_quickscan (struct nn_rsample_info *dest, const struct nn_rmsg *rmsg, const nn_plist_src_t *src);
void nn_plist_extract_wrinfo (nn_prismtech_writer_info_t *wri, const struct nn_rsample_info *sampleinfo, const struct nn_rdata *rdata);
  
#if defined (__cplusplus)
}
#endif

#endif /* NN_PLIST_H */

/* SHA1 not available (unoffical build.) */
