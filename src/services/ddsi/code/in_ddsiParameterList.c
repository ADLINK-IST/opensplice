/*
 * in_ddsiParameterList.c
 *
 *  Created on: Mar 3, 2009
 *      Author: frehberg
 */

#include "in__ddsiParameterList.h"

#include "in__ddsiSerializer.h"
#include "in__ddsiDeserializer.h"
#include "in__ddsiParameter.h"
#include "in__ddsiParticipant.h"
#include "in__ddsiSubscription.h"
#include "in__ddsiPublication.h"
#include "in_connectivityWriterFacade.h"
#include "kernelModule.h"
#include "in_ddsiParameterList.h"
#include "in_ddsiElements.h"
#include "Coll_Iter.h"
#include "Coll_List.h"
#include "in_locatorList.h"
#include "in_align.h"
#include "in_report.h"
#include "in__config.h"
#include "in__configPartitioning.h"
#include "in__configChannel.h"
#include "in__locator.h"
#include "in__endpointDiscoveryData.h"

static in_long
serializeUint32(in_ddsiSerializer serializer,
        os_ushort pid,
        os_uint32 value);

static in_long
serializeOctet(in_ddsiSerializer serializer,
        os_ushort pid,
        in_octet value);

static in_long
serializeBoolean(in_ddsiSerializer serializer,
        os_ushort pid,
        c_bool value);

static in_long
serializeVendor(in_ddsiSerializer serializer,
        in_ddsiVendor vendor);

in_long
serializeGuid(in_ddsiSerializer serializer,
        os_ushort pid,
        in_ddsiGuid guid);

static in_long
serializeBuiltinEndpointSet(
        in_ddsiSerializer serializer,
        in_ddsiBuiltinEndpointSet builtinEndpoint);

static in_long
serializeProtocolVersion(in_ddsiSerializer serializer,
        in_ddsiProtocolVersion protocolVersion);

static in_long
serializeLocator(
        in_ddsiSerializer serializer,
        os_ushort pid,
        in_locator locator);

static in_long
serializeLocatorList(
        in_ddsiSerializer serializer,
        os_ushort pid,
        in_locatorList *locatorList);

static in_long
serializeTime(in_ddsiSerializer serializer,
        os_ushort pid,
        const c_time *time);

static in_long
serializeDuration(in_ddsiSerializer serializer,
        os_ushort pid,
        const c_time *leaseDuration);

static in_long
serializeProductVersion(in_ddsiSerializer serializer,
        const in_ddsiProductVersion productVersion);

static in_long
serializeString(in_ddsiSerializer serializer,
        os_ushort pid,
        const char*entityName);

static in_long
serializePresentation(in_ddsiSerializer serializer,
        const struct v_presentationPolicy *policy);

static in_long
serializeDDSITime(in_ddsiSerializer serializer,
        os_ushort pid,
        const in_ddsiTime time);

static in_long
serializeLiveliness(in_ddsiSerializer serializer,
        const struct v_livelinessPolicy *liveliness);

static in_long
serializeReliability(in_ddsiSerializer serializer,
        const struct v_reliabilityPolicy *policy);

in_long
serializeSentinel(in_ddsiSerializer serializer);

/**/
static in_long
forParticipantParseParameter(
        os_ushort paramId,
        in_ddsiDiscoveredParticipantData data,
        in_ddsiDeserializer deserializer);

/**/
static in_long
forReaderParseParameter(
    os_ushort paramId,
    in_ddsiDiscoveredReaderData data,
    in_ddsiDeserializer deserializer,
    c_base base);

/**/
static in_long
forWriterParseParameter(
        os_ushort paramId,
        in_ddsiDiscoveredWriterData data,
        in_ddsiDeserializer deserializer,
        c_base base);
/** */
static in_long
in_ddsiParameterListSeekToEnd(in_ddsiDeserializer deserializer)
{
	OS_STRUCT(in_ddsiParameterHeader) header;
	os_boolean continueScan = OS_TRUE;
	in_long nofOctets = 0;
	in_long total = 0;
	in_long result = -1;

	/* skip all parameters to find end of list */
	while (continueScan) {
		nofOctets =
			in_ddsiParameterHeaderInitFromBuffer(&header, deserializer);
		if (nofOctets < 0) {
		    assert(FALSE);
			continueScan = OS_FALSE;
			result = -1;
		} else {
			/* header parsed successfully, now
			 * check for Sentinel, otherwise seek the body*/
			total += nofOctets;
			if (header.id.value == IN_PID_SENTINEL) {
				/* end of list reached */
				continueScan = OS_FALSE;
				result = total;
			} else {
				/* seek body of parameter */
				nofOctets = in_ddsiDeserializerSeek(deserializer,
						(os_size_t) header.octetsToNextParameter);
				if (nofOctets < 0) {
				    assert(FALSE);
					continueScan = OS_FALSE; /* error */
					result = -1;
				} else {
					total += nofOctets;
				}
			}
		}
	}

	return result;
}

/** */
os_boolean
in_ddsiParameterListInitFromEncapsulation(
        in_ddsiParameterList _this,
        in_ddsiSerializedData serializedData)
{
    os_boolean result = OS_FALSE;
    if (serializedData->codecId != IN_CODEC_PL_CDR_BE &&
            serializedData->codecId != IN_CODEC_PL_CDR_LE) {
        result = OS_FALSE;
    } else {
        const os_boolean isBigE =
                 serializedData->codecId==IN_CODEC_PL_CDR_BE
                 ? OS_TRUE
                 : OS_FALSE;

        in_octet *firstParameter =
            serializedData->begin +
            IN_DDSI_ENCAPSULATION_HEADER_SIZE;

        const os_size_t octetLength =
            serializedData->length - IN_DDSI_ENCAPSULATION_HEADER_SIZE;

        _this->firstParameter = firstParameter;
        _this->isBigEndian = isBigE;
        _this->totalOctetLength = octetLength;

       result = OS_TRUE;
    }
    return result;
}

/** */
in_long
in_ddsiParameterListInitFromBuffer(in_ddsiParameterList _this,
		in_ddsiDeserializer deserializer)
{
	in_long result = -1;

	in_ddsiParameterToken firstParameter =
		in_ddsiDeserializerGetIndex(deserializer);
	os_boolean isBigEndian =
		in_ddsiDeserializerIsBigEndian(deserializer);
	in_long totalOctetLength =
		in_ddsiParameterListSeekToEnd(deserializer);

	if (totalOctetLength < 0) {
		result = -1;
		/* init */
		in_ddsiParameterListInitEmpty(_this);
	} else {
		result = totalOctetLength;
		_this->firstParameter = firstParameter;
		_this->isBigEndian = isBigEndian;
		_this->totalOctetLength = (os_size_t) totalOctetLength;
	}

	return result;
}


