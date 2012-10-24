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
#ifndef NN_CONFIG_H
#define NN_CONFIG_H

#include "u_participant.h"
#include "nn_log.h"

enum nn_standards_conformance {
  NN_SC_PEDANTIC,
  NN_SC_STRICT,
  NN_SC_LAX
};

#define NN_PEDANTIC_P (config.standards_conformance <= NN_SC_PEDANTIC)
#define NN_STRICT_P (config.standards_conformance <= NN_SC_STRICT)

struct config
{
  int valid;
  logcat_t enabled_logcats;
  char *servicename;

  os_char *networkAddressString;
  FILE *tracingOutputFile;
  c_char *tracingOutputFileName;
  c_bool tracingTimestamps;
  c_bool tracingRelativeTimestamps;
  c_bool tracingAppendToFile;
  os_char *peers;
  c_bool allowMulticast;
  c_bool dontRoute;
  c_bool enableMulticastLoopback;
  int domainId;
  int participantIndex;
  c_bool coexistWithNativeNetworking;
  int port_base;
  int startup_mode_duration; /*ms*/

  int primary_reorder_maxsamples;
  int secondary_reorder_maxsamples;

  float servicelease_expiry_time;
  float servicelease_update_factor;

  int buggy_datafrag_flags_mode;

  /* debug/test/undoc features: */
  int xmit_out_of_order;        /**<< upper bound on random term added to xmit time */
  int xmit_lossiness;           /**<< fraction of packets to drop on xmit, in units of 1e-3 */
  int rmsg_chunk_size;          /**<< size of a chunk in the receive buffer */
  int minimal_sedp_endpoint_set;
  int aggressive_keep_last1_whc;
  int conservative_builtin_reader_startup;
  int meas_hb_to_ack_latency;
  int unicast_response_to_spdp_messages;
  int synchronous_delivery_priority_threshold;
  int whc_lowwater_mark;
  int whc_highwater_mark;
  int xevq_lowwater_mark;
  int xevq_highwater_mark;
  int defrag_unreliable_maxsamples;
  int defrag_reliable_maxsamples;
  int accelerate_rexmit_block_size;
  int max_xevents_batch_size;
  int max_participants;

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
};

extern struct config config;

struct cfgst;

struct cfgst *config_init (u_participant participant, const c_char *serviceName);
void config_print_and_free_cfgst (struct cfgst *cfgst);
void config_fini (void);

#endif /* NN_CONFIG_H */
