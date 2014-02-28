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
#ifndef NN_PROTOCOL_H
#define NN_PROTOCOL_H

#include "os_abstract.h"

#ifdef PA_LITTLE_ENDIAN
#define PLATFORM_IS_LITTLE_ENDIAN 1
#else
#define PLATFORM_IS_LITTLE_ENDIAN 0
#endif

#ifndef PLATFORM_IS_LITTLE_ENDIAN
#error "PLATFORM_IS_LITTLE_ENDIAN undefined"
#elif PLATFORM_IS_LITTLE_ENDIAN != 0 && PLATFORM_IS_LITTLE_ENDIAN != 1
#error "PLATFORM_IS_LITTLE_ENDIAN is not 0 or 1"
#endif

#include "q_rtps.h"
#include "q_time.h"

#if defined (__cplusplus)
extern "C" {
#endif

typedef struct {
  unsigned char id[4];
} nn_protocolid_t;
typedef struct {
  int high;
  unsigned low;
} nn_sequence_number_t;
typedef struct nn_sequence_number_set {
  nn_sequence_number_t bitmap_base;
  unsigned numbits;
  unsigned bits[1];
} nn_sequence_number_set_t; /* Why strict C90? zero-length/flexible array members are far nicer */
/* SequenceNumberSet size is base (2 words) + numbits (1 word) +
   bitmap ((numbits+31)/32 words), and this at 4 bytes/word */
#define NN_SEQUENCE_NUMBER_SET_BITS_SIZE(numbits) (4 * ((numbits + 31) / 32))
#define NN_SEQUENCE_NUMBER_SET_SIZE(numbits) (offsetof (nn_sequence_number_set_t, bits) + NN_SEQUENCE_NUMBER_SET_BITS_SIZE (numbits))
typedef unsigned nn_fragment_number_t;
typedef struct nn_fragment_number_set {
  nn_fragment_number_t bitmap_base;
  unsigned numbits;
  unsigned bits[1];
} nn_fragment_number_set_t;
/* FragmentNumberSet size is base (2 words) + numbits (1 word) +
   bitmap ((numbits+31)/32 words), and this at 4 bytes/word */
#define NN_FRAGMENT_NUMBER_SET_BITS_SIZE(numbits) (4 * ((numbits + 31) / 32))
#define NN_FRAGMENT_NUMBER_SET_SIZE(numbits) (offsetof (nn_fragment_number_set_t, bits) + NN_FRAGMENT_NUMBER_SET_BITS_SIZE (numbits))
typedef int nn_count_t;
#define DDSI_COUNT_MIN (-2147483647 - 1)
#define DDSI_COUNT_MAX (2147483647)
/* address field in locator maintained in network byte order, the rest
   in host (yes: that's a FIXME)  */
typedef struct {
  int kind;
  unsigned port;
  unsigned char address[16];
} nn_locator_t;

struct cdrstring {
  unsigned length;
  char contents[1]; /* C90 does not support flex. array members */
};

#define NN_STATUSINFO_DISPOSE      0x1
#define NN_STATUSINFO_UNREGISTER   0x2

#define NN_GUID_PREFIX_UNKNOWN_INITIALIZER {{0,0,0,0, 0,0,0,0, 0,0,0,0}}

#define NN_DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER (1 << 0)
#define NN_DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR (1 << 1)
#define NN_DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER (1 << 2)
#define NN_DISC_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR (1 << 3)
#define NN_DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER (1 << 4)
#define NN_DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR (1 << 5)
#define NN_DISC_BUILTIN_ENDPOINT_PARTICIPANT_PROXY_ANNOUNCER (1 << 6) /* undefined meaning */
#define NN_DISC_BUILTIN_ENDPOINT_PARTICIPANT_PROXY_DETECTOR (1 << 7) /* undefined meaning */
#define NN_DISC_BUILTIN_ENDPOINT_PARTICIPANT_STATE_ANNOUNCER (1 << 8) /* undefined meaning */
#define NN_DISC_BUILTIN_ENDPOINT_PARTICIPANT_STATE_DETECTOR (1 << 9) /* undefined meaning */
#define NN_BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_WRITER (1 << 10)
#define NN_BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_READER (1 << 11)

#define NN_LOCATOR_KIND_INVALID -1
#define NN_LOCATOR_KIND_RESERVED 0
#define NN_LOCATOR_KIND_UDPv4 1
#define NN_LOCATOR_KIND_UDPv6 2
#define NN_LOCATOR_KIND_TCPv4 4
#define NN_LOCATOR_KIND_TCPv6 8
#define NN_LOCATOR_PORT_INVALID 0

#define NN_VENDORID_UNKNOWN           {{ 0x00, 0x00 }}
#define NN_VENDORID_RTI               {{ 0x01, 0x01 }}
#define NN_VENDORID_PRISMTECH_OSPL    {{ 0x01, 0x02 }}
#define NN_VENDORID_OCI               {{ 0x01, 0x03 }}
#define NN_VENDORID_MILSOFT           {{ 0x01, 0x04 }}
#define NN_VENDORID_KONGSBERG         {{ 0x01, 0x05 }}
#define NN_VENDORID_TWINOAKS          {{ 0x01, 0x06 }}
#define NN_VENDORID_LAKOTA            {{ 0x01, 0x07 }}
#define NN_VENDORID_ICOUP             {{ 0x01, 0x08 }}
#define NN_VENDORID_ETRI              {{ 0x01, 0x09 }}
#define NN_VENDORID_RTI_MICRO         {{ 0x01, 0x0a }}
#define NN_VENDORID_PRISMTECH_JAVA    {{ 0x01, 0x0b }}
#define NN_VENDORID_PRISMTECH_GATEWAY {{ 0x01, 0x0c }}
#define NN_VENDORID_PRISMTECH_LITE    {{ 0x01, 0x0d }}
#define NN_VENDORID_TECHNICOLOR       {{ 0x01, 0x0e }}

#define MY_VENDOR_ID NN_VENDORID_PRISMTECH_OSPL

/* Only one specific version is grokked */
#define RTPS_MAJOR 2
#define RTPS_MINOR 1

typedef struct Header {
  nn_protocolid_t protocol;
  nn_protocol_version_t version;
  nn_vendorid_t vendorid;
  nn_guid_prefix_t guid_prefix;
} Header_t;
#define NN_PROTOCOLID_INITIALIZER {{ 'R','T','P','S' }}
#define NN_PROTOCOL_VERSION_INITIALIZER { RTPS_MAJOR, RTPS_MINOR }
#define NN_VENDORID_INITIALIER MY_VENDOR_ID
#define NN_HEADER_INITIALIZER { NN_PROTOCOLID_INITIALIZER, NN_PROTOCOL_VERSION_INITIALIZER, NN_VENDORID_INITIALIER, NN_GUID_PREFIX_UNKNOWN_INITIALIZER }
#define RTPS_MESSAGE_HEADER_SIZE (sizeof (Header_t))

typedef struct SubmessageHeader {
  unsigned char submessageId;
  unsigned char flags;
  unsigned short octetsToNextHeader;
} SubmessageHeader_t;
#define RTPS_SUBMESSAGE_HEADER_SIZE (sizeof (SubmessageHeader_t))
#define SMFLAG_ENDIANNESS 0x01

typedef enum SubmessageKind {
  SMID_PAD = 0x01,
  SMID_ACKNACK = 0x06,
  SMID_HEARTBEAT = 0x07,
  SMID_GAP = 0x08,
  SMID_INFO_TS = 0x09,
  SMID_INFO_SRC = 0x0c,
  SMID_INFO_REPLY_IP4 = 0x0d,
  SMID_INFO_DST = 0x0e,
  SMID_INFO_REPLY = 0x0f,
  SMID_NACK_FRAG = 0x12,
  SMID_HEARTBEAT_FRAG = 0x13,
  SMID_DATA = 0x15,
  SMID_DATA_FRAG = 0x16,
  /* vendor-specific sub messages (0x80 .. 0xff) */
  SMID_PT_INFO_CONTAINER = 0x80,
  SMID_PT_MSG_LEN = 0x81,
  SMID_PT_ENTITY_ID = 0x82
} SubmessageKind_t;

typedef struct InfoTimestamp {
  SubmessageHeader_t smhdr;
  nn_ddsi_time_t time;
} InfoTimestamp_t;

typedef struct InfoDST {
  SubmessageHeader_t smhdr;
  nn_guid_prefix_t guid_prefix;
} InfoDST_t;

typedef struct InfoSRC {
  SubmessageHeader_t smhdr;
  unsigned unused;
  nn_protocol_version_t version;
  nn_vendorid_t vendorid;
  nn_guid_prefix_t guid_prefix;
} InfoSRC_t;

#if PLATFORM_IS_LITTLE_ENDIAN
#define PL_CDR_BE 0x0200
#define PL_CDR_LE 0x0300
#else
#define PL_CDR_BE 0x0002
#define PL_CDR_LE 0x0003
#endif

typedef unsigned short nn_parameterid_t; /* spec says short */
typedef struct nn_parameter {
  nn_parameterid_t parameterid;
  unsigned short length; /* spec says short */
  /* char value[]; O! how I long for C99 */
} nn_parameter_t;

typedef struct Data_DataFrag_common {
  SubmessageHeader_t smhdr;
  unsigned short extraFlags;
  unsigned short octetsToInlineQos;
  nn_entityid_t readerId;
  nn_entityid_t writerId;
  nn_sequence_number_t writerSN;
} Data_DataFrag_common_t;

typedef struct Data {
  Data_DataFrag_common_t x;
} Data_t;
#define DATA_FLAG_INLINE_QOS 0x02
#define DATA_FLAG_DATAFLAG 0x04
#define DATA_FLAG_KEYFLAG 0x08

typedef struct DataFrag {
  Data_DataFrag_common_t x;
  nn_fragment_number_t fragmentStartingNum;
  unsigned short fragmentsInSubmessage;
  unsigned short fragmentSize;
  unsigned sampleSize;
} DataFrag_t;
#define DATAFRAG_FLAG_INLINE_QOS 0x02
#define DATAFRAG_FLAG_KEYFLAG 0x04

typedef struct MsgLen {
  SubmessageHeader_t smhdr;
  os_uint32 length;
} MsgLen_t;

typedef struct AckNack {
  SubmessageHeader_t smhdr;
  nn_entityid_t readerId;
  nn_entityid_t writerId;
  nn_sequence_number_set_t readerSNState;
  /* nn_count_t count; */
} AckNack_t;
#define ACKNACK_FLAG_FINAL 0x02
#define ACKNACK_SIZE(numbits) (offsetof (AckNack_t, readerSNState) + NN_SEQUENCE_NUMBER_SET_SIZE (numbits) + 4)
#define ACKNACK_SIZE_MAX ACKNACK_SIZE (256)

typedef struct Gap {
  SubmessageHeader_t smhdr;
  nn_entityid_t readerId;
  nn_entityid_t writerId;
  nn_sequence_number_t gapStart;
  nn_sequence_number_set_t gapList;
} Gap_t;
#define GAP_SIZE(numbits) (offsetof (Gap_t, gapList) + NN_SEQUENCE_NUMBER_SET_SIZE (numbits))
#define GAP_SIZE_MAX GAP_SIZE (256)

typedef struct InfoTS {
  SubmessageHeader_t smhdr;
  nn_ddsi_time_t time;
} InfoTS_t;
#define INFOTS_INVALIDATE_FLAG 0x2

typedef struct Heartbeat {
  SubmessageHeader_t smhdr;
  nn_entityid_t readerId;
  nn_entityid_t writerId;
  nn_sequence_number_t firstSN;
  nn_sequence_number_t lastSN;
  nn_count_t count;
} Heartbeat_t;
#define HEARTBEAT_FLAG_FINAL 0x02
#define HEARTBEAT_FLAG_LIVELINESS 0x04

typedef struct HeartbeatFrag {
  SubmessageHeader_t smhdr;
  nn_entityid_t readerId;
  nn_entityid_t writerId;
  nn_sequence_number_t writerSN;
  nn_fragment_number_t lastFragmentNum;
  nn_count_t count;
} HeartbeatFrag_t;

typedef struct NackFrag {
  SubmessageHeader_t smhdr;
  nn_entityid_t readerId;
  nn_entityid_t writerId;
  nn_sequence_number_t writerSN;
  nn_fragment_number_set_t fragmentNumberState;
  /* nn_count_t count; */
} NackFrag_t;
#define NACKFRAG_SIZE(numbits) (offsetof (NackFrag_t, fragmentNumberState) + NN_FRAGMENT_NUMBER_SET_SIZE (numbits) + 4)
#define NACKFRAG_SIZE_MAX NACKFRAG_SIZE (256)

typedef struct PT_InfoContainer {
  SubmessageHeader_t smhdr;
  os_uint32 id;
} PT_InfoContainer_t;
#define PTINFO_ID_ENCRYPT (0x01)

typedef union Submessage {
  SubmessageHeader_t smhdr;
  AckNack_t acknack;
  Data_t data;
  DataFrag_t datafrag;
  InfoTS_t infots;
  InfoDST_t infodst;
  InfoSRC_t infosrc;
  Heartbeat_t heartbeat;
  HeartbeatFrag_t heartbeatfrag;
  Gap_t gap;
  NackFrag_t nackfrag;
  PT_InfoContainer_t pt_infocontainer;
} Submessage_t;

typedef struct ParticipantMessageData {
  nn_guid_prefix_t participantGuidPrefix;
  unsigned kind; /* really 4 octets */
  unsigned length;
  char value[1 /* length */];
} ParticipantMessageData_t;
#define PARTICIPANT_MESSAGE_DATA_KIND_UNKNOWN 0x0
#define PARTICIPANT_MESSAGE_DATA_KIND_AUTOMATIC_LIVELINESS_UPDATE 0x1
#define PARTICIPANT_MESSAGE_DATA_KIND_MANUAL_LIVELINESS_UPDATE 0x2
#define PARTICIPANT_MESSAGE_DATA_VENDER_SPECIFIC_KIND_FLAG 0x8000000

#define PID_VENDORSPECIFIC_FLAG 0x8000
#define PID_UNRECOGNIZED_INCOMPATIBLE_FLAG 0x4000

#define PID_PAD                                 0x0
#define PID_SENTINEL                            0x1
#define PID_USER_DATA                           0x2c
#define PID_TOPIC_NAME                          0x5
#define PID_TYPE_NAME                           0x7
#define PID_GROUP_DATA                          0x2d
#define PID_TOPIC_DATA                          0x2e
#define PID_DURABILITY                          0x1d
#define PID_DURABILITY_SERVICE                  0x1e
#define PID_DEADLINE                            0x23
#define PID_LATENCY_BUDGET                      0x27
#define PID_LIVELINESS                          0x1b
#define PID_RELIABILITY                         0x1a
#define PID_LIFESPAN                            0x2b
#define PID_DESTINATION_ORDER                   0x25
#define PID_HISTORY                             0x40
#define PID_RESOURCE_LIMITS                     0x41
#define PID_OWNERSHIP                           0x1f
#define PID_OWNERSHIP_STRENGTH                  0x6
#define PID_PRESENTATION                        0x21
#define PID_PARTITION                           0x29
#define PID_TIME_BASED_FILTER                   0x4
#define PID_TRANSPORT_PRIORITY                  0x49
#define PID_PROTOCOL_VERSION                    0x15
#define PID_VENDORID                            0x16
#define PID_UNICAST_LOCATOR                     0x2f
#define PID_MULTICAST_LOCATOR                   0x30
#define PID_MULTICAST_IPADDRESS                 0x11
#define PID_DEFAULT_UNICAST_LOCATOR             0x31
#define PID_DEFAULT_MULTICAST_LOCATOR           0x48
#define PID_METATRAFFIC_UNICAST_LOCATOR         0x32
#define PID_METATRAFFIC_MULTICAST_LOCATOR       0x33
#define PID_DEFAULT_UNICAST_IPADDRESS           0xc
#define PID_DEFAULT_UNICAST_PORT                0xe
#define PID_METATRAFFIC_UNICAST_IPADDRESS       0x45
#define PID_METATRAFFIC_UNICAST_PORT            0xd
#define PID_METATRAFFIC_MULTICAST_IPADDRESS     0xb
#define PID_METATRAFFIC_MULTICAST_PORT          0x46
#define PID_EXPECTS_INLINE_QOS                  0x43
#define PID_PARTICIPANT_MANUAL_LIVELINESS_COUNT 0x34
#define PID_PARTICIPANT_BUILTIN_ENDPOINTS       0x44
#define PID_PARTICIPANT_LEASE_DURATION          0x2
#define PID_CONTENT_FILTER_PROPERTY             0x35
#define PID_PARTICIPANT_GUID                    0x50
#define PID_PARTICIPANT_ENTITYID                0x51
#define PID_GROUP_GUID                          0x52
#define PID_GROUP_ENTITYID                      0x53
#define PID_BUILTIN_ENDPOINT_SET                0x58
#define PID_PROPERTY_LIST                       0x59
#define PID_TYPE_MAX_SIZE_SERIALIZED            0x60
#define PID_ENTITY_NAME                         0x62
#define PID_KEYHASH                             0x70
#define PID_STATUSINFO                          0x71
#define PID_CONTENT_FILTER_INFO                 0x55
#define PID_COHERENT_SET                        0x56
#define PID_DIRECTED_WRITE                      0x57
#define PID_ORIGINAL_WRITER_INFO                0x61
#define PID_ENDPOINT_GUID                       0x5a /* !SPEC <=> PRISMTECH_ENDPOINT_GUID */

/* Deprecated parameter IDs (accepted but ignored) */
#define PID_PERSISTENCE                         0x03
#define PID_TYPE_CHECKSUM                       0x08
#define PID_TYPE2_NAME                          0x09
#define PID_TYPE2_CHECKSUM                      0x0a
#define PID_EXPECTS_ACK                         0x10
#define PID_MANAGER_KEY                         0x12
#define PID_SEND_QUEUE_SIZE                     0x13
#define PID_RELIABILITY_ENABLED                 0x14
#define PID_VARGAPPS_SEQUENCE_NUMBER_LAST       0x17
#define PID_RECV_QUEUE_SIZE                     0x18
#define PID_RELIABILITY_OFFERED                 0x19

#define PID_PRISMTECH_WRITER_INFO               (PID_VENDORSPECIFIC_FLAG | 0x1)
#define PID_PRISMTECH_PARTICIPANT_VERSION_INFO  (PID_VENDORSPECIFIC_FLAG | 0x7)

/* parameter ids for READER_DATA_LIFECYCLE & WRITER_DATA_LIFECYCLE are
   undefined, but let's publish them anyway */
#define PID_PRISMTECH_READER_DATA_LIFECYCLE     (PID_VENDORSPECIFIC_FLAG | 0x2)
#define PID_PRISMTECH_WRITER_DATA_LIFECYCLE     (PID_VENDORSPECIFIC_FLAG | 0x3)

/* ENDPOINT_GUID is formally undefined, so in strictly conforming
   mode, we use our own */
#define PID_PRISMTECH_ENDPOINT_GUID             (PID_VENDORSPECIFIC_FLAG | 0x4)

#if defined (__cplusplus)
}
#endif

/* Relaxed QoS matching readers/writers are best ignored by
   implementations that don't understand them.  This also covers "old"
   DDSI2's, although they may emit an error. */
#define PID_PRISMTECH_RELAXED_QOS_MATCHING      (PID_VENDORSPECIFIC_FLAG | PID_UNRECOGNIZED_INCOMPATIBLE_FLAG | 0x6)

#endif /* NN_PROTOCOL_H */

/* SHA1 not available (unoffical build.) */
