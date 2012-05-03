/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef NN_RTPS_PRIVATE_H
#define NN_RTPS_PRIVATE_H

#include "os_socket.h" /* for in_addr */

#include "nn_xqos.h"
#include "nn_plist.h" /* for nn_prismtech_writer_info */
#include "nn_time.h"
#include "nn_xmsg.h"
#include "nn_lat_estim.h"
#include "nn_osplser.h"

struct addrset;
struct xeventq;
struct xevent;

enum payload_kind {
  PK_NONE,
  PK_RAW_ALIASED,
  PK_RAW_MALLOCED,
  PK_V_MESSAGE
};

struct proxy_participant {
  STRUCT_AVLNODE (proxy_participant_avlnode, struct proxy_participant *) avlnode;
  nn_guid_t guid;
  int refc;
  nn_vendorid_t vendor;
  unsigned bes;
  long long tlease_dur;
  long long tlease_end;
  long long min_tlease_end; /* in subtree */
  nn_guid_t guid_min_tlease_end; /* in subtree */
  struct addrset *as_default;
  struct addrset *as_meta;
};

struct proxy_endpoint_common {
  STRUCT_AVLNODE (proxy_endpoint_avlnode, struct proxy_endpoint_common *) avlnode;
  struct proxy_participant *pp;
  nn_guid_t guid;
  nn_xqos_t *xqos;
  long long tlease_end;
  long long tlease_dur;
  long long min_tlease_end; /* in subtree */
  nn_guid_t guid_min_tlease_end; /* in subtree */
  struct addrset *as;
};

struct proxy_reader {
  struct proxy_endpoint_common c;
  STRUCT_AVLTREE (proxy_endpoint_whc_avltree, struct whc_readers_node *) writers;
};

struct proxy_writer {
  struct proxy_endpoint_common c;
  STRUCT_AVLTREE (proxy_endpoint_rhc_avltree, struct rhc_writers_node *) readers;
  struct nn_groupset *groups;
  c_array v_message_qos;
  int n_reliable_readers;
  long long last_seq; /* last known seq, not last delivered */
  nn_count_t nackfragcount; /* last nackfrag seq number */
  int deliver_synchronously;
  struct nn_defrag *defrag;
  struct nn_reorder *reorder;
};

struct whc_node {
  STRUCT_AVLNODE (whc_node_avlnode_seq, struct whc_node *) avlnode_seq;
  STRUCT_AVLNODE (whc_node_avlnode_tlidx, struct whc_node *) avlnode_tlidx;
  long long seq;
  long long minseq, maxseq;
  int in_tlidx;
  serdata_t serdata;
};

struct whc_readers_node {
  STRUCT_AVLNODE (whc_readers_node_avlnode, struct whc_readers_node *) avlnode;
  STRUCT_AVLNODE (whc_readers_node_proxyavltree, struct whc_readers_node *) proxyavlnode;
  struct writer *writer; /* for cleaning up given just the whc_readers_node */
  struct proxy_reader *proxy_reader;
  unsigned pure_ack_received: 1; /* set to 1 upon receipt of ack not nack'ing msgs */
  long long min_seq; /* smallest ack'd seq nr in subtree */
  nn_guid_t reader_guid; /* key for avlnode (guid of proxy) */
  nn_guid_t writer_guid; /* key for proxyavlnode (guid of local writer) */
  long long seq; /* highest acknowledged seq nr */
  nn_count_t last_acknack;
  nn_count_t last_nackfrag;
  struct nn_lat_estim hb_to_ack_latency;
  os_int64 hb_to_ack_latency_tlastlog;
};

