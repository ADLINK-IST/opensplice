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
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>

#include "os_stdlib.h"
#include "u_participant.h"
#include "u_cfElement.h"
#include "u_cfData.h"

#include "nn_config.h"
#include "nn_log.h"
#include "nn_avl.h"
#include "nn_unused.h"
#include "nn_misc.h"

#define MAX_PATH_DEPTH 10 /* max nesting level of configuration elements */

struct cfgelem;
struct cfgst;

typedef int (*update_fun_t) (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int first, const char *value);
typedef void (*free_fun_t) (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem);
typedef void (*print_fun_t) (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default);

struct cfgelem {
  const char *name;
  const struct cfgelem *children;
  const struct cfgelem *attributes;
  int allow_many;
  const char *defvalue; /* NULL -> no default */
  int relative_offset;
  int elem_offset;
  update_fun_t update;
  free_fun_t free;
  print_fun_t print;
};

struct cfgst_nodekey {
  const struct cfgelem *e;
};

struct cfgst_node {
  STRUCT_AVLNODE (cfgst_node_avlnode, struct cfgst_node *) avlnode;
  struct cfgst_nodekey key;
  int count;
  int failed;
  int is_default;
};

struct cfgst {
  STRUCT_AVLTREE (cfgst_node_avltree, struct cfgst_node *) found;
  struct config *cfg;

  /* Servicename is used by uf_service_name to use as a default value
     when the supplied string is empty, which happens when the service
     is started without a DDSI2Service configuration item, i.e. when
     everything is left at the default. */
  const char *servicename;

  /* path_depth, isattr and path together control the formatting of
     error messages by cfg_error() */
  int path_depth;
  int isattr[MAX_PATH_DEPTH];
  const struct cfgelem *path[MAX_PATH_DEPTH];
};

/* "trace" is special: it enables (nearly) everything */
static const char *logcat_names[] = {
  "fatal", "error", "warning", "config", "info", "discovery", "data", "radmin", "trace", NULL
};
static const logcat_t logcat_codes[] = {
  LC_FATAL, LC_ERROR, LC_WARNING, LC_CONFIG, LC_INFO, LC_DISCOVERY, LC_DATA, LC_RADMIN, LC_ALLCATS
};

/* Want to keep backwards compatibility with the old Tracing/Verbosity
   element. So we set logging categories in this global variable, and
   use these if no categories have been specified. */
static unsigned tracingVerbosityLevel;

static int walk_element (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, u_cfElement base);

#define DU(fname) static int fname (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int first, const char *value)
DU (uf_networkAddress);
DU (uf_ipv4);
DU (uf_boolean);
DU (uf_negated_boolean);
DU (uf_string);
DU (uf_tracingOutputFileName);
DU (uf_verbosity);
DU (uf_logcat);
DU (uf_peers);
DU (uf_float);
DU (uf_int);
DU (uf_natint);
DU (uf_domainId);
DU (uf_participantIndex);
DU (uf_port);
DU (uf_int_milliseconds);
DU (uf_standards_conformance);
DU (uf_service_name);
#undef DU

#define DF(fname) static void fname (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem)
DF (ff_free);
#undef DF

#define PF(fname) static void fname (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
PF (pf_nop);
PF (pf_string);
PF (pf_networkAddress);
PF (pf_int);
PF (pf_float);
PF (pf_boolean);
PF (pf_negated_boolean);
PF (pf_logcat);
PF (pf_standards_conformance);
#undef PF

#define CO(name) ((int) offsetof (struct config, name))
#define ABSOFF(name) 0, CO (name)
#define RELOFF(name) 1, CO (name)
#define NODATA 0, NULL, 0, 0, 0, 0, 0
#define END_MARKER { NULL, NULL, NULL, NODATA }
#define WILDCARD { "*", NULL, NULL, NODATA }
#define LEAF(name) name, NULL, NULL
#define LEAF_W_ATTRS(name, attrs) name, NULL, attrs
#define GROUP(name, children) name, children, NULL, NODATA
#define ATTR(name) name, NULL, NULL
static const struct cfgelem timestamp_cfgattrs[] = {
  { ATTR ("absolute"), 0, "false", ABSOFF (tracingRelativeTimestamps), uf_negated_boolean, 0, pf_negated_boolean },
  END_MARKER
};

static const struct cfgelem general_cfgelems[] = {
  { LEAF ("NetworkInterfaceAddress"), 0, "auto", ABSOFF (networkAddressString), uf_networkAddress, ff_free, pf_networkAddress },
  { LEAF ("AllowMulticast"), 0, "true", ABSOFF (allowMulticast), uf_boolean, 0, pf_boolean },
  { LEAF ("DontRoute"), 0, "false", ABSOFF (dontRoute), uf_boolean, 0, pf_boolean },
  { LEAF ("EnableMulticastLoopback"), 0, "true", ABSOFF (enableMulticastLoopback), uf_boolean, 0, pf_boolean },
  { LEAF ("CoexistWithNativeNetworking"), 0, "false", ABSOFF (coexistWithNativeNetworking), uf_boolean, 0, pf_boolean },
  { LEAF ("StartupModeDuration"), 0, "2000", ABSOFF (startup_mode_duration), uf_int_milliseconds, 0, pf_int },
  END_MARKER
};