/**/
static in_long
forParticipantParseParameter(
        os_ushort paramId,
        in_ddsiDiscoveredParticipantData data,
        in_ddsiDeserializer deserializer)
{
    in_long result = -1;

    in_locator locator;
    OS_STRUCT(in_locator) tmpLocator;
    OS_STRUCT(in_ddsiVendor) vendor;
    switch(paramId) {
    case IN_PID_PAD: /* ( 0x0000 ) */
        /* just seek to next parameter */
        result = 0;
        break;

    case IN_PID_SENTINEL: /* ( 0x0001 ) */
        /* should never be reached */
        result = 0;
        break;
    case IN_PID_TOPIC_NAME: /* ( 0x0005 ) */
        /* ignore unexpected parameter*/
        result = 0;
        break;
    case IN_PID_TYPE_NAME: /* ( 0x0007 ) */
        /* ignore unexpected parameter*/
        result = 0;
        break;
    case IN_PID_PARTICIPANT_GUID: /* ( 0x005a ) */
        result = in_ddsiGuidPrefixInitFromBuffer(data->proxy.guidPrefix, deserializer);
        break;
    case IN_PID_EXPECTS_INLINE_QOS: /* ( 0x0043 ) */
        IN_TRACE_1(Receive,6, "Take note: expects_inline_qos (%0x) defaults to false\n",
                ((os_int) paramId) );
        data->proxy.expectsInlineQos = FALSE;
    case IN_PID_DEFAULT_UNICAST_LOCATOR: /* ( 0x0031 ) */
        result = in_locatorCopyFromBuffer(&tmpLocator, deserializer);
        if (result>=0) {
            locator = in_locatorClone(&tmpLocator);
            Coll_List_pushBack(&(data->proxy.defaultUnicastLocatorList), locator);
        }
        break;
    case IN_PID_DEFAULT_MULTICAST_LOCATOR: /* ( 0x0048 ) */
        result = in_locatorCopyFromBuffer(&tmpLocator, deserializer);
        if (result>=0) {
            locator = in_locatorClone(&tmpLocator);
            Coll_List_pushBack(&(data->proxy.defaultMulticastLocatorList), locator);
        }
        break;
    case IN_PID_METATRAFFIC_UNICAST_LOCATOR: /* ( 0x0032 ) */
        result = in_locatorCopyFromBuffer(&tmpLocator, deserializer);
        if (result>=0) {
            locator = in_locatorClone(&tmpLocator);
            Coll_List_pushBack(&(data->proxy.metatrafficUnicastLocatorList), locator);
        }
        break;
    case IN_PID_METATRAFFIC_MULTICAST_LOCATOR: /* ( 0x0033 ) */
        result = in_locatorCopyFromBuffer(&tmpLocator, deserializer);
        if (result) {
            locator = in_locatorClone(&tmpLocator);
            Coll_List_pushBack(&(data->proxy.metatrafficMulticastLocatorList), locator);
        }
        break;
    case IN_PID_VENDOR_ID: /* ( 0x0016 ) */
        result = in_ddsiVendorInitFromBuffer(&vendor, deserializer);
        if (result>=0) {
            data->proxy.vendorId[0] = vendor.vendorId[0];
            data->proxy.vendorId[1] = vendor.vendorId[1];
        }
        break;
    case IN_PID_PARTICIPANT_MANUAL_LIVELINESS_COUNT: /* ( 0x0034 ) */
        result = in_ddsiCountInitFromBuffer(
            &(data->proxy.manualLivelinessCount),
            deserializer);
        break;
    case IN_PID_PARTICIPANT_BUILTIN_ENDPOINTS: /* ( 0x0044 ) */
        result = in_ddsiBuiltinEndpointSetInitFromBuffer(
            &(data->proxy.availableBuiltinEndpoints),
            deserializer);
        break;
    case IN_PID_UNICAST_LOCATOR: /* ( 0x002f ) */
    case IN_PID_MULTICAST_LOCATOR: /* ( 0x0030 ) */
    case IN_PID_PARTICIPANT_LEASE_DURATION: /* ( 0x0002 ) */
    case IN_PID_TIME_BASED_FILTER: /* ( 0x0004 ) */
    case IN_PID_OWNERSHIP_STRENGTH: /* ( 0x0006 ) */
    case IN_PID_METATRAFFIC_MULTICAST_IPADDRESS: /* ( 0x000b ) */
    case IN_PID_DEFAULT_UNICAST_IPADDRESS: /* ( 0x000c ) */
    case IN_PID_METATRAFFIC_UNICAST_PORT: /* ( 0x000d ) */
    case IN_PID_DEFAULT_UNICAST_PORT: /* ( 0x000e ) */
    case IN_PID_MULTICAST_IPADDRESS: /* ( 0x0011 ) */
    case IN_PID_PROTOCOL_VERSION: /* ( 0x0015 ) */
    case IN_PID_RELIABILITY: /* ( 0x001a ) */
    case IN_PID_LIVELINESS: /* ( 0x001b ) */
    case IN_PID_DURABILITY: /* ( 0x001d ) */
    case IN_PID_DURABILITY_SERVICE: /* ( 0x001e ) */
    case IN_PID_OWNERSHIP: /* ( 0x001f ) */
    case IN_PID_PRESENTATION: /* ( 0x0021 ) */
    case IN_PID_DEADLINE: /* ( 0x0023 ) */
    case IN_PID_DESTINATION_ORDER: /* ( 0x0025 ) */
    case IN_PID_LATENCY_BUDGET: /* ( 0x0027 ) */
    case IN_PID_PARTITION: /* ( 0x0029 ) */
    case IN_PID_LIFESPAN: /* ( 0x002b ) */
    case IN_PID_USER_DATA: /* ( 0x002c ) */
    case IN_PID_GROUP_DATA: /* ( 0x002d ) */
    case IN_PID_TOPIC_DATA: /* ( 0x002e ) */

    case IN_PID_CONTENT_FILTER_PROPERTY: /* ( 0x0035 ) */
    case IN_PID_PROPERTY_LIST_OLD: /* ( 0x0036 ) */        /* For compatibility between 4.2d and 4.2e */
    case IN_PID_HISTORY: /* ( 0x0040 ) */
    case IN_PID_RESOURCE_LIMIT: /* ( 0x0041 ) */

    case IN_PID_METATRAFFIC_UNICAST_IPADDRESS: /* ( 0x0045 ) */
    case IN_PID_METATRAFFIC_MULTICAST_PORT: /* ( 0x0046 ) */
    case IN_PID_TRANSPORT_PRIORITY: /* ( 0x0049 ) */
    case IN_PID_ENDPOINT_GUID: /* ( 0x0050 ) */
    case IN_PID_PARTICIPANT_ENTITY_ID: /* ( 0x0051 ) */
    case IN_PID_GROUP_GUID: /* ( 0x0052 ) */
    case IN_PID_GROUP_ENTITY_ID: /* ( 0x0053 ) */
    case IN_PID_CONTENT_FILTER_INFO: /* ( 0x0055 ) */
    case IN_PID_COHERENT_SET: /* ( 0x0056 ) */
    case IN_PID_DIRECTED_WRITE: /* ( 0x0057 ) */
    case IN_PID_PROPERTY_LIST: /* ( 0x0059 ) */        /* RTI DDS 4.2e and newer */
    case IN_PID_TYPE_MAX_SIZE_SERIALIZED: /* ( 0x0060 ) */
    case IN_PID_ORIGINAL_WRITER_INFO: /* ( 0x0061 ) */
    case IN_PID_ENTITY_NAME: /* ( 0x0062 ) */
    case IN_PID_KEY_HASH: /* ( 0x0070 ) */
    case IN_PID_STATUS_INFO: /* ( 0x0071 ) */

    /* Vendor-specific: RTI */
    case IN_PID_PRODUCT_VERSION: /* ( 0x8000 ) */
    case IN_PID_PLUGIN_PROMISCUITY_KIND: /* ( 0x8001 ) */
    case IN_PID_ENTITY_VIRTUAL_GUID: /* ( 0x8002 ) */
    case IN_PID_SERVICE_KIND: /* ( 0x8003 ) */
    case IN_PID_TYPECODE: /* ( 0x8004 ) */        /* Was: 0x47 in RTPS 1.2 */
        /* should never be reached */
        IN_TRACE_1(Receive,6, "PID %0x not implemented, skipping",
                ((os_int) paramId));

        result = 0;
        break;
    default:
        /* unknown parameter, skip */
        IN_TRACE_1(Receive,6, "PID %0x unknown, skipping",
                ((os_int) paramId));
        result = 0;
        break;
    }

    return result;
}



