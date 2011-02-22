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
#ifndef RTPS_H
#define RTPS_H

#include "os_defs.h" /* for va_list */
#include "os_stdlib.h" /* for va_list */
#include "kernelModule.h"

typedef union {
  unsigned char s[12];
  unsigned u[3];
} guid_prefix_t;
typedef union {
  struct { unsigned char key[3]; unsigned char kind; } s;
  unsigned u;
} entityid_t;
typedef struct { guid_prefix_t prefix; entityid_t entityid; } guid_t;

/* predefined entity ids; here viewed as an unsigned int, on the
   network as four bytes corresponding to the integer in network byte
   order */
#define ENTITYID_UNKNOWN 0x0
#define ENTITYID_PARTICIPANT 0x1c1
#define ENTITYID_SEDP_BUILTIN_TOPIC_WRITER 0x2c2
#define ENTITYID_SEDP_BUILTIN_TOPIC_READER 0x2c7
#define ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER 0x3c2
#define ENTITYID_SEDP_BUILTIN_PUBLICATIONS_READER 0x3c7
#define ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER 0x4c2
#define ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_READER 0x4c7
#define ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER 0x100c2
#define ENTITYID_SPDP_BUILTIN_PARTICIPANT_READER 0x100c7
#define ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_WRITER 0x200c2
#define ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_READER 0x200c7
#define ENTITYID_SOURCE_MASK 0xc0
#define ENTITYID_SOURCE_USER 0x00
#define ENTITYID_SOURCE_BUILTIN 0xc0
#define ENTITYID_SOURCE_VENDOR 0x40
#define ENTITYID_KIND_MASK 0x3f
#define ENTITYID_KIND_WRITER_WITH_KEY 0x02
#define ENTITYID_KIND_WRITER_NO_KEY 0x03
#define ENTITYID_KIND_READER_NO_KEY 0x04
#define ENTITYID_KIND_READER_WITH_KEY 0x07

struct writer;
struct reader;
struct participant;

/* QoS that this DDSI implementation acts upon:
   (qos & RELIABLE) = 0    => best-effort transmission
                      1    => reliable transmission
                      
   (qos & DUR..MASK)       => durability advertised over
     discovery protocol and used in matching local readers
     and writers to remote writers and readers
     
   (qos & HANDLE_AS..) = 0 =>
     - for writer: do not retain latest sample for each
       instance in the WHC
     - for reader: do not request old data from writer
       (though it may end up doing it anyway, that depends
       on whether the remote writer's sequence number is
       known or not, usually the sequence number is known
       if a heartbeat or data has been received previously)
     Obligatory if durability = VOLATILE
   (qos & HANDLE_AS..) = 1 =>
     - for writer: do retain latest sample for each
       instance in the WHC
     - for reader: request all data the writer advertises
       as available.
     (Forbidden if durability = VOLATILE)
     
   Note that the usefulness of TRANSIENT_LOCAL without
   HANDLE_AS_TRANSIENT_LOCAL seems somewhat questionable, but is
   allowed. */
#define MF_MASK				0xff
#define MF_RELIABLE			0x1
#define MF_HANDLE_AS_TRANSIENT_LOCAL	0x2
#define MF_DURABILITY_MASK		(3 << 2)
#define MF_DURABILITY_VOLATILE		(0 << 2)
#define MF_DURABILITY_TRANSIENT_LOCAL	(1 << 2)
#define MF_DURABILITY_TRANSIENT		(2 << 2)
#define MF_DURABILITY_PERSISTENT	(3 << 2)
#define MF_ACCESS_SCOPE_MASK		(3 << 4)
#define MF_ACCESS_SCOPE_INSTANCE	(0 << 4)
#define MF_ACCESS_SCOPE_TOPIC		(1 << 4)
#define MF_ACCESS_SCOPE_GROUP		(2 << 4)
#define MF_EXCLUSIVE_OWNERSHIP		(1 << 6)
#define MF_DESTINATION_ORDER_MASK	(1 << 7)
#define MF_DESTINATION_ORDER_RECEPTION	(0 << 7)
#define MF_DESTINATION_ORDER_SOURCE	(1 << 7)

#define STATUSINFO_DISPOSE	0x1
#define STATUSINFO_UNREGISTER	0x2

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
#define RTPS_USE_MCAST (1u << 0)
#define RTPS_USE_MCAST_LOOPBACK (1u << 1)
#define RTPS_IGNORE_OWN_VENDOR (1u << 2)
#define RTPS_AGGRESSIVE_KEEP_LAST1_WHC (1u << 3)
#define RTPS_CONSERVATIVE_BUILTIN_READER_STARTUP (1u << 4)
#define RTPS_NOQUEUE_HEARTBEAT_MESSAGES (1u << 5)

/* Set this flag in new_participant to prevent the creation SPDP, SEDP
   and PMD readers for that participant. It doesn't really need it,
   they all share the information anyway. But you do need it once. */
#define RTPS_PF_NO_BUILTIN_READERS 1

struct datainfo {
  guid_t dst;          /* GUID of local reader */
  guid_t src;          /* GUID of remote writer */
  unsigned statusinfo; /* write, dispose or unregister */
  unsigned qos;	       /* MF_... bits */
  long long tstamp;    /* time stamp in ns since 1-1-1970 */
  long long seqnr;     /* writer sequence number, starting at 1 */
};

typedef void (*data_recv_cb_fun_t) (struct datainfo *di, v_message message, void *arg);

struct participant *new_participant (guid_prefix_t idprefix, unsigned flags);
struct writer *new_writer (struct participant *pp, entityid_t id, unsigned qos, C_STRUCT (v_topic) const * const topic, const char *partition);
struct reader *new_reader (struct participant *pp, entityid_t id, unsigned qos, C_STRUCT (v_topic) const * const topic, const char *partition, data_recv_cb_fun_t data_recv_cb, void *cb_arg);
void delete_participant (struct participant *pp);
void delete_writer (struct writer *wr);
void delete_reader (struct reader *rd);

int rtps_write (struct writer *wr, C_STRUCT (v_message) const *msg);
int rtps_dispose (struct writer *wr, C_STRUCT (v_message) const *keymsg);
int rtps_unregister (struct writer *wr, C_STRUCT (v_message) const *keymsg);

void rtps_init (void *base, int domainid, int ppid, unsigned flags, const char *ownip, const char *peer_addrs);
void rtps_term (void);

/* Debugging output & settings:
     TRACING	     causes LOTS of output, via TRACE_FUNCTION
     MEMCHECK        causes chasing of all pointers in the admin, so valgrind/purify
                     catch dangling pointers even earlier
   flags default to 0, TRACE_FUNCTION defaults to vprintf()
   these can be changed at any time, also outside rtps_init ... rtps_term
 */
#define DBGFLAG_TRACING 1
#define DBGFLAG_MEMCHECK 2
#define DBGFLAG_PERFINFO 4
#define DBGFLAG_TROUBLE 8
#define DBGFLAG_INFO 16
unsigned rtps_get_debug_flags (void);
unsigned rtps_set_debug_flags (unsigned flags);
void rtps_set_trace_function (int (*trace_function) (const char *fmt, va_list ap));

#endif /* RTPS_H */