/* RTPS_USE_MCAST controls whether or not the RTPS protocol
   implementation will ever send a multicast packet (it always accepts
   them); if ..._LOOPBACK is set, IP_MULTICAST_LOOP will be set to
   true (loopback enabled), else to false (loopback disabled).

   If there is only one OpenSplice DDSI service on a host, LOOPBACK
   disabled will do just fine and give better performance. If
   communication with another DDSI implemetation running on the same
   host is desired, better leave it enabled.

   RTPS_IGNORE_OWN_VENDOR causes the message interpreter to ignore all
   messages originating from a node with the own vendor id, allowing
   DDSI2 to coexist with a native networking service.

   RTPS_AGGRESSIVE_KEEP_LAST1_WHC causes a reliable writer to drop a
   sample from its WHC as soon as a new sample for the same instance
   is accepted for transmission, no longer keeping the old one
   available for retransmission.

   RTPS_CONSERVATIVE_BUILTIN_READER_STARTUP causes all SEDP and PMD
   readers to try to get all data, even though all-but-one have no
   need for it.

   RTPS_NOQUEUE_HEARTBEAT_MESSAGES causes heartbeats to be transmitted
   immediately upon generation of the heartbeat messages, instead of
   queuing them so they end up behind data queued for immediate
   transmission (i.e., enables old behaviour). At high data rates,
   this behaviour results in heartbeats advertising sequence numbers
   of messages still lingering in the transmit queue, and likely cause
   NAKs. */
static const struct cfgelem compatibility_cfgelems[] = {
  { LEAF ("StandardsConformance"), 0, "strict", ABSOFF (standards_conformance), uf_standards_conformance, 0, pf_standards_conformance },
  { LEAF ("ExplicitlyPublishQosSetToDefault"), 0, "false", ABSOFF (explicitly_publish_qos_set_to_default), uf_boolean, 0, pf_boolean },
  { LEAF ("ManySocketsMode"), 0, "true", ABSOFF (many_sockets_mode), uf_boolean, 0, pf_boolean },
  { LEAF ("ArrivalOfDataAssertsPpAndEpLiveliness"), 0, "true", ABSOFF (arrival_of_data_asserts_pp_and_ep_liveliness), uf_boolean, 0, pf_boolean },
  /* How to encode an AckNack without NAK'ing any messages, that is,
     how to represent an empty NAK set in an AckNack submessage. The
     specification requires the set to contain at least one entry,
     i.e. ACKNACK_NUMBITS_EMPTYSET=1, but that causes RTI's
     implementation to immediately respond with a HeartBeat without
     the Final set, triggering an AckNack ... */
  { LEAF ("AckNackNumbitsEmptySet"), 0, "0", ABSOFF (acknack_numbits_emptyset), uf_natint, 0, pf_int },
  /* A reliable writer in this DDSI implementation seems to not
     cooperate very well with a reliable reader from RTI if the writer
     is at a high sequence number at the time the reader is
     discovered. We attempt two things: one is to respond to an
     invalid AckNack we get from RTI (seemingly) upon discovery of the
     writer, the second is to more aggressively schedule the
     retransmission of missing messages until a pure Ack (that is,
     without also being a Nack) is received.

     The first part seems to work. If the WHC is not empty we can
     simply respond with a Heartbeat; but for an empty, no valid
     Heartbeat can be generated. If RESPOND_... is true, we just
     transmit an out-of-spec Heartbeat (like RTI does ...) anyway, but
     if it is false, we can delay the heartbeat until the WHC is no
     longer empty, but just before the data is sent out. Both
     techniques seem to work.

     The second part is mostly untested, for the simple reason that
     the problem hasn't really surfaced yet. */
  { LEAF ("RespondToRtiInitZeroAckWithInvalidHeartbeat"), 0, "false", ABSOFF (respond_to_rti_init_zero_ack_with_invalid_heartbeat), uf_boolean, 0, pf_boolean },
  { LEAF ("AssumeRtiHasPmdEndpoints"), 0, "false", ABSOFF (assume_rti_has_pmd_endpoints), uf_boolean, 0, pf_boolean },
  END_MARKER
};

static const struct cfgelem unsupp_test_cfgelems[] = {
  { LEAF ("XmitOutOfOrder"), 0, "0", ABSOFF (xmit_out_of_order), uf_int, 0, pf_int },
  { LEAF ("XmitLossiness"), 0, "0", ABSOFF (xmit_lossiness), uf_int, 0, pf_int },
  { LEAF ("RmsgChunkSize"), 0, "131072", ABSOFF (rmsg_chunk_size), uf_int, 0, pf_int },
  END_MARKER
};

static const struct cfgelem unsupp_watermarks_cfgelems[] = {
  { LEAF ("WhcLow"), 0, "10", ABSOFF (whc_lowwater_mark), uf_natint, 0, pf_int },
  { LEAF ("WhcHigh"), 0, "200", ABSOFF (whc_highwater_mark), uf_natint, 0, pf_int },
  { LEAF ("XeventsLow"), 0, "5", ABSOFF (xevq_lowwater_mark), uf_natint, 0, pf_int },
  { LEAF ("XeventsHigh"), 0, "30", ABSOFF (xevq_highwater_mark), uf_natint, 0, pf_int },
  END_MARKER
};