/**/
static in_long
forReaderParseParameter(
    os_ushort paramId,
    in_ddsiDiscoveredReaderData data,
    in_ddsiDeserializer deserializer,
    c_base base)
{
    in_long result = -1;
    os_char* tmpStr;
    os_uint32 strLen;
    in_locator locator;
    OS_STRUCT(in_locator) tmpLocator;

    switch(paramId) {
    case IN_PID_PAD: /* ( 0x0000 ) */
        /* just seek to next parameter */
        result = 0;
        break;

    case IN_PID_SENTINEL: /* ( 0x0001 ) */
        /* should never be reached */
        result = 0;
        break;
    case IN_PID_TOPIC_NAME: /* ( 0x0005 ) */
        result = in_ddsiDeserializerReferenceString(deserializer, &tmpStr, &strLen);
        if (result>=0) {
            data->topicData.info.topic_name = c_stringNew(base, tmpStr);
        }
        break;
    case IN_PID_TYPE_NAME: /* ( 0x0007 ) */
        result = in_ddsiDeserializerReferenceString(deserializer, &tmpStr, &strLen);
        if (result>=0) {
            data->topicData.info.type_name = c_stringNew(base, tmpStr);
        }
        break;
    case IN_PID_UNICAST_LOCATOR: /* ( 0x002f ) */
        result = in_locatorCopyFromBuffer(&tmpLocator, deserializer);
        if (result>=0) {
            locator = in_locatorClone(&tmpLocator);
            Coll_List_pushBack(&(data->proxy.unicastLocatorList), locator);
        }
        break;
    case IN_PID_MULTICAST_LOCATOR: /* ( 0x0030 ) */
        result = in_locatorCopyFromBuffer(&tmpLocator, deserializer);
        if (result>=0) {
            locator = in_locatorClone(&tmpLocator);
            Coll_List_pushBack(&(data->proxy.multicastLocatorList), locator);
        }
        break;
    case IN_PID_ENDPOINT_GUID: /* ( 0x005a ) */
        result = in_ddsiGuidInitFromBuffer(&(data->proxy.remoteReaderGuid), deserializer);
        break;
    case IN_PID_EXPECTS_INLINE_QOS: /* ( 0x0043 ) */
        IN_TRACE_1(Receive,6, "Take note: expects_inline_qos (%0x) defaults to false\n",
                ((os_int) paramId) );
        data->proxy.expectsInlineQos = FALSE;
    case IN_PID_PARTICIPANT_LEASE_DURATION: /* ( 0x0002 ) */
    case IN_PID_TIME_BASED_FILTER: /* ( 0x0004 ) */

    case IN_PID_OWNERSHIP_STRENGTH: /* ( 0x0006 ) */

    case IN_PID_METATRAFFIC_MULTICAST_IPADDRESS: /* ( 0x000b ) */
    case IN_PID_DEFAULT_UNICAST_IPADDRESS: /* ( 0x000c ) */
    case IN_PID_METATRAFFIC_UNICAST_PORT: /* ( 0x000d ) */
    case IN_PID_DEFAULT_UNICAST_PORT: /* ( 0x000e ) */
    case IN_PID_MULTICAST_IPADDRESS: /* ( 0x0011 ) */
    case IN_PID_PROTOCOL_VERSION: /* ( 0x0015 ) */
    case IN_PID_VENDOR_ID: /* ( 0x0016 ) */
    case IN_PID_RELIABILITY: /* ( 0x001a ) */
    case IN_PID_LIVELINESS: /* ( 0x001b ) */
    case IN_PID_DURABILITY: /* ( 0x001d ) */
    case IN_PID_DURABILITY_SERVICE: /* ( 0x001e ) */
    case IN_PID_OWNERSHIP: /* ( 0x001f ) */
    case IN_PID_PRESENTATION: /* ( 0x0021 ) */
    case IN_PID_DEADLINE: /* ( 0x0023 ) */
    case IN_PID_DESTINATION_ORDER: /* ( 0x0025 ) */
    case IN_PID_LATENCY_BUDGET: /* ( 0x0027 ) */
    case IN_PID_PARTITION: /* ( 0x0029 ) */
    case IN_PID_LIFESPAN: /* ( 0x002b ) */
    case IN_PID_USER_DATA: /* ( 0x002c ) */
    case IN_PID_GROUP_DATA: /* ( 0x002d ) */
    case IN_PID_TOPIC_DATA: /* ( 0x002e ) */
    case IN_PID_DEFAULT_UNICAST_LOCATOR: /* ( 0x0031 ) */
    case IN_PID_METATRAFFIC_UNICAST_LOCATOR: /* ( 0x0032 ) */
    case IN_PID_METATRAFFIC_MULTICAST_LOCATOR: /* ( 0x0033 ) */
    case IN_PID_PARTICIPANT_MANUAL_LIVELINESS_COUNT: /* ( 0x0034 ) */
    case IN_PID_CONTENT_FILTER_PROPERTY: /* ( 0x0035 ) */
    case IN_PID_PROPERTY_LIST_OLD: /* ( 0x0036 ) */        /* For compatibility between 4.2d and 4.2e */
    case IN_PID_HISTORY: /* ( 0x0040 ) */
    case IN_PID_RESOURCE_LIMIT: /* ( 0x0041 ) */

    case IN_PID_PARTICIPANT_BUILTIN_ENDPOINTS: /* ( 0x0044 ) */
    case IN_PID_METATRAFFIC_UNICAST_IPADDRESS: /* ( 0x0045 ) */
    case IN_PID_METATRAFFIC_MULTICAST_PORT: /* ( 0x0046 ) */
    case IN_PID_DEFAULT_MULTICAST_LOCATOR: /* ( 0x0048 ) */
    case IN_PID_TRANSPORT_PRIORITY: /* ( 0x0049 ) */
    case IN_PID_PARTICIPANT_GUID: /* ( 0x0050 ) */
    case IN_PID_PARTICIPANT_ENTITY_ID: /* ( 0x0051 ) */
    case IN_PID_GROUP_GUID: /* ( 0x0052 ) */
    case IN_PID_GROUP_ENTITY_ID: /* ( 0x0053 ) */
    case IN_PID_CONTENT_FILTER_INFO: /* ( 0x0055 ) */
    case IN_PID_COHERENT_SET: /* ( 0x0056 ) */
    case IN_PID_DIRECTED_WRITE: /* ( 0x0057 ) */
    case IN_PID_BUILTIN_ENDPOINT_SET: /* ( 0x0058 ) */
    case IN_PID_PROPERTY_LIST: /* ( 0x0059 ) */        /* RTI DDS 4.2e and newer */
    case IN_PID_TYPE_MAX_SIZE_SERIALIZED: /* ( 0x0060 ) */
    case IN_PID_ORIGINAL_WRITER_INFO: /* ( 0x0061 ) */
    case IN_PID_ENTITY_NAME: /* ( 0x0062 ) */
    case IN_PID_KEY_HASH: /* ( 0x0070 ) */
    case IN_PID_STATUS_INFO: /* ( 0x0071 ) */

    /* Vendor-specific: RTI */
    case IN_PID_PRODUCT_VERSION: /* ( 0x8000 ) */
    case IN_PID_PLUGIN_PROMISCUITY_KIND: /* ( 0x8001 ) */
    case IN_PID_ENTITY_VIRTUAL_GUID: /* ( 0x8002 ) */
    case IN_PID_SERVICE_KIND: /* ( 0x8003 ) */
    case IN_PID_TYPECODE: /* ( 0x8004 ) */        /* Was: 0x47 in RTPS 1.2 */
        /* should never be reached */
        IN_TRACE_1(Receive,6, "PID %0x not implemented, skipping",
                ((os_int) paramId) );

        result = 0;
        break;
    default:
        /* unknown parameter, skip */
        IN_TRACE_1(Receive,6, "PID %0x unknown, skipping",
                ((os_int) paramId));
        result = 0;
        break;
    }

    return result;
}



/**/









static in_long
forWriterParseParameter(
    os_ushort paramId,
    in_ddsiDiscoveredWriterData data,
    in_ddsiDeserializer deserializer,
    c_base base)
{
    in_long result = -1;
    os_char* tmpStr;
    os_uint32 strLen;
    in_locator locator;
    OS_STRUCT(in_locator) tmpLocator;

    switch(paramId) {
    case IN_PID_PAD: /* ( 0x0000 ) */
        /* just seek to next parameter */
        result = 0;
        break;

    case IN_PID_SENTINEL: /* ( 0x0001 ) */
        /* should never be reached */
        result = 0;
        break;
    case IN_PID_TOPIC_NAME: /* ( 0x0005 ) */
        result = in_ddsiDeserializerReferenceString(deserializer, &tmpStr, &strLen);
        if (result>=0) {
            data->topicData.info.topic_name = c_stringNew(base, tmpStr);
        }
        break;
    case IN_PID_TYPE_NAME: /* ( 0x0007 ) */
        result = in_ddsiDeserializerReferenceString(deserializer, &tmpStr, &strLen);
        if (result >= 0) {
            data->topicData.info.type_name = c_stringNew(base, tmpStr);
        }
        break;
    case IN_PID_UNICAST_LOCATOR: /* ( 0x002f ) */
        result = in_locatorCopyFromBuffer(&tmpLocator, deserializer);
        if (result>=0) {
            locator = in_locatorClone(&tmpLocator);
            Coll_List_pushBack(&(data->proxy.unicastLocatorList), locator);
        }
        break;
    case IN_PID_MULTICAST_LOCATOR: /* ( 0x0030 ) */
        result = in_locatorCopyFromBuffer(&tmpLocator, deserializer);
        if (result>=0) {
            locator = in_locatorClone(&tmpLocator);
            Coll_List_pushBack(&(data->proxy.multicastLocatorList), locator);
        }
        break;
    case IN_PID_ENDPOINT_GUID: /* ( 0x005a ) */
        result = in_ddsiGuidInitFromBuffer(&(data->proxy.remoteWriterGuid), deserializer);
        break;
    case IN_PID_PARTITION: /* ( 0x0029 ) */
    case IN_PID_PARTICIPANT_LEASE_DURATION: /* ( 0x0002 ) */
    case IN_PID_TIME_BASED_FILTER: /* ( 0x0004 ) */
    case IN_PID_OWNERSHIP_STRENGTH: /* ( 0x0006 ) */
    case IN_PID_METATRAFFIC_MULTICAST_IPADDRESS: /* ( 0x000b ) */
    case IN_PID_DEFAULT_UNICAST_IPADDRESS: /* ( 0x000c ) */
    case IN_PID_METATRAFFIC_UNICAST_PORT: /* ( 0x000d ) */
    case IN_PID_DEFAULT_UNICAST_PORT: /* ( 0x000e ) */
    case IN_PID_MULTICAST_IPADDRESS: /* ( 0x0011 ) */
    case IN_PID_PROTOCOL_VERSION: /* ( 0x0015 ) */
    case IN_PID_VENDOR_ID: /* ( 0x0016 ) */
    case IN_PID_RELIABILITY: /* ( 0x001a ) */
    case IN_PID_LIVELINESS: /* ( 0x001b ) */
    case IN_PID_DURABILITY: /* ( 0x001d ) */
    case IN_PID_DURABILITY_SERVICE: /* ( 0x001e ) */
    case IN_PID_OWNERSHIP: /* ( 0x001f ) */
    case IN_PID_PRESENTATION: /* ( 0x0021 ) */
    case IN_PID_DEADLINE: /* ( 0x0023 ) */
    case IN_PID_DESTINATION_ORDER: /* ( 0x0025 ) */
    case IN_PID_LATENCY_BUDGET: /* ( 0x0027 ) */
    case IN_PID_LIFESPAN: /* ( 0x002b ) */
    case IN_PID_USER_DATA: /* ( 0x002c ) */
    case IN_PID_GROUP_DATA: /* ( 0x002d ) */
    case IN_PID_TOPIC_DATA: /* ( 0x002e ) */
    case IN_PID_DEFAULT_UNICAST_LOCATOR: /* ( 0x0031 ) */
    case IN_PID_METATRAFFIC_UNICAST_LOCATOR: /* ( 0x0032 ) */
    case IN_PID_METATRAFFIC_MULTICAST_LOCATOR: /* ( 0x0033 ) */
    case IN_PID_PARTICIPANT_MANUAL_LIVELINESS_COUNT: /* ( 0x0034 ) */
    case IN_PID_CONTENT_FILTER_PROPERTY: /* ( 0x0035 ) */
    case IN_PID_PROPERTY_LIST_OLD: /* ( 0x0036 ) */        /* For compatibility between 4.2d and 4.2e */
    case IN_PID_HISTORY: /* ( 0x0040 ) */
    case IN_PID_RESOURCE_LIMIT: /* ( 0x0041 ) */
    case IN_PID_EXPECTS_INLINE_QOS: /* ( 0x0043 ) */
    case IN_PID_PARTICIPANT_BUILTIN_ENDPOINTS: /* ( 0x0044 ) */
    case IN_PID_METATRAFFIC_UNICAST_IPADDRESS: /* ( 0x0045 ) */
    case IN_PID_METATRAFFIC_MULTICAST_PORT: /* ( 0x0046 ) */
    case IN_PID_DEFAULT_MULTICAST_LOCATOR: /* ( 0x0048 ) */
    case IN_PID_TRANSPORT_PRIORITY: /* ( 0x0049 ) */
    case IN_PID_PARTICIPANT_GUID: /* ( 0x0050 ) */
    case IN_PID_PARTICIPANT_ENTITY_ID: /* ( 0x0051 ) */
    case IN_PID_GROUP_GUID: /* ( 0x0052 ) */
    case IN_PID_GROUP_ENTITY_ID: /* ( 0x0053 ) */
    case IN_PID_CONTENT_FILTER_INFO: /* ( 0x0055 ) */
    case IN_PID_COHERENT_SET: /* ( 0x0056 ) */
    case IN_PID_DIRECTED_WRITE: /* ( 0x0057 ) */
    case IN_PID_BUILTIN_ENDPOINT_SET: /* ( 0x0058 ) */
    case IN_PID_PROPERTY_LIST: /* ( 0x0059 ) */        /* RTI DDS 4.2e and newer */
    case IN_PID_TYPE_MAX_SIZE_SERIALIZED: /* ( 0x0060 ) */
    case IN_PID_ORIGINAL_WRITER_INFO: /* ( 0x0061 ) */
    case IN_PID_ENTITY_NAME: /* ( 0x0062 ) */
    case IN_PID_KEY_HASH: /* ( 0x0070 ) */
    case IN_PID_STATUS_INFO: /* ( 0x0071 ) */

    /* Vendor-specific: RTI */
    case IN_PID_PRODUCT_VERSION: /* ( 0x8000 ) */
    case IN_PID_PLUGIN_PROMISCUITY_KIND: /* ( 0x8001 ) */
    case IN_PID_ENTITY_VIRTUAL_GUID: /* ( 0x8002 ) */
    case IN_PID_SERVICE_KIND: /* ( 0x8003 ) */
    case IN_PID_TYPECODE: /* ( 0x8004 ) */        /* Was: 0x47 in RTPS 1.2 */
        /* should never be reached */
        IN_TRACE_1(Receive,6, "PID %0x not implemented, skipping",
                ((os_int) paramId) );

        result = 0;
        break;
    default:
        /* unknown parameter, skip */
        IN_TRACE_1(Receive,6, "PID %0x unknown, skipping",
                ((os_int) paramId));
        result = 0;
        break;
    }

    return result;
}

