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
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>

#include "os_stdlib.h"
#include "os_socket.h"
#include "os_heap.h"

#include "ut_crc.h"
#include "u_participant.h"
#include "u_cfElement.h"
#include "u_cfData.h"

#include "q_config.h"
#include "q_log.h"
#include "ut_avl.h"
#include "q_unused.h"
#include "q_misc.h"
#include "q_addrset.h"
#include "q_nwif.h"
#include "q_error.h"
#include "sysdeps.h"


#define WARN_DEPRECATED_ALIAS 1
#define WARN_DEPRECATED_UNIT 1
#define MAX_PATH_DEPTH 10 /* max nesting level of configuration elements */

struct cfgelem;
struct cfgst;

typedef int (*init_fun_t) (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem);
typedef int (*update_fun_t) (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int first, const char *value);
typedef void (*free_fun_t) (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem);
typedef void (*print_fun_t) (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default);

struct unit {
  const char *name;
  os_int64 multiplier;
};

struct cfgelem {
  const char *name;
  const struct cfgelem *children;
  const struct cfgelem *attributes;
  int multiplicity;
  const char *defvalue; /* NULL -> no default */
  int relative_offset;
  int elem_offset;
  init_fun_t init;
  update_fun_t update;
  free_fun_t free;
  print_fun_t print;
  const char *description;
};

struct cfgst_nodekey {
  const struct cfgelem *e;
};

struct cfgst_node {
  ut_avlNode_t avlnode;
  struct cfgst_nodekey key;
  int count;
  int failed;
  int is_default;
};

struct cfgst {
  ut_avlTree_t found;
  struct config *cfg;

  /* internal_seen is set by the Internal group's init function and
     determines whether or not we warn for Unsupported. */
  int internal_seen;

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
  void *parent[MAX_PATH_DEPTH];
};

/* "trace" is special: it enables (nearly) everything */
static const char *logcat_names[] = {
  "fatal", "error", "warning", "config", "info", "discovery", "data", "radmin", "timing", "traffic", "topic", "trace", NULL
};
static const logcat_t logcat_codes[] = {
  LC_FATAL, LC_ERROR, LC_WARNING, LC_CONFIG, LC_INFO, LC_DISCOVERY, LC_DATA, LC_RADMIN, LC_TIMING, LC_TRAFFIC, LC_TOPIC, LC_ALLCATS
};

/* We want the tracing/verbosity settings to be fixed while parsing
   the configuration, so we update this variable instead. */
static unsigned enabled_logcats;

static int cfgst_node_cmp (const void *va, const void *vb);
static const ut_avlTreedef_t cfgst_found_treedef =
  UT_AVL_TREEDEF_INITIALIZER (offsetof (struct cfgst_node, avlnode), offsetof (struct cfgst_node, key), cfgst_node_cmp, 0);

static int walk_element (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, u_cfElement base);

#define DU(fname) static int fname (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int first, const char *value)
DU (uf_networkAddress);
DU (uf_networkAddresses);
DU (uf_ipv4);
DU (uf_boolean);
DU (uf_negated_boolean);
DU (uf_string);
DU (uf_tracingOutputFileName);
DU (uf_verbosity);
DU (uf_logcat);
DU (uf_float);
DU (uf_int);
DU (uf_int32);
DU (uf_natint);
DU (uf_natint_255);
DU (uf_participantIndex);
DU (uf_port);
DU (uf_dyn_port);
DU (uf_memsize);
DU (uf_duration_inf);
DU (uf_duration_ms_1hr);
DU (uf_duration_ms_1s);
DU (uf_duration_us_1s);
DU (uf_standards_conformance);
DU (uf_locators);
DU (uf_besmode);
DU (uf_retransmit_merging);
DU (uf_sched_prio_class);
DU (uf_sched_class);
DU (uf_maybe_memsize);
DU (uf_maybe_duration_inf);
DU (uf_maybe_int32);
DU (uf_domainId);
DU (uf_service_name);
#undef DU

#define DF(fname) static void fname (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem)
DF (ff_free);
DF (ff_networkAddresses);
#undef DF

#define DI(fname) static int fname (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem)
DI (if_internal);
DI (if_unsupported);
DI (if_peer);
DI (if_thread_properties);
#undef DI

#define PF(fname) static void fname (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
PF (pf_nop);
PF (pf_string);
PF (pf_networkAddress);
PF (pf_participantIndex);
PF (pf_networkAddresses);
PF (pf_memsize);
PF (pf_duration);
PF (pf_int);
PF (pf_int32);
PF (pf_float);
PF (pf_boolean);
PF (pf_negated_boolean);
PF (pf_logcat);
PF (pf_standards_conformance);
PF (pf_locators);
PF (pf_besmode);
PF (pf_retransmit_merging);
PF (pf_sched_prio_class);
PF (pf_sched_class);
PF (pf_maybe_memsize);
PF (pf_maybe_duration);
PF (pf_maybe_int32);
#undef PF

#define CO(name) ((int) offsetof (struct config, name))
#define ABSOFF(name) 0, CO (name)
#define RELOFF(parent,name) 1, ((int) offsetof (struct parent, name))
#define NODATA 1, NULL, 0, 0, 0, 0, 0, 0
#define END_MARKER { NULL, NULL, NULL, NODATA, NULL }
#define WILDCARD { "*", NULL, NULL, NODATA, NULL }
#define LEAF(name) name, NULL, NULL
#define LEAF_W_ATTRS(name, attrs) name, NULL, attrs
#define GROUP(name, children) name, children, NULL, 1, NULL, 0, 0, 0, 0, 0, 0
#define MGROUP(name, children, attrs) name, children, attrs
#define ATTR(name) name, NULL, NULL
static const struct cfgelem timestamp_cfgattrs[] = {
  { ATTR ("absolute"), 1, "false", ABSOFF (tracingRelativeTimestamps), 0, uf_negated_boolean, 0, pf_negated_boolean,
    "This attribute specifies whether the timestamps in the log file are absolute or relative to the startup time of the service.  Currently not implemented in DDSI2, all timestamps are absolute." },
  END_MARKER
};

static const struct cfgelem general_cfgelems[] = {
  { LEAF ("NetworkInterfaceAddress"), 1, "auto", ABSOFF (networkAddressString), 0, uf_networkAddress, ff_free, pf_networkAddress,
    "<p>This element specifies the preferred network interface for use by DDSI2. The preferred network interface determines the IP address that DDSI2 advertises in the discovery protocol (but see also General/ExternalNetworkAddress), and is also the only interface over which multicasts are transmitted. The interface can be identified by its IP address, network interface name or network portion of the address. If the value \"auto\" is entered here, DDSI2 will select what it considers the most suitable interface.</p>" },
  { LEAF ("MulticastRecvNetworkInterfaceAddresses"), 1, "preferred", ABSOFF (networkRecvAddressStrings), 0, uf_networkAddresses, ff_networkAddresses, pf_networkAddresses,
    "<p>This element specifies on which network interfaces DDSI2 listens to multicasts. The following options are available:\n\
<ul><li><i>preferred</i>: listen for multicasts on the preferred interface (General/NetworkInterfaceAddress); or</li>\n\
<li><i>all</i>: listen for multicasts on all multicast-capable interfaces; or</li>\n\
<li><i>any</i>: listen for multicasts on the operating system default interface; or</li>\n\
<li><i>none</i>: does not listen for multicasts on any interface; or</li>\n\
<li>a comma-separated list of network addresses: configures DDSI2 to listen for multicasts on all of the listed addresses.</li></ul>\n\
If DDSI2 is in IPv6 mode and the address of the preferred network interface is a link-local address, \"all\" is treated as a synonym for \"preferred\" and a comma-separated list is treated as \"preferred\" if it contains the preferred interface and as \"none\" if not.</p>" },
  { LEAF ("ExternalNetworkAddress"), 1, "auto", ABSOFF (externalAddressString), 0, uf_networkAddress, ff_free, pf_networkAddress,
    "<p>This element allows explicitly overruling the network address the DDSI2 service advertises in the discovery protocol, which by default is the address of the preferred network interface (General/NetworkInterfaceAddress), to allow DDSI2 to communicate across a Network Address Translation (NAT) device.</p>" },
  { LEAF ("ExternalNetworkMask"), 1, "0.0.0.0", ABSOFF (externalMaskString), 0, uf_string, ff_free, pf_string,
    "<p>This element specifies the network mask of the external network address. This element is relevant only when an external network address (General/ExternalNetworkAddress) is explicitly configured. In this case locators received via the discovery protocol that are within the same external subnet (as defined by this mask) will be translated to an internal address by replacing the network portion of the external address with the corresponding portion of the preferred network interface address. This option is IPv4-only.</p>" },
  { LEAF ("AllowMulticast"), 1, "true", ABSOFF (allowMulticast), 0, uf_boolean, 0, pf_boolean,
    "<p>This element controls whether the DDSI2 service uses multicasts for data traffic.</p>\n\
<p>When set to \"false\", DDSI2 will never advertise multicast addresses and never accept multicast addresses advertised by remote nodes, but it will still listen for multicast packets and perform multicast-based participant discovery. Listening for multicasts can be controlled by General/MulticastRecvNetworkInterfaceAddresses, and transmitting participant discovery multicasts by Internal/SuppressSPDPMulticast.</p>\n\
<p>The default, \"true\", enables full use of multicasts.</p>" },
  { LEAF ("MulticastTimeToLive"), 1, "32", ABSOFF (multicast_ttl), 0, uf_natint_255, 0, pf_int,
    "<p>This element specifies the time-to-live setting for outgoing multicast packets.</p>" },
  { LEAF ("DontRoute"), 1, "false", ABSOFF (dontRoute), 0, uf_boolean, 0, pf_boolean,
    "<p>This element allows setting the SO_DONTROUTE option for outgoing packets, to bypass the local routing tables. This is generally useful only when the routing tables cannot be trusted, which is highly unusual.</p>" },
  { LEAF ("UseIPv6"), 1, "false", ABSOFF (useIpv6), 0, uf_boolean, 0, pf_boolean,
    "<p>This element can be used to make DDSI2 service use IPv6 instead of IPv4. This is currently an either/or switch.</p>" },
  { LEAF ("EnableMulticastLoopback"), 1, "true", ABSOFF (enableMulticastLoopback), 0, uf_boolean, 0, pf_boolean,
    "<p>This element specifies whether the DDSI2 service will allow IP multicast packets to be visible to all DDSI participants in the same node, including itself. It must be \"true\" for intra-node multicast communications, but if a node runs only a single DDSI2 service and does not host any other DDSI-capable programs, it may be set to \"false\" for improved performance.</p>" },
  { LEAF ("CoexistWithNativeNetworking"), 1, "false", ABSOFF (coexistWithNativeNetworking), 0, uf_boolean, 0, pf_boolean,
    "<p>This element specifies whether the DDSI2 service operates in conjunction with the OpenSplice RT Networking service. When \"false\", the DDSI2 service will take care of all communications, including those between OpenSplice nodes; when \"true\", the DDSI2 service only communicates with DDS implementations other than OpenSplice. In this case the RT Networking service should be configured as well.</p>" },
  { LEAF ("StartupModeDuration"), 1, "2 s", ABSOFF (startup_mode_duration), 0, uf_duration_ms_1hr, 0, pf_duration,
    "<p>This element specifies how long the DDSI2 remains in its \"startup\" mode. While in \"startup\" mode all volatile reliable data published on the local node is retained as-if it were transient-local data, allowing existing readers on remote nodes to obtain the data even though discovering them takes some time. Best-effort data by definition need not arrive, and transient and persistent data are covered by the durability service.</p>\n\
<p>Once the system is stable, the DDSI2 service keeps track of the existence of remote readers whether or not matching writers exist locally, avoiding this discovery delay and ensuring this is merely a node startup issue.</p>\n\
<p>Setting General/StartupModeDuration to 0s will disable it.</p>" },
  { LEAF ("StartupModeCoversTransient"), 1, "true", ABSOFF (startup_mode_full), 0, uf_boolean, 0, pf_boolean,
    "<p>This element configures whether startup-mode should also cover transient and persistent data, for configurations where the durability servie does not take care of it. Configurations without defined merge policies best leave this enabled.</p>" },
  END_MARKER
};




static const struct cfgelem thread_properties_sched_cfgelems[] = {
  { LEAF ("Class"), 1, "default", RELOFF (config_thread_properties_listelem, sched_class), 0, uf_sched_class, 0, pf_sched_class,
    "<p>This element specifies the thread scheduling class (<i>realtime</i>, <i>timeshare</i> or <i>default</i>). The user may need special privileges from the underlying operating system to be able to assign some of the privileged scheduling classes.</p>" },
  { LEAF ("Priority"), 1, "default", RELOFF (config_thread_properties_listelem, sched_priority), 0, uf_maybe_int32, 0, pf_maybe_int32,
    "<p>This element specifies the thread priority (decimal integer or <i>default</i>). Only priorities that are supported by the underlying operating system can be assigned to this element. The user may need special privileges from the underlying operating system to be able to assign some of the privileged priorities.</p>" },
  END_MARKER
};