static const struct cfgelem unsupp_cfgelems[] = {
  { LEAF ("PrimaryReorderMaxSamples"), 0, "64", ABSOFF (primary_reorder_maxsamples), uf_natint, 0, pf_int },
  { LEAF ("SecondaryReorderMaxSamples"), 0, "16", ABSOFF (secondary_reorder_maxsamples), uf_natint, 0, pf_int },
  { LEAF ("DefragUnreliableMaxSamples"), 0, "4", ABSOFF (defrag_unreliable_maxsamples), uf_natint, 0, pf_int },
  { LEAF ("DefragReliableMaxSamples"), 0, "16", ABSOFF (defrag_reliable_maxsamples), uf_natint, 0, pf_int },
  { LEAF ("MinimalSedpEndpointSet"), 0, "true", ABSOFF (minimal_sedp_endpoint_set), uf_boolean, 0, pf_boolean },
  { LEAF ("AggressiveKeepLast1Whc"), 0, "false", ABSOFF (aggressive_keep_last1_whc), uf_boolean, 0, pf_boolean },
  { LEAF ("ConservativeBuiltinReaderStartup"), 0, "false", ABSOFF (conservative_builtin_reader_startup), uf_boolean, 0, pf_boolean },
  { LEAF ("MeasureHbToAckLatency"), 0, "false", ABSOFF (meas_hb_to_ack_latency), uf_boolean, 0, pf_boolean },
  { LEAF ("UnicastResponseToSpdpMessages"), 0, "true", ABSOFF (unicast_response_to_spdp_messages), uf_boolean, 0, pf_boolean },
  { LEAF ("SynchronousDeliveryPriorityThreshold"), 0, "0"/*"2147483647"*/, ABSOFF (synchronous_delivery_priority_threshold), uf_int, 0, pf_int },
  { LEAF ("MaxParticipants"), 0, "0", ABSOFF (max_participants), uf_natint, 0, pf_int },
  { LEAF ("AccelerateRexmitBlockSize"), 0, "32", ABSOFF (accelerate_rexmit_block_size), uf_natint, 0, pf_int },
  { LEAF ("MaxXeventsBatchSize"), 0, "8", ABSOFF (max_xevents_batch_size), uf_natint, 0, pf_int },
  { LEAF ("LegacyFragmentation"), 0, "false", ABSOFF (buggy_datafrag_flags_mode), uf_boolean, 0, pf_boolean },
  { GROUP ("Test", unsupp_test_cfgelems) },
  { GROUP ("Watermarks", unsupp_watermarks_cfgelems) },
  END_MARKER
};

static const struct cfgelem discovery_ports_cfgelems[] = {
  { LEAF ("Base"), 0, "7400", ABSOFF (port_base), uf_port, 0, pf_int },
  { LEAF ("DomainGain"), 0, "250", ABSOFF (port_dg), uf_int, 0, pf_int },
  { LEAF ("ParticipantGain"), 0, "2", ABSOFF (port_pg), uf_int, 0, pf_int },
  { LEAF ("MulticastMetaOffset"), 0, "0", ABSOFF (port_d0), uf_int, 0, pf_int },
  { LEAF ("UnicastMetaOffset"), 0, "10", ABSOFF (port_d1), uf_int, 0, pf_int },
  { LEAF ("MulticastDataOffset"), 0, "1", ABSOFF (port_d2), uf_int, 0, pf_int },
  { LEAF ("UnicastDataOffset"), 0, "11", ABSOFF (port_d3), uf_int, 0, pf_int },
  END_MARKER
};

static const struct cfgelem discovery_cfgelems[] = {
  { LEAF ("Peer"), 1, "", ABSOFF (peers), uf_peers, ff_free, pf_string },
  { LEAF ("DomainId"), 0, "0", ABSOFF (domainId), uf_domainId, 0, pf_int },
  { LEAF ("ParticipantIndex"), 0, "auto", ABSOFF (participantIndex), uf_participantIndex, 0, pf_int },
  { GROUP ("Ports", discovery_ports_cfgelems) },
  END_MARKER
};

static const struct cfgelem tracing_cfgelems[] = {
  { LEAF ("EnableCategory"), 1, "warning,error,fatal", ABSOFF (enabled_logcats), uf_logcat, 0, pf_logcat },
  { LEAF ("Verbosity"), 0, "none", 0, 0, uf_verbosity, 0, pf_nop },
  { LEAF ("OutputFile"), 0, "", ABSOFF (tracingOutputFileName), uf_tracingOutputFileName, ff_free, pf_string },
  { LEAF_W_ATTRS ("Timestamps", timestamp_cfgattrs), 0, "true", ABSOFF (tracingTimestamps), uf_boolean, 0, pf_boolean },
  { LEAF ("AppendToFile"), 0, "true", ABSOFF (tracingAppendToFile), uf_boolean, 0, pf_boolean },
  END_MARKER
};
#define ENABLECATEGORY_CFGELEM_INDEX 0 /* hack for backwards compatibility */
#define VERBOSITY_CFGELEM_INDEX 1 /* hack for backwards compatibility */

static const struct cfgelem ddsi2_cfgelems[] = {
  { GROUP ("General", general_cfgelems) },
  { GROUP ("Compatibility", compatibility_cfgelems) },
  { GROUP ("Discovery", discovery_cfgelems) },
  { GROUP ("Tracing", tracing_cfgelems) },
  { GROUP ("Unsupported", unsupp_cfgelems) },
  END_MARKER
};

static const struct cfgelem lease_expiry_time_cfgattrs[] = {
  { ATTR ("update_factor"), 0, "0.1", ABSOFF (servicelease_update_factor), uf_float, 0, pf_float },
  END_MARKER
};

static const struct cfgelem lease_cfgelems[] = {
  { LEAF_W_ATTRS ("ExpiryTime", lease_expiry_time_cfgattrs), 0, "10.0", ABSOFF (servicelease_expiry_time), uf_float, 0, pf_float },
  END_MARKER
};