/* may return with "out of memory" */
in_result
in_ddsiParameterListForParticipantParse(
        in_ddsiParameterList _this,
        in_ddsiDiscoveredParticipantData data)
{
    in_result result = IN_RESULT_OK;
    in_long seekSize = 0;
    in_long paddingSize = 0;
    in_long contentSize = 0;
    in_long headerSize = 0;
    in_long octetsToNextParameter = 0;
    in_long total = 0;
    os_boolean continueLoop = OS_TRUE;
    OS_STRUCT(in_ddsiDeserializer) deserializer;
    OS_STRUCT(in_ddsiParameterHeader) parameterHeader;
    in_ddsiDeserializerInitRaw(
            &deserializer,
            _this->firstParameter,
            _this->totalOctetLength,
            _this->isBigEndian);


    do {
        if ((os_size_t) total == _this->totalOctetLength) {
            /* end reached, but sentinel missing */
            continueLoop = OS_FALSE;
            result = IN_RESULT_OK;
        } else {
            headerSize =
                in_ddsiParameterHeaderInitFromBuffer(
                        &parameterHeader,
                        &deserializer);
            if (headerSize<0 ||
                headerSize>(in_long)IN_DDSI_PARAMETER_HEADER_SIZE) {
                IN_REPORT_WARNING(IN_SPOT, "invalid parameter header");
                continueLoop = OS_FALSE;
                result = IN_RESULT_ERROR;
            } else if (in_ddsiParameterHeaderId(&parameterHeader) == IN_PID_SENTINEL) {
                continueLoop = OS_FALSE;
                result = IN_RESULT_OK;
            } else {
                octetsToNextParameter =
                    parameterHeader.octetsToNextParameter;

                contentSize =
                    forParticipantParseParameter(
                        in_ddsiParameterHeaderId(&parameterHeader),
                        data,
                        &deserializer);

                if (contentSize < 0 ||
                    contentSize > octetsToNextParameter) {
                    IN_REPORT_WARNING(IN_SPOT,
                            "invalid parameter encoding");
                    continueLoop = OS_FALSE;
                    result = IN_RESULT_ERROR;
                } else {
                    paddingSize = octetsToNextParameter - contentSize;

                    if (contentSize > 0 && paddingSize >= (in_long) IN_DDSI_PARAMETER_HEADER_ALIGNMENT) {
                        IN_REPORT_WARNING(IN_SPOT, "inefficient padding");
                    }

                    seekSize =
                        in_ddsiDeserializerSeek(
                            &deserializer,
                            paddingSize);

                    assert(seekSize == paddingSize);

                    /* no further alignment expected. Verify! */
                    if (seekSize < 0 ||
                        0 != in_ddsiDeserializerAlign(
                                &deserializer,
                                IN_DDSI_PARAMETER_HEADER_ALIGNMENT)) {
                        IN_REPORT_WARNING(IN_SPOT, "alignment failed");
                        continueLoop = OS_FALSE;
                        result = IN_RESULT_ERROR;
                    } else {
                        total += headerSize + contentSize + paddingSize;
                    }
                }
            }
        }
    } while (continueLoop);
    return result;
}



/* may return with "out of memory" */
in_result
in_ddsiParameterListForReaderParse(
    in_ddsiParameterList _this,
    in_ddsiDiscoveredReaderData data,
    c_base base)
{
    in_result result = IN_RESULT_OK;
    in_long seekSize = 0;
    in_long paddingSize = 0;
    in_long contentSize = 0;
    in_long headerSize = 0;
    in_long octetsToNextParameter = 0;
    in_long total = 0;
    os_boolean continueLoop = OS_TRUE;
    OS_STRUCT(in_ddsiDeserializer) deserializer;
    OS_STRUCT(in_ddsiParameterHeader) parameterHeader;

    in_ddsiDeserializerInitRaw(
            &deserializer,
            _this->firstParameter,
            _this->totalOctetLength,
            _this->isBigEndian);


    do {
        if ((os_size_t)total == _this->totalOctetLength) {
            /* end reached */
        } else {
            headerSize =
                in_ddsiParameterHeaderInitFromBuffer(
                        &parameterHeader,
                        &deserializer);
            if (headerSize<0 ||
                headerSize>(in_long)IN_DDSI_PARAMETER_HEADER_SIZE) {
                IN_REPORT_WARNING(IN_SPOT, "invalid parameter header");
                continueLoop = OS_FALSE;
                result = IN_RESULT_ERROR;
            } else if (in_ddsiParameterHeaderId(&parameterHeader) == IN_PID_SENTINEL) {
                continueLoop = OS_FALSE;
                result = IN_RESULT_OK;
            } else {
                octetsToNextParameter =
                    parameterHeader.octetsToNextParameter;

                contentSize =
                    forReaderParseParameter(
                        in_ddsiParameterHeaderId(&parameterHeader),
                        data,
                        &deserializer,
                        base);

                if (contentSize < 0 ||
                    contentSize > octetsToNextParameter) {
                    IN_REPORT_WARNING(IN_SPOT,
                            "invalid parameter encoding");
                    continueLoop = OS_FALSE;
                    result = IN_RESULT_ERROR;
                } else {
                    paddingSize = octetsToNextParameter - contentSize;

                    if (contentSize > 0 && paddingSize >= (in_long) IN_DDSI_PARAMETER_HEADER_ALIGNMENT) {
                        IN_REPORT_WARNING(IN_SPOT, "inefficient padding");
                    }

                    seekSize =
                        in_ddsiDeserializerSeek(
                            &deserializer,
                            paddingSize);

                    assert(seekSize == paddingSize);

                    /* no further alignment expected. Verify! */
                    if (seekSize < 0 ||
                        0 != in_ddsiDeserializerAlign(
                                &deserializer,
                                IN_DDSI_PARAMETER_HEADER_ALIGNMENT)) {
                        IN_REPORT_WARNING(IN_SPOT, "alignment failed");
                        continueLoop = OS_FALSE;
                        result = IN_RESULT_ERROR;
                    } else {
                        total += headerSize + contentSize + paddingSize;
                    }
                }
            }
        }
    } while (continueLoop);

    return result;
}


