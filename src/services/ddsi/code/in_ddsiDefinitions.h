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
#ifndef IN_DDSIDEFINITIONS_H_
#define IN_DDSIDEFINITIONS_H_

/* Core alignment of message headers */
#define IN_DDSI_CDR_BODY_ALIGNMENT (4U) /* should be 8, to support "longlong" */
#define IN_DDSI_SUBMESSAGE_HEADER_ALIGNMENT (4U)
#define IN_DDSI_PARAMETER_HEADER_ALIGNMENT (4U)
#define IN_DDSI_PAYLOAD_ALIGNMENT (4U)
#define IN_DDSI_PARAMETER_HEADER_SIZE (4U)
#define IN_DDSI_SUBMESSAGE_BODY_ALIGNMENT IN_DDSI_CDR_BODY_ALIGNMENT
#define IN_DDSI_MESSAGE_ALIGNMENT (4L)
#define IN_DDSI_SUBMESSAGE_ALIGNMENT (4L)

#define IN_DDSI_MESSAGE_HEADER_SIZE (20U)
#define IN_DDSI_SUBMESSAGE_HEADER_SIZE (4U)
#define IN_DDSI_SUBMESSAGE_DATA_HEADER_SIZE (20U)
#define IN_DDSI_ENCAPSULATION_HEADER_SIZE  (4U)
#define IN_DDSI_SUBMESSAGE_INFOTIMESTAMP_BODY_SIZE OS_SIZEOF(in_ddsiTime)
#define IN_DDSI_SUBMESSAGE_INFODESTINATION_BODY_SIZE sizeof(in_ddsiGuidPrefix)

/* Traffic type */
#define IN_PORT_BASE           (7400)
#define IN_PORT_METATRAFFIC_UNICAST    (0)
#define IN_PORT_USERTRAFFIC_MULTICAST  (1)
#define IN_PORT_METATRAFFIC_MULTICAST  (2)
#define IN_PORT_USERTRAFFIC_UNICAST    (3)

/* Flags defined in the 'flag' bitmask of a submessage */
#define IN_FLAG_E          (0x01)  /* Common to all the submessages */
#define IN_FLAG_DATA_Q     (0x02)
#define IN_FLAG_DATA_D     (0x04)
#define IN_FLAG_DATA_H     (0x08)
#define IN_FLAG_DATA_I     (0x10)

#define IN_FLAG_DATA_FRAG_Q    (0x02)
#define IN_FLAG_DATA_FRAG_H    (0x04)

#define IN_FLAG_NOKEY_DATA_Q   (0x02)
#define IN_FLAG_NOKEY_DATA_D   (0x04)
#define IN_FLAG_NOKEY_DATA_FRAG_Q  (0x02)
#define IN_FLAG_NOKEY_DATA_FRAG_D  (0x04)
#define IN_FLAG_ACKNACK_F      (0x02)

#define IN_FLAG_HEARTBEAT_F    (0x02)
#define IN_FLAG_HEARTBEAT_L    (0x04)

#define IN_FLAG_INFO_TS_T      (0x02)

#define IN_FLAG_INFO_REPLY_IP4_M   (0x02)

#define IN_FLAG_INFO_REPLY_M   (0x02)

#define IN_FLAG_RTPS_DATA_Q        (0x02)
#define IN_FLAG_RTPS_DATA_D        (0x04)

#define IN_FLAG_RTPS_DATA_FRAG_Q   (0x02)