static const struct cfgelem thread_properties_cfgattrs[] = {
  { ATTR ("Name"), 1, NULL, RELOFF (config_thread_properties_listelem, name), 0, uf_string, ff_free, pf_string,
    "<p>The Name of the thread for which properties are being set. The following threads exist:\n\
<ul><li><i>gc</i>: garbage collector thread involved in deleting entities;</li></li>\n\
<li><i>main</i>: main thread, primarily handling local discovery;</li>\n\
<li><i>recv</i>: receive thread, taking data from the network and running the protocol state machine;</li>\n\
<li><i>dq.builtins</i>: delivery thread for DDSI-builtin data, primarily for discovery;</li>\n\
<li><i>lease</i>: DDSI2 liveliness monitoring;</li>\n\
<li><i>tev</i>: general timed-event handling, retransmits and discovery;</li>\n\
<li><i>xmit.CHAN</i>: transmit thread for channel CHAN;</li>\n\
<li><i>dq.CHAN</i>: delivery thread for channel CHAN;</li>\n\
<li><i>tev.CHAN</i>: timed-even thread for channel CHAN.</li></ul></p>" },
  END_MARKER
};

static const struct cfgelem thread_properties_cfgelems[] = {
  { GROUP ("Scheduling", thread_properties_sched_cfgelems),
    "<p>This element configures the scheduling properties of the thread.</p>" },
  { LEAF ("StackSize"), 1, "default", RELOFF (config_thread_properties_listelem, stack_size), 0, uf_maybe_memsize, 0, pf_maybe_memsize,
    "<p>This element configures the stack size for this thread. The default value <i>default</i> leaves the stack size at the the operating system default.</p>" },
  END_MARKER
};

static const struct cfgelem threads_cfgelems[] = {
  { MGROUP ("Thread", thread_properties_cfgelems, thread_properties_cfgattrs), 1000, 0, ABSOFF (thread_properties), if_thread_properties, 0, 0, 0,
    "<p>This element specifies thread properties, such as scheduling parameters and stack size.</p>" },
  END_MARKER
};

static const struct cfgelem compatibility_cfgelems[] = {
  { LEAF ("StandardsConformance"), 1, "lax", ABSOFF (standards_conformance), 0, uf_standards_conformance, 0, pf_standards_conformance,
    "<p>This element sets the level of standards conformance of this instance of the DDSI2 Service. Stricter conformance typically means less interoperability with other implementations. Currently three modes are defined:\n\
<ul><li><i>pedantic</i>: very strictly conform to the specification, ultimately for compliancy testing, but currently of little value because it adheres even to what will most likely turn out to be editing errors in the DDSI standard. Arguably, as long as no errata have been published it is the current text that is in effect, and that is what pedantic currently does.</li>\n\
<li><i>strict</i>: a slightly less strict view of the standard than does pedantic: it follows the established behaviour where the standard is obviously in error.</li>\n\
<li><i>lax</i>: attempt to provide the smoothest possible interoperability, anticipating future revisions of elements in the standard in areas that other implementations do not adhere to, even though there is no good reason not to.</li></ul><p>\n\
<p>The default setting is \"lax\".</p>" },
  { LEAF ("ExplicitlyPublishQosSetToDefault"), 1, "false", ABSOFF (explicitly_publish_qos_set_to_default), 0, uf_boolean, 0, pf_boolean,
    "<p>This element specifies whether QoS settings set to default values are explicitly published in the discovery protocol. Implementations are to use the default value for QoS settings not published, which allows a significant reduction of the amount of data that needs to be exchanged for the discovery protocol, but this requires all implementations to adhere to the default values specified by the specifications.</p>\n\
<p>When interoperability is required with an implementation that does not follow the specifications in this regard, setting this option to true will help.</p>" },
  { LEAF ("ManySocketsMode"), 1, "false", ABSOFF (many_sockets_mode), 0, uf_boolean, 0, pf_boolean,
    "<p>This option specifies whether a network socket will be created for each domain participant on a host. The specification seems to assume that each participant has a unique address, and setting this option will ensure this to be the case. This is not the default.</p>\n\
<p>Disabling it slightly improves performance and reduces network traffic somewhat. It also causes the set of port numbers needed by DDSI2 to become predictable, which may be useful for firewall and NAT configuration.</p>" },
  { LEAF ("ArrivalOfDataAssertsPpAndEpLiveliness"), 1, "true", ABSOFF (arrival_of_data_asserts_pp_and_ep_liveliness), 0, uf_boolean, 0, pf_boolean,
    "<p>This setting is currently ignored (accepted for backwards compatibility).</p>" },
  { LEAF ("AckNackNumbitsEmptySet"), 1, "0", ABSOFF (acknack_numbits_emptyset), 0, uf_natint, 0, pf_int,
    "<p>This element governs the representation of an acknowledgement message that does not also negatively-acknowledge some samples. If set to 0, the generated acknowledgements have an invalid form and will be reject by the strict and pedantic conformance modes, but several other implementation require this setting for smooth interoperation.</p>\n\
<p>If set to 1, all acknowledgements sent by DDSI2 adhere the form of acknowledgement messages allowed by the standard, but this causes problems when interoperating with these other implementations. The strict and pedantic standards conformance modes always overrule a AckNackNumbitsEmptySet=0 to prevent the transmitting of invalid messages.</p>" },
  { LEAF ("RespondToRtiInitZeroAckWithInvalidHeartbeat"), 1, "false", ABSOFF (respond_to_rti_init_zero_ack_with_invalid_heartbeat), 0, uf_boolean, 0, pf_boolean,
    "<p>This element allows a closer mimicking of the behaviour of some other DDSI implementations, albeit at the cost of generating even more invalid messages. Setting it to true ensures a Heartbeat can be sent at any time when a remote node requests one, setting it to false delays it until a valid one can be sent.</p>\n\
<p>The latter is fully compliant with the specification, and no adverse effects have been observed. It is the default.</p>" },
  { LEAF ("AssumeRtiHasPmdEndpoints"), 1, "false", ABSOFF (assume_rti_has_pmd_endpoints), 0, uf_boolean, 0, pf_boolean,
    "<p>This option assumes ParticipantMessageData endpoints required by the liveliness protocol are present in RTI participants even when not properly advertised by the participant discovery protocol.</p>" },
  END_MARKER
};

static const struct cfgelem unsupp_test_cfgelems[] = {
  { LEAF ("XmitLossiness"), 1, "0", ABSOFF (xmit_lossiness), 0, uf_int, 0, pf_int,
    "<p><b>Internal</b> This element controls the fraction of outgoing packets to drop, specified as a per mil.</p>" },
  END_MARKER
};

static const struct cfgelem unsupp_watermarks_cfgelems[] = {
  { LEAF ("WhcLow"), 1, "100", ABSOFF (whc_lowwater_mark), 0, uf_natint, 0, pf_int,
    "<p><b>Internal</b> This element sets the low-water mark for the DDSI2 WHCs, expressed in samples. A suspended writer resumes transmitting when its DDSI2 WHC shrinks to this size.</p>" },
  { LEAF ("WhcHigh"), 1, "400", ABSOFF (whc_highwater_mark), 0, uf_natint, 0, pf_int,
    "<p><b>Internal</b> This element sets the high-water mark for the DDSI2 WHCs, expressed in samples. A writer is suspended when the WHC reaches this size.</p>" },
  END_MARKER
};

