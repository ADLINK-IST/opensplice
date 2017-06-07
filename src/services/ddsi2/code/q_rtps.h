/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#ifndef NN_RTPS_H
#define NN_RTPS_H

#include "os_defs.h"

#if defined (__cplusplus)
extern "C" {
#endif

typedef struct {
  unsigned char id[2];
} nn_vendorid_t;
typedef struct {
  unsigned char major, minor;
} nn_protocol_version_t;
typedef union nn_guid_prefix {
  unsigned char s[12];
  unsigned u[3];
} nn_guid_prefix_t;
typedef union nn_entityid {
  unsigned u;
} nn_entityid_t;
typedef struct nn_guid {
  nn_guid_prefix_t prefix;
  nn_entityid_t entityid;
} nn_guid_t;

#define PGUIDPREFIX(gp) (gp).u[0], (gp).u[1], (gp).u[2]
#define PGUID(g) PGUIDPREFIX ((g).prefix), (g).entityid.u

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
#define NN_ENTITYID_SEDP_BUILTIN_CM_PARTICIPANT_WRITER 0x142
#define NN_ENTITYID_SEDP_BUILTIN_CM_PARTICIPANT_READER 0x147
#define NN_ENTITYID_SEDP_BUILTIN_CM_PUBLISHER_WRITER 0x242
#define NN_ENTITYID_SEDP_BUILTIN_CM_PUBLISHER_READER 0x247
#define NN_ENTITYID_SEDP_BUILTIN_CM_SUBSCRIBER_WRITER 0x342
#define NN_ENTITYID_SEDP_BUILTIN_CM_SUBSCRIBER_READER 0x347
#define NN_ENTITYID_SOURCE_MASK 0xc0
#define NN_ENTITYID_SOURCE_USER 0x00
#define NN_ENTITYID_SOURCE_BUILTIN 0xc0
#define NN_ENTITYID_SOURCE_VENDOR 0x40
#define NN_ENTITYID_KIND_MASK 0x3f
#define NN_ENTITYID_KIND_WRITER_WITH_KEY 0x02
#define NN_ENTITYID_KIND_WRITER_NO_KEY 0x03
#define NN_ENTITYID_KIND_READER_NO_KEY 0x04
#define NN_ENTITYID_KIND_READER_WITH_KEY 0x07
#define NN_ENTITYID_KIND_PRISMTECH_SUBSCRIBER 0x0a /* source = VENDOR */
#define NN_ENTITYID_KIND_PRISMTECH_PUBLISHER 0x0b /* source = VENDOR */
#define NN_ENTITYID_ALLOCSTEP 0x100

struct cfgst;
int rtps_config_prep (struct cfgst *cfgst);
int rtps_config_open (void);
int rtps_init (void);
void ddsi_impl_init (void);
void rtps_term_prep (void);
void rtps_term (void);

#if defined (__cplusplus)
}
#endif

#endif /* NN_RTPS_H */

/* SHA1 not available (unoffical build.) */