/* The following PIDs are defined since RTPS 1.0 */
#define IN_PID_PAD                 (0x0000)
#define IN_PID_SENTINEL                (0x0001)
#define IN_PID_PARTICIPANT_LEASE_DURATION      (0x0002)
#define IN_PID_TIME_BASED_FILTER           (0x0004)
#define IN_PID_TOPIC_NAME              (0x0005)
#define IN_PID_OWNERSHIP_STRENGTH          (0x0006)
#define IN_PID_TYPE_NAME               (0x0007)
#define IN_PID_METATRAFFIC_MULTICAST_IPADDRESS (0x000b)
#define IN_PID_DEFAULT_UNICAST_IPADDRESS       (0x000c)
#define IN_PID_METATRAFFIC_UNICAST_PORT        (0x000d)
#define IN_PID_DEFAULT_UNICAST_PORT        (0x000e)
#define IN_PID_MULTICAST_IPADDRESS         (0x0011)
#define IN_PID_PROTOCOL_VERSION            (0x0015)
#define IN_PID_VENDOR_ID               (0x0016)
#define IN_PID_RELIABILITY             (0x001a)
#define IN_PID_LIVELINESS              (0x001b)
#define IN_PID_DURABILITY                  (0x001d)
#define IN_PID_DURABILITY_SERVICE          (0x001e)
#define IN_PID_OWNERSHIP                   (0x001f)
#define IN_PID_PRESENTATION                (0x0021)
#define IN_PID_DEADLINE                    (0x0023)
#define IN_PID_DESTINATION_ORDER               (0x0025)
#define IN_PID_LATENCY_BUDGET                  (0x0027)
#define IN_PID_PARTITION                   (0x0029)
#define IN_PID_LIFESPAN                (0x002b)
#define IN_PID_USER_DATA               (0x002c)
#define IN_PID_GROUP_DATA              (0x002d)
#define IN_PID_TOPIC_DATA              (0x002e)
#define IN_PID_UNICAST_LOCATOR         (0x002f)
#define IN_PID_MULTICAST_LOCATOR           (0x0030)
#define IN_PID_DEFAULT_UNICAST_LOCATOR     (0x0031)
#define IN_PID_METATRAFFIC_UNICAST_LOCATOR     (0x0032)
#define IN_PID_METATRAFFIC_MULTICAST_LOCATOR   (0x0033)
#define IN_PID_PARTICIPANT_MANUAL_LIVELINESS_COUNT (0x0034)
#define IN_PID_CONTENT_FILTER_PROPERTY     (0x0035)
#define IN_PID_PROPERTY_LIST_OLD           (0x0036)        /* For compatibility between 4.2d and 4.2e */
#define IN_PID_HISTORY             (0x0040)
#define IN_PID_RESOURCE_LIMIT          (0x0041)
#define IN_PID_EXPECTS_INLINE_QOS              (0x0043)
#define IN_PID_PARTICIPANT_BUILTIN_ENDPOINTS   (0x0044)
#define IN_PID_METATRAFFIC_UNICAST_IPADDRESS   (0x0045)
#define IN_PID_METATRAFFIC_MULTICAST_PORT      (0x0046)
#define IN_PID_DEFAULT_MULTICAST_LOCATOR           (0x0048)
#define IN_PID_TRANSPORT_PRIORITY                  (0x0049)
#define IN_PID_PARTICIPANT_GUID            (0x0050)
#define IN_PID_PARTICIPANT_ENTITY_ID       (0x0051)
#define IN_PID_GROUP_GUID              (0x0052)
#define IN_PID_GROUP_ENTITY_ID         (0x0053)
#define IN_PID_CONTENT_FILTER_INFO         (0x0055)
#define IN_PID_COHERENT_SET            (0x0056)
#define IN_PID_DIRECTED_WRITE                      (0x0057)
#define IN_PID_BUILTIN_ENDPOINT_SET                (0x0058)
#define IN_PID_PROPERTY_LIST           (0x0059)        /* RTI DDS 4.2e and newer */
#define IN_PID_ENDPOINT_GUID           (0x005a)
#define IN_PID_TYPE_MAX_SIZE_SERIALIZED            (0x0060)
#define IN_PID_ORIGINAL_WRITER_INFO                (0x0061)
#define IN_PID_ENTITY_NAME                         (0x0062)
#define IN_PID_KEY_HASH                            (0x0070)
#define IN_PID_STATUS_INFO                         (0x0071)

/* Vendor-specific: RTI */
#define IN_PID_PRODUCT_VERSION                     (0x8000)
#define IN_PID_PLUGIN_PROMISCUITY_KIND             (0x8001)
#define IN_PID_ENTITY_VIRTUAL_GUID                 (0x8002)
#define IN_PID_SERVICE_KIND                        (0x8003)
#define IN_PID_TYPECODE                (0x8004)        /* Was: 0x47 in RTPS 1.2 */

/* The following QoS are deprecated (used in RTPS 1.0 and older) */
#define IN_PID_PERSISTENCE             (0x0003)
#define IN_PID_TYPE_CHECKSUM           (0x0008)
#define IN_PID_TYPE2_NAME              (0x0009)
#define IN_PID_TYPE2_CHECKSUM          (0x000a)
#define IN_PID_IS_RELIABLE             (0x000f)
#define IN_PID_EXPECTS_ACK             (0x0010)
#define IN_PID_MANAGER_KEY             (0x0012)
#define IN_PID_SEND_QUEUE_SIZE         (0x0013)
#define IN_PID_RECV_QUEUE_SIZE         (0x0018)
#define IN_PID_VARGAPPS_SEQUENCE_NUMBER_LAST   (0x0017)
#define IN_PID_RELIABILITY_ENABLED         (0x0014)
#define IN_PID_RELIABILITY_OFFERED         (0x0019)
#define IN_PID_LIVELINESS_OFFERED          (0x001c)
#define IN_PID_OWNERSHIP_OFFERED           (0x0020)
#define IN_PID_PRESENTATION_OFFERED        (0x0022)
#define IN_PID_DEADLINE_OFFERED            (0x0024)
#define IN_PID_DESTINATION_ORDER_OFFERED       (0x0026)
#define IN_PID_LATENCY_BUDGET_OFFERED      (0x0028)
#define IN_PID_PARTITION_OFFERED           (0x002a)