static const struct cfgelem unsupp_cfgelems[] = {
  { LEAF ("MaxMessageSize"), 1, "4096 B", ABSOFF (max_msg_size), 0, uf_memsize, 0, pf_memsize,
    "<p><b>Internal</b> This element specifies the maximum size of the UDP payload that DDSI2 will generate. DDSI2 will try to maintain this limit within the bounds of the DDSI specification, which means that in some cases (especially for very low values of MaxMessageSize) larger payloads may sporadically be observed (currently up to 1192 B).</p>\n\
<p>On some networks it may be necessary to set this item to keep the packetsize below the MTU to prevent IP fragmentation. In those cases, it is generally advisable to also consider reducing Internal/FragmentSize.</p>" },
  { LEAF ("FragmentSize"), 1, "1280 B", ABSOFF (fragment_size), 0, uf_memsize, 0, pf_memsize,
    "<p><b>Internal</b> This element specifies the size of DDSI sample fragments generated by DDSI2. Samples larger than FragmentSize are fragmented into fragments of FragmentSize bytes each, except the last one, which may be smaller. The DDSI spec mandates a minimum fragment size of 1025 bytes, but DDSI2 will do whatever size is requested, accepting fragments of which the size is at least the minimum of 1025 and FragmentSize.</p>" },
  { LEAF ("DeliveryQueueMaxSamples"), 1, "256", ABSOFF (delivery_queue_maxsamples), 0, uf_natint, 0, pf_int,
    "<p><b>Internal</b> This element controls the Maximum size of a delivery queue, expressed in samples. Once a delivery queue is full, incoming samples destined for that queue are dropped until space becomes available again.</p>" },
  { LEAF ("PrimaryReorderMaxSamples"), 1, "64", ABSOFF (primary_reorder_maxsamples), 0, uf_natint, 0, pf_int,
    "<p><b>Internal</b> This element sets the maximum size in samples of a primary re-order administration. Each proxy writer has one primary re-order administration to buffer the packet flow in case some packets arrive out of order. Old samples are forwarded to secondary re-order administrations associated with readers in need of historical data.</p>" },
  { LEAF ("SecondaryReorderMaxSamples"), 1, "16", ABSOFF (secondary_reorder_maxsamples), 0, uf_natint, 0, pf_int,
    "<p><b>Internal</b> This element sets the maximum size in samples of a secondary re-order administration. The secondary re-order administration is per reader in need of historical data.</p>" },
  { LEAF ("DefragUnreliableMaxSamples"), 1, "4", ABSOFF (defrag_unreliable_maxsamples), 0, uf_natint, 0, pf_int,
    "<p><b>Internal</b> This element sets the maximum number of samples that can be defragmented simultaneously for a best-effort writers.</p>" },
  { LEAF ("DefragReliableMaxSamples"), 1, "16", ABSOFF (defrag_reliable_maxsamples), 0, uf_natint, 0, pf_int,
    "<p><b>Internal</b> This element sets the maximum number of samples that can be defragmneted simultaneously for a reliable writer. This has to be large enough to handle retransmissions of historical data in addition to new samples.</p>" },
  { LEAF ("BuiltinEndpointSet"), 1, "writers", ABSOFF (besmode), 0, uf_besmode, 0, pf_besmode,
    "<p><b>Internal</b> This element controls which participants will have which built-in endpoints for the discovery and liveliness protocols. Valid values are:\n\
<ul><li><i>full</i>: all participants have all endpoints;</li>\n\
<li><i>writers</i>: all participants have the writers, but just one has the readers;</li>\n\
<li><i>minimal</i>: only one participant has built-in endpoints.</li></ul>\n\
The default is <i>writers</i>, as this is thought to be compliant and reasonably efficient. <i>Minimal</i> may or may not be compliant but is most efficient, and <i>full</i> is inefficient but certain to be compliant. See also Internal/ConservativeBuiltinReaderStartup.</p>" },
  { LEAF ("AggressiveKeepLast1Whc"), 1, "false", ABSOFF (aggressive_keep_last1_whc), 0, uf_boolean, 0, pf_boolean,
    "<p><b>Internal</b> This element controls whether to drop a reliable sample from a DDSI2 WHC before all readers have acknowledged it as soon as a later sample becomes available. It only affects DCPS data writers with a history QoS setting of KEEP_LAST with depth 1. The default setting, <i>false</i>, mimics the behaviour of the OpenSplice RT networking and is necessary to make the behaviour of wait_for_acknowledgements() consistent across the networking services.</p>" },
  { LEAF ("ConservativeBuiltinReaderStartup"), 1, "false", ABSOFF (conservative_builtin_reader_startup), 0, uf_boolean, 0, pf_boolean,
    "<p><b>Internal</b> This element forces all DDSI2 built-in discovery-related readers to request all historical data, instead of just one for each \"topic\". There is no indication that any of the current DDSI implementations requires changing of this setting, but it is conceivable that an implementation might track which participants have been informed of the existence of endpoints and which have not been, refusing communication with those that have \"can't\" know.</p>\n\
<p>Should it be necessary to hide DDSI2's shared discovery behaviour, set this to <i>true</i> and Internal/BuiltinEndpointSet to <i>full</i>.</p>" },
  { LEAF ("MeasureHbToAckLatency"), 1, "false", ABSOFF (meas_hb_to_ack_latency), 0, uf_boolean, 0, pf_boolean,
    "<p><b>Internal</b> This element enables heartbeat-to-ack latency among DDSI2 services by prepending timestamps to Heartbeat and AckNack messages and calculating round trip times. This is non-standard behaviour. The measured latencies are quite noisy and are currently not used anywhere.</p>" },
  { LEAF ("SuppressSPDPMulticast"), 1, "false", ABSOFF (suppress_spdp_multicast), 0, uf_boolean, 0, pf_boolean,
    "<p><b>Internal</b> The element controls whether the mandatory multicasting of the participant discovery packets occurs. Completely disabling multicasting requires this element be set to <i>true</i>, and generally requires explicitly listing peers to ping for unicast discovery.</p>\n\
<p>See also General/AllowMulticast.</p>" },
  { LEAF ("UnicastResponseToSPDPMessages"), 1, "true", ABSOFF (unicast_response_to_spdp_messages), 0, uf_boolean, 0, pf_boolean,
    "<p><b>Internal</b> This element controls whether the response to a newly discovered participant is sent as a unicasted SPDP packet, instead of rescheduling the periodic multicasted one. There is no known benefit to setting this to <i>false</i>.</p>" },
  { LEAF ("SynchronousDeliveryPriorityThreshold"), 1, "0", ABSOFF (synchronous_delivery_priority_threshold), 0, uf_int, 0, pf_int,
    "<p><b>Internal</b> This element controls whether samples sent by a writer with QoS settings latency_budget <= SynchronousDeliveryLatencyBound and transport_priority greater than or equal to this element's value will be delivered synchronously from the \"recv\" thread, all others will be delivered asynchronously through delivery queues. This reduces latency at the expense of aggregate bandwidth.</p>" },
  { LEAF ("SynchronousDeliveryLatencyBound"), 1, "inf", ABSOFF (synchronous_delivery_latency_bound), 0, uf_duration_inf, 0, pf_duration,
    "<p><b>Internal</b> This element controls whether samples sent by a writer with QoS settings transport_priority >= SynchronousDeliveryPriorityThreshold and a latency_budget at most this element's value will be delivered synchronously from the \"recv\" thread, all others will be delivered asynchronously through delivery queues. This reduces latency at the expense of aggregate bandwidth.</p>" },
  { LEAF ("MaxParticipants"), 1, "0", ABSOFF (max_participants), 0, uf_natint, 0, pf_int,
    "<p><b>Internal</b> This elements configures the maximum number of DCPS domain participants this DDSI2 service instance is willing to service. 0 is unlimited.</p>" },
  { LEAF ("AccelerateRexmitBlockSize"), 1, "0", ABSOFF (accelerate_rexmit_block_size), 0, uf_natint, 0, pf_int,
    "<p><b>Internal</b> Proxy readers that are assumed to sill be retrieving historical data get this many samples retransmitted when they NACK something, even if some of these samples have sequence numbers outside the set covered by the NACK.</p>" },
  { LEAF ("ResponsivenessTimeout"), 1, "1 s", ABSOFF (responsiveness_timeout), 0, uf_duration_ms_1hr, 0, pf_duration,
    "<p><b>Internal</b> This element controls for how long an unresponsive reader can block the transmit thread by failing to acknowledge data when a writer's DDSI2 WHC is full. If after this time the writer's WHC has not shrunk to below the low-water mark, the reader is considered unresponsive and degraded to unreliable. It will be restored to reliable service once it resumes acknowledging ing samples.</p>" },
  { LEAF ("RetransmitMerging"), 1, "adaptive", ABSOFF (retransmit_merging), 0, uf_retransmit_merging, 0, pf_retransmit_merging,
    "<p><b>Internal</b> This elements controls the addressing and timing of retransmits. Possible values are:\n\
<ul><li><i>never</i>: retransmit only to the NACK-ing reader;</li>\n\
<li><i>adaptive</i>: attempt to combine retransmits needed for reliability, but send historical (transient-local) data to the requesting reader only;</li>\n\
<li><i>always</i>: do not distinguish between different causes, always try to merge.</li></ul>\n\
The default is <i>adaptive</i>. See also Internal/RetransmitMergingPeriod.</p>" },
  { LEAF ("RetransmitMergingPeriod"), 1, "5 ms", ABSOFF (retransmit_merging_period), 0, uf_duration_us_1s, 0, pf_duration,
    "<p><b>Internal</b> This setting determines the size of the time window in which a NACK of some sample is ignored because a retransmit of that sample has been multicasted too recently. This setting has no effect of unicasted retransmits.</p>\n\
<p>See also Internal/RetransmitMerging.</p>" },
  { LEAF ("MaxQueuedRexmitBytes"), 1, "0 B", ABSOFF (max_queued_rexmit_bytes), 0, uf_memsize, 0, pf_memsize,
    "<p><b>Internal</b> This setting limits the maximum number of bytes queued for retransmission. The default value of 0 is unlimited unless an AuxiliaryBandwidthLimit has been set, in which case it becomes NackDelay * AuxiliaryBandwidthLimit. It must be large enough to contain the largest sample that may need to be retransmitted.</p>" },
  { LEAF ("MaxQueuedRexmitMessages"), 1, "200", ABSOFF (max_queued_rexmit_msgs), 0, uf_natint, 0, pf_int,
    "<p><b>Internal</b> This settings limits the maximum number of samples queued for retransmission.</p>" },
  { LEAF ("LeaseDuration"), 1, "100 s", ABSOFF (lease_duration), 0, uf_duration_ms_1hr, 0, pf_duration,
    "<p><b>Internal</b> This setting controls the default participant lease duration.<p>" },
  { LEAF ("WriterLingerDuration"), 1, "1 s", ABSOFF (writer_linger_duration), 0, uf_duration_ms_1hr, 0, pf_duration,
    "<p><b>Internal</b> This setting controls the maximum duration for which actual deletion of a reliable writer with unacknowledged data in its history will be postponed to provide proper reliable transmission.<p>" },
  { LEAF ("MinimumSocketReceiveBufferSize"), 1, "64 KiB", ABSOFF (socket_min_rcvbuf_size), 0, uf_memsize, 0, pf_memsize,
    "<p><b>Internal</b> This setting controls the minimum size of socket receive buffers. This setting can only increase the size of the receive buffer, if the operating system by default creates a larger buffer, it is left unchanged.</p>" },
  { LEAF ("MinimumSocketSendBufferSize"), 1, "32 KiB", ABSOFF (socket_min_sndbuf_size), 0, uf_memsize, 0, pf_memsize,
    "<p><b>Internal</b> This setting controls the minimum size of socket send buffers. This setting can only increase the size of the send buffer, if the operating system by default creates a larger buffer, it is left unchanged.</p>" },
  { LEAF ("NackDelay"), 1, "0 s", ABSOFF (nack_delay), 0, uf_duration_ms_1hr, 0, pf_duration,
    "<p><b>Internal</b> This setting controls the delay between receipt of a HEARTBEAT indicating missing samples and a NACK (ignored when the HEARTBEAT requires an answer). However, no NACK is sent if a NACK had been scheduled already for a response earlier than the delay requests: then that NACK will incorporate the latest information.</p>"},
  { LEAF ("PreEmptiveAckDelay"), 1, "10 ms", ABSOFF (preemptive_ack_delay), 0, uf_duration_ms_1hr, 0, pf_duration,
    "<p><b>Internal</b> This setting controls the delay between the discovering a remote writer and sending a pre-emptive AckNack to discover the range of data available.</p>"},
  { LEAF ("ScheduleTimeRounding"), 1, "0 ms", ABSOFF (schedule_time_rounding), 0, uf_duration_ms_1hr, 0, pf_duration,
    "<p><b>Internal</b> This setting allows the timing of scheduled events to be rounded up so that more events can be handled in a single cycle of the event queue. The default is 0 and causes no rounding at all, i.e. are scheduled exactly, whereas a value of 10ms would mean that events are rounded up to the nearest 10 milliseconds.</p>"},
  { LEAF ("DDSI2DirectMaxThreads"), 1, "1", ABSOFF (ddsi2direct_max_threads), 0, uf_natint, 0, pf_int,
    "<p><b>Internal</b> This element sets the maximum number of extra threads for an experimental, undocumented and unsupported direct mode.</p>" },
  { LEAF ("SquashParticipants"), 1, "false", ABSOFF (squash_participants), 0, uf_boolean, 0, pf_boolean,
    "<p><b>Internal</b> This element controls whether DDSI2 advertises all the domain participants it serves in DDSI (when set to <i>false</i>), or rather only one domain participant (the one corresponding to the DDSI2 process; when set to <i>true</i>). In the latter case DDSI2 becomes the virtual owner of all readers and writers of all domain participants, dramatically reducing discovery traffic.</p>" },
  { LEAF ("LegacyFragmentation"), 1, "false", ABSOFF (buggy_datafrag_flags_mode), 0, uf_boolean, 0, pf_boolean,
    "<p><b>Internal</b> This option enables a backwards-compatible, non-compliant setting and interpretation of the control flags in fragmented data messages. To be enabled <i>only</i> when requiring interoperability between compliant and non-compliant versions of DDSI2 for large messages.</p>" },
  { LEAF ("SPDPResponseMaxDelay"), 1, "0 ms", ABSOFF (spdp_response_delay_max), 0, uf_duration_ms_1s, 0, pf_duration,
    "<p><b>Internal</b> Maximum pseudo-random delay in milliseocnds between discovering a remote participant and responding to it.</p>" },
  { LEAF ("LateAckMode"), 1, "false", ABSOFF (late_ack_mode), 0, uf_boolean, 0, pf_boolean,
    "<p><b>Internal</b> Ack a sample only when it has been delivered, instead of when committed to delivering it.</p>" },
  { LEAF ("ForwardAllMessages"), 1, "false", ABSOFF (forward_all_messages), 0, uf_boolean, 0, pf_boolean,
     "<p><b>Internal</b> Forward all messages from a writer, rather than trying to forward each sample only once. The default of trying to forward each sample only once filters out duplicates for writers in multiple partitions under nearly all circumstances, but may still publish the odd duplicate. Note: the current implementation also can lose in contrived test cases, that publish more than 2**32 samples using a single data writer in conjunction with carefully controlled management of the writer history via cooperating local readers.</p>" },
  { LEAF ("RetryOnRejectDuration"), 1, "default", ABSOFF (retry_on_reject_duration), 0, uf_maybe_duration_inf, 0, pf_maybe_duration,
    "<p><b>Internal</b> How long to keep locally retrying pushing a received sample into the reader caches when resource limits are reached. Default is dependent on Internal/LateAckMode: if the latter is false, it is 80% of Internal/ResponsivenessTimeout, otherwise it is 0.</p>" },
  { LEAF ("RetryOnRejectBestEffort"), 1, "false", ABSOFF (retry_on_reject_besteffort), 0, uf_boolean, 0, pf_boolean,
    "<p><b>Internal</b> Whether or not to locally retry pusing a received best-effort sample into the reader caches when resource limits are reached.</p>" },
  { LEAF ("GenerateKeyhash"), 1, "true", ABSOFF (generate_keyhash), 0, uf_boolean, 0, pf_boolean,
    "<p><b>Internal</b> When true, include keyhashes in outgoing data for topics with keys.</p>" },
  { LEAF ("MaxSampleSize"), 1, "2147483647 B", ABSOFF (max_sample_size), 0, uf_memsize, 0, pf_memsize,
    "<p><b>Internal</b> This setting controls the maximum (CDR) serialised size of samples that DDSI2 will forward in either direction. Samples larger than this are discarded with a warning.</p>" },
  { GROUP ("Test", unsupp_test_cfgelems), "<p><b>Internal</b> Testing options.</p>" },
  { GROUP ("Watermarks", unsupp_watermarks_cfgelems), "<p><b>Internal</b> Watermarks for flow-control.</p>" },
  END_MARKER
};

static const struct cfgelem sizing_cfgelems[] = {
  { LEAF ("ReceiveBufferSize"), 1, "1 MiB", ABSOFF (rbuf_size), 0, uf_memsize, 0, pf_memsize,
    "<p>This element sets the size of a single receive buffer. Many receive buffers may be needed. Their size must be greater than ReceiveBufferChunkSize by a modest amount.</p>" },
  { LEAF ("ReceiveBufferChunkSize"), 1, "128 KiB", ABSOFF (rmsg_chunk_size), 0, uf_memsize, 0, pf_memsize,
    "<p>Size of one allocation unit in the receive buffer. Must be greater than the maximum packet size by a modest amount (too large packets are dropped). Each allocation is shrunk immediately after processing a message, or freed straightaway.</p>" },
  { LEAF ("LocalEndpoints"), 1, "1000", ABSOFF (gid_hash_softlimit), 0, uf_int, 0, pf_int,
    "<p>This element specifies the expected maximum number of endpoints local to one DDSI2 service. Underestimating this number will have a significant performance impact, but will not affect correctness; signficantly overestimating it will cause more memory to be used than necessary.</p>" },
  { LEAF ("EndpointsInSystem"), 1, "20000", ABSOFF (guid_hash_softlimit), 0, uf_int, 0, pf_int,
    "<p>This endpoint specifies the expected maximum number of endpoints in the network. Underestimating this number will have a significant performance impact, but will not affect correctness; signficantly overestimating it will cause more memory to be used than necessary.</p>" },
  { LEAF ("NetworkQueueSize"), 1, "1000", ABSOFF (nw_queue_size), 0, uf_int, 0, pf_int,
    "<p>This element specifies the maximum number of samples in the network queue. Write/dispose operations add samples to this queue, the DDSI2 service drains it. Larger values allow large bursts of writes to occur without forcing synchronization between the application and the DDSI2 service, but do introduce the potential for longer latencies and increase the maximum amount of memory potentially occupied by messages in the queue.</p>" },
  END_MARKER
};

