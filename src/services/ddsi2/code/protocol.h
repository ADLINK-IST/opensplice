/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2010 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#ifndef PROTOCOL_H
#define PROTOCOL_H

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

#include "rtps.h"

typedef struct { unsigned char id[2]; } vendorid_t;
typedef struct { unsigned char id[4]; } protocolid_t;
typedef struct { unsigned char major, minor; } protocol_version_t;
typedef struct { int high; unsigned low; } sequence_number_t;
typedef struct { sequence_number_t bitmap_base; unsigned numbits; unsigned bits[1]; } sequence_number_set_t; /* Why strict C90? zero-length/flexible array members are far nicer */
/* SequenceNumberSet size is base (2 words) + numbits (1 word) +
   bitmap ((numbits+31)/32 words), and this at 4 bytes/word */
#define SEQUENCE_NUMBER_SET_BITS_SIZE(numbits) (4 * ((numbits + 31) / 32))
#define SEQUENCE_NUMBER_SET_SIZE(numbits) (offsetof (sequence_number_set_t, bits) + SEQUENCE_NUMBER_SET_BITS_SIZE (numbits))
typedef struct { int seconds; unsigned fraction; } ddsi_time_t;
typedef ddsi_time_t duration_t; /* reverse engineered ... */
typedef int count_t;
/* port and address field in locator & locator_udpv4 maintained in
   network byte order - we don't do any processing on it, so why
   not */
typedef struct { int kind; unsigned port; unsigned char address[16]; } locator_t;
typedef struct { unsigned address; unsigned port; } locator_udpv4_t;

#define GUID_PREFIX_UNKNOWN_INITIALIZER {{0,0,0,0, 0,0,0,0, 0,0,0,0}}
typedef unsigned builtin_endpoint_set_t;
#define DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER (1 << 0)
#define DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR (1 << 1)
#define DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER (1 << 2)
#define DISC_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR (1 << 3)
#define DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER (1 << 4)
#define DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR (1 << 5)
/* couple of unspecified ones left out */
#define BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_WRITER (1 << 10)
#define BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_READER (1 << 11)

typedef enum reliability_kind {
  /* !SPEC - should be 0 and 1 */
  BEST_EFFORT_RELIABILITY_QOS = 1,
  RELIABLE_RELIABILITY_QOS = 2
} reliability_kind_t;

typedef struct reliability_qos {
  reliability_kind_t kind;
  duration_t max_blocking_time;
} reliability_qos_t;

typedef enum durability_kind {
  VOLATILE_DURABILITY_QOS = 0,
  TRANSIENT_LOCAL_DURABILITY_QOS = 1,
  TRANSIENT_DURABILITY_QOS = 2,
  PERSISTENT_DURABILITY_QOS = 3
} durability_kind_t;

#define LOCATOR_KIND_INVALID -1
#define LOCATOR_KIND_RESERVED 0
#define LOCATOR_KIND_UDPv4 1
#define LOCATOR_KIND_UDPv6 2
#define LOCATOR_PORT_INVALID 0

#define VENDORID_UNKNOWN	{{ 0x00, 0x00 }}
#define VENDORID_RTI		{{ 0x01, 0x01 }}
#define VENDORID_PRISMTECH	{{ 0x01, 0x02 }}
#define VENDORID_OCI		{{ 0x01, 0x03 }}
#define VENDORID_MILSOFT	{{ 0x01, 0x04 }}
#define VENDORID_KONGSBERG	{{ 0x01, 0x05 }}
#define VENDORID_TWINOAKS	{{ 0x01, 0x06 }}
#define VENDORID_LAKOTA		{{ 0x01, 0x07 }}
#define VENDORID_ICOUP		{{ 0x01, 0x08 }}

#define MY_VENDOR_ID VENDORID_PRISMTECH

/* Only one specific version is grokked */
#define RTPS_MAJOR 2
#define RTPS_MINOR 1

typedef struct Header {
  protocolid_t protocol;
  protocol_version_t version;
  vendorid_t vendorid;
  guid_prefix_t guid_prefix;
} Header_t;
#define PROTOCOLID_INITIALIZER {{ 'R','T','P','S' }}
#define PROTOCOL_VERSION_INITIALIZER {{ RTPS_MAJOR, RTPS_MINOR }}
#define VENDORID_INITIALIER MY_VENDOR_ID
#define HEADER_INITIALIZER { PROTOCOLID_INITIALIZER, PROTOCOL_VERSION_INITIALIZER, VENDORID_INITIALIER, GUID_PREFIX_UNKNOWN_INITIALIZER }
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
  SMID_DATA_FRAG = 0x16
} SubmessageKind_t;