/* may return with "out of memory" */
in_result
in_ddsiParameterListForWriterParse(
    in_ddsiParameterList _this,
    in_ddsiDiscoveredWriterData data,
    c_base base)
{
    in_result result = IN_RESULT_OK;
    in_long seekSize = 0;
    in_long paddingSize = 0;
    in_long contentSize = 0;
    in_long headerSize = 0;
    in_long octetsToNextParameter = 0;
    in_long total = 0;
    os_boolean continueLoop = OS_TRUE;
    OS_STRUCT(in_ddsiDeserializer) deserializer;
    OS_STRUCT(in_ddsiParameterHeader) parameterHeader;

    in_ddsiDeserializerInitRaw(
            &deserializer,
            _this->firstParameter,
            _this->totalOctetLength,
            _this->isBigEndian);


    do {
        if ((os_size_t)total == _this->totalOctetLength) {
            /* end reached */
        } else {
            headerSize =
                in_ddsiParameterHeaderInitFromBuffer(
                        &parameterHeader,
                        &deserializer);
            if (headerSize<0 ||
                headerSize>(in_long)IN_DDSI_PARAMETER_HEADER_SIZE) {
                IN_REPORT_WARNING(IN_SPOT, "invalid parameter header");
                continueLoop = OS_FALSE;
                result = IN_RESULT_ERROR;
            } else if (in_ddsiParameterHeaderId(&parameterHeader) == IN_PID_SENTINEL) {
                continueLoop = OS_FALSE;
                result = IN_RESULT_OK;
            } else {
                octetsToNextParameter =
                    parameterHeader.octetsToNextParameter;

                contentSize =
                    forWriterParseParameter(
                        in_ddsiParameterHeaderId(&parameterHeader),
                        data,
                        &deserializer,
                    base);

                if (contentSize < 0 ||
                    contentSize > octetsToNextParameter) {
                    IN_REPORT_WARNING(IN_SPOT,
                            "invalid parameter encoding");
                    continueLoop = OS_FALSE;
                    result = IN_RESULT_ERROR;
                } else {
                    paddingSize = octetsToNextParameter - contentSize;

                    if (contentSize > 0 && paddingSize >= (in_long) IN_DDSI_PARAMETER_HEADER_ALIGNMENT) {
                        IN_REPORT_WARNING(IN_SPOT, "inefficient padding");
                    }

                    seekSize =
                        in_ddsiDeserializerSeek(
                            &deserializer,
                            paddingSize);

                    assert(seekSize == paddingSize);

                    /* no further alignment expected. Verify! */
                    if (seekSize < 0 ||
                        0 != in_ddsiDeserializerAlign(
                                &deserializer,
                                IN_DDSI_PARAMETER_HEADER_ALIGNMENT)) {
                        IN_REPORT_WARNING(IN_SPOT, "alignment failed");
                        continueLoop = OS_FALSE;
                        result = IN_RESULT_ERROR;
                    } else {
                        total += headerSize + contentSize + paddingSize;
                    }
                }
            }
        }
    } while (continueLoop);

    return result;
}

/**  */
os_boolean
in_ddsiParameterListInitEmpty(in_ddsiParameterList _this)
{
	_this->firstParameter = NULL;
	_this->totalOctetLength = 0L;
	_this->isBigEndian = OS_FALSE;
	return OS_TRUE;
}


/** */
in_long
in_ddsiParameterListForPublicationSerializeInstantly(
		in_connectivityWriterFacade facade,
		in_ddsiSerializer serializer,
		in_endpointDiscoveryData discoveryData)
{
	/*
	 * +---------------+---------------+---------------+---------------+
	 * |                                                               |
	 * ~ ParameterList inlineQos [only if Q==1]                        ~
	 * |                                                               |
	 * +---------------+---------------+---------------+---------------+
	 */
    OS_STRUCT(in_ddsiProductVersion) productVersion =
         IN_DDSI_PRODUCT_VERSION;
    const os_char *entityName =
        IN_ENTITY_NAME_ENTITY;
    const c_time leaseDuration = {6,500000000};
    /*    C_TIME_INFINITE;  TODO */
    in_long result = -1;
	in_long nofOctets = 0;
	in_long total = 0;
    struct v_publicationInfo *info;
    in_ddsiGuid guid;

    struct v_livelinessPolicy dummyLiveliness =
        { V_LIVELINESS_AUTOMATIC,
            {0x7fffffff,0x7fffffffU} /* C_TIME_INFINITE */};

    struct v_reliabilityPolicy dummyReliability =
        { V_RELIABILITY_BESTEFFORT,
         {0, 250*1000*1000} /* 250 msecs blocking time, not sure this is relevant */};

    struct v_presentationPolicy dummyPresentation =
        { V_PRESENTATION_INSTANCE,
          FALSE, FALSE };


    assert(facade);
    assert(serializer);

	info = in_connectivityWriterFacadeGetInfo(facade);
    guid = in_connectivityEntityFacadeGetGuid(in_connectivityEntityFacade(facade));

	do
    {
	    nofOctets = serializeGuid(
	            serializer,
	            IN_PID_ENDPOINT_GUID,
	            guid);
	    if (nofOctets<0) break;
	    total += nofOctets;

        nofOctets = serializeString(
                serializer,
                IN_PID_TOPIC_NAME,
                info->topic_name);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = serializeString(
                serializer,
                IN_PID_TYPE_NAME,
                info->type_name);
        if (nofOctets<0) break;
        total += nofOctets;

//////////////////////////////
        nofOctets = serializeReliability(
                 serializer,
                 &dummyReliability);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = serializeUint32(
                  serializer,
                  IN_PID_OWNERSHIP_STRENGTH,
                  0);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = serializeLiveliness(
                serializer,
                &dummyLiveliness);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = serializeUint32(
                serializer,
                IN_PID_DURABILITY,
                V_DURABILITY_VOLATILE);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = serializeUint32(
                  serializer,
                  IN_PID_OWNERSHIP,
                  V_OWNERSHIP_SHARED);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = serializePresentation(
                    serializer,
                    &dummyPresentation);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = serializeUint32(
                   serializer,
                   IN_PID_DESTINATION_ORDER,
                   V_ORDERBY_RECEPTIONTIME);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = serializeTime(
                serializer,
                IN_PID_DEADLINE,
                &C_TIME_INFINITE);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = serializeTime(
                 serializer,
                 IN_PID_LATENCY_BUDGET,
                 &C_TIME_ZERO);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = serializeTime(
                 serializer,
                 IN_PID_LIFESPAN,
                 &C_TIME_INFINITE);
        if (nofOctets<0) break;
        total += nofOctets;

////////////////////////////

/*
        nofOctets = serializeBoolean(
                serializer,
                IN_PID_EXPECTS_ACK,
                OS_TRUE);
        if (nofOctets<0) break;
        total += nofOctets;
*/
        nofOctets = serializeDuration(
                serializer,
                IN_PID_PARTICIPANT_LEASE_DURATION,
                &leaseDuration);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = serializeProtocolVersion(
                serializer,
                &discoveryData->protocolVersion);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = serializeVendor(
                serializer,
                &discoveryData->vendor);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = serializeProductVersion(
                serializer,
                &productVersion);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = serializeString(
                serializer,
                IN_PID_ENTITY_NAME,
                entityName);
        if (nofOctets<0) break;
        total += nofOctets;

        /* write sentinel */
        nofOctets = serializeSentinel(serializer);
        if (nofOctets<0) break;
        total += nofOctets;
		/* final assignment */
		result = total;
	} while (0);

	return  result;
}



