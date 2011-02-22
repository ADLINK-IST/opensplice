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
#ifndef RTPS_PRIVATE_H
#define RTPS_PRIVATE_H

struct msg {
  int refc;
  int maxlen;
  int len;
  serdata_t serdata;
  struct xevent *tx_event;
  char msg[1]; /* C90 ... */
};

enum xeventkind {
  XEVK_HEARTBEAT,
  XEVK_ACKNACK,
  XEVK_MSG,
  XEVK_DATA,
  XEVK_DATA_RESEND,
  XEVK_SPDP,
  XEVK_GAP,
  XEVK_PMD_UPDATE,
  XEVK_INFO
};

struct xevent {
  STRUCT_AVLNODE (xevent_avlnode, struct xevent *) avlnode;
  struct xeventq *evq;
  long long tsched;
  long long min_tsched; /* in subtree */
  int size; /* number of events in subtree scheduled for ~immediate transmission */
  enum xeventkind kind;
  union {
    struct {
      struct writer *wr;
    } heartbeat;
    struct {
      struct rhc_writers_node *rwn;
    } acknack;
    struct {
      struct addrset *dest;
      struct msg *msg;
    } msg;
    struct {
      struct addrset *dest_all;
      struct msg *msg;
    } data;
    struct {
      struct proxy_endpoint *prd;
      struct addrset *dest_all;
      struct msg *msg;
    } data_resend;
    struct {
      struct msg *msg;
    } spdp;
    struct {
      struct proxy_endpoint *prd;
      struct msg *msg;
    } gap;
    struct {
      struct participant *pp;
    } pmd_update;
#if 0
    struct {
    } info;
#endif
  } u;
};

struct xeventq {
  STRUCT_AVLTREE (xeventq_avltree, struct xevent *) xevents;
  int oldsize; /* number of events in subtree scheduled for ~immediate transmission */
};

struct addrset_node {
  STRUCT_AVLNODE (addrset_node_avlnode, struct addrset_node *) avlnode;
  locator_udpv4_t addr;
};

struct addrset {
  int refc;
  STRUCT_AVLTREE (addrset_avltree, struct addrset_node *) ucaddrs, mcaddrs;
};

enum payload_kind {
  PK_NONE,
  PK_RAW_ALIASED,
  PK_RAW_MALLOCED,
  PK_V_MESSAGE
};

struct rtps_prismtech_writer_info
{
  c_ulong transactionId;
  v_gid writerGID;
  v_gid writerInstanceGID;
};

struct handle_regular_helper_arg {
  guid_t src, dst;
  long long seq;
  struct rtps_prismtech_writer_info wri;
  unsigned statusinfo;
  long long tstamp;
  enum payload_kind payload_kind;
  union {
    struct {
      int len;
      void *ptr;
    } raw;
    v_message v_message;
  } payload;
  int deliver;
  int out_of_sequence;
};

struct out_of_seq_msg {
  STRUCT_AVLNODE (out_of_seq_msg_avlnode, struct out_of_seq_msg *) avlnode;
  long long gaplength; /* = 0 => data; != 0 => gap */
  struct handle_regular_helper_arg info;
};

typedef struct out_of_seq_admin {
  STRUCT_AVLTREE (out_of_seq_admin_msgs_avltree, struct out_of_seq_msg *) msgs;
  int size;
} out_of_seq_admin_t;

struct proxy_participant {
  STRUCT_AVLNODE (proxy_participant_avlnode, struct proxy_participant *) avlnode;
  guid_t guid;
  int refc;
  vendorid_t vendor;
  builtin_endpoint_set_t bes;
  long long tlease_dur;
  long long tlease_end;
  long long min_tlease_end; /* in subtree */
  guid_t guid_min_tlease_end; /* in subtree */
  struct addrset *as_default;
  struct addrset *as_meta;
};

struct proxy_endpoint {
  STRUCT_AVLNODE (proxy_endpoint_avlnode, struct proxy_endpoint *) avlnode;
  struct proxy_participant *pp;
  guid_t guid;
  unsigned reliable: 1;
  durability_kind_t durability;
  liveliness_kind_t liveliness_kind;
  ownership_kind_t ownership_kind;
  destination_order_kind_t destination_order_kind;
  presentation_qospolicy_t presentation_qospolicy;
  long long tlease_dur;
  long long tlease_end;
  long long min_tlease_end; /* in subtree */
  guid_t guid_min_tlease_end; /* in subtree */
  char *topic;
  char *typename;
  char *partition;
  struct addrset *as;
  union {
    STRUCT_AVLTREE (proxy_endpoint_rhc_avltree, struct rhc_writers_node *) readers;
    STRUCT_AVLTREE (proxy_endpoint_whc_avltree, struct whc_readers_node *) writers;
  } matched_locals;
  union {
    struct {
      long long last_seq;
      /* buffer for out-of-sequence messages could be added here */
    } wr;
  } u;
};

