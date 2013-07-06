/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef NN_CONFIG_H
#define NN_CONFIG_H

#include "os_socket.h"
#include "u_participant.h"
#include "q_log.h"
#include "q_thread.h"
#include "q_xqos.h"

enum nn_standards_conformance {
  NN_SC_PEDANTIC,
  NN_SC_STRICT,
  NN_SC_LAX
};

#define NN_PEDANTIC_P (config.standards_conformance <= NN_SC_PEDANTIC)
#define NN_STRICT_P (config.standards_conformance <= NN_SC_STRICT)

enum besmode {
  BESMODE_FULL,
  BESMODE_WRITERS,
  BESMODE_MINIMAL
};

enum retransmit_merging {
  REXMIT_MERGE_NEVER,
  REXMIT_MERGE_ADAPTIVE,
  REXMIT_MERGE_ALWAYS
};

#define PARTICIPANT_INDEX_AUTO -1
#define PARTICIPANT_INDEX_NONE -2

/* config_listelem must be an overlay for all used listelem types */
struct config_listelem {
  struct config_listelem *next;
};




struct config_maybe_int32 {
  int isdefault;
  os_int32 value;
};

struct config_maybe_uint32 {
  int isdefault;
  os_uint32 value;
};

struct config_thread_properties_listelem {
  struct config_thread_properties_listelem *next;
  char *name;
  os_schedClass sched_class;
  struct config_maybe_int32 sched_priority;
  struct config_maybe_uint32 stack_size;
};

struct config_peer_listelem
{
   struct config_peer_listelem *next;
   char *peer;
};

struct config
{
  int valid;
  logcat_t enabled_logcats;
  char *servicename;
  char *pcap_file;

  os_char *networkAddressString;
  os_char **networkRecvAddressStrings;
  os_char *externalAddressString;
  os_char *externalMaskString;
  FILE *tracingOutputFile;
  c_char *tracingOutputFileName;
  c_bool tracingTimestamps;
  c_bool tracingRelativeTimestamps;
  c_bool tracingAppendToFile;
  c_bool allowMulticast;
  c_bool useIpv6;
  c_bool dontRoute;
  c_bool enableMulticastLoopback;
  int domainId;
  int participantIndex;
  c_bool coexistWithNativeNetworking;
  int port_base;
  struct config_maybe_int32 discoveryDomainId;
  os_char *spdpMulticastAddressString;
  os_int64 spdp_interval;
  os_int64 spdp_response_delay_max;
  os_int64 startup_mode_duration;
  enum retransmit_merging retransmit_merging;
  os_int64 retransmit_merging_period;
  c_bool squash_participants;
  c_bool startup_mode_full;
  c_bool forward_all_messages;

  int primary_reorder_maxsamples;
  int secondary_reorder_maxsamples;

  int delivery_queue_maxsamples;

  float servicelease_expiry_time;
  float servicelease_update_factor;

  int gid_hash_softlimit;
  int guid_hash_softlimit;

  int nw_queue_size;

  int buggy_datafrag_flags_mode;

  unsigned max_msg_size;
  unsigned fragment_size;

  struct config_peer_listelem *peers;
  struct config_thread_properties_listelem *thread_properties;

  /* debug/test/undoc features: */
  int xmit_lossiness;           /**<< fraction of packets to drop on xmit, in units of 1e-3 */
  int rmsg_chunk_size;          /**<< size of a chunk in the receive buffer */
  int rbuf_size;                /* << size of a single receiver buffer */
  enum besmode besmode;
  int aggressive_keep_last1_whc;
  int conservative_builtin_reader_startup;
  int meas_hb_to_ack_latency;
  int suppress_spdp_multicast;
  int unicast_response_to_spdp_messages;
  int synchronous_delivery_priority_threshold;
  os_int64 synchronous_delivery_latency_bound;
  int whc_lowwater_mark;
  int whc_highwater_mark;
  int defrag_unreliable_maxsamples;
  int defrag_reliable_maxsamples;
  int accelerate_rexmit_block_size;
  os_int64 responsiveness_timeout;
  os_uint32 max_participants;
  os_int64 writer_linger_duration;
  int multicast_ttl;
  int socket_min_rcvbuf_size;
  int socket_min_sndbuf_size;
  os_int64 nack_delay;
  os_int64 preemptive_ack_delay;
  os_int64 schedule_time_rounding;
  unsigned max_queued_rexmit_bytes;
  unsigned max_queued_rexmit_msgs;
  int ddsi2direct_max_threads;
  int late_ack_mode;

  /* compability options */
  enum nn_standards_conformance standards_conformance;
  int explicitly_publish_qos_set_to_default;
  int many_sockets_mode;
  int arrival_of_data_asserts_pp_and_ep_liveliness;
  int acknack_numbits_emptyset;
  int respond_to_rti_init_zero_ack_with_invalid_heartbeat;
  int assume_rti_has_pmd_endpoints;

  int port_dg;
  int port_pg;
  int port_d0;
  int port_d1;
  int port_d2;
  int port_d3;

  /* not used by ddsi2, only validated; user layer directly accesses
     the configuration tree */
  os_schedClass watchdog_sched_class;
  os_int32 watchdog_sched_priority;
};

extern struct config config;

struct cfgst;

struct cfgst *config_init (u_participant participant, const c_char *serviceName);
void config_print_and_free_cfgst (struct cfgst *cfgst);
void config_fini (void);


#endif /* NN_CONFIG_H */

/* SHA1 not available (unoffical build.) */