/** */
in_long
in_ddsiParameterListForSubscriptionSerializeInstantly(
    in_connectivityReaderFacade facade,
    in_ddsiSerializer serializer,
    in_endpointDiscoveryData discoveryData)
{
	/*
	 * +---------------+---------------+---------------+---------------+
	 * |                                                               |
	 * ~ ParameterList inlineQos [only if Q==1]                        ~
	 * |                                                               |
	 * +---------------+---------------+---------------+---------------+
	 */
    OS_STRUCT(in_ddsiProductVersion) productVersion =
         IN_DDSI_PRODUCT_VERSION;
    const os_char *entityName =
        IN_ENTITY_NAME_ENTITY;

    struct v_livelinessPolicy dummyLiveliness =
        { V_LIVELINESS_AUTOMATIC,
            {0x7fffffff,0x7fffffffU} /* C_TIME_INFINITE */};

    struct v_reliabilityPolicy dummyReliability =
        { V_RELIABILITY_BESTEFFORT,
         {0, 250*1000*1000} /* 250 msecs blocking time, not sure this is relevant */};

    struct v_presentationPolicy dummyPresentation =
        { V_PRESENTATION_INSTANCE,
          FALSE, FALSE };

    c_bool dummyExpectsInlineQos = 0x0; /* FALSE */

    in_long result = -1;
	in_long nofOctets = 0;
	in_long total = 0;
	struct v_subscriptionInfo *info;
    in_ddsiGuid guid;
    in_locatorList *uniLocators;
	in_locatorList *multiLocators;

    assert(facade);
    assert(serializer);

    info = in_connectivityReaderFacadeGetInfo(facade);
    guid = in_connectivityEntityFacadeGetGuid(in_connectivityEntityFacade(facade));
    uniLocators =
        &discoveryData->defaultUnicastLocatorList;
    multiLocators =
        &discoveryData->defaultMulticastLocatorList;

    do
    {
	    nofOctets = serializeGuid(serializer,
	            IN_PID_ENDPOINT_GUID,
	            guid);
	    if (nofOctets<0) break;
	    total += nofOctets;

        nofOctets = serializeString(
                serializer,
                IN_PID_TOPIC_NAME,
                info->topic_name);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = serializeString(
                serializer,
                IN_PID_TYPE_NAME,
                info->type_name);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = serializeUint32(
                serializer,
                IN_PID_DURABILITY,
                V_DURABILITY_VOLATILE);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = serializeTime(
                serializer,
                IN_PID_DEADLINE,
                &C_TIME_INFINITE);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = serializeTime(
                 serializer,
                 IN_PID_LATENCY_BUDGET,
                 &C_TIME_ZERO);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = serializeLiveliness(
                serializer,
                &dummyLiveliness);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = serializeReliability(
                 serializer,
                 &dummyReliability);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = serializeUint32(
                  serializer,
                  IN_PID_OWNERSHIP,
                  V_OWNERSHIP_SHARED);
        if (nofOctets<0) break;
        total += nofOctets;
//todo just added to see if it helps get things to work
        nofOctets = serializeUint32(
                  serializer,
                  IN_PID_TYPE_MAX_SIZE_SERIALIZED,
                  8);
        if (nofOctets<0) break;
        total += nofOctets;
//todo just added to see if it helps get things to work
        nofOctets = serializeTime(
                 serializer,
                 IN_PID_LIFESPAN,
                 &C_TIME_INFINITE);
        if (nofOctets<0) break;
        total += nofOctets;

// not done for ospl, but is done for RTI        IN_PID_GROUP_ENTITY_ID
//todo just added to see if it helps get things to work
        nofOctets = serializeUint32(
                  serializer,
                  IN_PID_OWNERSHIP_STRENGTH,
                  0);
        if (nofOctets<0) break;
        total += nofOctets;


        nofOctets = serializeUint32(
                   serializer,
                   IN_PID_DESTINATION_ORDER,
                   V_ORDERBY_RECEPTIONTIME);
        if (nofOctets<0) break;
        total += nofOctets;
/*
        nofOctets = serializeTime(
                    serializer,
                    IN_PID_TIME_BASED_FILTER,
                    &C_TIME_ZERO);
        if (nofOctets<0) break;
        total += nofOctets;
*/
        nofOctets = serializePresentation(
                    serializer,
                    &dummyPresentation);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = serializeBoolean(
                    serializer,
                    IN_PID_EXPECTS_INLINE_QOS,
                    dummyExpectsInlineQos);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = serializeProtocolVersion(
                serializer,
                &discoveryData->protocolVersion);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = serializeVendor(
                serializer,
                &discoveryData->vendor);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = serializeProductVersion(
                serializer,
                &productVersion);
        if (nofOctets<0) break;
        total += nofOctets;

        /* TODO: not sure if these are really necessary, should suffice to
         * submit the locators together with participant data
        nofOctets = serializeLocatorList(
                     serializer,
                     IN_PID_UNICAST_LOCATOR,
                     &(discoveryData->defaultUnicastLocatorList));
        if (nofOctets<0) break;
        total += nofOctets;
*/
        /* TODO: not sure if these are really necessary, should suffice to
          * submit the locators together with participant data
        nofOctets = serializeLocatorList(
                     serializer,
                     IN_PID_MULTICAST_LOCATOR,
                     &(discoveryData->defaultMulticastLocatorList));
        if (nofOctets<0) break;
        total += nofOctets;
*/
        /*
        nofOctets = serializeLeaseDuration(
                serializer,
                &leaseDuration);
        if (nofOctets<0) break;
        total += nofOctets;



        nofOctets = serializeString(
                serializer,
                IN_PID_ENTITY_NAME,
                entityName);
        if (nofOctets<0) break;
        total += nofOctets;
*/

        /* write sentinel */
        nofOctets = serializeSentinel(serializer);
        if (nofOctets<0) break;
        total += nofOctets;

		/* final assignment */
		result = total;
	} while (0);

	return  result;
}


/** */
in_long
in_ddsiParameterListForParticipantSerializeInstantly(
    in_connectivityParticipantFacade facade,
    in_ddsiSerializer serializer,
    in_endpointDiscoveryData discoveryData)
{
	/*
	 * +---------------+---------------+---------------+---------------+
	 * |                                                               |
	 * ~ ParameterList inlineQos [only if Q==1]                        ~
	 * |                                                               |
	 * +---------------+---------------+---------------+---------------+
	 */
    OS_STRUCT(in_ddsiProductVersion) productVersion =
        IN_DDSI_PRODUCT_VERSION;
    const os_char *entityName = IN_ENTITY_NAME_ENTITY;
    const c_time leaseDuration = {6,500000000}; /* C_TIME_INFINITE;  TODO */

    in_long result = -1;
	in_long nofOctets = 0;
	in_long total = 0;
	in_locatorList* defaultUnicastLocatorList =
        &discoveryData->defaultUnicastLocatorList;

	in_locatorList* defaultMulticastLocatorList =
	    &discoveryData->defaultMulticastLocatorList;

	in_locatorList* metaTrafficUnicastLocatorList =
	    &discoveryData->metatrafficUnicastLocatorList;

	in_locatorList* metaTrafficMulticastLocatorList =
	    &discoveryData->metatrafficMulticastLocatorList;

	/* should be provided by facade, check with Patrick */
	in_ddsiBuiltinEndpointSet endpointSet =
	    &discoveryData->availableBuiltinEndpoints;

    in_ddsiProtocolVersion protocolVersion =
        &discoveryData->protocolVersion;

    /* TODO add to discoveryData and rename endpointDiscoveryData to serviceConfig */
    in_ddsiVendor vendor = &discoveryData->vendor;

    struct v_participantInfo *info;
    in_ddsiGuid guid;

    assert(facade);
    assert(serializer);

    /* Step 1: init all relevant variables */

	guid = in_connectivityEntityFacadeGetGuid(in_connectivityEntityFacade(facade));
	info = in_connectivityParticipantFacadeGetInfo(facade);

	/* Step 2: start serialisation */
    do
    {
	    nofOctets = serializeGuid(
	            serializer,
	            IN_PID_PARTICIPANT_GUID,
	            guid);
	    if (nofOctets<0) break;
	    total += nofOctets;

	    nofOctets = serializeBuiltinEndpointSet(
	            serializer,
	            endpointSet);
	    if (nofOctets<0) break;
	    total += nofOctets;

	    nofOctets = serializeProtocolVersion(
	            serializer,
	            protocolVersion);
	    if (nofOctets<0) break;
	    total += nofOctets;

	    nofOctets = serializeVendor(
	            serializer,
	            vendor);
	    if (nofOctets<0) break;
	    total += nofOctets;

	    nofOctets = serializeLocatorList(
	            serializer,
	            IN_PID_DEFAULT_UNICAST_LOCATOR,
	            defaultUnicastLocatorList);
        if (nofOctets<0) break;
        total += nofOctets;

	    nofOctets = serializeLocatorList(
	            serializer,
	            IN_PID_DEFAULT_MULTICAST_LOCATOR,
	            defaultMulticastLocatorList);
        if (nofOctets<0) break;
        total += nofOctets;

	    nofOctets = serializeLocatorList(
	            serializer,
	            IN_PID_METATRAFFIC_UNICAST_LOCATOR,
	            metaTrafficUnicastLocatorList);
        if (nofOctets<0) break;
        total += nofOctets;

	    nofOctets = serializeLocatorList(
	            serializer,
	            IN_PID_METATRAFFIC_MULTICAST_LOCATOR,
	            metaTrafficMulticastLocatorList);
	    if (nofOctets<0) break;
        total += nofOctets;

	    nofOctets = serializeDuration(
	            serializer,
	            IN_PID_PARTICIPANT_LEASE_DURATION,
	            &leaseDuration);
	    if (nofOctets<0) break;
	    total += nofOctets;

	    nofOctets = serializeProductVersion(
	            serializer,
	            &productVersion);
        if (nofOctets<0) break;
        total += nofOctets;

	    nofOctets = serializeString(
	            serializer,
	            IN_PID_ENTITY_NAME,
	            entityName);
	    if (nofOctets<0) break;
	    total += nofOctets;

	    /* write sentinel */
	    nofOctets = serializeSentinel(serializer);
	    if (nofOctets<0) break;
	    total += nofOctets;

		/* final assignment */
		result = total;
	} while (0);

	return  result;
}

/* \return value multiple of 4 */
os_size_t
in_ddsiParameterListForParticipantCalculateSize(
    in_connectivityParticipantFacade facade,
    in_endpointDiscoveryData discoveryData)
{
    OS_STRUCT(in_ddsiSerializer) nilSerializer;

    os_size_t result = 0;
    in_long nofOctets;

    /* Nil mode allows dry run */
    in_ddsiSerializerInitNil(&nilSerializer);

    /**/
    nofOctets =
        in_ddsiParameterListForParticipantSerializeInstantly(
            facade,
            &nilSerializer,
            discoveryData);
    assert(nofOctets>=4);
    result = (os_size_t) nofOctets;
    assert(result % 4 == 0);

    in_ddsiSerializerDeinit(&nilSerializer);

    return result;
}

/* \return value multiple of 4 */
os_size_t
in_ddsiParameterListForPublicationCalculateSize(
        in_connectivityWriterFacade facade,
        in_endpointDiscoveryData discoveryData)
{
    OS_STRUCT(in_ddsiSerializer) nilSerializer;

    os_size_t result = 0;
    in_long nofOctets;

    in_ddsiSerializerInitNil(&nilSerializer);

    /**/
    nofOctets =
        in_ddsiParameterListForPublicationSerializeInstantly(
             facade,
             &nilSerializer,
        discoveryData);
    assert(nofOctets>=4);
    result = (os_size_t) nofOctets;

    in_ddsiSerializerDeinit(&nilSerializer);

    assert(result % 4 == 0);

    return result;
}