typedef struct InfoTimestamp {
  SubmessageHeader_t smhdr;
  ddsi_time_t time;
} InfoTimestamp_t;

typedef struct InfoDST {
  SubmessageHeader_t smhdr;
  guid_prefix_t guid_prefix;
} InfoDST_t;

#if PLATFORM_IS_LITTLE_ENDIAN
#define PL_CDR_BE 0x0200
#define PL_CDR_LE 0x0300
#else
#define PL_CDR_BE 0x0002
#define PL_CDR_LE 0x0003
#endif

typedef unsigned short parameterid_t; /* spec says short */
typedef struct parameter {
  parameterid_t parameterid;
  unsigned short length; /* spec says short */
  /* char value[]; O! how I long for C99 */
} parameter_t;

typedef struct Data {
  SubmessageHeader_t smhdr;
  unsigned short extraFlags;
  unsigned short octetsToInlineQos;
  entityid_t readerId;
  entityid_t writerId;
  sequence_number_t writerSN;
} Data_t;
#define DATA_FLAG_INLINE_QOS 0x02
#define DATA_FLAG_DATAFLAG 0x04
#define DATA_FLAG_KEYFLAG 0x08

typedef struct AckNack {
  SubmessageHeader_t smhdr;
  entityid_t readerId;
  entityid_t writerId;
  sequence_number_set_t readerSNState;
  /* Count_t count; */
} AckNack_t;
#define ACKNACK_FLAG_FINAL 0x02
#define ACKNACK_SIZE(numbits) (offsetof (AckNack_t, readerSNState) + SEQUENCE_NUMBER_SET_SIZE (numbits) + 4)

typedef struct Gap {
  SubmessageHeader_t smhdr;
  entityid_t readerId;
  entityid_t writerId;
  sequence_number_t gapStart;
  sequence_number_set_t gapList;
} Gap_t;
#define GAP_SIZE(numbits) (offsetof (Gap_t, gapList) + SEQUENCE_NUMBER_SET_SIZE (numbits))

typedef struct InfoTS {
  SubmessageHeader_t smhdr;
  ddsi_time_t time;
} InfoTS_t;
#define INFOTS_INVALIDATE_FLAG 0x2

typedef struct Heartbeat {
  SubmessageHeader_t smhdr;
  entityid_t readerId;
  entityid_t writerId;
  sequence_number_t firstSN;
  sequence_number_t lastSN;
  count_t count;
} Heartbeat_t;
#define HEARTBEAT_FLAG_FINAL 0x02
#define HEARTBEAT_FLAG_LIVELINESS 0x04

typedef union Submessage {
  SubmessageHeader_t smhdr;
  AckNack_t acknack;
  Data_t data;
  InfoTS_t infots;
  InfoDST_t infodst;
  Heartbeat_t heartbeat;
  Gap_t gap;
} Submessage_t;

typedef struct ParticipantMessageData {
  guid_prefix_t participantGuidPrefix;
  unsigned kind;
  unsigned length;
  char value[1]; /* -- please move on beyond C90 */
} ParticipantMessageData_t;
#define PARTICIPANT_MESSAGE_DATA_KIND_UNKNOWN 0x0
#define PARTICIPANT_MESSAGE_DATA_KIND_AUTOMATIC_LIVELINESS_UPDATE 0x1
#define PARTICIPANT_MESSAGE_DATA_KIND_MANUAL_LIVELINESS_UPDATE 0x2
#define PARTICIPANT_MESSAGE_DATA_VENDER_SPECIFIC_KIND_FLAG 0x8000000

typedef enum liveliness_kind {
  AUTOMATIC_LIVELINESS_QOS,
  MANUAL_BY_PARTICIPANT_LIVELINESS_QOS,
  MANUAL_BY_TOPIC_LIVELINESS_QOS
} liveliness_kind_t;

typedef struct liveliness_qospolicy {
  liveliness_kind_t kind;
  duration_t lease_duration;
} liveliness_qospolicy_t;

typedef struct deadline_qospolicy {
  duration_t deadline;
} deadline_qospolicy_t;

typedef enum ownership_kind {
  SHARED_OWNERSHIP_QOS,
  EXCLUSIVE_OWNERSHIP_QOS
} ownership_kind_t;

typedef struct ownership_qospolicy {
  ownership_kind_t kind;
} ownership_qospolicy_t;

typedef enum destination_order_kind {
  BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS,
  BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS
} destination_order_kind_t;

typedef struct destination_order_qospolicy {
  destination_order_kind_t kind;
} destination_order_qospolicy_t;