static const struct cfgelem domain_cfgelems[] = {
  { GROUP ("Lease", lease_cfgelems) },
  WILDCARD,
  END_MARKER
};

static const struct cfgelem ddsi2_cfgattrs[] = {
  { ATTR ("name"), 0, "", ABSOFF (servicename), uf_service_name, ff_free, pf_string },
  END_MARKER
};

static const struct cfgelem root_cfgelems[] = {
  { "DDSI2Service", ddsi2_cfgelems, ddsi2_cfgattrs, NODATA },
  { "Domain", domain_cfgelems, NULL, NODATA },
  END_MARKER
};
#undef ATTR
#undef GROUP
#undef LEAF_W_ATTRS
#undef LEAF
#undef WILDCARD
#undef END_MARKER
#undef NODATA
#undef RELOFF
#undef ABSOFF
#undef CO

struct config config;

static void cfgst_push (struct cfgst *cfgst, int isattr, const struct cfgelem *elem)
{
  assert (cfgst->path_depth < MAX_PATH_DEPTH);
  assert (isattr == 0 || isattr == 1);
  cfgst->isattr[cfgst->path_depth] = isattr;
  cfgst->path[cfgst->path_depth] = elem;
  cfgst->path_depth++;
}

static void cfgst_pop (struct cfgst *cfgst)
{
  assert (cfgst->path_depth > 0);
  cfgst->path_depth--;
}

static void cfg_note (struct cfgst *cfgst, logcat_t cat, const char *fmt, va_list ap)
{
  int i;
  LOGBUF_DECLNEW (lb);
  nn_logb (lb, cat, "config: ");

  /* Path to element/attribute causing the error. Have to stop once an
     attribute is reached: a NULL marker may have been pushed onto the
     stack afterward in the default handling. */
  for (i = 0; i < cfgst->path_depth && (i == 0 || !cfgst->isattr[i-1]); i++)
  {
    if (cfgst->path[i] == NULL)
    {
      assert (i > 0);
      nn_logb (lb, cat, "/#text");
    }
    else if (cfgst->isattr[i])
    {
      nn_logb (lb, cat, "[@%s]", cfgst->path[i]->name);
    }
    else
    {
      nn_logb (lb, cat, "%s%s", (i == 0) ? "" : "/", cfgst->path[i]->name);
    }
  }

  /* Nn_Log message */
  nn_logb (lb, cat, ": ");
  nn_vlogb (lb, cat, fmt, ap);
  nn_logb (lb, cat, "\n");
  nn_logb_flush (lb);
  LOGBUF_FREE (lb);
}

#if 0
static void cfg_warning (struct cfgst *cfgst, const char *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  cfg_note (cfgst, LC_WARNING, fmt, ap);
  va_end (ap);
}
#endif

static int cfg_error (struct cfgst *cfgst, const char *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  cfg_note (cfgst, LC_ERROR, fmt, ap);
  va_end (ap);
  return 0;
}

static int cfg_log (struct cfgst *cfgst, const char *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  cfg_note (cfgst, LC_CONFIG, fmt, ap);
  va_end (ap);
  return 0;
}

static void *cfg_address (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem)
{
  if (cfgelem->relative_offset)
    return (char *) parent + cfgelem->elem_offset;
  else
    return (char *) cfgst->cfg + cfgelem->elem_offset;
}

static int list_index (const char *list[], const char *elem)
{
  int i;
  for (i = 0; list[i] != NULL; i++)
  {
    if (os_strcasecmp (list[i], elem) == 0)
      return i;
  }
  return -1;
}

static void ff_free (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem)
{
  void **elem = cfg_address (cfgst, parent, cfgelem);
  os_free (*elem);
}

static int uf_boolean (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
  static const char *vs[] = { "false", "true", NULL };
  c_bool *elem = cfg_address (cfgst, parent, cfgelem);
  int idx = list_index (vs, value);
  if (idx < 0)
    return cfg_error (cfgst, "%s: undefined value", value);
  else
  {
    *elem = idx;
    return 1;
  }
}

static int uf_negated_boolean (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int first, const char *value)
{
  if (!uf_boolean (cfgst, parent, cfgelem, first, value))
    return 0;
  else
  {
    c_bool *elem = cfg_address (cfgst, parent, cfgelem);
    *elem = ! *elem;
    return 1;
  }
}

static int uf_logcat (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int first, const char *value)
{
  static const char **vs = logcat_names;
  static const logcat_t *lc = logcat_codes;
  unsigned *elem = cfg_address (cfgst, parent, cfgelem);
  char *copy = os_strdup (value), *cursor = copy, *tok;
  if (copy == NULL)
    return cfg_error (cfgst, "out of memory");
  if (first)
    *elem = 0;
  while ((tok = ddsi2_strsep (&cursor, ",")) != NULL)
  {
    int idx = list_index (vs, tok);
    if (idx < 0)
      return cfg_error (cfgst, "%s: undefined value", value);
    *elem |= lc[idx];
  }
  os_free (copy);
  return 1;
}

