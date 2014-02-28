#ifndef Q_GLOBALS_H
#define Q_GLOBALS_H

#include <stdio.h>

#include "os_defs.h"
#include "os_mutex.h"
#include "os_rwlock.h"
#include "os_cond.h"
#include "os_socket.h"

#include "c_base.h"
#include "kernelModule.h"

#include "q_xqos.h"
#include "q_protocol.h"
#include "q_nwif.h"
#include "q_sockwaitset.h"
#include "q_fibheap.h"


#if defined (__cplusplus)
extern "C" {
#endif

struct nn_xmsgpool;
struct serstatepool;
struct nn_dqueue;
struct nn_reorder;
struct nn_defrag;
struct addrset;
struct xeventq;
struct gcreq_queue;
struct ephash;
struct lease;
struct ddsi_tran_conn;
struct ddsi_tran_listener;
struct ddsi_tran_factory;
struct ut_thread_pool_s;

typedef struct ospl_in_addr_node {
   os_sockaddr_storage addr;
   struct ospl_in_addr_node *next;
} ospl_in_addr_node;

enum recvips_mode {
  RECVIPS_MODE_ALL,             /* all MC capable interfaces */
  RECVIPS_MODE_ANY,             /* kernel-default interface */
  RECVIPS_MODE_PREFERRED,       /* selected interface only */
  RECVIPS_MODE_NONE,            /* no interfaces at all */
  RECVIPS_MODE_SOME             /* explicit list of interfaces; only one requiring recvips */
};

#define N_LEASE_LOCKS_LG2 4
#define N_LEASE_LOCKS ((int) (1 << N_LEASE_LOCKS_LG2))

struct q_globals {
  volatile int terminate;

  /* Process-scope mutex & cond. var. attributes, so we don't have to
   initialise attributes all the time. */
  os_mutexAttr mattr;
  os_condAttr cattr;
  os_rwlockAttr rwattr;

  /* OpenSplice base & kernel pointers, QoS type */
  c_base ospl_base;
  v_kernel ospl_kernel;
  c_collectionType ospl_qostype;

  /* Hash tables for participants, readers, writers, proxy
     participants, proxy readers and proxy writers by GUID
     (guid_hash) */
  struct ephash *guid_hash;

  /* Timed events admin */
  struct xeventq *xevents;

  /* Queue for garbage collection requests */
  struct gcreq_queue *gcreq_queue;
  struct nn_servicelease *servicelease;

  /* Lease junk */
  os_mutex leaseheap_lock;
  os_mutex lease_locks[N_LEASE_LOCKS];
  struct fibheap leaseheap;

  /* Transport factory */

  struct ddsi_tran_factory * m_factory;

  /* Connections for multicast discovery & data, and those that correspond
     to the one DDSI participant index that the DDSI2 service uses. The
     DCPS participant of DDSI2 itself will be mirrored in a DDSI
     participant, and in multi-socket mode that one gets its own
     socket. */

  struct ddsi_tran_conn * disc_conn_mc;
  struct ddsi_tran_conn * data_conn_mc;
  struct ddsi_tran_conn * disc_conn_uc;
  struct ddsi_tran_conn * data_conn_uc;

  /* TCP listener */

  struct ddsi_tran_listener * listener;

  /* Thread pool */

  struct ut_thread_pool_s * thread_pool;

  /* Receive thread triggering: must have a socket per receive thread
     because all receive threads must be triggered, even though each
     receive thread takes the trigger message from the socket. With one
     trigger socket, we can only have one receive thread (which enables
     other optimisations that we don't currently do). */
  os_sockWaitset waitset;

  /* In many sockets mode, the receive threads maintain a local array
     with participant GUIDs and sockets, participant_set_generation is
     used to notify them. */
  volatile os_uint32 participant_set_generation;

  /* nparticipants is used primarily for limiting the number of active
     participants, but also during shutdown to determine when it is
     safe to stop the GC thread. */
  os_mutex participant_set_lock;
  os_cond participant_set_cond;
  os_uint32 nparticipants;

  /* For participants without (some) built-in writers, we fall back to
     this participant, which is the first one created with all
     built-in writers present.  It MUST be created before any in need
     of it pops up! */
  struct participant *privileged_pp;
  os_mutex privileged_pp_lock;


  /* number of up, non-loopback, IPv4/IPv6 interfaces, the index of
     the selected/preferred one, and the discovered interfaces. */
  int n_interfaces;
  int selected_interface;
  struct nn_interface interfaces[MAX_INTERFACES];

#if OS_SOCKET_HAS_IPV6
  /* whether we're using an IPv6 link-local address (and therefore
     only listening to multicasts on that interface) */
  int ipv6_link_local;
#endif

  /* Addressing: actual own (preferred) IP address, IP address
     advertised in discovery messages (so that an external IP address on
     a NAT may be advertised), and the DDSI multi-cast address. */
  enum recvips_mode recvips_mode;
  struct ospl_in_addr_node *recvips;
  struct in_addr extmask;

  os_sockaddr_storage ownip;
  os_sockaddr_storage extip;

  /* InterfaceNo that the OwnIP is tied to */
  os_uint interfaceNo;

  /* Locators */

  nn_locator_t loc_meta_mc;
  nn_locator_t loc_meta_uc;
  nn_locator_t loc_default_mc;
  nn_locator_t loc_default_uc;

  /* Initial discovery address set, and the current discovery address
     set. These are the addresses that SPDP pings get sent to. FIXME:
     as_disc_init to be removed. */
  struct addrset *as_disc_init;
  struct addrset *as_disc;

  /* qoslock serializes QoS changes, probably not strictly necessary,
     but a lot more straightforward that way */
  os_rwlock qoslock;

  os_mutex lock;

  
  /* Receive thread. (We can only has one for now, cos of the signal
     trigger socket.) Receive buffer pool is per receive thread,
     practical considerations led to it being a global variable
     TEMPORARILY. */
  struct thread_state1 *recv_ts;
  struct nn_rbufpool *rbufpool;

  /* Flag cleared when stopping (receive threads). FIXME. */
  int rtps_keepgoing;

  /* Startup mode causes data to be treated as transient-local with
     depth 1 (i.e., stored in the WHCs and regurgitated on request) to
     cover the start-up delay of the discovery protocols. Because all
     discovery data is shared, this is strictly a start-up issue of the
     service. */
  int startup_mode;

  /* Start time of the DDSI2 service, for logging relative time stamps,
     should I ever so desire. */
  os_int64 tstart;

  /* Default QoSs for readers and writers (needed for eliminating
     default values in outgoing discovery packets, and for supplying
     values for missing QoS settings in incoming discovery packets);
     plus the actual QoSs needed for the builtin endpoints. */
  nn_xqos_t default_xqos_rd;
  nn_xqos_t default_xqos_wr;
  nn_xqos_t spdp_endpoint_xqos;
  nn_xqos_t builtin_endpoint_xqos_rd;
  nn_xqos_t builtin_endpoint_xqos_wr;

  /* Unique 64-bit ID generator for entities */
  
  /* SPDP packets get very special treatment (they're the only packets
     we accept from writers we don't know) and have their very own
     do-nothing defragmentation and reordering thingummies, as well as a
     global mutex to in lieu of the proxy writer lock. */
  os_mutex spdp_lock;
  struct nn_defrag *spdp_defrag;
  struct nn_reorder *spdp_reorder;

  /* Built-in stuff other than SPDP gets funneled through the builtins
     delivery queue; currently just SEDP and PMD */
  struct nn_dqueue *builtins_dqueue;

  /* Connection used by general timed-event queue for transmitting data */

  struct ddsi_tran_conn * tev_conn;

  os_uint32 networkQueueId;
  struct thread_state1 *channel_reader_ts;

  /* Application data gets its own delivery queue */
  struct nn_dqueue *user_dqueue;

  /* Transmit side: pools for the serializer & transmit messages and a
     transmit queue*/
  struct serstatepool *serpool;
  struct nn_xmsgpool *xmsgpool;

  /* Network ID needed by v_groupWrite -- FIXME: might as well pass it
     to the receive thread instead of making it global (and that would
     remove the need to include kernelModule.h) */
  os_uint32 myNetworkId;


  /* File for dumping captured packets, NULL if disabled */
  FILE *pcap_fp;
  os_mutex pcap_lock;

  /* Static log buffer, for those rare cases a thread calls nn_vlogb
     without having its own log buffer (happens during config file
     processing and for listeners, &c. */
  int static_logbuf_lock_inited;
  os_mutex static_logbuf_lock;
  struct logbuf static_logbuf;
};

extern struct q_globals gv;

#if defined (__cplusplus)
}
#endif

#endif /* Q_GLOBALS_H */

/* SHA1 not available (unoffical build.) */