struct writer {
  STRUCT_AVLNODE (writer_avlnode, struct writer *) avlnode; /* for per-participant set of writers */
  struct participant *participant;
  nn_guid_t guid;
  long long seq; /* last sequence number (transmitted seqs are 1 ... seq) */
  nn_count_t hbcount; /* last hb seq number */
  nn_count_t hbfragcount; /* last hb frag seq number */
  long long t_of_last_heartbeat;
  long long tcreate;
  int throttling;
  nn_xqos_t *xqos;
  unsigned deleting: 1; /* set upon entry to delete_writer() - intent */
  unsigned dying: 1; /* set upon entry to free_writer() - really dying */
  unsigned hb_before_next_msg: 1; /* to avoid sending invalid HBs in response to init rti ack */
  unsigned reliable: 1;
  unsigned handle_as_transient_local: 1;
  unsigned startup_mode: 1; /* causes data to be treated as T-L for a while */
  unsigned with_key: 1;
  topic_t topic;
  struct addrset *as;
  struct xevent *heartbeat_xevent;
  long long lease_duration;
  long long min_lease_duration;
  int whc_seq_size;
  int whc_tlidx_size;
  STRUCT_AVLTREE (whc_avltree_seq, struct whc_node *) whc_seq; /* reliable|transient_local */
  STRUCT_AVLTREE (whc_avltree_tlidx, struct whc_node *) whc_tlidx; /* transient_local */
  STRUCT_AVLTREE (whc_readers_avltree, struct whc_readers_node *) readers;
};

struct rhc_writers_node {
  STRUCT_AVLNODE (rhc_writers_node_avlnode, struct rhc_writers_node *) avlnode;
  STRUCT_AVLNODE (rhc_writers_node_proxyavltree, struct whc_readers_node *) proxyavlnode;
  struct reader *reader; /* for cleaning up given just the rhc_writers_node */
  struct proxy_writer *proxy_writer;
  nn_guid_t writer_guid; /* key for avlnode (guid of proxy) */
  nn_guid_t reader_guid; /* key for proxyavlnode (guid of local reader) */
  nn_count_t count; /* last ack sequence number */
  nn_count_t last_heartbeat; /* last heartbeat seen (see also add_proxy_writer_to_reader) */
  os_int64 hb_timestamp; /* time of most recent heartbeat that rescheduled the ack event */
  struct xevent *acknack_xevent; /* entry in xevent queue for sending acknacks */
  int in_sync; /* whether in sync with the proxy writer */
  union {
    struct {
      long long end_of_tl_seq; /* when seq >= end_of_tl_seq, it's in sync, =0 when not tl */
      struct nn_reorder *reorder; /* can be done (mostly) per proxy writer, but that is harder */
    } not_in_sync;
  } u;
};

struct reader {
  STRUCT_AVLNODE (reader_avlnode, struct reader *) avlnode; /* for per-participant set of readers */
  struct participant *participant;
  nn_guid_t guid;
  long long tcreate;
  long long ndelivered;
  nn_xqos_t *xqos;
  unsigned dying: 1;
  unsigned reliable: 1;
  unsigned handle_as_transient_local: 1;
  topic_t topic;
  struct nn_groupset *matching_groups;
  long long lease_duration;
  long long min_lease_duration;
  STRUCT_AVLTREE (rhc_writers_avltree, struct rhc_writers_node *) writers;
};

struct participant {
  STRUCT_AVLNODE (participant_avlnode, struct participant *) avlnode;
  nn_guid_t guid;
  STRUCT_AVLTREE (writer_avltree, struct writer *) writers;
  STRUCT_AVLTREE (reader_avltree, struct reader *) readers;
  long long lease_duration;
  unsigned dying: 1;
  unsigned bes;
  struct writer *spdp_pp_writer, *sedp_reader_writer, *sedp_writer_writer;
  struct reader *spdp_pp_reader, *sedp_reader_reader, *sedp_writer_reader;
  struct writer *participant_message_writer;
  struct reader *participant_message_reader;
  struct xevent *spdp_xevent;
  struct xevent *pmd_update_xevent;
  nn_locator_t sockloc;
  os_socket sock;
  int participant_set_index;
};