static const struct cfgelem discovery_ports_cfgelems[] = {
  { LEAF ("Base"), 1, "7400", ABSOFF (port_base), 0, uf_port, 0, pf_int,
    "<p>This element specifies the base port number (refer to the DDSI 2.1 specification, section 9.6.1, constant PB).</p>" },
  { LEAF ("DomainGain"), 1, "250", ABSOFF (port_dg), 0, uf_int, 0, pf_int,
    "<p>This element specifies the domain gain, relating domain ids to sets of port numbers (refer to the DDSI 2.1 specification, section 9.6.1, constant DG).</p>" },
  { LEAF ("ParticipantGain"), 1, "2", ABSOFF (port_pg), 0, uf_int, 0, pf_int,
    "<p>This element specifies the participant gain, relating p0, articipant index to sets of port numbers (refer to the DDSI 2.1 specification, section 9.6.1, constant PG).</p>" },
  { LEAF ("MulticastMetaOffset"), 1, "0", ABSOFF (port_d0), 0, uf_int, 0, pf_int,
    "<p>This element specifies the port number for multicast meta traffic (refer to the DDSI 2.1 specification, section 9.6.1, constant d0).</p>" },
  { LEAF ("UnicastMetaOffset"), 1, "10", ABSOFF (port_d1), 0, uf_int, 0, pf_int,
    "<p>This element specifies the port number for unicast meta traffic (refer to the DDSI 2.1 specification, section 9.6.1, constant d1).</p>" },
  { LEAF ("MulticastDataOffset"), 1, "1", ABSOFF (port_d2), 0, uf_int, 0, pf_int,
    "<p>This element specifies the port number for multicast meta traffic (refer to the DDSI 2.1 specification, section 9.6.1, constant d2).</p>" },
  { LEAF ("UnicastDataOffset"), 1, "11", ABSOFF (port_d3), 0, uf_int, 0, pf_int,
    "<p>This element specifies the port number for unicast meta traffic (refer to the DDSI 2.1 specification, section 9.6.1, constant d3).</p>" },
  END_MARKER
};

static const struct cfgelem tcp_cfgelems[] = {
  { LEAF ("Enable"), 1, "false", ABSOFF (tcp_enable), 0, uf_boolean, 0, pf_boolean,
    "<p>This element enables the optional TCP transport.</p>" },
  { LEAF ("NoDelay"), 1, "false", ABSOFF (tcp_nodelay), 0, uf_boolean, 0, pf_boolean,
    "<p>This element enables the TCP_NODELAY socket option, preventing multiple DDSI messages being sent in the same TCP request.</p>" },
  { LEAF ("Port"), 1, "-1", ABSOFF (tcp_port), 0, uf_dyn_port, 0, pf_int,
    "<p>This element specifies the port number used for DDSI discovery (the TCP listening port). Dynamically allocated if zero. Disabled if -1 or not configured.</p>" },
  { LEAF ("Locators"), 1, "local", ABSOFF (tcp_locators), 0, uf_locators, 0, pf_locators,
    "<p>This element specifies what endpoints should be placed in unicast locators (local or none). If set as none, no unicast locators will be advertised via DDSI discovery so peers will use the discovery connection for communication. The default value is local which means that the listener endpoint is advertised so peers will use this to establish a new connection back to the process.</p>" },
  END_MARKER
};

static const struct cfgelem tp_cfgelems[] = {
  { LEAF ("Enable"), 1, "false", ABSOFF (tp_enable), 0, uf_boolean, 0, pf_boolean,
    "<p>This element enables the optional thread pool.</p>" },
  { LEAF ("Threads"), 1, "4", ABSOFF (tp_threads), 0, uf_natint, 0, pf_int,
    "<p>This elements configures the initial number of threads in the thread pool.</p>" },
  { LEAF ("ThreadMax"), 1, "8", ABSOFF (tp_max_threads), 0, uf_natint, 0, pf_int,
    "<p>This elements configures the maximum number of threads in the thread pool.</p>" },
  END_MARKER
};

static const struct cfgelem discovery_peer_cfgattrs[] = {
  { ATTR ("Address"), 1, NULL, RELOFF (config_peer_listelem, peer), 0, uf_ipv4, ff_free, pf_string,
    "<p>This element specifies an IP address to which discovery packets must be sent, in addition to the default multicast address (see also General/AllowMulticast and Internal/SuppressSPDPMulticast). Both a hostnames and a numerical IP address is accepted; the hostname or IP address may be suffixed with :PORT to explicitly set the port to which it must be sent. Multiple Peers may be specified.</p>" },
  END_MARKER
};

static const struct cfgelem discovery_peers_cfgelems[] = {
  { MGROUP ("Peer", NULL, discovery_peer_cfgattrs), 0, NULL, ABSOFF (peers), if_peer, 0, 0, 0,
    "<p>This element statically configures addresses for discovery.</p>" },
  END_MARKER
};

static const struct cfgelem discovery_cfgelems[] = {
  { LEAF ("DomainId"), 1, "default", ABSOFF (discoveryDomainId), 0, uf_maybe_int32, 0, pf_maybe_int32,
    "<P>This element allows overriding of the DDS Domain Id that is used for this DDSI2 service.</p>" },
  { GROUP ("Peers", discovery_peers_cfgelems),
    "<p>This element statically configures addresses of discovery.</p>" },
  { LEAF ("ParticipantIndex"), 1, "auto", ABSOFF (participantIndex), 0, uf_participantIndex, 0, pf_participantIndex,
    "<p>This element specifies the DDSI participant index used by this instance of the DDSI2 service for discovery purposes. Only one such participant id is used, independent of the number of actual DomainParticipants on the node. It is either:\n\
<ul><li><i>auto</i>: which will attempt to automatically determine an available participant index, or</li>\n\
<li>a non-negative integer less than 120, or</li>\n\
<li><i>none</i>:, which causes it to use arbitrary port numbers for unicast sockets which entirely removes the constraints on the participant index but makes unicast discovery impossible.</li></ul>\n\
The default is <i>auto</i>. The participant index is part of the port number calculation and if predictable port numbers are needed and fixing the participant index has no adverse effects, it is recommended that the second be option be used.</p>" },
  { LEAF ("SPDPMulticastAddress"), 1, "239.255.0.1", ABSOFF (spdpMulticastAddressString), 0, uf_ipv4, ff_free, pf_string,
    "<p>This element specifies the multicast address that is used as the destination for the participant discovery packets. In IPv4 mode the default is the (standardised) 239.255.0.1, in IPv6 mode it becomes ff02::ffff:239.255.0.1, which is a non-standardised link-local multicast address.</p>" },
  { LEAF ("SPDPInterval"), 1, "30 s", ABSOFF (spdp_interval), 0, uf_duration_ms_1hr, 0, pf_duration,
    "<p>This element specifies the interval between spontaneous transmissions of participant discovery packets.</p>" },
  { GROUP ("Ports", discovery_ports_cfgelems),
    "<p>The Ports element allows specifying various parameters related to the port numbers used for discovery. These all have default values specified by the DDSI 2.1 specification and rarely need to be changed.</p>" },
  END_MARKER
};

static const struct cfgelem tracing_cfgelems[] = {
  { LEAF ("EnableCategory"), 1, "", 0, 0, 0, uf_logcat, 0, pf_logcat,
    "<p>This element enables individual logging categories. These are enabled in addition to those enabled by Tracing/Verbosity. Recognised categories are:\n\
<ul><li><i>fatal</i>: all fatal errors, errors causing immediate termination</li>\n\
<li><i>error</i>: failures probably impacting correctness but not necessarily causing immediate termination</li>\n\
<li><i>warning</i>: abnormal situations that will likely not impact correctness</li>\n\
<li><i>config</i>: full dump of the configuration</li>\n\
<li><i>info</i>: general informational notices</li>\n\
<li><i>discovery</i>: all discovery activity</li>\n\
<li><i>data</i>: include data content of samples in traces</li>\n\
<li><i>radmin</i>: receive buffer administration</li>\n\
<li><i>timing</i>: periodic reporting of CPU loads per thread</li>\n\
<li><i>traffic</i>: periodic reporting of total outgoing data</li></ul>\n\
In addition, there is the keyword <i>trace</i> that enables all but <i>radmin</i></p>.\n\
<p>The categorisation of tracing output is incomplete and hence most of the verbosity levels and categories are not of much use in the current release. This is an ongoing process and here we describe the target situation rather than the current situation. Currently, the most useful is <i>trace</i>.</p>" },
  { LEAF ("Verbosity"), 1, "none", 0, 0, 0, uf_verbosity, 0, pf_nop,
    "<p>This element enables standard groups of categories, based on a desired verbosity level. This is in addition to the categories enabled by the Tracing/EnableCategory setting. Recognised verbosity levels and the categories they map to are:\n\
<ul><li><i>none</i>: no DDSI2 log</li>\n\
<li><i>severe</i>: error and fatal</li>\n\
<li><i>warning</i>: <i>severe</i> + warning</li>\n\
<li><i>info</i>: equivalent to <i>warning</i></li>\n\
<li><i>config</i>: <i>info</i> + config</li>\n\
<li><i>fine</i>: <i>config</i> + discovery</li>\n\
<li><i>finer</i>: <i>fine</i> + traffic, timing & info</li>\n\
<li><i>finest</i>: <i>finer</i> + trace</li></ul>\n\
While <i>none</i>prevents any message from being written to a DDSI2 log file, warnings and errors are still logged in the ospl-info.log and ospl-error.log files.</p>\n\
<p>The categorisation of tracing output is incomplete and hence most of the verbosity levels and categories are not of much use in the current release. This is an ongoing process and here we describe the target situation rather than the current situation. Currently, the most useful verbosity levels are <i>config</i> and <i>finest</i>.</p>" },
  { LEAF ("OutputFile"), 1, "ddsi2.log", ABSOFF (tracingOutputFileName), 0, uf_tracingOutputFileName, ff_free, pf_string,
    "<p>This option specifies where the logging is printed to. Note that <i>stdout</i> and <i>stderr</i> are treated as special values, representing \"standard out\" and \"standard error\" respectively. No file is created unless logging categories are enabled using the Tracing/Verbosity or Tracing/EnabledCategory settings.</p>" },
  { LEAF_W_ATTRS ("Timestamps", timestamp_cfgattrs), 1, "true", ABSOFF (tracingTimestamps), 0, uf_boolean, 0, pf_boolean,
    "<p>This option has no effect.</p>" },
  { LEAF ("AppendToFile"), 1, "false", ABSOFF (tracingAppendToFile), 0, uf_boolean, 0, pf_boolean,
    "<p>This option specifies whether the output is to be appended to an existing log file. The default is to create a new log file each time, which is generally the best option if a detailed log is generated.</p>" },
  { LEAF ("PacketCaptureFile"), 1, "", ABSOFF (pcap_file), 0, uf_string, ff_free, pf_string,
    "<p>This option specifies the file to which received and sent packets will be logged in the \"pcap\" format suitable for analysis using common networking tools, such as WireShark. IP and UDP headers are ficitious, in particular the destination address of received packets. The TTL may be used to distinguish between sent and received packets: it is 255 for sent packets and 128 for received ones. Currently IPv4 only.</p>" },
  END_MARKER
};

static const struct cfgelem sched_prio_cfgattrs[] = {
  { ATTR ("priority_kind"), 1, "relative", ABSOFF (watchdog_sched_priority_class), 0, uf_sched_prio_class, 0, pf_sched_prio_class,
    "<p>This attribute specifies whether the specified Priority is a relative or absolute priority.</p>" },
  END_MARKER
};

static const struct cfgelem sched_cfgelems[] = {
  { LEAF ("Class"), 1, "default", ABSOFF (watchdog_sched_class), 0, uf_sched_class, 0, pf_sched_class,
    "<p>This element specifies the thread scheduling class that will be used by the watchdog thread. The user may need the appropriate privileges from the underlying operating system to be able to assign some of the privileged scheduling classes.</p>" },
  { LEAF_W_ATTRS ("Priority", sched_prio_cfgattrs), 1, "0", ABSOFF (watchdog_sched_priority), 0, uf_int32, 0, pf_int32,
    "<p>This element specifies the thread priority. Only priorities that are supported by the underlying operating system can be assigned to this element. The user may need special privileges from the underlying operating system to be able to assign some of the privileged priorities.</p>" },
  END_MARKER
};