typedef enum presentation_access_scope_kind {
  INSTANCE_PRESENTATION_QOS,
  TOPIC_PRESENTATION_QOS,
  GROUP_PRESENTATION_QOS
} presentation_access_scope_kind_t;

typedef struct presentation_qospolicy {
  presentation_access_scope_kind_t access_scope;
  char coherent_access;
  char ordered_access;
} presentation_qospolicy_t;

#define PID_VENDORSPECIFIC_FLAG 0x8000
#define PID_UNRECOGNIZED_INCOMPATIBLE_FLAG 0x4000

#define PID_PAD					0x0
#define PID_SENTINEL				0x1
#define PID_USER_DATA				0x2c
#define PID_TOPIC_NAME				0x5
#define PID_TYPE_NAME				0x7
#define PID_GROUP_DATA				0x2d
#define PID_TOPIC_DATA				0x2e
#define PID_DURABILITY				0x1d
#define PID_DURABILITY_SERVICE			0x1e
#define PID_DEADLINE				0x23
#define PID_LATENCY_BUDGET			0x27
#define PID_LIVELINESS				0x1b
#define PID_RELIABILITY				0x1a
#define PID_LIFESPAN				0x2b
#define PID_DESTINATION_ORDER			0x25
#define PID_HISTORY				0x40
#define PID_RESOURCE_LIMITS			0x41
#define PID_OWNERSHIP				0x1f
#define PID_OWNERSHIP_STRENGTH			0x6
#define PID_PRESENTATION			0x21
#define PID_PARTITION				0x29
#define PID_TIME_BASED_FILTER			0x4
#define PID_TRANSPORT_PRIORITY			0x49
#define PID_PROTOCOL_VERSION			0x15
#define PID_VENDORID				0x16
#define PID_UNICAST_LOCATOR			0x2f
#define PID_MULTICAST_LOCATOR			0x30
#define PID_MULTICAST_IPADDRESS			0x11
#define PID_DEFAULT_UNICAST_LOCATOR		0x31
#define PID_DEFAULT_MULTICAST_LOCATOR		0x48
#define PID_METATRAFFIC_UNICAST_LOCATOR		0x32
#define PID_METATRAFFIC_MULTICAST_LOCATOR	0x33
#define PID_DEFAULT_UNICAST_IPADDRESS		0xc
#define PID_DEFAULT_UNICAST_PORT		0xe
#define PID_METATRAFFIC_UNICAST_IPADDRESS	0x45
#define PID_METATRAFFIC_UNICAST_PORT		0xd
#define PID_METATRAFFIC_MULTICAST_IPADDRESS	0xb
#define PID_METATRAFFIC_MULTICAST_PORT		0x46
#define PID_EXPECTS_INLINE_QOS			0x43
#define PID_PARTICIPANT_MANUAL_LIVELINESS_COUNT	0x34
#define PID_PARTICIPANT_BUILTIN_ENDPOINTS	0x44
#define PID_PARTICIPANT_LEASE_DURATION		0x2
#define PID_CONTENT_FILTER_PROPERTY		0x35
#define PID_PARTICIPANT_GUID			0x50
#define PID_PARTICIPANT_ENTITYID		0x51
#define PID_GROUP_GUID				0x52
#define PID_GROUP_ENTITYID			0x53
#define PID_BUILTIN_ENDPOINT_SET		0x58
#define PID_PROPERTY_LIST			0x59
#define PID_TYPE_MAX_SIZE_SERIALIZED		0x60
#define PID_ENTITY_NAME				0x62
#define PID_KEY_HASH				0x70
#define PID_STATUS_INFO				0x71
#define PID_CONTENT_FILTER_INFO			0x55
#define PID_COHERENT_SET			0x56
#define PID_DIRECTED_WRITE			0x57
#define PID_ORIGINAL_WRITER_INFO		0x61
#define PID_ENDPOINT_GUID			0x5a /* !SPEC */

#define PID_PRISMTECH_WRITER_INFO		(PID_VENDORSPECIFIC_FLAG | 0x1)

#define SPDP_WELL_KNOWN_UNICAST_PORT 7410
#define SPDP_WELL_KNOWN_MULTICAST_PORT 7400
#define DEFAULT_MULTICAST_PORT 7401
#define DEFAULT_UNICAST_PORT 7411

#define PORT_d0 0
#define PORT_d1 10
#define PORT_d2 1
#define PORT_d3 11
#define PORT_BASE 7400
#define PORT_PG 2
#define PORT_DG 250

#endif /* PROTOCOL_H */
