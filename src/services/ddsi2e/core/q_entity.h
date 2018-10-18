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
#ifndef Q_ENTITY_H
#define Q_ENTITY_H

#include "os_defs.h"
#include "os_mutex.h"
#include "os_cond.h"
#include "os_socket.h"

#if LITE
#include "dds_types.h"
#else
#include "kernelModuleI.h"
#endif

#include "ut_avl.h"
#include "q_rtps.h"
#include "q_protocol.h"
#include "q_lat_estim.h"
#include "q_ephash.h"
#include "q_hbcontrol.h"
#include "q_feature_check.h"

#include "ddsi_tran.h"

#if defined (__cplusplus)
extern "C" {
#endif

struct xevent;
struct nn_reorder;
#if ! LITE
struct nn_groupset;
#endif
struct nn_defrag;
struct nn_dqueue;
struct addrset;
struct sertopic;
struct whc;
struct nn_xqos;
struct nn_plist;
struct v_gid_s;

struct proxy_group;
struct proxy_endpoint_common;

#if LITE
typedef struct status_cb_data
{
  uint32_t status;
  uint32_t extra;
  uint64_t handle;
  bool add;
}
status_cb_data_t;

typedef void (*status_cb_t) (void * entity, const status_cb_data_t * data);
#endif

struct prd_wr_match {
  ut_avlNode_t avlnode;
  nn_guid_t wr_guid;
};

struct rd_pwr_match {
  ut_avlNode_t avlnode;
  nn_guid_t pwr_guid;
#ifdef DDSI_INCLUDE_SSM
  nn_locator_t ssm_mc_loc;
  nn_locator_t ssm_src_loc;
#endif
};

struct wr_prd_match {
  ut_avlNode_t avlnode;
  nn_guid_t prd_guid; /* guid of the proxy reader */
  unsigned assumed_in_sync: 1; /* set to 1 upon receipt of ack not nack'ing msgs */
  unsigned has_replied_to_hb: 1; /* we must keep sending HBs until all readers have this set */
  unsigned all_have_replied_to_hb: 1; /* true iff 'has_replied_to_hb' for all readers in subtree */
  unsigned is_reliable: 1; /* true iff reliable proxy reader */
  os_int64 min_seq; /* smallest ack'd seq nr in subtree */
  os_int64 max_seq; /* sort-of highest ack'd seq nr in subtree (see augment function) */
  os_int64 seq; /* highest acknowledged seq nr */
  int num_reliable_readers_where_seq_equals_max;
  nn_guid_t arbitrary_unacked_reader;
  nn_count_t next_acknack; /* next acceptable acknack sequence number */
  nn_count_t next_nackfrag; /* next acceptable nackfrag sequence number */
  nn_etime_t t_acknack_accepted; /* (local) time an acknack was last accepted */
  struct nn_lat_estim hb_to_ack_latency;
  nn_wctime_t hb_to_ack_latency_tlastlog;
  os_uint32 non_responsive_count;
  os_uint32 rexmit_requests;
};

enum pwr_rd_match_syncstate {
  PRMSS_SYNC, /* in sync with proxy writer, has caught up with historical data */
  PRMSS_TLCATCHUP, /* in sync with proxy writer, still catching up on historical data */
  PRMSS_OUT_OF_SYNC /* not in sync with proxy writer */
};

struct pwr_rd_match {
  ut_avlNode_t avlnode;
  nn_guid_t rd_guid;
  nn_mtime_t tcreate;
  nn_count_t count; /* most recent acknack sequence number */
  nn_count_t next_heartbeat; /* next acceptable heartbeat (see also add_proxy_writer_to_reader) */
  nn_wctime_t hb_timestamp; /* time of most recent heartbeat that rescheduled the ack event */
  nn_etime_t t_heartbeat_accepted; /* (local) time a heartbeat was last accepted */
  nn_mtime_t t_last_nack; /* (local) time we last sent a NACK */  /* FIXME: probably elapsed time is better */
  os_int64 seq_last_nack; /* last seq for which we requested a retransmit */
  struct xevent *acknack_xevent; /* entry in xevent queue for sending acknacks */
  enum pwr_rd_match_syncstate in_sync; /* whether in sync with the proxy writer */
  union {
    struct {
      os_int64 end_of_tl_seq; /* when seq >= end_of_tl_seq, it's in sync, =0 when not tl */
      os_int64 end_of_out_of_sync_seq; /* when seq >= end_of_tl_seq, it's in sync, =0 when not tl */
      struct nn_reorder *reorder; /* can be done (mostly) per proxy writer, but that is harder; only when state=OUT_OF_SYNC */
    } not_in_sync;
  } u;
};

enum entity_kind {
  EK_PARTICIPANT,
  EK_PROXY_PARTICIPANT,
  EK_WRITER,
  EK_PROXY_WRITER,
  EK_READER,
  EK_PROXY_READER
};
#define EK_NKINDS ((int) EK_PROXY_READER + 1)

struct nn_rsample_info;
struct nn_rdata;

struct entity_common {
  struct ephash_chain_entry guid_hash_chain;
  enum entity_kind kind;
  nn_guid_t guid;
  char *name;
#if LITE
  uint64_t iid;
#endif
  os_mutex lock;
};

struct participant
{
  struct entity_common e;
  long long lease_duration;
  unsigned bes;
  unsigned prismtech_bes;
  unsigned is_ddsi2_pp: 1;
  nn_plist_t *plist;
  struct xevent *spdp_xevent;
  struct xevent *pmd_update_xevent;
  nn_locator_t m_locator;
  ddsi_tran_conn_t m_conn;
  unsigned next_entityid;
  os_int32 user_refc;
  os_int32 builtin_refc;
  os_mutex refc_lock;
  int builtins_deleted;
};

struct endpoint_common {
  struct participant *pp;
#if ! LITE
  struct ephash_chain_entry gid_hash_chain;
  v_gid gid;
  v_gid group_gid;
#endif
  nn_guid_t group_guid;
};

struct generic_endpoint {
  struct entity_common e;
  struct endpoint_common c;
};

enum writer_state {
  WRST_OPERATIONAL,
  WRST_LINGERING,
  WRST_DELETING
};

struct writer
{
  struct entity_common e;
  struct endpoint_common c;
#if LITE
  status_cb_t status_cb;
  void * status_cb_entity;
#endif
  os_cond throttle_cond;
  long long seq; /* last sequence number (transmitted seqs are 1 ... seq) */
  long long cs_seq; /* 1st seq in coherent set (or 0) */
  long long seq_xmit; /* last sequence number actually transmitted */
  nn_count_t hbcount; /* last hb seq number */
  nn_count_t hbfragcount; /* last hb frag seq number */
  os_uint32 last_kernel_seq; /* last v_message sequenceNumber processed */
  int throttling; /* non-zero when some thread is waiting for the WHC to shrink */
  struct hbcontrol hbcontrol;
  struct nn_xqos *xqos;
  enum writer_state state;
  unsigned reliable: 1;
  unsigned handle_as_transient_local: 1;
  unsigned aggressive_keep_last: 1;
  unsigned startup_mode: 1; /* causes data to be treated as T-L for a while */
  unsigned include_keyhash: 1;
  unsigned retransmitting: 1;
#ifdef DDSI_INCLUDE_SSM
  unsigned supports_ssm: 1;
  struct addrset *ssm_as;
#endif
  const struct sertopic * topic;
  struct addrset *as; /* set of addresses to publish to */
  struct addrset *as_group;
  struct xevent *heartbeat_xevent;
  long long lease_duration;
  struct whc *whc;
  os_uint32 whc_low, whc_high;
  nn_etime_t t_rexmit_end;
  nn_etime_t t_whc_high_upd;
  int num_reliable_readers;
  ut_avlTree_t readers;
#ifdef DDSI_INCLUDE_NETWORK_PARTITIONS
  os_uint32 partition_id;
#endif
  os_uint32 num_acks_received;
  os_uint32 num_nacks_received;
  os_uint32 throttle_count;
  os_uint32 throttle_tracing;
  os_uint32 rexmit_count;
  os_uint32 rexmit_lost_count;
  struct xeventq *evq;
};

struct reader
{
  struct entity_common e;
  struct endpoint_common c;
#if LITE
  status_cb_t status_cb;
  void * status_cb_entity;
  struct rhc * rhc;
#else
  struct nn_groupset *matching_groups;
#endif
  struct nn_xqos *xqos;
  unsigned reliable: 1;
  unsigned handle_as_transient_local: 1;
#ifdef DDSI_INCLUDE_SSM
  unsigned favours_ssm: 1;
#endif
  nn_count_t init_acknack_count;
#ifdef DDSI_INCLUDE_NETWORK_PARTITIONS
  struct addrset *as;
#endif
  const struct sertopic * topic;
  ut_avlTree_t writers;
};

struct proxy_participant
{
  struct entity_common e;
  os_uint32 refc;
  nn_vendorid_t vendor;
  unsigned bes;
  unsigned prismtech_bes;
  nn_guid_t privileged_pp_guid; /* if this PP depends on another PP for its SEDP writing */
  nn_plist_t *plist;
#if ! LITE
  v_gid gid;
#endif
  pa_voidp_t lease; /* struct lease *; owns_lease flag indicates whether the proxypp owns the lease */
  struct addrset *as_default;
  struct addrset *as_meta;
  struct proxy_endpoint_common *endpoints;
  ut_avlTree_t groups;
  unsigned kernel_sequence_numbers : 1;
  unsigned implicitly_created : 1;
  unsigned is_ddsi2_pp: 1;
  unsigned minimal_bes_mode: 1;
  unsigned lease_expired: 1;
  unsigned proxypp_have_spdp: 1;
  unsigned proxypp_have_cm: 1;
  unsigned owns_lease: 1;
};

/* Representing proxy subscriber & publishers as "groups": until DDSI2
   gets a reason to care about these other than for the generation of
   CM topics, there's little value in distinguishing between the two.
   In another way, they're secondly-class citizens, too: "real"
   entities are garbage collected and found using lock-free hash
   tables, but "groups" only live in the context of a proxy
   participant. */
struct proxy_group {
  ut_avlNode_t avlnode;
  nn_guid_t guid;
#if ! LITE
  v_gid gid;
#endif
  char *name;
  struct proxy_participant *proxypp;
  struct nn_xqos *xqos;
};

struct proxy_endpoint_common
{
  struct proxy_participant *proxypp;
  struct proxy_endpoint_common *next_ep;
  struct proxy_endpoint_common *prev_ep;
  struct nn_xqos *xqos;
  const struct sertopic * topic;
  struct addrset *as;
  nn_guid_t group_guid; /* 0:0:0:0 if not available */
  nn_vendorid_t vendor; /* cached from proxypp->vendor */
#if ! LITE
  v_gid gid; /* 0:0:0 for built-in endpoints */
  v_gid group_gid; /* 0:0:0 if not available */
#endif
};

struct proxy_writer {
  struct entity_common e;
  struct proxy_endpoint_common c;
  ut_avlTree_t readers;
#if ! LITE
  struct nn_groupset *groups;
  c_array v_message_qos;
  /* Transactions: current transaction id for the kernel and DDSI
     sequence number at which the coherent seq started. Both 0 if not
     currently in a transaction. The transaction id need not be the
     lower 32 bits of the sequence numbers, as we pass on the
     OpenSplice kernel sequence numbers unchanged. */
  c_ulong transaction_id;
  os_int64 cs_seq;
  os_int64 seq_offset;
#endif
  int n_reliable_readers;
  int n_readers_out_of_sync;
  long long last_seq; /* last known seq, not last delivered */
  os_uint32 last_fragnum; /* last known frag for last_seq, or ~0u if last_seq not partial */
  nn_count_t nackfragcount; /* last nackfrag seq number */
  pa_uint32_t next_deliv_seq_lowword; /* for generating acks; 32-bit so atomic reads on all supported platforms */
  unsigned last_fragnum_reset: 1; /* iff set, heartbeat advertising last_seq as highest seq resets last_fragnum */
  unsigned deliver_synchronously: 1;
  unsigned have_seen_heartbeat: 1;
  unsigned assert_pp_lease: 1;
#ifdef DDSI_INCLUDE_SSM
  unsigned supports_ssm: 1;
#endif
  struct nn_defrag *defrag;
  struct nn_reorder *reorder;
  struct nn_dqueue *dqueue;
  struct xeventq *evq;
#if LITE
  os_mutex rdary_lock;
  int n_readers;
  struct reader **rdary; /* for efficient delivery, null-pointer terminated */
  unsigned deleting: 1; /* set when being deleted and (about to be) removed from GUID hash */
#endif
};

struct proxy_reader {
  struct entity_common e;
  struct proxy_endpoint_common c;
  unsigned deleting: 1; /* set when being deleted */
  unsigned is_fict_trans_reader: 1; /* only true when this is certain */
  unsigned assert_pp_lease: 1;
#ifdef DDSI_INCLUDE_SSM
  unsigned favours_ssm: 1;
#endif
  ut_avlTree_t writers;
};

extern const ut_avlTreedef_t wr_readers_treedef;
extern const ut_avlTreedef_t rd_writers_treedef;
extern const ut_avlTreedef_t pwr_readers_treedef;
extern const ut_avlTreedef_t prd_writers_treedef;
extern const ut_avlTreedef_t deleted_participants_treedef;

#define DPG_LOCAL 1
#define DPG_REMOTE 2

int deleted_participants_admin_init (void);
void deleted_participants_admin_fini (void);
int is_deleted_participant_guid (const struct nn_guid *guid, unsigned for_what);

nn_entityid_t to_entityid (unsigned u);
int is_builtin_entityid (nn_entityid_t id, nn_vendorid_t vendorid);
int is_writer_entityid (nn_entityid_t id);
int is_reader_entityid (nn_entityid_t id);

int pp_allocate_entityid (nn_entityid_t *id, unsigned kind, struct participant *pp);

/* Interface for glue code between the OpenSplice kernel and the DDSI
   entities. These all return 0 iff successful. All GIDs supplied
   __MUST_BE_UNIQUE__. All hell may break loose if they aren't.

   All delete operations synchronously remove the entity being deleted
   from the various global hash tables on GUIDs. This ensures no new
   operations can be invoked by the glue code, discovery, protocol
   messages, &c.  The entity is then scheduled for garbage collection.

     There is one exception: a participant without built-in
     endpoints: that one synchronously reaches reference count zero
     and is then freed immediately.

     If new_writer() and/or new_reader() may be called in parallel to
     delete_participant(), trouble ensues. The current glue code
     performs all local discovery single-threaded, and can't ever get
     into that issue.

   A garbage collector thread is used to perform the actual freeing of
   an entity, but it never does so before all threads have made
   sufficient progress to guarantee they are not using that entity any
   longer, with the exception of use via internal pointers in the
   entity data structures.

   An example of the latter is that (proxy) endpoints have a pointer
   to the owning (proxy) participant, but the (proxy) participant is
   reference counted to make this safe.

   The case of a proxy writer is particularly complicated is it has to
   pass through a multiple-stage delay in the garbage collector before
   it may be freed: first there is the possibility of a parallel
   delete or protocol message, then there is still the possibility of
   data in a delivery queue.  This is dealt by requeueing garbage
   collection and sending bubbles through the delivery queue. */

/* Set this flag in new_participant to prevent the creation SPDP, SEDP
   and PMD readers for that participant.  It doesn't really need it,
   they all share the information anyway.  But you do need it once. */
#define RTPS_PF_NO_BUILTIN_READERS 1u
/* Set this flag to prevent the creation of SPDP, SEDP and PMD
   writers.  It will then rely on the "privileged participant", which
   must exist at the time of creation.  It creates a reference to that
   "privileged participant" to ensure it won't disappear too early. */
#define RTPS_PF_NO_BUILTIN_WRITERS 2u
/* Set this flag to mark the participant as the "privileged
   participant", there can only be one of these.  The privileged
   participant MUST have all builtin readers and writers. */
#define RTPS_PF_PRIVILEGED_PP 4u
  /* Set this flag to mark the participant as is_ddsi2_pp. */
#define RTPS_PF_IS_DDSI2_PP 8u

/* To create a DDSI participant given a GUID. May return ERR_OUT_OF_IDS
   (a.o.) */
int new_participant_guid (const nn_guid_t *ppguid, unsigned flags, const struct nn_plist *plist);

#if LITE
int new_participant (struct nn_guid *ppguid, unsigned flags, const struct nn_plist *plist);
#endif

/* To delete a DDSI participant: this only removes the participant
   from the hash tables and schedules the actual delete operation,
   which will start doing scary things once all but the DDSI built-in
   endpoints are gone.  It is acceptable to call delete_participant()
   before all its readers and writers have been deleted (which also
   fits nicely with model where the glue calls merely schedules
   garbage-collection). */
int delete_participant (const struct nn_guid *ppguid);

/* To obtain the builtin writer to be used for publishing SPDP, SEDP,
   PMD stuff for PP and its endpoints, given the entityid.  If PP has
   its own writer, use it; else use the privileged participant. */
struct writer *get_builtin_writer (const struct participant *pp, unsigned entityid);

/* To create a new DDSI writer or reader belonging to participant with
   GUID "ppguid". May return NULL if participant unknown or
   writer/reader already known. */

struct writer * new_writer
(
  struct nn_guid *wrguid,
  const struct nn_guid *group_guid,
  const struct nn_guid *ppguid,
  const struct sertopic *topic,
  const struct nn_xqos *xqos,
#if LITE
  status_cb_t status_cb,
  void * status_cb_arg
#else
  const struct v_gid_s *gid,
  const struct v_gid_s *group_gid,
  const char *endpoint_name
#endif
);

struct reader * new_reader
(
  struct nn_guid *rdguid,
  const struct nn_guid *group_guid,
  const struct nn_guid *ppguid,
  const struct sertopic *topic,
  const struct nn_xqos *xqos,
#if LITE
  struct rhc * rhc,
  status_cb_t status_cb,
  void * status_cb_arg
#else
  const struct v_gid_s *gid,
  const struct v_gid_s *group_gid,
  const char *endpoint_name
#endif
);

unsigned remove_acked_messages (struct writer *wr);
os_int64 writer_max_drop_seq (const struct writer *wr);
int writer_must_have_hb_scheduled (const struct writer *wr);
void writer_set_retransmitting (struct writer *wr);
void writer_clear_retransmitting (struct writer *wr);

int delete_writer (const struct nn_guid *guid);
int delete_writer_nolinger (const struct nn_guid *guid);
int delete_writer_nolinger_locked (struct writer *wr);

int delete_reader (const struct nn_guid *guid);

/* To create or delete a new proxy participant: "guid" MUST have the
   pre-defined participant entity id. Unlike delete_participant(),
   deleting a proxy participant will automatically delete all its
   readers & writers. Delete removes the participant from a hash table
   and schedules the actual deletion.

      -- XX what about proxy participants without built-in endpoints?
      XX --
*/

/* Set this custom flag when using nn_prismtech_writer_info_t iso nn_prismtech_writer_info_old_t */
#define CF_INC_KERNEL_SEQUENCE_NUMBERS         (1 << 0)
/* Set when this proxy participant is created implicitly and has to be deleted upon disappearance
   of its last endpoint.  FIXME: Currently there is a potential race with adding a new endpoint
   in parallel to deleting the last remaining one. The endpoint will then be created, added to the
   proxy participant and then both are deleted. With the current single-threaded discovery
   this can only happen when it is all triggered by lease expiry. */
#define CF_IMPLICITLY_CREATED_PROXYPP          (1 << 1)
/* Set when this proxy participant is a DDSI2 participant, to help Cloud figure out whom to send
   discovery data when used in conjunction with the networking bridge */
#define CF_PARTICIPANT_IS_DDSI2                (1 << 2)
/* Set when this proxy participant is not to be announced on the built-in topics yet */
#define CF_PROXYPP_NO_SPDP                     (1 << 3)

void new_proxy_participant (const struct nn_guid *guid, unsigned bes, unsigned prismtech_bes, const struct nn_guid *privileged_pp_guid, struct addrset *as_default, struct addrset *as_meta, const struct nn_plist *plist, os_int64 tlease_dur, nn_vendorid_t vendor, unsigned custom_flags, nn_wctime_t timestamp);
int delete_proxy_participant_by_guid (const struct nn_guid * guid, nn_wctime_t timestamp, int isimplicit);

enum update_proxy_participant_source {
  UPD_PROXYPP_SPDP,
  UPD_PROXYPP_CM
};

int update_proxy_participant_plist_locked (struct proxy_participant *proxypp, const struct nn_plist *datap, enum update_proxy_participant_source source, nn_wctime_t timestamp);
int update_proxy_participant_plist (struct proxy_participant *proxypp, const struct nn_plist *datap, enum update_proxy_participant_source source, nn_wctime_t timestamp);
void proxy_participant_reassign_lease (struct proxy_participant *proxypp, struct lease *newlease);

void purge_proxy_participants (const nn_locator_t *loc, c_bool delete_from_as_disc);

/* To create a new proxy writer or reader; the proxy participant is
   determined from the GUID and must exist. */
int new_proxy_writer (const struct nn_guid *ppguid, const struct nn_guid *guid, struct addrset *as, const struct nn_plist *plist, struct nn_dqueue *dqueue, struct xeventq *evq, nn_wctime_t timestamp);
int new_proxy_reader (const struct nn_guid *ppguid, const struct nn_guid *guid, struct addrset *as, const struct nn_plist *plist, nn_wctime_t timestamp
#ifdef DDSI_INCLUDE_SSM
                      , int favours_ssm
#endif
                      );

/* To delete a proxy writer or reader; these synchronously hide it
   from the outside world, preventing it from being matched to a
   reader or writer. Actual deletion is scheduled in the future, when
   no outstanding references may still exist (determined by checking
   thread progress, &c.). */
int delete_proxy_writer (const struct nn_guid *guid, nn_wctime_t timestamp, int isimplicit);
int delete_proxy_reader (const struct nn_guid *guid, nn_wctime_t timestamp, int isimplicit);

void update_proxy_reader (struct proxy_reader * prd, struct addrset *as);
void update_proxy_writer (struct proxy_writer * pwr, struct addrset *as);

int new_proxy_group (const struct nn_guid *guid, const struct v_gid_s *gid, const char *name, const struct nn_xqos *xqos, nn_wctime_t timestamp);
void delete_proxy_group (const struct nn_guid *guid, nn_wctime_t timestamp, int isimplicit);

void writer_exit_startup_mode (struct writer *wr);

#if ! LITE
/* To update readers/proxy writers with new groups when they are created by the kernel */
void add_group_to_readers_and_proxy_writers (const struct sertopic *topic, const char *partition, v_group group);
/* to signal historical data is complete, pwr may be NULL */
void notify_wait_for_historical_data (struct proxy_writer *pwr, const nn_guid_t *rd_guid);
#endif

#if defined (__cplusplus)
}
#endif

#endif /* Q_ENTITY_H */