static const struct cfgelem watchdog_cfgelems[] = {
  { GROUP ("Scheduling", sched_cfgelems),
    "<p>This element specifies the type of OS scheduling class will be used by the thread that announces its liveliness periodically.</p>" },
  END_MARKER
};

static const struct cfgelem unsupp_ignore_cfgelems[] = {
  WILDCARD,
  END_MARKER
};

/* Note: adding "Unsupported" with NULL for a description hides it
   from the configurator */
static const struct cfgelem ddsi2_cfgelems[] = {
  { GROUP ("General", general_cfgelems),
    "<p>The General element specifies overall DDSI2 service settings.</p>" },
  { GROUP ("Threads", threads_cfgelems),
    "<p>This element is used to set thread properties.</p>" },
  { GROUP ("Sizing", sizing_cfgelems),
    "<p>The Sizing element specifies a variety of configuration settings dealing with expected system sizes, buffer sizes, &c.</p>" },
  { GROUP ("Compatibility", compatibility_cfgelems),
    "<p>The Compatibility elements allows specifying various settings related to compatability with standards and with other DDSI implementations.</p>" },
  { GROUP ("Discovery", discovery_cfgelems),
    "<p>The Discovery element allows specifying various parameters related to the discovery of peers.</p>" },
  { GROUP ("Tracing", tracing_cfgelems),
    "<p>The Tracing element controls the amount and type of information that is written into the tracing log by the DDSI service. This is useful to track the DDSI service during application development.</p>" },
  { MGROUP ("Internal", unsupp_cfgelems, NULL), 1, NULL, 0, 0, if_internal, 0, 0, 0,
    "<p>The Internal elements deal with a variety of settings that are still evolving.</p>" },
  { GROUP ("Unsupported", unsupp_ignore_cfgelems), NULL },
  { GROUP ("Watchdog", watchdog_cfgelems),
    "<p>This element specifies the type of OS scheduling class will be used by the thread that announces its liveliness periodically.</p>" },
  { GROUP ("TCP", tcp_cfgelems),
    "<p>The TCP element allows specifying various parameters related to running DDSI over TCP.</p>" },
  { GROUP ("ThreadPool", tp_cfgelems),
    "<p>The ThreadPool element allows specifying various parameters related to ing a thread pool for sending DDSI messages</p>" },
  END_MARKER
};

/* Note: using 2e-1 instead of 0.2 to avoid use of the decimal
   separator, which is locale dependent. */
static const struct cfgelem lease_expiry_time_cfgattrs[] = {
  { ATTR ("update_factor"), 1, "2e-1", ABSOFF (servicelease_update_factor), 0, uf_float, 0, pf_float, NULL },
  END_MARKER
};

static const struct cfgelem lease_cfgelems[] = {
  { LEAF_W_ATTRS ("ExpiryTime", lease_expiry_time_cfgattrs), 1, "10", ABSOFF (servicelease_expiry_time), 0, uf_float, 0, pf_float, NULL },
  END_MARKER
};

static const struct cfgelem domain_cfgelems[] = {
  { GROUP ("Lease", lease_cfgelems), NULL },
  { LEAF ("Id"), 1, "0", ABSOFF (domainId), 0, uf_domainId, 0, pf_int, NULL },
  WILDCARD,
  END_MARKER
};

static const struct cfgelem ddsi2_cfgattrs[] = {
  { ATTR ("name"), 1, NULL, ABSOFF (servicename), 0, uf_service_name, ff_free, pf_string,
    "<p>This attribute identifies the configuration for the DDSI2 Service. Multiple DDSI2 service configurations can be specified in one single resource. The actual applicable configuration is determined by the value of the name attribute, which must match the specified under the element OpenSplice/Domain/Service[@name] in the Domain Service configuration.</p>" },
  END_MARKER
};

static const struct cfgelem root_cfgelems[] = {
  { "DDSI2Service", ddsi2_cfgelems, ddsi2_cfgattrs, NODATA,
    "<p>The root element of a DDSI2 networking service configuration.</p>" },
  { "Lease", lease_cfgelems, NULL, NODATA, NULL },
  { "Domain", domain_cfgelems, NULL, NODATA, NULL },
  END_MARKER
};


/* Legacy "Unsupported" handling.  Aliases are supported by DDSI2
   (such as Internal|Unsupported), but too hard for the configurator
   tool to deal with.

   As an alternative way of providing an interface that is at least
   somewhat reasonable, we now process the configuration ignoring any
   "Unsupported" (but processing "Internal"), and only then start
   looking at "Unsupported" items.

   There're multiple issues, each with multiple reasonable approaches:

     1. one option is to always warn when we encounter Unsupported,
        the other is to warn only when it exists in conjunction with
        Internal;

     2. one is to ignore whatever configuration settings are in the
        Unsupported category if Internal is present, the other is to
        process them; assuming we process them, it seems reasonable to
        accept settings in Unsupported but to never let them override
        those set in Internal;

     3. one is to warn about possible duplicates, the other is to
        treat them as errors.

   For (1), we warn when we encounter both.  We have told quite a few
   customers to use some Unsupported settings, and this option allows
   them to upgrade OpenSplice without touching the configuration file.
   But IF they modify it with the configurator and add an Internal
   section, they will get a warning.

   For (2), we do interpret Unsupported.  This is because it allows
   adding options using the configurator without invalidating the
   configuration file.

   For (3), duplicates we treat as errors.  There are no customers
   using Internal yet, we really want them to change, and this is by
   far the most effective way.

   We do this by having "Unsupported" part of the normal
   configuration, but with a WILDCARD for its contents (so that it
   skips everything), and by introducing a second set of configuration
   options that aliases the Internal ones, but are accessed via
   "Unsupported".

   We can't use the aliasing capability of DDSI2 because of the
   warning behaviour, but instead we define an init function that
   generates the correct warning.

   Finally, errors on duplicate entries is the default behaviour for
   DDSI2.  */
static const struct cfgelem legacy_ddsi2_cfgelems[] = {
  { MGROUP ("Unsupported", unsupp_cfgelems, NULL), 1, NULL, 0, 0, if_unsupported, 0, 0, 0, NULL },
  WILDCARD,
  END_MARKER
};