struct whc_node {
  STRUCT_AVLNODE (whc_node_avlnode_seq, struct whc_node *) avlnode_seq;
  STRUCT_AVLNODE (whc_node_avlnode_tlidx, struct whc_node *) avlnode_tlidx;
  long long seq;
  long long minseq, maxseq;
  int in_tlidx;
  serdata_t serdata;
  struct msg *msg;
};

struct whc_readers_node {
  STRUCT_AVLNODE (whc_readers_node_avlnode, struct whc_readers_node *) avlnode;
  STRUCT_AVLNODE (whc_readers_node_proxyavltree, struct whc_readers_node *) proxyavlnode;
  struct writer *writer; /* for cleaning up given just the whc_readers_node */
  struct proxy_endpoint *proxy_reader;
  unsigned pure_ack_received: 1; /* set to 1 upon receipt of ack not nack'ing msgs */
  long long min_seq; /* smallest ack'd seq nr in subtree */
  guid_t reader_guid; /* key for avlnode (guid of proxy) */
  guid_t writer_guid; /* key for proxyavlnode (guid of local writer) */
  long long seq; /* highest acknowledged seq nr */
  count_t last_acknack;
};

struct writer {
  STRUCT_AVLNODE (writer_avlnode, struct writer *) avlnode; /* for per-participant set of writers */
  struct participant *participant;
  guid_t guid;
  long long seq; /* last sequence number (transmitted seqs are 1 ... seq) */
  count_t hbcount; /* last hb seq number */
  long long t_of_last_heartbeat;
  char *partition;
  long long tcreate;
  int throttling;
  durability_kind_t durability;
  presentation_access_scope_kind_t access_scope;
  destination_order_kind_t destination_order;
  unsigned deleting: 1; /* set upon entry to delete_writer() - intent */
  unsigned dying: 1; /* set upon entry to free_writer() - really dying */
#if ! RESPOND_TO_RTI_INIT_ZERO_ACK_WITH_INVALID_HEARTBEAT
  unsigned hb_before_next_msg: 1;
#endif
  unsigned reliable: 1;
  unsigned handle_as_transient_local: 1;
  unsigned with_key: 1;
  unsigned exclusive_ownership: 1;
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
  struct proxy_endpoint *proxy_writer;
  guid_t writer_guid; /* key for avlnode (guid of proxy) */
  guid_t reader_guid; /* key for proxyavlnode (guid of local reader) */
  long long seq; /* highest delivered seq nr */
  count_t count; /* last ack sequence number */
  count_t last_heartbeat; /* last heartbeat seen (see also add_proxy_writer_to_reader) */
  struct xevent *acknack_xevent; /* entry in xevent queue for sending acknacks */
  out_of_seq_admin_t osa; /* can be done (mostly) per proxy writer, but that is harder */
};

typedef void (*data_recv_raw_cb_fun_t) (const struct datainfo *di, const void *msg, int len);

struct reader {
  STRUCT_AVLNODE (reader_avlnode, struct reader *) avlnode; /* for per-participant set of readers */
  struct participant *participant;
  guid_t guid;
  long long tcreate;
  long long ndelivered;
  char *partition;
  durability_kind_t durability;
  presentation_access_scope_kind_t access_scope;
  destination_order_kind_t destination_order;
  unsigned dying: 1;
  unsigned reliable: 1;
  unsigned handle_as_transient_local: 1;
  unsigned exclusive_ownership: 1;
  topic_t topic;
  union {
    void (*generic) ();
    data_recv_raw_cb_fun_t raw;
    struct {
      data_recv_cb_fun_t f;
      void *arg;
    } cooked;
  } data_recv_cb;
  long long lease_duration;
  long long min_lease_duration;
  STRUCT_AVLTREE (rhc_writers_avltree, struct rhc_writers_node *) writers;
};

struct participant {
  STRUCT_AVLNODE (participant_avlnode, struct participant *) avlnode;
  guid_t guid;
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
#if MANY_SOCKETS
  locator_t sockloc;
  os_socket sock;
  int participant_set_index;
#endif
};

struct receiver_state {
  guid_prefix_t source_guid_prefix;
  guid_prefix_t dest_guid_prefix;
  struct addrset *reply_locators;
  ddsi_time_t timestamp;
  unsigned have_timestamp: 1;
  vendorid_t vendor;
};

struct tracebuf_st {
  char buf[2048];
  unsigned cat;
  int bufsz;
  int pos;
};
typedef struct tracebuf_st *tracebuf_t;
#define TRACEBUF_DECLNEW(name) { struct tracebuf_st name##_ST; tracebuf_t name = &name##_ST; name##_ST.pos = 0; name##_ST.bufsz = sizeof (name##_ST.buf); name##_ST.cat = 0; name##_ST.buf[0] = 0; {
#define TRACEBUF_FREE(name) } }

int rtps_trace (unsigned cat, const char *fmt, ...);
int rtps_vtrace (unsigned cat, const char *fmt, va_list ap);

int fill_v_message_qos (const struct proxy_endpoint *pwr, struct handle_regular_helper_arg *arg, unsigned statusinfo, c_base base);

#endif /* RTPS_PRIVATE_H */