/* appId.appKind possible values */
#define IN_APPKIND_UNKNOWN                     (0x00)
#define IN_APPKIND_MANAGED_APPLICATION         (0x01)
#define IN_APPKIND_MANAGER                     (0x02)


#define IN_GUIDPREFIX_UNKNOWN        {0x00,0x00,0x00,0x00, \
									  0x00,0x00,0x00,0x00, \
									  0x00,0x00,0x00,0x00}

#define UINT32_TO_OCTET(_uint) ((in_octet)((_uint) & 0xff))
#define UINT32_TO_ENTITYID(_uint) \
	{ { UINT32_TO_OCTET((_uint) >> 24), \
		UINT32_TO_OCTET((_uint) >> 16), \
		UINT32_TO_OCTET((_uint) >> 8) },\
	  UINT32_TO_OCTET((_uint)) } /* kind */

/*
 *  Predefined EntityIds */
#define IN_ENTITYID_UNKNOWN                    (0x00000000)
#define IN_ENTITYID_PARTICIPANT                    (0x000001c1)
#define IN_ENTITYID_SEDP_BUILTIN_TOPIC_WRITER              (0x000002c2)        /* Was: ENTITYID_BUILTIN_TOPIC_WRITER */
#define IN_ENTITYID_SEDP_BUILTIN_TOPIC_READER              (0x000002c7)        /* Was: ENTITYID_BUILTIN_TOPIC_READER */
#define IN_ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER       (0x000003c2)        /* Was: ENTITYID_BUILTIN_PUBLICATIONS_WRITER */
#define IN_ENTITYID_SEDP_BUILTIN_PUBLICATIONS_READER       (0x000003c7)        /* Was: ENTITYID_BUILTIN_PUBLICATIONS_READER */
#define IN_ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER      (0x000004c2)        /* Was: ENTITYID_BUILTIN_SUBSCRIPTIONS_WRITER */
#define IN_ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_READER      (0x000004c7)        /* Was: ENTITYID_BUILTIN_SUBSCRIPTIONS_READER */
#define IN_ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER        (0x000100c2)        /* Was: ENTITYID_BUILTIN_SDP_PARTICIPANT_WRITER */
#define IN_ENTITYID_SPDP_BUILTIN_PARTICIPANT_READER        (0x000100c7)        /* Was: ENTITYID_BUILTIN_SDP_PARTICIPANT_READER */
#define IN_ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_WRITER (0x000200c2)
#define IN_ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_READER (0x000200c7)


/* Deprecated EntityId */
#define IN_ENTITYID_APPLICATIONS_WRITER                    (0x000001c2)
#define IN_ENTITYID_APPLICATIONS_READER                (0x000001c7)
#define IN_ENTITYID_CLIENTS_WRITER                 (0x000005c2)
#define IN_ENTITYID_CLIENTS_READER                 (0x000005c7)
#define IN_ENTITYID_SERVICES_WRITER                (0x000006c2)
#define IN_ENTITYID_SERVICES_READER                (0x000006c7)
#define IN_ENTITYID_MANAGERS_WRITER                (0x000007c2)
#define IN_ENTITYID_MANAGERS_READER                (0x000007c7)
#define IN_ENTITYID_APPLICATION_SELF               (0x000008c1)
#define IN_ENTITYID_APPLICATION_SELF_WRITER            (0x000008c2)
#define IN_ENTITYID_APPLICATION_SELF_READER            (0x000008c7)