static const struct cfgelem legacy_root_cfgelems[] = {
  { "DDSI2Service", legacy_ddsi2_cfgelems, NULL, NODATA, NULL },
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

static const struct unit unittab_duration[] = {
  { "ns", 1 },
  { "us", 1000 },
  { "ms", T_MILLISECOND },
  { "s", T_SECOND },
  { "min", 60 * T_SECOND },
  { "hr", 3600 * T_SECOND },
  { "day", 24 * 3600 * T_SECOND },
  { NULL, 0 }
};

/* note: order affects whether printed as KiB or kB, for consistency
   with bandwidths and pedanticness, favour KiB. */
static const struct unit unittab_memsize[] = {
  { "B", 1 },
  { "KiB", 1024 },
  { "kB", 1024 },
  { "MiB", 1048576 },
  { "MB", 1048576 },
  { "GiB", 1073741824 },
  { "GB", 1073741824 },
  { NULL, 0 }
};


static void cfgst_push (struct cfgst *cfgst, int isattr, const struct cfgelem *elem, void *parent)
{
  assert (cfgst->path_depth < MAX_PATH_DEPTH);
  assert (isattr == 0 || isattr == 1);
  cfgst->isattr[cfgst->path_depth] = isattr;
  cfgst->path[cfgst->path_depth] = elem;
  cfgst->parent[cfgst->path_depth] = parent;
  cfgst->path_depth++;
}

static void cfgst_pop (struct cfgst *cfgst)
{
  assert (cfgst->path_depth > 0);
  cfgst->path_depth--;
}


struct cfg_note_buf {
  int bufpos;
  int bufsize;
  char *buf;
};

static int cfg_note_vsnprintf (struct cfg_note_buf *bb, const char *fmt, va_list ap)
{
  int x;
  x = os_vsnprintf (bb->buf + bb->bufpos, bb->bufsize - bb->bufpos, fmt, ap);
  if (x >= bb->bufsize - bb->bufpos)
  {
    int nbufsize = ((bb->bufsize + x+1) + 1023) & -1024;
    char *nbuf = os_realloc (bb->buf, nbufsize);
    if (nbuf == NULL)
      NN_FATAL0 ("out of memory when formatting an error message concerning the configuration\n");
    bb->buf = nbuf;
    bb->bufsize = nbufsize;
    return nbufsize;
  }
  if (x < 0)
    NN_FATAL0 ("cfg_note_vsnprintf: os_vsnprintf failed\n");
  else
    bb->bufpos += x;
  return 0;
}

static void cfg_note_snprintf (struct cfg_note_buf *bb, const char *fmt, ...)
{
  /* The reason the 2nd call to os_vsnprintf is here and not inside
     cfg_note_vsnprintf is because I somehow doubt that all platforms
     implement va_copy() */
  va_list ap;
  int r;
  va_start (ap, fmt);
  r = cfg_note_vsnprintf (bb, fmt, ap);
  va_end (ap);
  if (r > 0)
  {
    va_start (ap, fmt);
    r = os_vsnprintf (bb->buf + bb->bufpos, bb->bufsize - bb->bufpos, fmt, ap);
    if (r < 0 || r >= bb->bufsize - bb->bufpos)
      NN_FATAL0 ("cfg_note_snprintf: os_vsnprintf failed\n");
    va_end (ap);
    bb->bufpos += r;
  }
}

static int cfg_note (struct cfgst *cfgst, logcat_t cat, int bsz, const char *fmt, va_list ap)
{
  /* Have to snprintf our way to a single string so we can OS_REPORT
     as well as nn_log.  Otherwise configuration errors will be lost
     completely on platforms where stderr doesn't actually work for
     outputting error messages (this includes Windows because of the
     way "ospl start" does its thing). */
  struct cfg_note_buf bb;
  int i, r;

  bb.bufpos = 0;
  bb.bufsize = (bsz == 0) ? 1024 : bsz;
  if ((bb.buf = os_malloc (bb.bufsize)) == NULL)
    NN_FATAL0 ("cfg_note: out of memory\n");

  cfg_note_snprintf (&bb, "config: ");

  /* Path to element/attribute causing the error. Have to stop once an
     attribute is reached: a NULL marker may have been pushed onto the
     stack afterward in the default handling. */
  for (i = 0; i < cfgst->path_depth && (i == 0 || !cfgst->isattr[i-1]); i++)
  {
    if (cfgst->path[i] == NULL)
    {
      assert (i > 0);
      cfg_note_snprintf (&bb, "/#text");
    }
    else if (cfgst->isattr[i])
    {
      cfg_note_snprintf (&bb, "[@%s]", cfgst->path[i]->name);
    }
    else
    {
      const char *p = strchr (cfgst->path[i]->name, '|');
      int n = p ? (int) (p - cfgst->path[i]->name) : (int) strlen (cfgst->path[i]->name);
      cfg_note_snprintf (&bb, "%s%*.*s", (i == 0) ? "" : "/", n, n, cfgst->path[i]->name);
    }
  }

  cfg_note_snprintf (&bb, ": ");
  if ((r = cfg_note_vsnprintf (&bb, fmt, ap)) > 0)
  {
    /* Can't reset ap ... and va_copy isn't widely available - so
       instead abort and hope the caller tries again with a larger
       initial buffer */
    os_free (bb.buf);
    return r;
  }

  switch (cat)
  {
    case LC_CONFIG:
      nn_log (cat, "%s\n", bb.buf);
      break;
    case LC_WARNING:
      NN_WARNING1 ("%s\n", bb.buf);
      break;
    case LC_ERROR:
      NN_ERROR1 ("%s\n", bb.buf);
      break;
    default:
      NN_FATAL2 ("cfg_note unhandled category %u for message %s\n", (unsigned) cat, bb.buf);
      break;
  }

  os_free (bb.buf);
  return 0;
}

#if WARN_DEPRECATED_ALIAS || WARN_DEPRECATED_UNIT
static void cfg_warning (struct cfgst *cfgst, const char *fmt, ...)
{
  va_list ap;
  int bsz = 0;
  do {
    va_start (ap, fmt);
    bsz = cfg_note (cfgst, LC_WARNING, bsz, fmt, ap);
    va_end (ap);
  } while (bsz > 0);
}
#endif

static int cfg_error (struct cfgst *cfgst, const char *fmt, ...)
{
  va_list ap;
  int bsz = 0;
  do {
    va_start (ap, fmt);
    bsz = cfg_note (cfgst, LC_ERROR, bsz, fmt, ap);
    va_end (ap);
  } while (bsz > 0);
  return 0;
}

static int cfg_log (struct cfgst *cfgst, const char *fmt, ...)
{
  va_list ap;
  int bsz = 0;
  do {
    va_start (ap, fmt);
    bsz = cfg_note (cfgst, LC_CONFIG, bsz, fmt, ap);
    va_end (ap);
  } while (bsz > 0);
  return 0;
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

static os_int64 lookup_multiplier (struct cfgst *cfgst, const struct unit *unittab, const char *value, int unit_pos, int value_is_zero, os_int64 def_mult, int err_on_unrecognised)
{
  assert (0 <= unit_pos && (os_size_t) unit_pos <= strlen (value));
  while (value[unit_pos] == ' ')
    unit_pos++;
  if (value[unit_pos] == 0)
  {
    if (value_is_zero)
    {
      /* No matter what unit, 0 remains just that.  For convenience,
         always allow 0 to be specified without a unit */
      return 1;
    }
    else if (def_mult == 0 && err_on_unrecognised)
    {
      cfg_error (cfgst, "%s: unit is required", value);
      return 0;
    }
    else
    {
#if WARN_DEPRECATED_UNIT
      cfg_warning (cfgst, "%s: use of default unit is deprecated", value);
#endif
      return def_mult;
    }
  }
  else
  {
    int i;
    for (i = 0; unittab[i].name != NULL; i++)
    {
      if (strcmp (unittab[i].name, value + unit_pos) == 0)
        return unittab[i].multiplier;
    }
    if (err_on_unrecognised)
      cfg_error (cfgst, "%s: unrecognised unit", value + unit_pos);
    return 0;
  }
}

static void *cfg_address (UNUSED_ARG (struct cfgst *cfgst), void *parent, struct cfgelem const * const cfgelem)
{
  assert (cfgelem->multiplicity == 1);
  return (char *) parent + cfgelem->elem_offset;
}

static void *cfg_deref_address (UNUSED_ARG (struct cfgst *cfgst), void *parent, struct cfgelem const * const cfgelem)
{
  assert (cfgelem->multiplicity != 1);
  return *((void **) ((char *) parent + cfgelem->elem_offset));
}

static int if_internal (struct cfgst *cfgst, UNUSED_ARG (void *parent), UNUSED_ARG (struct cfgelem const * const cfgelem))
{
  cfgst->internal_seen = 1;
  return 0;
}

static int if_unsupported (struct cfgst *cfgst, UNUSED_ARG (void *parent), UNUSED_ARG (struct cfgelem const * const cfgelem))
{
#if WARN_DEPRECATED_ALIAS
  if (cfgst->internal_seen)
    cfg_warning (cfgst, "'Unsupported': deprecated alias for 'Internal'");
#else
  (void) cfgst;
#endif
  return 0;
}

static void *if_common (UNUSED_ARG (struct cfgst *cfgst), void *parent, struct cfgelem const * const cfgelem, unsigned size)
{
  struct config_listelem **current = (struct config_listelem **) ((char *) parent + cfgelem->elem_offset);
  struct config_listelem *new = os_malloc (size);
  if (new == NULL)
  {
    cfg_error (cfgst, "out of memory allocating a new element");
    NN_FATAL0 ("out of memory\n");
  }
  else
  {
    new->next = *current;
    *current = new;
  }
  return new;
}

static int if_thread_properties (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem)
{
  struct config_thread_properties_listelem *new = if_common (cfgst, parent, cfgelem, sizeof (*new));
  if (new == NULL)
    return -1;
  new->name = NULL;
  return 0;
}




static int if_peer (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem)
{
  struct config_peer_listelem *new = if_common (cfgst, parent, cfgelem, sizeof (struct config_peer_listelem));
  if (new == NULL)
    return -1;
  new->peer = NULL;
  return 0;
}

static void ff_free (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem)
{
  void **elem = cfg_address (cfgst, parent, cfgelem);
  os_free (*elem);
}

static int uf_boolean (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
  static const char *vs[] = { "false", "true", NULL };
  int *elem = cfg_address (cfgst, parent, cfgelem);
  int idx = list_index (vs, value);
  if (idx < 0)
    return cfg_error (cfgst, "'%s': undefined value", value);
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
    int *elem = cfg_address (cfgst, parent, cfgelem);
    *elem = ! *elem;
    return 1;
  }
}

static int uf_logcat (struct cfgst *cfgst, UNUSED_ARG (void *parent), UNUSED_ARG (struct cfgelem const * const cfgelem), UNUSED_ARG (int first), const char *value)
{
  static const char **vs = logcat_names;
  static const logcat_t *lc = logcat_codes;
  char *copy = os_strdup (value), *cursor = copy, *tok;
  if (copy == NULL)
    return cfg_error (cfgst, "out of memory");
  while ((tok = os_strsep (&cursor, ",")) != NULL)
  {
    int idx = list_index (vs, tok);
    if (idx < 0)
    {
      int ret = cfg_error (cfgst, "'%s' in '%s' undefined", tok, value);
      os_free (copy);
      return ret;
    }
    enabled_logcats |= lc[idx];
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
    LC_ALLCATS, LC_TRAFFIC | LC_TIMING | LC_INFO, LC_DISCOVERY, LC_CONFIG, 0, LC_WARNING, LC_ERROR | LC_FATAL, 0, 0
  };
  int idx = list_index (vs, value);
  assert (sizeof (vs) / sizeof (*vs) == sizeof (lc) / sizeof (*lc));
  if (idx < 0)
    return cfg_error (cfgst, "'%s': undefined value", value);
  else
  {
    int i;
    for (i = (int) (sizeof (vs) / sizeof (*vs)) - 1; i >= idx; i--)
      enabled_logcats |= lc[i];
    return 1;
  }
}

static int uf_besmode (struct cfgst *cfgst, UNUSED_ARG (void *parent), UNUSED_ARG (struct cfgelem const * const cfgelem), UNUSED_ARG (int first), const char *value)
{
  static const char *vs[] = {
    "full", "writers", "minimal", NULL
  };
  static const enum besmode ms[] = {
    BESMODE_FULL, BESMODE_WRITERS, BESMODE_MINIMAL, 0,
  };
  int idx = list_index (vs, value);
  enum besmode *elem = cfg_address (cfgst, parent, cfgelem);
  assert (sizeof (vs) / sizeof (*vs) == sizeof (ms) / sizeof (*ms));
  if (idx < 0)
    return cfg_error (cfgst, "'%s': undefined value", value);
  *elem = ms[idx];
  return 1;
}

static void pf_besmode (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  enum besmode *p = cfg_address (cfgst, parent, cfgelem);
  const char *str = "INVALID";
  switch (*p)
  {
    case BESMODE_FULL: str = "full"; break;
    case BESMODE_WRITERS: str = "writers"; break;
    case BESMODE_MINIMAL: str = "minimal"; break;
  }
  cfg_log (cfgst, "%s%s", str, is_default ? " [def]" : "");
}

static int uf_retransmit_merging (struct cfgst *cfgst, UNUSED_ARG (void *parent), UNUSED_ARG (struct cfgelem const * const cfgelem), UNUSED_ARG (int first), const char *value)
{
  static const char *vs[] = {
    "never", "adaptive", "always", NULL
  };
  static const enum retransmit_merging ms[] = {
    REXMIT_MERGE_NEVER, REXMIT_MERGE_ADAPTIVE, REXMIT_MERGE_ALWAYS, 0,
  };
  int idx = list_index (vs, value);
  enum retransmit_merging *elem = cfg_address (cfgst, parent, cfgelem);
  assert (sizeof (vs) / sizeof (*vs) == sizeof (ms) / sizeof (*ms));
  if (idx < 0)
    return cfg_error (cfgst, "'%s': undefined value", value);
  *elem = ms[idx];
  return 1;
}

static void pf_retransmit_merging (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  enum retransmit_merging *p = cfg_address (cfgst, parent, cfgelem);
  const char *str = "INVALID";
  switch (*p)
  {
    case REXMIT_MERGE_NEVER: str = "never"; break;
    case REXMIT_MERGE_ADAPTIVE: str = "adaptive"; break;
    case REXMIT_MERGE_ALWAYS: str = "always"; break;
  }
  cfg_log (cfgst, "%s%s", str, is_default ? " [def]" : "");
}

static int uf_string (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
  char **elem = cfg_address (cfgst, parent, cfgelem);
  *elem = os_strdup (value);
  return 1;
}

static int uf_natint64_unit (struct cfgst *cfgst, os_int64 *elem, const char *value, const struct unit *unittab, os_int64 def_mult, os_int64 max)
{
  int pos;
  double v_dbl;
  os_int64 v_int;
  os_int64 mult;
  /* try convert as integer + optional unit; if that fails, try
     floating point + optional unit (round, not truncate, to integer) */
  if (*value == 0)
  {
    *elem = 0; /* some static analyzers don't "get it" */
    return cfg_error (cfgst, "%s: empty string is not a valid value", value);
  }
  else if (sscanf (value, "%lld%n", (long long int *) &v_int, &pos) == 1 && (mult = lookup_multiplier (cfgst, unittab, value, pos, v_int == 0, def_mult, 0)) != 0)
  {
    assert (mult > 0);
    if (v_int < 0 || v_int > max / mult)
      return cfg_error (cfgst, "%s: value out of range", value);
    *elem = mult * v_int;
    return 1;
  }
  else if (sscanf (value, "%lf%n", &v_dbl, &pos) == 1 && (mult = lookup_multiplier (cfgst, unittab, value, pos, v_dbl == 0, def_mult, 1)) != 0)
  {
    assert (mult > 0);
    if (v_dbl < 0 || (os_int64) (v_dbl * mult + 0.5) > max)
      return cfg_error (cfgst, "%s: value out of range", value);
    *elem = (os_int64) (v_dbl * mult + 0.5);
    return 1;
  }
  else
  {
    *elem = 0; /* some static analyzers don't "get it" */
    return cfg_error (cfgst, "%s: invalid value", value);
  }
}


static int uf_memsize (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
  os_int64 size = 0;
  if (!uf_natint64_unit (cfgst, &size, value, unittab_memsize, 1, INT32_MAX))
    return 0;
  else
  {
    int *elem = cfg_address (cfgst, parent, cfgelem);
    *elem = (int) size;
    return 1;
  }
}


static int uf_service_name (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
  char **elem = cfg_address (cfgst, parent, cfgelem);
  if (*value == 0)
    *elem = os_strdup (cfgst->servicename);
  else
    *elem = os_strdup (value);
  return 1;
}

static int uf_tracingOutputFileName (struct cfgst *cfgst, UNUSED_ARG (void *parent), UNUSED_ARG (struct cfgelem const * const cfgelem), UNUSED_ARG (int first), const char *value)
{
  struct config *cfg = cfgst->cfg;
  if (os_strcasecmp (value, "stdout") != 0 && os_strcasecmp (value, "stderr") != 0)
    cfg->tracingOutputFileName = os_fileNormalize (value);
  else
  {
    cfg->tracingOutputFileName = os_strdup (value);
  }
  return 1;
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
    char **elem = cfg_address (cfgst, parent, cfgelem);
    *elem = NULL;
    return 1;
  }
}

static void ff_networkAddresses (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem)
{
  char ***elem = cfg_address (cfgst, parent, cfgelem);
  int i;
  for (i = 0; (*elem)[i]; i++)
    os_free ((*elem)[i]);
  os_free (*elem);
}

static int uf_networkAddresses_simple (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
  char ***elem = cfg_address (cfgst, parent, cfgelem);
  if ((*elem = os_malloc (2 * sizeof(char *))) == NULL)
    return cfg_error (cfgst, "out of memory");
  if (((*elem)[0] = os_strdup (value)) == NULL)
  {
    os_free (*elem);
    *elem = NULL;
    return cfg_error (cfgst, "out of memory");
  }
  (*elem)[1] = NULL;
  return 1;
}

static int uf_networkAddresses (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int first, const char *value)
{
  /* Check for keywords first */
  {
    static const char *keywords[] = { "all", "any", "none" };
    int i;
    for (i = 0; i < (int) (sizeof (keywords) / sizeof (*keywords)); i++)
    {
      if (os_strcasecmp (value, keywords[i]) == 0)
        return uf_networkAddresses_simple (cfgst, parent, cfgelem, first, keywords[i]);
    }
  }

  /* If not keyword, then comma-separated list of addresses */
  {
    char ***elem = cfg_address (cfgst, parent, cfgelem);
    char *copy;
    int count;

    /* First count how many addresses we have - but do it stupidly by
       counting commas and adding one (so two commas in a row are both
       counted) */
    {
      const char *scan = value;
      count = 1;
      while (*scan)
        count += (*scan++ == ',');
    }

    if ((copy = os_strdup (value)) == NULL)
      return cfg_error (cfgst, "out of memory");

    /* Allocate an array of address strings (which may be oversized a
       bit because of the counting of the commas) */
    if ((*elem = os_malloc ((count+1) * sizeof (char *))) == NULL)
    {
      os_free (copy);
      return cfg_error (cfgst, "out of memory (%d addresses)", count);
    }

    {
      char *cursor = copy, *tok;
      int idx = 0;
      while ((tok = os_strsep (&cursor, ",")) != NULL)
      {
        assert (idx < count);
        if (((*elem)[idx] = os_strdup (tok)) == NULL)
        {
          while (idx-- > 0)
            os_free ((*elem)[idx]);
          os_free (*elem);
          os_free (copy);
          *elem = NULL;
          return cfg_error (cfgst, "out of memory");
        }
        idx++;
      }
      (*elem)[idx] = NULL;
    }
    os_free (copy);
  }
  return 1;
}