static int uf_verbosity (struct cfgst *cfgst, UNUSED_ARG (void *parent), UNUSED_ARG (struct cfgelem const * const cfgelem), UNUSED_ARG (int first), const char *value)
{
  static const char *vs[] = {
    "finest", "finer", "fine", "config", "info", "warning", "severe", "none", NULL
  };
  static const logcat_t lc[] = {
    LC_ALLCATS, LC_INFO, LC_DISCOVERY, LC_CONFIG, 0, LC_WARNING, LC_ERROR | LC_FATAL, 0, 0
  };
  unsigned *elem = &tracingVerbosityLevel;
  int idx = list_index (vs, value);
  assert (sizeof (vs) / sizeof (*vs) == sizeof (lc) / sizeof (*lc));
#if 0
  cfg_warning (cfgst, "deprecated, please switch to DDSI2Service/Tracing/EnableCategory");
#endif
  if (idx < 0)
    return cfg_error (cfgst, "%s: undefined value", value);
  else if (idx == (int) (sizeof (vs) / sizeof (*vs)) - 1)
  {
    *elem = 0;
    return 1;
  }
  else
  {
    unsigned cats = 0;
    int i;
    for (i = (int) (sizeof (vs) / sizeof (*vs)) - 1; i >= idx; i--)
      cats |= lc[i];
    *elem = cats;
    return 1;
  }
}

static int uf_string (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
  c_char **elem = cfg_address (cfgst, parent, cfgelem);
  *elem = os_strdup (value);
  return 1;
}

static int uf_service_name (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
  c_char **elem = cfg_address (cfgst, parent, cfgelem);
  if (*value == 0)
    *elem = os_strdup (cfgst->servicename);
  else
    *elem = os_strdup (value);
  return 1;
}

static int uf_tracingOutputFileName (struct cfgst *cfgst, UNUSED_ARG (void *parent), UNUSED_ARG (struct cfgelem const * const cfgelem), UNUSED_ARG (int first), const char *value)
{
  struct config *cfg = cfgst->cfg;
  if (os_strcasecmp (value, "stdout") == 0 || os_strcasecmp (value, "stderr") == 0)
  {
    cfg->tracingOutputFileName = os_strdup (value);
    return 1;
  }
  else
  {
    cfg->tracingOutputFileName = os_fileNormalize (value);
    return 1;
  }
}

static int uf_ipv4 (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int first, const char *value)
{
  /* Not actually doing any checking yet */
  return uf_string (cfgst, parent, cfgelem, first, value);
}

static int uf_networkAddress (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int first, const char *value)
{
  if (os_strcasecmp (value, "auto") != 0)
    return uf_ipv4 (cfgst, parent, cfgelem, first, value);
  else
  {
    c_char **elem = cfg_address (cfgst, parent, cfgelem);
    *elem = NULL;
    return 1;
  }
}

static int uf_peers (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int first, const char *value)
{
  c_char **ps = cfg_address (cfgst, parent, cfgelem);
  c_char *new;
  if (first)
    new = os_strdup (value);
  else
  {
    new = os_malloc (strlen (*ps) + 1 + strlen (value) + 1);
    if (new != NULL)
    {
      os_sprintf (new, "%s,%s", *ps, value);
      os_free (*ps);
    }
  }
  *ps = new;
  if (new == NULL)
    return cfg_error (cfgst, "%s: out of memory", value);
  else
    return 1;
}

static int uf_float (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
  float *elem = cfg_address (cfgst, parent, cfgelem);
  char *endptr;
  float f = (float) strtod (value, &endptr);
  if (*value == 0 || *endptr != 0)
    return cfg_error (cfgst, "%s: not a floating point number", value);
  *elem = f;
  return 1;
}

static int uf_int (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
  int *elem = cfg_address (cfgst, parent, cfgelem);
  char *endptr;
  long v = strtol (value, &endptr, 10);
  if (*value == 0 || *endptr != 0)
    return cfg_error (cfgst, "%s: not a decimal integer", value);
  if (v != (int) v)
    return cfg_error (cfgst, "%s: value out of range", value);
  *elem = (int) v;
  return 1;
}

static int uf_int_min_max (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int first, const char *value, int min, int max)
{
  int *elem = cfg_address (cfgst, parent, cfgelem);
  if (!uf_int (cfgst, parent, cfgelem, first, value))
    return 0;
  else if (*elem < min || *elem > max)
    return cfg_error (cfgst, "%s: out of range", value);
  else
    return 1;
}

static int uf_domainId (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int first, const char *value)
{
  return uf_int_min_max (cfgst, parent, cfgelem, first, value, 0, 230);
}

static int uf_participantIndex (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int first, const char *value)
{
  if (os_strcasecmp (value, "auto") != 0)
    return uf_int_min_max (cfgst, parent, cfgelem, first, value, 0, 120);
  else
  {
    int *elem = cfg_address (cfgst, parent, cfgelem);
    *elem = -1;
    return 1;
  }
}

static int uf_port (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int first, const char *value)
{
  return uf_int_min_max (cfgst, parent, cfgelem, first, value, 1, 65535);
}

static int uf_int_milliseconds (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int first, const char *value)
{
  /* limit to a minute: even that is ridiculous */
  return uf_int_min_max (cfgst, parent, cfgelem, first, value, 0, 60000);
}

static int uf_natint (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int first, const char *value)
{
  /* only positive numbers */
  return uf_int_min_max (cfgst, parent, cfgelem, first, value, 0, INT_MAX);
}