/* Predefined Entity Kind */
#define IN_ENTITYKIND_APPDEF_UNKNOWN               (0x00)
#define IN_ENTITYKIND_APPDEF_PARTICIPANT               (0x01)
#define IN_ENTITYKIND_APPDEF_WRITER_WITH_KEY           (0x02)
#define IN_ENTITYKIND_APPDEF_WRITER_NO_KEY             (0x03)
#define IN_ENTITYKIND_APPDEF_READER_NO_KEY             (0x04)
#define IN_ENTITYKIND_APPDEF_READER_WITH_KEY           (0x07)
#define IN_ENTITYKIND_BUILTIN_PARTICIPANT              (0xc1)
#define IN_ENTITYKIND_BUILTIN_WRITER_WITH_KEY          (0xc2)
#define IN_ENTITYKIND_BUILTIN_WRITER_NO_KEY            (0xc3)
#define IN_ENTITYKIND_BUILTIN_READER_NO_KEY            (0xc4)
#define IN_ENTITYKIND_BUILTIN_READER_WITH_KEY          (0xc7)


/* Submessage Type */
#define IN_SENTINEL                            (0x0)
#define IN_PAD                                 (0x01)
#define IN_DATA                                (0x02)
#define IN_NOKEY_DATA                          (0x03)
#define IN_ACKNACK                             (0x06)
#define IN_HEARTBEAT                           (0x07)
#define IN_GAP                             (0x08)
#define IN_INFO_TS                             (0x09)
#define IN_INFO_SRC                            (0x0c)
#define IN_INFO_REPLY_IP4                          (0x0d)
#define IN_INFO_DST                            (0x0e)
#define IN_INFO_REPLY                          (0x0f)
#define IN_DATA_FRAG                                       (0x10)  /* RTPS 2.0 Only */
#define IN_NOKEY_DATA_FRAG                                 (0x11)  /* RTPS 2.0 Only */
#define IN_NACK_FRAG                                       (0x12)  /* RTPS 2.0 Only */
#define IN_HEARTBEAT_FRAG                                  (0x13)  /* RTPS 2.0 Only */

#define IN_RTPS_DATA                                       (0x15)  /* RTPS 2.1 only */
#define IN_RTPS_DATA_FRAG                                  (0x16)  /* RTPS 2.1 only */
#define IN_ACKNACK_BATCH                                   (0x17)  /* RTPS 2.1 only */
#define IN_RTPS_DATA_BATCH                                 (0x18)  /* RTPS 2.1 Only */
#define IN_HEARTBEAT_BATCH                                 (0x19)  /* RTPS 2.1 only */

/* Data encapsulation */
#define IN_ENCAPSULATION_CDR_BE            (0x0000)
#define IN_ENCAPSULATION_CDR_LE            (0x0001)
#define IN_ENCAPSULATION_PL_CDR_BE         (0x0002)
#define IN_ENCAPSULATION_PL_CDR_LE         (0x0003)

/** Identical to IN_ENCAPSULATION* constants, but different byte-size */
typedef enum {
	IN_CODEC_CDR_BE    = IN_ENCAPSULATION_CDR_BE,
	IN_CODEC_CDR_LE    = IN_ENCAPSULATION_CDR_LE,
	IN_CODEC_PL_CDR_BE = IN_ENCAPSULATION_PL_CDR_BE,
	IN_CODEC_PL_CDR_LE = IN_ENCAPSULATION_PL_CDR_LE
} in_ddsiCodecId;
/* An invalid IP Address:
 */
#define IN_IPADDRESS_INVALID           (0)
#define IN_IPADDRESS_INVALID_STRING    "ADDRESS_INVALID (0x00000000)"
#define IN_IPADDRESS_ANY               {0x0,0x0,0x0,0x0, \
										0x0,0x0,0x0,0x0, \
										0x0,0x0,0x0,0x0, \
										0x0,0x0,0x0,0x0}
/* Identifies the value of an invalid port number:
 */
#define IN_PORT_INVALID            (0)
#define IN_PORT_INVALID_STRING     "PORT_INVALID"

/* Protocol Vendor Information (guint16) */
#define IN_DDSI_VENDOR_UNKNOWN     {0x00,0x00}
#define IN_DDSI_VENDOR_UNKNOWN_STRING  "VENDOR_ID_UNKNOWN (0x0000)"
#define IN_DDSI_VENDOR_RTI         {0x01,0x01}
#define IN_DDSI_VENDOR_RTI_STRING      "Real-Time Innovations, Inc."
#define IN_DDSI_VENDOR_PT_VAL1          0x00
#define IN_DDSI_VENDOR_PT_VAL2          0x00
#define IN_DDSI_VENDOR_PT          {IN_DDSI_VENDOR_PT_VAL1,IN_DDSI_VENDOR_PT_VAL2}
#define IN_DDSI_VENDOR_PT_STRING      "OpenSplice DDS PrismTech Ltd.www.prismtech.com"