/* \return value multiple of 4 */
os_size_t
in_ddsiParameterListForSubscriptionCalculateSize(
        in_connectivityReaderFacade facade,
        in_endpointDiscoveryData discoveryData)
{
    OS_STRUCT(in_ddsiSerializer) nilSerializer;

    os_size_t result = 0;
    in_long nofOctets;


    in_ddsiSerializerInitNil(&nilSerializer);

    /**/
    nofOctets =
        in_ddsiParameterListForSubscriptionSerializeInstantly(
                facade,
                &nilSerializer,
        discoveryData);
    assert(nofOctets>=4);
    result = (os_size_t) nofOctets;

    in_ddsiSerializerDeinit(&nilSerializer);

    assert(result % 4 == 0);

    return result;
}



static in_long
serializeBuiltinEndpointSet(
        in_ddsiSerializer serializer,
        in_ddsiBuiltinEndpointSet builtinEndpoint)
{
    in_long nofOctets = 0;
    in_long result = -1;
    in_long total = 0;

    /* PID builtin endpoint */
    do {
        nofOctets = in_ddsiSerializerAppendUshort(serializer,
            IN_PID_BUILTIN_ENDPOINT_SET);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets =
            in_ddsiSerializerAppendUshort(serializer,
                    sizeof(in_ddsiBuiltinEndpointSet));
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets =
            in_ddsiBuiltinEndpointSetSerialize(
                    builtinEndpoint,
                    serializer);
        if (nofOctets<0) break;
        total += nofOctets;

        assert(in_ddsiSerializerAlign(serializer,
                IN_DDSI_SUBMESSAGE_HEADER_ALIGNMENT) == 0);

        result = total;
    } while (0);

    return result;
}

static in_long
serializeVendor(in_ddsiSerializer serializer,
        in_ddsiVendor vendor)

{
    in_long nofOctets = 0;
    in_long result = -1;
    in_long total = 0;

    const os_ushort octetsToNextParameter = (os_ushort)
        IN_ALIGN_UINT_CEIL(OS_SIZEOF(in_ddsiVendor),
                           IN_DDSI_PARAMETER_HEADER_ALIGNMENT);

     /* PID builtin endpoint */
    do {
        nofOctets = in_ddsiSerializerAppendUshort(
                serializer, IN_PID_VENDOR_ID);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = in_ddsiSerializerAppendUshort(
                serializer, octetsToNextParameter);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = in_ddsiVendorSerialize(vendor, serializer);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = in_ddsiSerializerAlign(
                serializer, IN_DDSI_PARAMETER_HEADER_ALIGNMENT); /* adding 2 padding octets */
        if (nofOctets<0) break;
        total += nofOctets;

        assert(in_ddsiSerializerAlign(serializer,
                IN_DDSI_SUBMESSAGE_HEADER_ALIGNMENT) == 0);

        result = total;
    } while (0);

    return result;
}

in_long
serializeSentinel(in_ddsiSerializer serializer)
{
    in_long nofOctets = 0;
    in_long result = -1;
    in_long total = 0;

    const os_ushort octetsToNextParameter = 0U;

    /* PID builtin endpoint */
    do {
        nofOctets = in_ddsiSerializerAppendUshort(serializer,
                IN_PID_SENTINEL);
        if (nofOctets<0) break;
        total += nofOctets;


        nofOctets = in_ddsiSerializerAppendUshort(serializer,
                octetsToNextParameter);
        if (nofOctets<0) break;
        total += nofOctets;

        assert(in_ddsiSerializerAlign(serializer,
                IN_DDSI_SUBMESSAGE_HEADER_ALIGNMENT) == 0);

        result = total;
    } while (0);

    return result;
}

in_long
serializeGuid(in_ddsiSerializer serializer,
        os_ushort pid,
        in_ddsiGuid guid)
{
    in_long nofOctets = 0;
     in_long result = -1;
     in_long total = 0;

     const os_ushort octetsToNextParameter =
         (os_ushort) sizeof(OS_STRUCT(in_ddsiGuid));

      /* PID builtin endpoint */
     do {
         nofOctets = in_ddsiSerializerAppendUshort(
                 serializer, pid);
         if (nofOctets<0) break;
         total += nofOctets;

         nofOctets = in_ddsiSerializerAppendUshort(
                 serializer, octetsToNextParameter);
         if (nofOctets<0) break;
         total += nofOctets;

         nofOctets = in_ddsiGuidSerialize(guid, serializer);
         if (nofOctets<0) break;
         total += nofOctets;

      //   assert(in_ddsiSerializerAlign(serializer,
      //           IN_DDSI_SUBMESSAGE_HEADER_ALIGNMENT) == 0);

         result = total;
     } while (0);

     return result;
}

static in_long
serializeUint32(in_ddsiSerializer serializer,
        os_ushort pid,
        os_uint32 value)
{
    in_long nofOctets = 0;
     in_long result = -1;
     in_long total = 0;

     const os_ushort octetsToNextParameter =
         (os_ushort) sizeof(value);

     assert(octetsToNextParameter == 4);

      /* PID builtin endpoint */
     do {
         nofOctets = in_ddsiSerializerAppendUshort(
                 serializer, pid);
         if (nofOctets<0) break;
         total += nofOctets;

         nofOctets = in_ddsiSerializerAppendUshort(
                 serializer, octetsToNextParameter);
         if (nofOctets<0) break;
         total += nofOctets;

         nofOctets = in_ddsiSerializerAppendUlong(
                 serializer,
                 value);
         if (nofOctets<0) break;
         total += nofOctets;

         assert(in_ddsiSerializerAlign(serializer,
                 IN_DDSI_SUBMESSAGE_HEADER_ALIGNMENT) == 0);

         result = total;
     } while (0);

     return result;
}


static in_long
serializeOctet(in_ddsiSerializer serializer,
        os_ushort pid,
        in_octet value)
{
    in_long nofOctets = 0;
    in_long result = -1;
    in_long total = 0;

    const os_ushort octetsToNextParameter =
        (os_ushort) sizeof(value) /* octet */ + 3 /* padding */ ;

    assert(octetsToNextParameter == 4);

    do {
        nofOctets = in_ddsiSerializerAppendUshort(
                serializer, pid);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = in_ddsiSerializerAppendUshort(
                serializer, octetsToNextParameter);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = in_ddsiSerializerAppendOctet(
                serializer,
                value);
        if (nofOctets<0) break;
        total += nofOctets;

        /* requires 3 octet padding */
        nofOctets = in_ddsiSerializerAlign(serializer,
                IN_DDSI_SUBMESSAGE_HEADER_ALIGNMENT);
        assert(nofOctets == 3);
        if (nofOctets<0) break;
        total += nofOctets;

        result = total;
    } while (0);

    return result;
}


static in_long
serializeBoolean(in_ddsiSerializer serializer,
        os_ushort pid,
        c_bool value)
{
    if (value) {
        return serializeOctet(serializer, pid, (in_octet)0x01);
    } else {
        return serializeOctet(serializer, pid, (in_octet)0x00);
    }
}

static in_long
serializeProtocolVersion(in_ddsiSerializer serializer,
        in_ddsiProtocolVersion protocolVersion)
{
    in_long nofOctets = 0;
    in_long result = -1;
    in_long total = 0;
    const os_size_t payloadSize =
        OS_SIZEOF(in_ddsiProtocolVersion);

    const os_size_t payloadSizeAligned =
        IN_ALIGN_UINT_CEIL(payloadSize, IN_DDSI_PARAMETER_HEADER_ALIGNMENT);

    const os_ushort octetsToNextParameter =
        (os_ushort) payloadSizeAligned;

    assert(payloadSizeAligned - payloadSize == 2U);

      /* PID builtin endpoint */
    do {
        nofOctets = in_ddsiSerializerAppendUshort(serializer,
            IN_PID_PROTOCOL_VERSION);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = in_ddsiSerializerAppendUshort(serializer,
            octetsToNextParameter);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = in_ddsiSerializerAppendOctet(
                serializer, protocolVersion->major);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = in_ddsiSerializerAppendOctet(
                serializer, protocolVersion->minor);
        if (nofOctets<0) break;
        total += nofOctets;

        /* Adding two padding octets */
        nofOctets = in_ddsiSerializerAlign(
                serializer, IN_DDSI_PARAMETER_HEADER_ALIGNMENT); /* two padding octets */
        if (nofOctets<0) break;
        total += nofOctets;

        assert(in_ddsiSerializerAlign(serializer,
                IN_DDSI_SUBMESSAGE_HEADER_ALIGNMENT) == 0);

        result = total;
    } while (0);

    return result;
}

static in_long
serializeLocator(
        in_ddsiSerializer serializer,
        os_ushort pid,
        in_locator locator)
{
    in_long nofOctets = 0;
    in_long result = -1;
    in_long total = 0;

    const os_ushort octetsToNextParameter = (os_ushort)
        OS_SIZEOF(in_ddsiLocator); /* already aligned*/

    assert(octetsToNextParameter % 4 == 0);

    /* PID *-Locator */
    assert(pid == IN_PID_DEFAULT_MULTICAST_LOCATOR ||
           pid == IN_PID_DEFAULT_UNICAST_LOCATOR ||
           pid == IN_PID_UNICAST_LOCATOR ||
           pid == IN_PID_MULTICAST_LOCATOR ||
           pid == IN_PID_METATRAFFIC_MULTICAST_LOCATOR ||
           pid == IN_PID_METATRAFFIC_UNICAST_LOCATOR);

    do {
        nofOctets = in_ddsiSerializerAppendUshort(serializer, pid);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = in_ddsiSerializerAppendUshort(serializer,
                octetsToNextParameter);
        if (nofOctets<0) break;
        total += nofOctets;

        /* serialize in_ddsiLocator, the parent-class of "locator" */
        nofOctets = in_locatorSerialize(locator, serializer);
        if (nofOctets<0) break;
        total += nofOctets;

        assert(in_ddsiSerializerAlign(serializer,
                IN_DDSI_SUBMESSAGE_HEADER_ALIGNMENT) == 0);

        result = total;
    } while (0);

    return result;
}