static int do_update (struct cfgst *cfgst, update_fun_t upd, void *parent, struct cfgelem const * const cfgelem, const char *value, int is_default)
{
  struct cfgst_node *n;
  struct cfgst_nodekey key;
  avlparent_t np;
  int ok;
  key.e = cfgelem;
  if ((n = avl_lookup (&cfgst->found, &key, &np)) == NULL)
  {
    n = os_malloc (sizeof (*n));
    avl_init_node (&n->avlnode, np);
    n->key = key;
    n->count = 0;
    n->failed = 0;
    n->is_default = is_default;
    avl_insert (&cfgst->found, n);
  }
  if (n->count == 0 || cfgelem->allow_many)
    ok = upd (cfgst, parent, cfgelem, (n->count == n->failed), value);
  else
    ok = cfg_error (cfgst, "only one instance allowed");
  n->count++;
  if (!ok)
  {
    n->failed++;
  }
  return ok;
}

static int set_default (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem)
{
  if (cfgelem->defvalue == NULL)
    return cfg_error (cfgst, "element missing in configuration");
  return do_update (cfgst, cfgelem->update, parent, cfgelem, cfgelem->defvalue, 1);
}

static int set_defaults (struct cfgst *cfgst, void *parent, int isattr, struct cfgelem const * const cfgelem)
{
  const struct cfgelem *ce;
  int ok = 1;
  for (ce = cfgelem; ce && ce->name; ce++)
  {
    struct cfgst_nodekey key;
    key.e = ce;
    cfgst_push (cfgst, isattr, ce);
    if (avl_lookup (&cfgst->found, &key, NULL) == NULL)
    {
      if (ce->update)
      {
        int ok1;
        cfgst_push (cfgst, 0, NULL);
        ok1 = set_default (cfgst, parent, ce);
        cfgst_pop (cfgst);
        ok = ok && ok1;
      }
    }
    if (ce->children)
    {
      int ok1 = set_defaults (cfgst, cfg_address (cfgst, parent, ce), 0, ce->children);
      ok = ok && ok1;
    }
    if (ce->attributes)
    {
      int ok1 = set_defaults (cfgst, cfg_address (cfgst, parent, ce), 1, ce->attributes);
      ok = ok && ok1;
    }
    cfgst_pop (cfgst);
  }
  return ok;
}

static void pf_nop (UNUSED_ARG (struct cfgst *cfgst), UNUSED_ARG (void *parent), UNUSED_ARG (struct cfgelem const * const cfgelem), UNUSED_ARG (int is_default))
{
}

static void pf_string (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  c_char **p = cfg_address (cfgst, parent, cfgelem);
  cfg_log (cfgst, "%s%s", *p ? *p : "(null)", is_default ? " [def]" : "");
}

static void pf_networkAddress (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  c_char **p = cfg_address (cfgst, parent, cfgelem);
  cfg_log (cfgst, "%s%s", *p ? *p : "auto", is_default ? " [def]" : "");
}

static void pf_int (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  int *p = cfg_address (cfgst, parent, cfgelem);
  cfg_log (cfgst, "%d%s", *p, is_default ? " [def]" : "");
}

static void pf_float (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  float *p = cfg_address (cfgst, parent, cfgelem);
  cfg_log (cfgst, "%f%s", *p, is_default ? " [def]" : "");
}

static void pf_boolean (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  c_bool *p = cfg_address (cfgst, parent, cfgelem);
  cfg_log (cfgst, "%s%s", *p ? "true" : "false", is_default ? " [def]" : "");
}

static void pf_negated_boolean (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  c_bool *p = cfg_address (cfgst, parent, cfgelem);
  cfg_log (cfgst, "%s%s", *p ? "false" : "true", is_default ? " [def]" : "");
}

static int uf_standards_conformance (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
  static const char *vs[] = {
    "pedantic", "strict", "lax", NULL
  };
  static const logcat_t lc[] = {
    NN_SC_PEDANTIC, NN_SC_STRICT, NN_SC_LAX, 0
  };
  enum nn_standards_conformance *elem = cfg_address (cfgst, parent, cfgelem);
  int idx = list_index (vs, value);
  assert (sizeof (vs) / sizeof (*vs) == sizeof (lc) / sizeof (*lc));
  if (idx < 0)
    return cfg_error (cfgst, "%s: undefined value", value);
  else
  {
    *elem = lc[idx];
    return 1;
  }
}

static void pf_standards_conformance (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  enum nn_standards_conformance *p = cfg_address (cfgst, parent, cfgelem);
  const char *str = "INVALID";
  switch (*p)
  {
    case NN_SC_PEDANTIC: str = "pedantic"; break;
    case NN_SC_STRICT: str = "strict"; break;
    case NN_SC_LAX: str = "lax"; break;
  }
  cfg_log (cfgst, "%s%s", str, is_default ? " [def]" : "");
}

static void pf_logcat (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  logcat_t *p = cfg_address (cfgst, parent, cfgelem);
  logcat_t remaining = *p;
  char res[256] = "", *resp = res;
  const char *prefix = "";
  int i;
#ifndef NDEBUG
  {
    int max;
    for (i = 0, max = 0; i < (int) (sizeof (logcat_codes) / sizeof (*logcat_codes)); i++)
      max += 1 + strlen (logcat_names[i]);
    max += 11; /* ,0x%x */
    max += 1; /* \0 */
    assert (max <= (int) sizeof (res));
  }
#endif
  /* TRACE enables ALLCATS, all the others just one */
  if ((remaining & LC_ALLCATS) == LC_ALLCATS)
  {
    resp += sprintf (resp, "%strace", prefix);
    remaining &= ~LC_ALLCATS;
    prefix = ",";
  }
  for (i = 0; i < (int) (sizeof (logcat_codes) / sizeof (*logcat_codes)); i++)
  {
    if (remaining & logcat_codes[i])
    {
      resp += sprintf (resp, "%s%s", prefix, logcat_names[i]);
      remaining &= ~logcat_codes[i];
      prefix = ",";
    }
  }
  if (remaining)
  {
    resp += sprintf (resp, "%s0x%x", prefix, (unsigned) remaining);
  }
  assert (resp <= res + sizeof (res));
  cfg_log (cfgst, "%s%s", res, is_default ? " [default]" : "");
}