#define IN_DDSI_PROTOCOL_ID_2_0 "RTPS"
#define IN_DDSI_PROTOCOL_VERSION_2_1      {0x02,0x01}
#define IN_DDSI_PRODUCT_VERSION_4_1_a_0   {0x4,0x1,'a',0x0}
#define IN_DDSI_PRODUCT_VERSION  { IN_DDSI_PRODUCT_VERSION_4_1_a_0 }

/***
 * TODO figure out correct vendor Ids
 * */

/* Parameter Liveliness */
#define IN_LIVELINESS_AUTOMATIC        (0)
#define IN_LIVELINESS_BY_PARTICIPANT   (1)
#define IN_LIVELINESS_BY_TOPIC     (2)

/* Parameter Durability */
#define IN_DURABILITY_VOLATILE     (0)
#define IN_DURABILITY_TRANSIENT_LOCAL  (1)
#define IN_DURABILITY_TRANSIENT        (2)
#define IN_DURABILITY_PERSISTENT       (3)

/* Parameter Ownership */
#define IN_OWNERSHIP_SHARED        (0)
#define IN_OWNERSHIP_EXCLUSIVE     (1)

/* Parameter Presentation */
#define IN_PRESENTATION_INSTANCE       (0)
#define IN_PRESENTATION_TOPIC      (1)
#define IN_PRESENTATION_GROUP      (2)


#define IN_LOCATOR_KIND_INVALID        (-1)
#define IN_LOCATOR_KIND_RESERVED       (0)
#define IN_LOCATOR_KIND_UDPV4      (1)
#define IN_LOCATOR_KIND_UDPV6      (2)

/* History Kind */
#define IN_HISTORY_KIND_KEEP_LAST          (0)
#define IN_HISTORY_KIND_KEEP_ALL           (1)

/* Reliability Values */
#define IN_RELIABILITY_BEST_EFFORT     (1)
#define IN_RELIABILITY_RELIABLE        (3)

/* Destination Order */
#define IN_BY_RECEPTION_TIMESTAMP      (0)
#define IN_BY_SOURCE_TIMESTAMP     (1)



/* Participant message data kind */
#define IN_PARTICIPANT_MESSAGE_DATA_KIND_UNKNOWN (0x00000000)
#define IN_PARTICIPANT_MESSAGE_DATA_KIND_AUTOMATIC_LIVELINESS_UPDATE (0x00000001)
#define IN_PARTICIPANT_MESSAGE_DATA_KIND_MANUAL_LIVELINESS_UPDATE (0x00000002)


#define IN_DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER (0x00000001 << 0)
#define IN_DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR (0x00000001 << 1)
#define IN_DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER (0x00000001 << 2)
#define IN_DISC_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR (0x00000001 << 3)
#define IN_DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER (0x00000001 << 4)
#define IN_DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR (0x00000001 << 5)
#define IN_DISC_BUILTIN_ENDPOINT_PARTICIPANT_PROXY_ANNOUNCER (0x00000001 << 6)
#define IN_DISC_BUILTIN_ENDPOINT_PARTICIPANT_PROXY_DETECTOR (0x00000001 << 7)
#define IN_DISC_BUILTIN_ENDPOINT_PARTICIPANT_STATE_ANNOUNCER (0x00000001 << 8)
#define IN_DISC_BUILTIN_ENDPOINT_PARTICIPANT_STATE_DETECTOR (0x00000001 << 9)
#define IN_BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_WRITER (0x00000001 << 10)
#define IN_BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_READER (0x00000001 << 11)

#define IN_DISC_BUILTIN_ENDPOINT_SET_ZERO (0U)
#define IN_DISC_BUILTIN_ENDPOINT_SET_DEFAULT ((os_uint32) ( \
        IN_DISC_BUILTIN_ENDPOINT_PARTICIPANT_ANNOUNCER | \
        IN_DISC_BUILTIN_ENDPOINT_PARTICIPANT_DETECTOR | \
		IN_DISC_BUILTIN_ENDPOINT_PUBLICATION_ANNOUNCER | \
		IN_DISC_BUILTIN_ENDPOINT_PUBLICATION_DETECTOR | \
        IN_DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_ANNOUNCER | \
		IN_DISC_BUILTIN_ENDPOINT_SUBSCRIPTION_DETECTOR | \
        IN_BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_WRITER | \
        IN_BUILTIN_ENDPOINT_PARTICIPANT_MESSAGE_DATA_READER))


#define  IN_ENTITY_NAME_ENTITY  "[ENTITY]"

#endif /*IN_DDSIDEFINITIONS_H_*/