struct receiver_state {
  nn_guid_prefix_t source_guid_prefix;    /* 12 */
  nn_guid_prefix_t dest_guid_prefix;      /* 12 */
  struct addrset *reply_locators;         /* 4/8 */
  nn_ddsi_time_t timestamp;               /* 8 */
  nn_vendorid_t vendor;                   /* 2 */
  nn_protocol_version_t protocol_version; /* 2 => 40/44 */
};

#define MAX_INTERFACES 32
struct nn_interface {
  struct in_addr addr;
  struct in_addr netmask;
  unsigned mc_capable: 1;
  unsigned point_to_point: 1;
};

/* number of up, non-loopback, IPv4 interfaces */
extern int n_interfaces;
extern struct nn_interface interfaces[MAX_INTERFACES];

extern os_mutex lock;
extern os_cond evcond;
extern os_cond throttle_cond;

extern STRUCT_AVLTREE (proxypptree, struct proxy_participant *) proxypptree;
extern STRUCT_AVLTREE (proxyrdtree, struct proxy_reader *) proxyrdtree;
extern STRUCT_AVLTREE (proxywrtree, struct proxy_writer *) proxywrtree;
extern STRUCT_AVLTREE (participant_avltree, struct participant *) pptree;

extern serstatepool_t serpool;
struct xeventq *xevents;

extern struct in_addr ownip, mcip;
extern nn_locator_t loc_meta_mc, loc_meta_uc, loc_default_mc, loc_default_uc;
extern nn_locator_udpv4_t udpv4_meta_mc, udpv4_meta_uc, udpv4_default_mc, udpv4_default_uc;

extern struct addrset *as_disc_init;
extern struct addrset *as_disc;

extern struct nn_xmsgpool *xmsgpool;

extern nn_xqos_t default_xqos_rd;
extern nn_xqos_t default_xqos_wr;
extern nn_xqos_t spdp_endpoint_xqos;
extern nn_xqos_t builtin_endpoint_xqos_rd;
extern nn_xqos_t builtin_endpoint_xqos_wr;

extern int use_mcast_flag;
extern int use_mcast_loopback;
extern int ignore_own_vendor;
extern int aggressive_keep_last1_whc_flag;
extern int conservative_builtin_reader_startup_flag;
extern int noqueue_heartbeat_messages_flag;
extern int meas_hb_to_ack_latency_flag;
extern int unicast_response_to_spdp_messages_flag;

int fill_v_message_qos (const struct proxy_writer *pwr, v_message vmsg, const struct nn_prismtech_writer_info *wri, os_int64 tstamp, os_int64 seq, unsigned statusinfo, int have_data);
c_array new_v_message_qos (const nn_xqos_t *xqos, c_collectionType qostype);

struct nn_xmsg *make_SPDP_message (struct participant *pp);
struct nn_xmsg *new_data_msg (ptrdiff_t *doff, struct writer *wr, os_int64 tstamp, unsigned flags);
int nn_loc_to_loc_udpv4 (nn_locator_udpv4_t *dst, const nn_locator_t *src);
int get_udpv4_locator (nn_locator_udpv4_t *loc, nn_locators_t *locs);

int vendor_is_rti (nn_vendorid_t vendor);
int vendor_is_twinoaks (nn_vendorid_t vendor);
int vendor_is_prismtech (nn_vendorid_t vendor);
int is_own_vendor (nn_vendorid_t vendor);

void add_proxy_writer_to_reader (struct reader *rd, struct proxy_writer *pwr);
void add_proxy_reader_to_writer (struct writer *wr, struct proxy_reader *prd);
struct proxy_reader *new_proxy_reader (nn_guid_t guid, struct proxy_participant *pp, struct addrset *as, nn_xqos_t *xqos);
struct proxy_writer *new_proxy_writer (nn_guid_t guid, struct proxy_participant *pp, struct addrset *as, nn_xqos_t *xqos);

int is_writer_entityid (nn_entityid_t id);

#endif /* NN_RTPS_PRIVATE_H */