static void print_configitems (struct cfgst *cfgst, void *parent, int isattr, struct cfgelem const * const cfgelem)
{
  const struct cfgelem *ce;
  for (ce = cfgelem; ce && ce->name; ce++)
  {
    struct cfgst_nodekey key;
    struct cfgst_node *n;
    key.e = ce;
    cfgst_push (cfgst, isattr, ce);
    if ((n = avl_lookup (&cfgst->found, &key, NULL)) == NULL)
    {
      assert (ce->update == 0);
      assert (ce->print == 0);
    }
    else
    {
      cfgst_push (cfgst, 0, NULL);
      ce->print (cfgst, parent, ce, n->is_default);
      cfgst_pop (cfgst);
    }
    if (ce->children)
      print_configitems (cfgst, cfg_address (cfgst, parent, ce), 0, ce->children);
    if (ce->attributes)
      print_configitems (cfgst, cfg_address (cfgst, parent, ce), 1, ce->attributes);
    cfgst_pop (cfgst);
  }
}

static void free_configured_elements (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem)
{
  const struct cfgelem *ce;
  for (ce = cfgelem; ce && ce->name; ce++)
  {
    struct cfgst_nodekey key;
    struct cfgst_node *n;
    key.e = ce;
    if ((n = avl_lookup (&cfgst->found, &key, NULL)) != NULL)
    {
      if (ce->free && n->count > n->failed)
        ce->free (cfgst, parent, ce);
    }
    if (ce->children)
      free_configured_elements (cfgst, cfg_address (cfgst, parent, ce), ce->children);
    if (ce->attributes)
      free_configured_elements (cfgst, cfg_address (cfgst, parent, ce), ce->attributes);
  }
}

static void free_all_elements (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem)
{
  const struct cfgelem *ce;
  for (ce = cfgelem; ce && ce->name; ce++)
  {
    if (ce->free)
      ce->free (cfgst, parent, ce);
    if (ce->children)
      free_all_elements (cfgst, cfg_address (cfgst, parent, ce), ce->children);
    if (ce->attributes)
      free_all_elements (cfgst, cfg_address (cfgst, parent, ce), ce->attributes);
  }
}

static int walk_attributes (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, u_cfElement base)
{
  c_iter iter;
  u_cfNode child;
  int ok = 1;
  iter = u_cfElementGetAttributes (base);
  child = u_cfNode (c_iterTakeFirst (iter));
  while (child)
  {
    const struct cfgelem *cfg_attr;
    u_cfAttribute attr;
    c_char *child_name;
    int ok1 = 0;
    child_name = u_cfNodeName (child);
    assert (child_name != NULL);
    assert (u_cfNodeKind (child) == V_CFATTRIBUTE);
    attr = u_cfAttribute (child);
    for (cfg_attr = cfgelem->attributes; cfg_attr && cfg_attr->name; cfg_attr++)
    {
      if (os_strcasecmp (cfg_attr->name, child_name) == 0)
        break;
    }
    if (cfg_attr == NULL || cfg_attr->name == NULL)
      cfg_error (cfgst, "%s: unknown attribute", child_name);
    else
    {
      c_char *str;
      if (!u_cfAttributeStringValue (attr, &str))
        cfg_error (cfgst, "failed to extract data");
      else
      {
        cfgst_push (cfgst, 1, cfg_attr);
        ok1 = do_update (cfgst, cfg_attr->update, cfg_address (cfgst, parent, cfgelem), cfg_attr, str, 0);
        os_free (str);
        cfgst_pop (cfgst);
      }
    }
    ok = ok && ok1;
    os_free (child_name);
    u_cfNodeFree (child);
    child = u_cfNode (c_iterTakeFirst (iter));
  }
  c_iterFree (iter);
  return ok;
}

static int walk_children (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, u_cfElement base)
{
  c_iter iter;
  u_cfNode child;
  int ok = 1;
  iter = u_cfElementGetChildren (base);
  child = u_cfNode (c_iterTakeFirst (iter));
  while (child)
  {
    c_char *child_name;
    int ok1 = 0;
    child_name = u_cfNodeName (child);
    assert (child_name != NULL);
    switch (u_cfNodeKind (child))
    {
      case V_CFELEMENT:
        {
          u_cfElement elem = u_cfElement (child);
          const struct cfgelem *cfg_subelem;
          for (cfg_subelem = cfgelem->children;
               cfg_subelem && cfg_subelem->name && strcmp (cfg_subelem->name, "*") != 0;
               cfg_subelem++)
          {
            if (os_strcasecmp (cfg_subelem->name, child_name) == 0)
              break;
          }
          if (cfg_subelem == NULL || cfg_subelem->name == NULL)
            cfg_error (cfgst, "%s: unknown element", child_name);
          else if (strcmp (cfg_subelem->name, "*") == 0)
            ok1 = 1;
          else
          {
            cfgst_push (cfgst, 0, cfg_subelem);
            ok1 = walk_element (cfgst, cfg_address (cfgst, parent, cfgelem), cfg_subelem, elem);
            cfgst_pop (cfgst);
          }
          break;
        }
      case V_CFDATA:
        {
          u_cfData data = u_cfData (child);
          c_char *str;
          if (!u_cfDataStringValue (data, &str))
            cfg_error (cfgst, "failed to extract data");
          else if (cfgelem->update == 0)
          {
            if (strspn (str, " \t\r\n") != strlen (str))
              cfg_error (cfgst, "%s: no data expected", str);
            else
              ok1 = 1;
            os_free (str);
          }
          else
          {
            cfgst_push (cfgst, 0, NULL);
            ok1 = do_update (cfgst, cfgelem->update, parent, cfgelem, str, 0);
            cfgst_pop (cfgst);
            os_free (str);
          }
          break;
        }
      default:
        abort ();
    }
    ok = ok && ok1;
    os_free (child_name);
    u_cfNodeFree (child);
    child = u_cfNode (c_iterTakeFirst (iter));
  }
  c_iterFree (iter);
  return ok;
}