static in_long
serializeLocatorList(
    in_ddsiSerializer serializer,
    os_ushort pid,
    Coll_List* locatorList)
{
    in_long result = 0;
    in_long nofOctets = 0;

    Coll_Iter *iter = Coll_List_getFirstElement(locatorList);
    while (iter!=NULL) {
        in_locator loc = Coll_Iter_getObject(iter);
        nofOctets = serializeLocator(serializer, pid, loc);
        if (nofOctets<0) {
            result = -1;
            break;
        }
        result += nofOctets;
        iter = Coll_Iter_getNext(iter);
    }

    return result;
}

static in_long
serializeDDSITime(in_ddsiSerializer serializer,
        os_ushort pid,
        const in_ddsiTime  time)
{
    in_long nofOctets = 0;
    in_long result = -1;
    in_long total = 0;

    do {
        nofOctets = in_ddsiSerializerAppendUshort(serializer,
            pid);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = in_ddsiSerializerAppendUshort(serializer,
            sizeof(*time)); /* 8 octets */
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = in_ddsiTimeSerialize(time, serializer);
        if (nofOctets<0) break;
        total += nofOctets;

        /* paranoid check */
        assert(in_ddsiSerializerAlign(serializer,
                IN_DDSI_SUBMESSAGE_HEADER_ALIGNMENT) == 0);

        result = total;
    } while (0);

    return result;
}

static in_long
serializeLiveliness(in_ddsiSerializer serializer,
        const struct v_livelinessPolicy *liveliness)
{
    in_long nofOctets = 0;
    in_long result = -1;
    in_long total = 0;

    OS_STRUCT(in_ddsiTime) ddsiDuration;

    in_ddsiTimeInit(&ddsiDuration, &(liveliness->lease_duration), TRUE);

    do {
        nofOctets = in_ddsiSerializerAppendUshort(serializer,
            IN_PID_LIVELINESS);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = in_ddsiSerializerAppendUshort(serializer,
            OS_SIZEOF(in_ddsiTime) + sizeof(os_uint32)); /* 12 octets */
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = in_ddsiSerializerAppendUlong(
                serializer,
                liveliness->kind);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = in_ddsiTimeSerialize(
                &ddsiDuration,
                serializer);
        if (nofOctets<0) break;
        total += nofOctets;

        /* paranoid check */
        assert(in_ddsiSerializerAlign(serializer,
                IN_DDSI_SUBMESSAGE_HEADER_ALIGNMENT) == 0);

        result = total;
    } while (0);

    in_ddsiTimeDeinit(&ddsiDuration);

    return result;
}

static in_long
serializeReliability(in_ddsiSerializer serializer,
        const struct v_reliabilityPolicy *policy)
{
    in_long nofOctets = 0;
    in_long result = -1;
    in_long total = 0;
    os_uint32 kind = 0;

    OS_STRUCT(in_ddsiTime) ddsiDuration;

    in_ddsiTimeInit(&ddsiDuration, &(policy->max_blocking_time), TRUE);

    kind = IN_RELIABILITY_BEST_EFFORT; /* TODO hack */

    do {
        nofOctets = in_ddsiSerializerAppendUshort(serializer,
            IN_PID_RELIABILITY);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = in_ddsiSerializerAppendUshort(serializer,
            OS_SIZEOF(in_ddsiTime) + sizeof(os_uint32)); /* 12 octets */
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = in_ddsiSerializerAppendUlong(
                serializer,
                kind);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = in_ddsiTimeSerialize(
                &ddsiDuration,
                serializer);
        if (nofOctets<0) break;
        total += nofOctets;

        /* paranoid check */
        assert(in_ddsiSerializerAlign(serializer,
                IN_DDSI_SUBMESSAGE_HEADER_ALIGNMENT) == 0);

        result = total;
    } while (0);

    in_ddsiTimeDeinit(&ddsiDuration);

    return result;
}

static in_long
serializeTime(in_ddsiSerializer serializer,
        os_ushort pid,
        const c_time *time)
{
    in_long nofOctets = 0;
    in_long result = -1;
    in_long total = 0;

    OS_STRUCT(in_ddsiTime) ddsiTime;

    in_ddsiTimeInit(&ddsiTime, time, FALSE);

      /* PID builtin endpoint */
    do {
        nofOctets = serializeDDSITime(serializer,
            pid,
            &ddsiTime);
        if (nofOctets<0) break;
        total += nofOctets;

        result = total;
    } while (0);

    in_ddsiTimeDeinit(&ddsiTime);

    return result;
}

static in_long
serializeDuration(in_ddsiSerializer serializer,
        os_ushort pid,
        const c_time *duration)
{
    in_long nofOctets = 0;
    in_long result = -1;
    in_long total = 0;

    OS_STRUCT(in_ddsiTime) ddsiDuration;

    in_ddsiTimeInit(&ddsiDuration, duration, TRUE);

      /* PID builtin endpoint */
    do {
        nofOctets = serializeDDSITime(serializer,
            IN_PID_PARTICIPANT_LEASE_DURATION,
            &ddsiDuration);
        if (nofOctets<0) break;
        total += nofOctets;

        result = total;
    } while (0);

    in_ddsiTimeDeinit(&ddsiDuration);

    return result;
}

static in_long
serializeProductVersion(in_ddsiSerializer serializer,
        const in_ddsiProductVersion productVersion)
{
    in_long nofOctets = 0;
    in_long result = -1;
    in_long total = 0;

    /* PID builtin endpoint */
    do {
        nofOctets = in_ddsiSerializerAppendUshort(serializer,
            IN_PID_PRODUCT_VERSION);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = in_ddsiSerializerAppendUshort(serializer,
            sizeof(*productVersion)); /* next=4 */
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = in_ddsiProductVersionSerialize(productVersion, serializer);
        if (nofOctets<0) break;
        total += nofOctets;

        assert(in_ddsiSerializerAlign(serializer,
                IN_DDSI_SUBMESSAGE_HEADER_ALIGNMENT) == 0);

        result = total;
    } while (0);

    return result;
}

static in_long
serializeString(in_ddsiSerializer serializer,
        os_ushort pid,
        const char *entityName)
{
    in_long nofOctets = 0;
    in_long result = -1;
    in_long total = 0;

    assert(sizeof(os_uint32) == 4);

    if (entityName) {
        /* PID entity name */
        do {
            const os_size_t entityNameLength =
                sizeof(os_uint32) + strlen(entityName) + 1; /* LenVal + string + NULL-char */
            /* round up to match alignment constraints */
            const os_size_t entityNameLengthAligned =
                IN_ALIGN_UINT_CEIL(entityNameLength,
                        IN_DDSI_PARAMETER_HEADER_ALIGNMENT);
            const os_ushort octetsToNextParameter =
                /* the lower 16 bit */
                (os_ushort) (entityNameLengthAligned & 0xffff);

            nofOctets = in_ddsiSerializerAppendUshort(
                    serializer,
                    pid);
            if (nofOctets<0) break;
            total += nofOctets;

            nofOctets = in_ddsiSerializerAppendUshort(
                    serializer,
                    octetsToNextParameter);
            if (nofOctets<0) break;
            total += nofOctets;

            nofOctets = in_ddsiSerializerAppendString(serializer,
                    entityName);
            if (nofOctets<0) break;
            total += nofOctets;

            /* append 0-3 padding octets */
            nofOctets = in_ddsiSerializerAlign(serializer,
                    IN_DDSI_SUBMESSAGE_HEADER_ALIGNMENT);
            if (nofOctets<0) break;
            total += nofOctets;

            assert(in_ddsiSerializerAlign(serializer,
                    IN_DDSI_SUBMESSAGE_HEADER_ALIGNMENT) == 0);

            result = total;
        } while(0);
    }

    return result;
}

static in_long
serializePresentation(in_ddsiSerializer serializer,
        const struct v_presentationPolicy *policy)
{
    in_long nofOctets = 0;
    in_long result = -1;
    in_long total = 0;

    do {
        nofOctets = in_ddsiSerializerAppendUshort(serializer,
            IN_PID_PRESENTATION);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = in_ddsiSerializerAppendUshort(serializer, 8);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = in_ddsiSerializerAppendUlong(
                serializer,
                policy->access_scope);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = in_ddsiSerializerAppendOctet(
                serializer,
                /* CORBA::boolean is mapped onto a single octet */
                (in_octet)(policy->coherent_access & 0xff));
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = in_ddsiSerializerAppendOctet(
                serializer,
                /* CORBA::boolean is mapped onto a single octet */
                (in_octet)(policy->ordered_access & 0xff));
        if (nofOctets<0) break;
        total += nofOctets;

        /* requires two-octet padding to suffice the following constraint */
        nofOctets = in_ddsiSerializerAlign(serializer,
                IN_DDSI_SUBMESSAGE_HEADER_ALIGNMENT);
        assert(nofOctets== 2);
        if (nofOctets<0) break;
        total += nofOctets;

        result = total;
    } while (0);

    return result;
}