static int uf_sched_prio_class (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem,UNUSED_ARG (int first), const char *value)
{
  int ret;
  q__schedPrioClass *prio;

  assert (value != NULL);

  prio = cfg_address (cfgst, parent, cfgelem);

  if (os_strcasecmp (value, "relative") == 0) {
    *prio = Q__SCHED_PRIO_RELATIVE;
    ret = 1;
  } else if (os_strcasecmp (value, "absolute") == 0) {
    *prio = Q__SCHED_PRIO_ABSOLUTE;
    ret = 1;
  } else {
    ret = cfg_error (cfgst, "'%s': undefined value", value);
  }

  return ret;
}

static void pf_sched_prio_class (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  char *str;
  q__schedPrioClass *prio = cfg_address (cfgst, parent, cfgelem);

  if (*prio == Q__SCHED_PRIO_RELATIVE) {
    str = "relative";
  } else if (*prio == Q__SCHED_PRIO_ABSOLUTE) {
    str = "absolute";
  } else {
    str = "INVALID";
  }

  cfg_log (cfgst, "%s%s", str, is_default ? " [def]" : "");
}

static int uf_sched_class (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
  static const char *vs[] = { "realtime", "timeshare", "default" };
  static const os_schedClass ms[] = { OS_SCHED_REALTIME, OS_SCHED_TIMESHARE, OS_SCHED_DEFAULT };
  int idx = list_index (vs, value);
  os_schedClass *elem = cfg_address (cfgst, parent, cfgelem);
  assert (sizeof (vs) / sizeof (*vs) == sizeof (ms) / sizeof (*ms));
  if (idx < 0)
    return cfg_error (cfgst, "'%s': undefined value", value);
  *elem = ms[idx];
  return 1;
}

static void pf_sched_class (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  os_schedClass *p = cfg_address (cfgst, parent, cfgelem);
  const char *str = "INVALID";
  switch (*p)
  {
    case OS_SCHED_DEFAULT: str = "default"; break;
    case OS_SCHED_TIMESHARE: str = "timeshare"; break;
    case OS_SCHED_REALTIME: str = "realtime"; break;
  }
  cfg_log (cfgst, "%s%s", str, is_default ? " [def]" : "");
}

static int uf_maybe_int32 (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
  struct config_maybe_int32 *elem = cfg_address (cfgst, parent, cfgelem);
  int pos;
  if (os_strcasecmp (value, "default") == 0) {
    elem->isdefault = 1;
    elem->value = 0;
    return 1;
  } else if (sscanf (value, "%d%n", &elem->value, &pos) == 1 && value[pos] == 0) {
    elem->isdefault = 0;
    return 1;
  } else {
    return cfg_error (cfgst, "'%s': neither 'default' nor a decimal integer\n");
  }
}

static int uf_maybe_memsize (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
  struct config_maybe_uint32 *elem = cfg_address (cfgst, parent, cfgelem);
  os_int64 size = 0;
  if (os_strcasecmp (value, "default") == 0) {
    elem->isdefault = 1;
    elem->value = 0;
    return 1;
  } else if (!uf_natint64_unit (cfgst, &size, value, unittab_memsize, 1, INT32_MAX)) {
    return 0;
  } else {
    elem->isdefault = 0;
    elem->value = (os_uint32) size;
    return 1;
  }
}