static int walk_element (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, u_cfElement base)
{
  return walk_attributes (cfgst, parent, cfgelem, base) && walk_children (cfgst, parent, cfgelem, base);
}

static int cfgst_node_cmp (const void *va, const void *vb)
{
  return memcmp (va, vb, sizeof (struct cfgst_nodekey));
}

struct cfgst *config_init (/* C_STRUCT (u_participant) const * */u_participant participant, const c_char *servicename)
{
  u_cfElement root, elem;
  c_iter iter;
  int ok = 1;
  int use_verbosity;
  struct cfgst *cfgst;
  int rootidx;

  if ((cfgst = os_malloc (sizeof (*cfgst))) == NULL)
    return NULL;

  assert (participant != NULL);
  avl_init (&cfgst->found, offsetof (struct cfgst_node, avlnode), offsetof (struct cfgst_node, key), cfgst_node_cmp, 0, os_free);
  cfgst->path_depth = 0;
  cfgst->cfg = &config;
  cfgst->servicename = servicename;

  /* Enable logging of errors &c. to stderr until configuration is read */
  config.enabled_logcats = LC_FATAL | LC_ERROR | LC_WARNING;
  config.tracingOutputFile = stderr;

  if ((root = u_participantGetConfiguration ((u_participant) participant)) == NULL)
  {
    nn_log (LC_FATAL, "config_init: u_participantGetConfiguration failed");
    avl_free (&cfgst->found);
    os_free (cfgst);
    return NULL;
  }

  /* Only suitable for Domain (without a attributes) and a service
     with a matching name attribute */
  for (rootidx = 0; root_cfgelems[rootidx].name; rootidx++)
  {
    const struct cfgelem *root_cfgelem = &root_cfgelems[rootidx];
    cfgst_push (cfgst, 0, root_cfgelem);
    iter = u_cfElementXPath (root, root_cfgelem->name);
    elem = u_cfElement (c_iterTakeFirst (iter));
    while (elem)
    {
      c_char *str;
      if (root_cfgelem->attributes == NULL)
      {
        ok = walk_element (cfgst, NULL, root_cfgelem, elem);
      }
      else if (u_cfElementAttributeStringValue (elem, "name", &str))
      {
        int ok1;
        if (strcmp (servicename, str) != 0)
          ok1 = 1;
        else
          ok1 = walk_element (cfgst, NULL, root_cfgelem, elem);
        ok = ok && ok1;
        os_free (str);
      }
      u_cfElementFree (elem);
      elem = u_cfElement (c_iterTakeFirst (iter));
    }
    c_iterFree (iter);
    cfgst_pop (cfgst);
    assert (cfgst->path_depth == 0);
  }
  u_cfElementFree (root);

  /* Backwards compatibility of Tracing/Verbose - an ugly hack - if
     Tracing/EnableCategory is not specified at all, copy from
     Tracing/Verbose. But do so after defaulting ... (because I want
     to be able to remove all "verbosity" stuff without any
     consequences beyond the configuration file format). */
  {
    struct cfgst_nodekey key;
    int have_enable_cat, have_verbosity;
    key.e = &tracing_cfgelems[ENABLECATEGORY_CFGELEM_INDEX];
    have_enable_cat = (avl_lookup (&cfgst->found, &key, NULL) != NULL);
    key.e = &tracing_cfgelems[VERBOSITY_CFGELEM_INDEX];
    have_verbosity = (avl_lookup (&cfgst->found, &key, NULL) != NULL);
    use_verbosity = (!have_enable_cat && have_verbosity);
  }

  /* Set defaults for everything not set that we have a default value
     for, signal errors for things unset but without a default. */
  set_defaults (cfgst, NULL, 0, root_cfgelems);

  /* Backwards compatibility of Tracing/Verbose */
  if (use_verbosity)
    config.enabled_logcats = tracingVerbosityLevel;

  if (!ok)
  {
    free_configured_elements (cfgst, NULL, root_cfgelems);
  }
  if (ok)
  {
    config.valid = 1;
    return cfgst;
  }
  else
  {
    os_free (cfgst);
    return NULL;
  }
}

void config_print_and_free_cfgst (struct cfgst *cfgst)
{
  if (cfgst == NULL)
    return;
  print_configitems (cfgst, NULL, 0, root_cfgelems);
  avl_free (&cfgst->found);
  os_free (cfgst);
}

void config_fini (void)
{
  if (config.valid)
  {
    struct cfgst cfgst;
    cfgst.cfg = &config;
    free_all_elements (&cfgst, NULL, root_cfgelems);
    memset (&config, 0, sizeof (config));
  }
}
