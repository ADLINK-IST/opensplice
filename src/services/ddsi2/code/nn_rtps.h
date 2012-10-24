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
#ifndef NN_RTPS_H
#define NN_RTPS_H

#include "os_defs.h" /* for va_list */
#include "os_stdlib.h" /* for va_list */
#include "kernelModule.h"

typedef struct {
  unsigned char id[2];
} nn_vendorid_t;
typedef struct {
  unsigned char major, minor;
} nn_protocol_version_t;
typedef union {
  unsigned char s[12];
  unsigned u[3];
} nn_guid_prefix_t;
typedef union {
  unsigned u;
} nn_entityid_t;
typedef struct {
  nn_guid_prefix_t prefix;
  nn_entityid_t entityid;
} nn_guid_t;

/* predefined entity ids; here viewed as an unsigned int, on the
   network as four bytes corresponding to the integer in network byte
   order */
#define NN_ENTITYID_UNKNOWN 0x0
#define NN_ENTITYID_PARTICIPANT 0x1c1
#define NN_ENTITYID_SEDP_BUILTIN_TOPIC_WRITER 0x2c2
#define NN_ENTITYID_SEDP_BUILTIN_TOPIC_READER 0x2c7
#define NN_ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER 0x3c2
#define NN_ENTITYID_SEDP_BUILTIN_PUBLICATIONS_READER 0x3c7
#define NN_ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER 0x4c2
#define NN_ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_READER 0x4c7
#define NN_ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER 0x100c2
#define NN_ENTITYID_SPDP_BUILTIN_PARTICIPANT_READER 0x100c7
#define NN_ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_WRITER 0x200c2
#define NN_ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_READER 0x200c7
#define NN_ENTITYID_SOURCE_MASK 0xc0
#define NN_ENTITYID_SOURCE_USER 0x00
#define NN_ENTITYID_SOURCE_BUILTIN 0xc0
#define NN_ENTITYID_SOURCE_VENDOR 0x40
#define NN_ENTITYID_KIND_MASK 0x3f
#define NN_ENTITYID_KIND_WRITER_WITH_KEY 0x02
#define NN_ENTITYID_KIND_WRITER_NO_KEY 0x03
#define NN_ENTITYID_KIND_READER_NO_KEY 0x04
#define NN_ENTITYID_KIND_READER_WITH_KEY 0x07

struct writer;
struct reader;
struct participant;

/* Set this flag in new_participant to prevent the creation SPDP, SEDP
   and PMD readers for that participant. It doesn't really need it,
   they all share the information anyway. But you do need it once. */
#define RTPS_PF_NO_BUILTIN_READERS 1

struct nn_xqos;
struct nn_xpack;

struct participant *new_participant (nn_guid_prefix_t idprefix, unsigned flags);
struct writer *new_writer (struct participant *pp, nn_entityid_t id, C_STRUCT (v_topic) const * const topic, const struct nn_xqos *xqos);
struct reader *new_reader (struct participant *pp, nn_entityid_t id, C_STRUCT (v_topic) const * const topic, const struct nn_xqos *xqos);
void delete_participant (struct participant *pp);
void delete_writer (struct writer *wr);
void delete_reader (struct reader *rd);

int rtps_write (struct nn_xpack *xp, struct writer *wr, C_STRUCT (v_message) const *msg);

void rtps_init (void *base, v_kernel kernel, int domainid, int ppid, unsigned flags, const char *ownip, const char *peer_addrs);
void rtps_term (void);

#endif /* NN_RTPS_H */
