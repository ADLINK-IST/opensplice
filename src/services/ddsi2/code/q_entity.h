#ifndef Q_ENTITY_H
#define Q_ENTITY_H

#include "os_defs.h"
#include "os_mutex.h"
#include "os_cond.h"
#include "os_socket.h"

#include "kernelModule.h"

#include "ut_avl.h"
#include "q_rtps.h"
#include "q_xqos.h"
#include "q_protocol.h"
#include "q_lat_estim.h"
#include "q_ephash.h"
#include "q_hbcontrol.h"

#include "ddsi_tran.h"

#if defined (__cplusplus)
extern "C" {
#endif

struct xevent;
struct nn_reorder;
struct nn_groupset;
struct nn_defrag;
struct nn_dqueue;
struct topic;
struct addrset;
struct whc;
struct lease;

struct proxy_endpoint_common;

/* FIXME: eventually will need a return value so reaer can reject data
   on hitting a resource limit, which then of course needs to be
   handled by the receiver state machine as well */
typedef void (*deliver_cb_t) (const nn_guid_t *rdguid, void *arg);

struct prd_wr_match {
  ut_avlNode_t avlnode;
  nn_guid_t wr_guid;
};

struct rd_pwr_match {
  ut_avlNode_t avlnode;
  nn_guid_t pwr_guid;
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
  os_int64 t_acknack_accepted; /* (local) time an acknack was last accepted */
  struct nn_lat_estim hb_to_ack_latency;
  os_int64 hb_to_ack_latency_tlastlog;
};

struct pwr_rd_match {
  ut_avlNode_t avlnode;
  nn_guid_t rd_guid;
  os_int64 tcreate;
  nn_count_t count; /* most recent acknack sequence number */
  nn_count_t next_heartbeat; /* next acceptable heartbeat (see also add_proxy_writer_to_reader) */
  os_int64 hb_timestamp; /* time of most recent heartbeat that rescheduled the ack event */
  os_int64 t_heartbeat_accepted; /* (local) time a heartbeat was last accepted */
  struct xevent *acknack_xevent; /* entry in xevent queue for sending acknacks */
  int in_sync; /* whether in sync with the proxy writer */
  union {
    struct {
      os_int64 end_of_tl_seq; /* when seq >= end_of_tl_seq, it's in sync, =0 when not tl */
      struct nn_reorder *reorder; /* can be done (mostly) per proxy writer, but that is harder */
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
  os_mutex lock;
};

struct participant 
{
  struct entity_common e;
  long long lease_duration;
  unsigned bes;
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
  struct ephash_chain_entry gid_hash_chain;
  v_gid gid;
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

struct writer {
  struct entity_common e;
  struct endpoint_common c;
  os_cond throttle_cond;
  long long seq; /* last sequence number (transmitted seqs are 1 ... seq) */
  long long seq_xmit; /* last sequence number actually transmitted */
  nn_count_t hbcount; /* last hb seq number */
  nn_count_t hbfragcount; /* last hb frag seq number */
  os_uint32 last_kernel_seq; /* last v_message sequenceNumber processed */
  int throttling; /* non-zero when some thread is waiting for the WHC to shrink */
  struct hbcontrol hbcontrol;
  nn_xqos_t *xqos;
  enum writer_state state;
  unsigned reliable: 1;
  unsigned handle_as_transient_local: 1;
  unsigned aggressive_keep_last1: 1;
  unsigned startup_mode: 1; /* causes data to be treated as T-L for a while */
  unsigned include_keyhash: 1;
  const struct topic *topic;
  struct addrset *as;
  struct xevent *heartbeat_xevent;
  long long lease_duration;
  long long min_lease_duration;
  struct whc *whc;
  int num_reliable_readers;
  ut_avlTree_t readers;
  struct xeventq *evq;
};

struct reader {
  struct entity_common e;
  struct endpoint_common c;
  nn_xqos_t *xqos;
  unsigned reliable: 1;
  unsigned handle_as_transient_local: 1;
  nn_count_t init_acknack_count;
  const struct topic *topic;
  struct nn_groupset *matching_groups;
  ut_avlTree_t writers;
};

struct proxy_participant
{
  struct entity_common e;
  os_uint32 refc;
  nn_vendorid_t vendor;
  unsigned bes;
  nn_guid_t privileged_pp_guid;
  struct lease *lease;
  struct addrset *as_default;
  struct addrset *as_meta;
  struct proxy_endpoint_common *endpoints;
  unsigned kernel_sequence_numbers : 1;
};

struct proxy_endpoint_common {
  struct proxy_participant *proxypp;
  struct proxy_endpoint_common *next_ep;
  struct proxy_endpoint_common *prev_ep;
  nn_xqos_t *xqos;
  const struct topic *topic;
  struct addrset *as;
};

struct proxy_writer {
  struct entity_common e;
  struct proxy_endpoint_common c;
  ut_avlTree_t readers;
  struct nn_groupset *groups;
  c_array v_message_qos;
  int n_reliable_readers;
  long long last_seq; /* last known seq, not last delivered */
  os_uint32 last_fragnum; /* last known frag for last_seq, or ~0u if last_seq not partial */
  nn_count_t nackfragcount; /* last nackfrag seq number */
  os_uint32 next_deliv_seq_lowword; /* for generating acks; 32-bit so atomic reads on all supported platforms */
  unsigned deliver_synchronously: 1;
  unsigned have_seen_heartbeat: 1;
  struct nn_defrag *defrag;
  struct nn_reorder *reorder;
  struct nn_dqueue *dqueue;
  struct xeventq *evq;
};

struct proxy_reader {
  struct entity_common e;
  struct proxy_endpoint_common c;
  unsigned deleting: 1; /* set when being deleted */
  ut_avlTree_t writers;
};

extern const ut_avlTreedef_t wr_readers_treedef;
extern const ut_avlTreedef_t rd_writers_treedef;
extern const ut_avlTreedef_t pwr_readers_treedef;
extern const ut_avlTreedef_t prd_writers_treedef;
extern const ut_avlTreedef_t deleted_participants_treedef;

int deleted_participants_admin_init (void);
void deleted_participants_admin_fini (void);
int is_deleted_participant_guid (const struct nn_guid *guid);

nn_entityid_t to_entityid (unsigned u);
int is_builtin_entityid (nn_entityid_t id);
int is_writer_entityid (nn_entityid_t id);
int is_reader_entityid (nn_entityid_t id);

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
#define RTPS_PF_NO_BUILTIN_READERS 1
/* Set this flag to prevent the creation of SPDP, SEDP and PMD
   writers.  It will then rely on the "privileged participant", which
   must exist at the time of creation.  It creates a reference to that
   "privileged participant" to ensure it won't disappear too early. */
#define RTPS_PF_NO_BUILTIN_WRITERS 2
/* Set this flag to mark the participant as the "privileged
   participant", there can only be one of these.  The privileged
   participant MUST have all builtin readers and writers. */
#define RTPS_PF_PRIVILEGED_PP 4

/* To create a DDSI participant given a GUID. May return ERR_OUT_OF_IDS
   (a.o.) */
int new_participant_guid (const nn_guid_t *ppguid, unsigned flags);


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
   GUID "ppguid". May return ERR_UNKNOWN_ENTITY (participant unknown)
   and ERR_ENTITY_EXISTS (writer/reader already known). */
int new_writer (struct nn_guid *wrguid, const struct nn_guid *ppguid, const struct topic *topic, const struct nn_xqos *xqos);
int new_reader (struct nn_guid *rdguid, const struct nn_guid *ppguid, const struct topic *topic, const struct nn_xqos *xqos, deliver_cb_t deliver_cb, void *deliver_cbarg);

int remove_acked_messages (struct writer *wr);
os_int64 writer_max_drop_seq (const struct writer *wr);
int writer_number_of_unacked_samples (const struct writer *wr);
int writer_must_have_hb_scheduled (const struct writer *wr);

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

void new_proxy_participant (const struct nn_guid *guid, unsigned bes, const struct nn_guid *privileged_pp_guid, struct addrset *as_default, struct addrset *as_meta, os_int64 tlease_dur, nn_vendorid_t vendor, unsigned custom_flags);
void delete_proxy_participant (struct proxy_participant * ppt);
int delete_proxy_participant_by_guid (const struct nn_guid * guid);

void purge_proxy_participants (os_sockaddr_storage * addr, os_uint32 port);

/* To create a new proxy writer or reader; the proxy participant is
   determined from the GUID and must exist. */
int new_proxy_writer (const struct nn_guid *guid, struct addrset *as, nn_xqos_t *xqos, struct nn_dqueue *dqueue, struct xeventq *evq);
int new_proxy_reader (const struct nn_guid *guid, struct addrset *as, nn_xqos_t *xqos);

/* To delete a proxy writer or reader; these synchronously hide it
   from the outside world, preventing it from being matched to a
   reader or writer. Actual deletion is scheduled in the future, when
   no outstanding references may still exist (determined by checking
   thread progress, &c.). */
int delete_proxy_writer (const struct nn_guid *guid);
int delete_proxy_reader (const struct nn_guid *guid);

void writer_exit_startup_mode (struct writer *wr);

#if defined (__cplusplus)
}
#endif

#endif /* Q_ENTITY_H */

/* SHA1 not available (unoffical build.) */