static int uf_maybe_duration_inf (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
  struct config_maybe_int64 *elem = cfg_address (cfgst, parent, cfgelem);
  if (os_strcasecmp (value, "default") == 0) {
    elem->isdefault = 1;
    elem->value = 0;
    return 1;
  } else if (os_strcasecmp (value, "inf") == 0) {
    elem->isdefault = 0;
    elem->value = T_NEVER;
    return 1;
  } else if (uf_natint64_unit (cfgst, &elem->value, value, unittab_duration, 0, T_NEVER - 1)) {
    elem->isdefault = 0;
    return 1;
  } else {
    return 0;
  }
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

static int uf_duration_gen (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, const char *value, os_int64 def_mult, os_int64 max_ns)
{
  return uf_natint64_unit (cfgst, cfg_address (cfgst, parent, cfgelem), value, unittab_duration, def_mult, max_ns);
}

static int uf_duration_inf (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
  if (os_strcasecmp (value, "inf") == 0)
  {
    os_int64 *elem = cfg_address (cfgst, parent, cfgelem);
    *elem = T_NEVER;
    return 1;
  }
  else
  {
    return uf_duration_gen (cfgst, parent, cfgelem, value, 0, T_NEVER - 1);
  }
}

static int uf_duration_ms_1hr (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
  return uf_duration_gen (cfgst, parent, cfgelem, value, T_MILLISECOND, 3600 * T_SECOND);
}

static int uf_duration_ms_1s (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
  return uf_duration_gen (cfgst, parent, cfgelem, value, T_MILLISECOND, T_SECOND);
}

static int uf_duration_us_1s (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
  return uf_duration_gen (cfgst, parent, cfgelem, value, 1000, T_SECOND);
}

static int uf_int32 (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
  os_int32 *elem = cfg_address (cfgst, parent, cfgelem);
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
  int *elem = cfg_address (cfgst, parent, cfgelem);
  if (os_strcasecmp (value, "auto") == 0) {
    *elem = PARTICIPANT_INDEX_AUTO;
    return 1;
  } else if (os_strcasecmp (value, "none") == 0) {
    *elem = PARTICIPANT_INDEX_NONE;
    return 1;
  } else {
    return uf_int_min_max (cfgst, parent, cfgelem, first, value, 0, 120);
  }
}

static int uf_port (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int first, const char *value)
{
  return uf_int_min_max (cfgst, parent, cfgelem, first, value, 1, 65535);
}

static int uf_dyn_port (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int first, const char *value)
{
  return uf_int_min_max (cfgst, parent, cfgelem, first, value, -1, 65535);
}

static int uf_natint (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int first, const char *value)
{
  return uf_int_min_max (cfgst, parent, cfgelem, first, value, 0, INT32_MAX);
}

static int uf_natint_255 (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int first, const char *value)
{
  return uf_int_min_max (cfgst, parent, cfgelem, first, value, 0, 255);
}

static int do_update (struct cfgst *cfgst, update_fun_t upd, void *parent, struct cfgelem const * const cfgelem, const char *value, int is_default)
{
  struct cfgst_node *n;
  struct cfgst_nodekey key;
  ut_avlIPath_t np;
  int ok;
  key.e = cfgelem;
  if ((n = ut_avlLookupIPath (&cfgst_found_treedef, &cfgst->found, &key, &np)) == NULL)
  {
    if ((n = os_malloc (sizeof (*n))) == NULL)
      return cfg_error (cfgst, "out of memory");

    n->key = key;
    n->count = 0;
    n->failed = 0;
    n->is_default = is_default;
    ut_avlInsertIPath (&cfgst_found_treedef, &cfgst->found, n, &np);
  }
  if (cfgelem->multiplicity == 0 || n->count < cfgelem->multiplicity)
    ok = upd (cfgst, parent, cfgelem, (n->count == n->failed), value);
  else
    ok = cfg_error (cfgst, "only %d instance(s) allowed",cfgelem->multiplicity);
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

static int set_defaults (struct cfgst *cfgst, void *parent, int isattr, struct cfgelem const * const cfgelem, int clear_found)
{
  const struct cfgelem *ce;
  int ok = 1;
  for (ce = cfgelem; ce && ce->name; ce++)
  {
    struct cfgst_node *n;
    struct cfgst_nodekey key;
    key.e = ce;
    cfgst_push (cfgst, isattr, ce, parent);
    if (ce->multiplicity == 1)
    {
      if (ut_avlLookup (&cfgst_found_treedef, &cfgst->found, &key) == NULL)
      {
        if (ce->update)
        {
          int ok1;
          cfgst_push (cfgst, 0, NULL, NULL);
          ok1 = set_default (cfgst, parent, ce);
          cfgst_pop (cfgst);
          ok = ok && ok1;
        }
      }
      if ((n = ut_avlLookup (&cfgst_found_treedef, &cfgst->found, &key)) != NULL)
      {
        if (clear_found)
        {
          ut_avlDelete (&cfgst_found_treedef, &cfgst->found, n);
          os_free (n);
        }
      }
      if (ce->children)
      {
        int ok1 = set_defaults (cfgst, parent, 0, ce->children, clear_found);
        ok = ok && ok1;
      }
      if (ce->attributes)
      {
        int ok1 = set_defaults (cfgst, parent, 1, ce->attributes, clear_found);
        ok = ok && ok1;
      }
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
  char **p = cfg_address (cfgst, parent, cfgelem);
  cfg_log (cfgst, "%s%s", *p ? *p : "(null)", is_default ? " [def]" : "");
}

static void pf_int64_unit (struct cfgst *cfgst, os_int64 value, int is_default, const struct unit *unittab, const char *zero_unit)
{
  if (value == 0)
  {
    /* 0s is a bit of a special case: we don't want to print 0hr (or
       whatever unit will have the greatest multiplier), so hard-code
       as 0s */
    cfg_log (cfgst, "0 %s%s", zero_unit, is_default ? " [def]" : "");
  }
  else
  {
    os_int64 m = 0;
    const char *unit = NULL;
    int i;
    for (i = 0; unittab[i].name != NULL; i++)
    {
      if (unittab[i].multiplier > m && (value % unittab[i].multiplier) == 0)
      {
        m = unittab[i].multiplier;
        unit = unittab[i].name;
      }
    }
    assert (m > 0);
    assert (unit != NULL);
    cfg_log (cfgst, "%lld %s%s", value / m, unit, is_default ? " [def]" : "");
  }
}


static void pf_networkAddress (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  char **p = cfg_address (cfgst, parent, cfgelem);
  cfg_log (cfgst, "%s%s", *p ? *p : "auto", is_default ? " [def]" : "");
}

static void pf_participantIndex (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  int *p = cfg_address (cfgst, parent, cfgelem);
  switch (*p)
  {
    case PARTICIPANT_INDEX_NONE:
      cfg_log (cfgst, "none%s", is_default ? " [def]" : "");
      break;
    case PARTICIPANT_INDEX_AUTO:
      cfg_log (cfgst, "auto%s", is_default ? " [def]" : "");
      break;
    default:
      cfg_log (cfgst, "%d%s", *p, is_default ? " [def]" : "");
      break;
  }
}

static void pf_networkAddresses (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  int i;
  char ***p = cfg_address (cfgst, parent, cfgelem);
  for (i = 0; (*p)[i] != NULL; i++)
    cfg_log (cfgst, "%s%s", (*p)[i], is_default ? " [def]" : "");
}

static void pf_int (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  int *p = cfg_address (cfgst, parent, cfgelem);
  cfg_log (cfgst, "%d%s", *p, is_default ? " [def]" : "");
}

static void pf_duration (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  const os_int64 *elem = cfg_address (cfgst, parent, cfgelem);
  if (*elem == T_NEVER)
    cfg_log (cfgst, "inf%s", is_default ? " [def]" : "");
  else
    pf_int64_unit (cfgst, *elem, is_default, unittab_duration, "s");
}


static void pf_memsize (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  const int *elem = cfg_address (cfgst, parent, cfgelem);
  pf_int64_unit (cfgst, *elem, is_default, unittab_memsize, "B");
}

static void pf_int32 (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  os_int32 *p = cfg_address (cfgst, parent, cfgelem);
  cfg_log (cfgst, "%d%s", *p, is_default ? " [def]" : "");
}

static void pf_maybe_int32 (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  struct config_maybe_int32 *p = cfg_address (cfgst, parent, cfgelem);
  if (p->isdefault)
    cfg_log (cfgst, "default%s", is_default ? " [def]" : "");
  else
    cfg_log (cfgst, "%d%s", p->value, is_default ? " [def]" : "");
}

static void pf_maybe_memsize (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  struct config_maybe_uint32 *p = cfg_address (cfgst, parent, cfgelem);
  if (p->isdefault)
    cfg_log (cfgst, "default%s", is_default ? " [def]" : "");
  else
    pf_int64_unit (cfgst, p->value, is_default, unittab_memsize, "B");
}

static void pf_maybe_duration (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  struct config_maybe_int64 *p = cfg_address (cfgst, parent, cfgelem);
  if (p->isdefault)
    cfg_log (cfgst, "default%s", is_default ? " [def]" : "");
  else if (p->value == T_NEVER)
    cfg_log (cfgst, "inf%s", is_default ? " [def]" : "");
  else
    pf_int64_unit (cfgst, p->value, is_default, unittab_duration, "s");
}

static void pf_float (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  float *p = cfg_address (cfgst, parent, cfgelem);
  cfg_log (cfgst, "%f%s", *p, is_default ? " [def]" : "");
}

static void pf_boolean (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  int *p = cfg_address (cfgst, parent, cfgelem);
  cfg_log (cfgst, "%s%s", *p ? "true" : "false", is_default ? " [def]" : "");
}

static void pf_negated_boolean (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  int *p = cfg_address (cfgst, parent, cfgelem);
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
    return cfg_error (cfgst, "'%s': undefined value", value);
  else
  {
    *elem = lc[idx];
    return 1;
  }
}

static int uf_locators (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, UNUSED_ARG (int first), const char *value)
{
  static const char *vs[] = { "local", "none", NULL };
  static const logcat_t lc[] = { DDSI_TCP_LOCATORS_LOCAL, DDSI_TCP_LOCATORS_NONE, 0 };

  enum ddsi_tcp_locators *elem = cfg_address (cfgst, parent, cfgelem);
  int idx = list_index (vs, value);
  assert (sizeof (vs) / sizeof (*vs) == sizeof (lc) / sizeof (*lc));
  if (idx < 0)
    return cfg_error (cfgst, "'%s': undefined value", value);
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

static void pf_locators (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem, int is_default)
{
  enum ddsi_tcp_locators *p = cfg_address (cfgst, parent, cfgelem);
  const char *str = "INVALID";
  switch (*p)
  {
    case DDSI_TCP_LOCATORS_LOCAL: str = "local"; break;
    case DDSI_TCP_LOCATORS_NONE: str = "none"; break;
  }
  cfg_log (cfgst, "%s%s", str, is_default ? " [def]" : "");
}

static void pf_logcat (struct cfgst *cfgst, UNUSED_ARG (void *parent), UNUSED_ARG (struct cfgelem const * const cfgelem), UNUSED_ARG (int is_default))
{
  logcat_t remaining = config.enabled_logcats;
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
  /* can't do default indicator: user may have specified Verbosity, in
     which case EnableCategory is at default, but for these two
     settings, I don't mind. */
  cfg_log (cfgst, "%s", res);
}


static void print_configitems (struct cfgst *cfgst, void *parent, int isattr, struct cfgelem const * const cfgelem, int unchecked)
{
  const struct cfgelem *ce;
  for (ce = cfgelem; ce && ce->name; ce++)
  {
    struct cfgst_nodekey key;
    struct cfgst_node *n;
    key.e = ce;
    cfgst_push (cfgst, isattr, ce, parent);
    if (ce->multiplicity == 1)
    {
      if ((n = ut_avlLookup (&cfgst_found_treedef, &cfgst->found, &key)) != NULL)
      {
        cfgst_push (cfgst, 0, NULL, NULL);
        ce->print (cfgst, parent, ce, n->is_default);
        cfgst_pop (cfgst);
      }
      else
      {
        if (unchecked && ce->print)
        {
          cfgst_push (cfgst, 0, NULL, NULL);
          ce->print (cfgst, parent, ce, 0);
          cfgst_pop (cfgst);
        }
      }

      if (ce->children)
        print_configitems (cfgst, parent, 0, ce->children, unchecked);
      if (ce->attributes)
        print_configitems (cfgst, parent, 1, ce->attributes, unchecked);
    }
    else
    {
      struct config_listelem *p = cfg_deref_address (cfgst, parent, ce);
      while (p)
      {
        cfgst_push (cfgst, 0, NULL, NULL);
        if (ce->print)
        {
          ce->print (cfgst, p, ce, 0);
        }
        cfgst_pop (cfgst);
        if (ce->attributes)
          print_configitems (cfgst, p, 1, ce->attributes, 1);
        if (ce->children)
          print_configitems (cfgst, p, 0, ce->children, 1);
        p = p->next;
      }
    }
    cfgst_pop (cfgst);
  }
}


static void free_all_elements (struct cfgst *cfgst, void *parent, struct cfgelem const * const cfgelem)
{
  const struct cfgelem *ce;

  for (ce = cfgelem; ce && ce->name; ce++)
  {
    if (ce->free)
      ce->free (cfgst, parent, ce);

    if (ce->multiplicity == 1)
    {
      if (ce->children)
        free_all_elements (cfgst, parent, ce->children);
      if (ce->attributes)
        free_all_elements (cfgst, parent, ce->attributes);
    }
    else
    {
      struct config_listelem *p = cfg_deref_address (cfgst, parent, ce);
      struct config_listelem *r ;
      while (p) {
        if (ce->attributes)
          free_all_elements (cfgst, p, ce->attributes);
        if (ce->children)
          free_all_elements (cfgst, p, ce->children);
        r = p;
        p = p->next;
        os_free(r);
      }
    }
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
    if ((n = ut_avlLookup (&cfgst_found_treedef, &cfgst->found, &key)) != NULL)
    {
      if (ce->free && n->count > n->failed)
        ce->free (cfgst, parent, ce);
    }

    if (ce->multiplicity == 1)
    {
      if (ce->children)
        free_configured_elements (cfgst, parent, ce->children);
      if (ce->attributes)
        free_configured_elements (cfgst, parent, ce->attributes);
    }
    else
    {
      struct config_listelem *p = cfg_deref_address (cfgst, parent, ce);
      struct config_listelem *r;
      while (p)
      {
        if (ce->attributes)
          free_all_elements (cfgst, p, ce->attributes);
        if (ce->children)
          free_all_elements (cfgst, p, ce->children);
        r = p;
        p = p->next;
        os_free(r);
      }
    }
  }
}

static int matching_name_index (const char *name_w_aliases, const char *name)
{
  const char *ns = name_w_aliases, *p = strchr (ns, '|');
  int idx = 0;
  while (p)
  {
    if (os_strncasecmp (ns, name, p-ns) == 0 && name[p-ns] == 0)
    {
      /* ns upto the pipe symbol is a prefix of name, and name is
       terminated at that point */
      return idx;
    }
    ns = p + 1;
    p = strchr (ns, '|');
    idx++;
  }
  return (os_strcasecmp (ns, name) == 0) ? idx : -1;
}

static int walk_attributes (struct cfgst *cfgst, void *parent, struct cfgelem const *
                             const cfgelem, u_cfElement base)
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
      ok1 = cfg_error (cfgst, "%s: unknown attribute", child_name);
    else
    {
      c_char *str;
      if (!u_cfAttributeStringValue (attr, &str))
        ok1 = cfg_error (cfgst, "failed to extract data");
      else
      {
        cfgst_push (cfgst, 1, cfg_attr, parent /* FIXME: parent or NULL? */);
        ok1 = do_update (cfgst, cfg_attr->update, parent, cfg_attr, str, 0);
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
             cfg_subelem && cfg_subelem->name && os_strcasecmp (cfg_subelem->name, "*") != 0;
             cfg_subelem++)
        {
          int idx = matching_name_index (cfg_subelem->name, child_name);
#if WARN_DEPRECATED_ALIAS
          if (idx > 0)
          {
            int n = (int) (strchr (cfg_subelem->name, '|') - cfg_subelem->name);
            cfg_warning (cfgst, "'%s': deprecated alias for '%*.*s'", child_name, n, n, cfg_subelem->name);
          }
#endif
          if (idx >= 0)
          {
            break;
          }
        }
        if (cfg_subelem == NULL || cfg_subelem->name == NULL)
          ok1 = cfg_error (cfgst, "%s: unknown element", child_name);
        else if (os_strcasecmp (cfg_subelem->name, "*") == 0)
          ok1 = 1;
        else
        {
          cfgst_push (cfgst, 0, cfg_subelem, parent /* FIXME: parent or NULL? */);
          ok1 = walk_element (cfgst, parent, cfg_subelem, elem);
          cfgst_pop (cfgst);
        }
        break;
      }
      case V_CFDATA:
      {
        u_cfData data = u_cfData (child);
        c_char *str;
        if (!u_cfDataStringValue (data, &str))
          ok1 = cfg_error (cfgst, "failed to extract data");
        else if (cfgelem->update == 0)
        {
          if (strspn (str, " \t\r\n") != strlen (str))
            ok1 = cfg_error (cfgst, "%s: no data expected", str);
          else
            ok1 = 1;
          os_free (str);
        }
        else
        {
          cfgst_push (cfgst, 0, NULL, NULL /* FIXME: parent or NULL? */);
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
  int ok = 1;
  void *dynparent;

  assert (cfgelem->init || cfgelem->multiplicity == 1); /*multi-items must have an init-func */
  if (cfgelem->init)
  {
    if (cfgelem->init (cfgst, parent, cfgelem) < 0)
      return -1;
  }

  if (cfgelem->multiplicity != 1)
    dynparent = cfg_deref_address (cfgst, parent, cfgelem);
  else
    dynparent = parent;

  ok = walk_attributes (cfgst, dynparent, cfgelem, base) &&
  walk_children (cfgst, dynparent, cfgelem, base);

  if (ok && cfgelem->multiplicity != 1)
  {
    int ok1;
    ok1 = set_defaults (cfgst, dynparent, 1, cfgelem->attributes, 1);
    ok = ok && ok1;
    ok1 = set_defaults (cfgst, dynparent, 0, cfgelem->children, 1);
    ok = ok && ok1;
  }

  return ok;
}

static int cfgst_node_cmp (const void *va, const void *vb)
{
  return memcmp (va, vb, sizeof (struct cfgst_nodekey));
}


struct cfgst *config_init
(
  /* C_STRUCT (u_participant) const * */u_participant participant, const char *servicename
)
{
  int ok = 1;
  struct cfgst *cfgst;
  u_cfElement root, elem;
  c_iter iter;
  int rootidx;
  assert (participant != NULL);

  /* Enable logging of errors &c. to stderr until configuration is read */
  config.enabled_logcats = LC_FATAL | LC_ERROR | LC_WARNING;
  config.tracingOutputFile = stderr;
  config.valid = 0;




  config.peers = NULL;

  if ((cfgst = os_malloc (sizeof (*cfgst))) == NULL)
    return NULL;
  ut_avlInit (&cfgst_found_treedef, &cfgst->found);
  cfgst->path_depth = 0;
  cfgst->cfg = &config;
  cfgst->internal_seen = 0;
  cfgst->servicename = servicename;

  if ((root = u_participantGetConfiguration ((u_participant) participant)) == NULL)
  {
    NN_ERROR0 ("config_init: u_participantGetConfiguration failed");
    ut_avlFree (&cfgst_found_treedef, &cfgst->found, os_free);
    os_free (cfgst);
    return NULL;
  }

  /* Only suitable for Domain (without a attributes) and a service
   with a matching name attribute */
  for (rootidx = 0; root_cfgelems[rootidx].name; rootidx++)
  {
    const struct cfgelem *root_cfgelem = &root_cfgelems[rootidx];
    cfgst_push (cfgst, 0, root_cfgelem, NULL);
    iter = u_cfElementXPath (root, root_cfgelem->name);
    elem = u_cfElement (c_iterTakeFirst (iter));
    while (elem)
    {
      c_char *str;
      if (root_cfgelem->attributes == NULL)
      {
        /* Domain element */
        int ok1;
        assert (strcmp (root_cfgelem->name, "Domain") == 0);
        ok1 = walk_element (cfgst, cfgst->cfg, root_cfgelem, elem);
        ok = ok && ok1;
      }
      else if (u_cfElementAttributeStringValue (elem, "name", &str))
      {
        int ok1;
        assert (strcmp (root_cfgelem->name, "DDSI2Service") == 0);
        if (os_strcasecmp (servicename, str) != 0)
          ok1 = 1;
        else
        {
          ok1 =
          walk_element (cfgst, cfgst->cfg, root_cfgelem, elem)
          && walk_children (cfgst, cfgst->cfg, &legacy_root_cfgelems[0], elem);
        }
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

  /* Set defaults for everything not set that we have a default value
     for, signal errors for things unset but without a default. */
  {
    int ok1 = set_defaults (cfgst, cfgst->cfg, 0, root_cfgelems, 0);
    ok = ok && ok1;
  }




  /* Now switch to configured tracing settings */
  config.enabled_logcats = enabled_logcats;

  if (!ok)
  {
    free_configured_elements (cfgst, cfgst->cfg, root_cfgelems);
  }

  if (ok)
  {
    config.valid = 1;
    return cfgst;
  }
  else
  {
    ut_avlFree (&cfgst_found_treedef, &cfgst->found, os_free);
    os_free (cfgst);
    return NULL;
  }
}

void config_print_and_free_cfgst (struct cfgst *cfgst)
{
  if (cfgst == NULL)
    return;
  print_configitems (cfgst, cfgst->cfg, 0, root_cfgelems, 0);
  ut_avlFree (&cfgst_found_treedef, &cfgst->found, os_free);
  os_free (cfgst);
}

void config_fini (void)
{
  if (config.valid)
  {
    struct cfgst cfgst;
    cfgst.cfg = &config;
    free_all_elements (&cfgst, cfgst.cfg, root_cfgelems);
    memset (&config, 0, sizeof (config));
  }
}



/* SHA1 not available (unoffical build.) */
