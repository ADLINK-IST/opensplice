/*
 * in_ddsiStreamWriterImpl.c
 *
 *  Created on: Mar 2, 2009
 *      Author: frehberg
 */

/* **** interface headers **** */
#include "in__ddsiStreamWriter.h"

/* **** implementation headers **** */
#include "in__object.h"
#include "os_heap.h"
#include "v_message.h"
#include "in_ddsiDefinitions.h"
#include "in_ddsiElements.h"
#include "in_ddsiSubmessage.h"
#include "in__ddsiSerializer.h"
#include "in_ddsiParameterList.h"
#include "in_transportSender.h"
#include "Coll_List.h"
#include "Coll_Iter.h"
#include "in_connectivityReaderFacade.h"
#include "in_connectivityParticipantFacade.h"
#include "in_connectivityWriterFacade.h"
#include "in_connectivityEntityFacade.h"
#include "v_topic.h"

#include "in__ddsiParticipant.h"
#include "in__ddsiSubscription.h"
#include "in__ddsiPublication.h"
#include "in_align.h"
#include "in_report.h"
#include "in_ddsiEncapsulationHeader.h"

/* ***** macro ***** */
#define IN_BREAK_IF(_cond) \
    if (_cond) { \
        IN_REPORT_ERROR(__FILE__, "serialization failure"); \
        break; \
    }

/* ***** constants ***** */

#define IN_DDSI_INFOTIMESTAMP_SUBMESSAGE_SIZE \
	(IN_DDSI_SUBMESSAGE_HEADER_SIZE + \
	 IN_DDSI_SUBMESSAGE_INFOTIMESTAMP_BODY_SIZE)

#define IN_DDSI_INFODESTINATION_SUBMESSAGE_SIZE \
    (IN_DDSI_SUBMESSAGE_HEADER_SIZE + \
     IN_DDSI_SUBMESSAGE_INFODESTINATION_BODY_SIZE)

static  OS_STRUCT(in_ddsiEntityId) unknownId =
    UINT32_TO_ENTITYID(IN_ENTITYID_UNKNOWN);

/* **** private functions **** */

/*TODO: put in corect headerfile*/

static void
in_ddsiStreamWriterImplDeinit(in_ddsiStreamWriterImpl _this)
{
    in_ddsiSerializerDeinit(&(_this->serializer));
    in_messageSerializerFree(_this->messageSerializer);

    if (_this->currentSendBuffer) {
        in_transportSenderReleaseBuffer(_this->transportSender,
                _this->currentSendBuffer);
        _this->currentSendBuffer = NULL;
    }

    if(_this->plugKernel)
    {
        in_plugKernelFree(_this->plugKernel);
        _this->plugKernel = NULL;
    }

    in_transportSenderFree(_this->transportSender);
    _this->transportSender = NULL;

	in_streamWriterDeinit(OS_SUPER(_this));
}

static os_size_t
calculateDataSubmessageSize(
        in_ddsiStreamWriterImpl _this,
		v_message message,
		in_connectivityWriterFacade facade,
		os_boolean recipientExpectsInlineQos)
{
	os_size_t result = 800; /* debug */

	assert(result % 4 == 0);
	return result;
}

/** non-fragmented transfer of data
 *
 * requries that all message items fit into the current buffer
 *
 * \return -1 in case of error, otherwise the number of octets
 * written in total */
static in_result
serializeParticipantData(
        in_ddsiStreamWriterImpl _this,
        os_size_t serializedPayloadSize,
        in_ddsiEntityId readerId,
        in_endpointDiscoveryData discoveryData,
        in_connectivityParticipantFacade facade)
{
    /* no inline Qos */
    const os_size_t inlineQosSize = 24U;


    in_result result = IN_RESULT_ERROR;

    in_long nofOctets = 0;
    os_size_t total = 0;


    in_ddsiGuid guid =
        in_connectivityEntityFacadeGetGuid(
                in_connectivityEntityFacade(facade));


    /* writer is the builtin writer */
   OS_STRUCT(in_ddsiEntityId) writerId =
        UI2ENTITYID(IN_ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER);

    do {
        /* (FR) AFAICS, participants are published best-effort with sequenceNumber=1 */
        OS_STRUCT(in_ddsiSequenceNumber) sequenceNumber = {0,1U}; /* TODO read from facade */
        /* PW: shouldn't this be a builtin writer related seq counter instead of writerfacade related*/

        os_time timestamp = in_connectivityEntityFacadeGetTimestamp(in_connectivityEntityFacade(facade));
        c_time allocTime;
        allocTime.seconds = timestamp.tv_sec;
        allocTime.nanoseconds = timestamp.tv_nsec;

        nofOctets =
            in_ddsiSubmessageInfoTimestampSerializeInstantly(
                    &allocTime,
                    &(_this->serializer));
        IN_BREAK_IF(nofOctets<0);
        total += nofOctets;

        nofOctets =
            in_ddsiSubmessageDataHeaderSerializeInstantly(
                    inlineQosSize,
                    serializedPayloadSize,
                    &sequenceNumber,
                    readerId,
                    &writerId,
                    &(_this->serializer));
        IN_BREAK_IF(nofOctets<0);
        total += nofOctets;

	    nofOctets = serializeGuid(
	            &(_this->serializer),
	            IN_PID_KEY_HASH,
	            guid);
	    if (nofOctets<0) break;
	    total += nofOctets;

        nofOctets = serializeSentinel(&(_this->serializer));
        if (nofOctets<0) break;
        total += nofOctets;

        if (serializedPayloadSize > 0) {
            const os_ushort encapsKind =
                 in_ddsiSerializerIsBigEndian(&(_this->serializer))
                 ? IN_ENCAPSULATION_PL_CDR_BE
                 : IN_ENCAPSULATION_PL_CDR_LE;

            /* DATA submessage does not support anything larger than that */
            assert(serializedPayloadSize <= IN_USHORT_MAX);

            /* add the encapsulation header  */
            nofOctets =
                in_ddsiSubmessageAppendEncapsulationHeaderInstantly(
                        encapsKind, &(_this->serializer));
            IN_BREAK_IF(nofOctets<0);

            /* TODO: handle fragmented parameter list */
            nofOctets =
                in_ddsiParameterListForParticipantSerializeInstantly(
                        facade,
                        &(_this->serializer),
                        discoveryData);
            IN_BREAK_IF(nofOctets<0);
            total += nofOctets;

            /* Assumption was, that no fragmentation is required  so
             * all data fits into the current buffer. Futheron the number
             * of octets of serialized parameter list must be a multiple
             * of 4 (PARAMETER_HEADER_ALIGNMENT). The following verifies
             * valid alignment, return with error if otherwise. */
            IN_BREAK_IF(0!=in_ddsiSerializerAlign(
                    &(_this->serializer),
                    IN_DDSI_PARAMETER_HEADER_ALIGNMENT));
        }
        /* finally assign result */
        result = IN_RESULT_OK;
    } while(0); /* never repeat */

    return result;
}

in_long
serializeParticipantMessage(
    in_ddsiSerializer serializer,
    os_size_t serializedPayloadSize,
    in_ddsiEntityId readerId,
    in_ddsiEntityId writerId,
    in_ddsiGuidPrefixRef destGuidPrefix,
    in_connectivityParticipantFacade facade)
{
	/*PID_KEY_HASH (20 bytes) + PID_SENTINEL (2 bytes) + 2 bytes padding*/
    const os_size_t inlineQosSize = 24U;
    in_long result = -1;
    in_long nofOctets = 0;
    os_size_t total = 0;
    in_ddsiGuid guid;
    OS_STRUCT(in_ddsiGuid) tmpGuid;
    c_time allocTime;
    os_time timestamp;
    /* TODO resolve sequence number from facade */
    OS_STRUCT(in_ddsiSequenceNumber) sequenceNumber = {0,1U};

    assert(serializer);
    assert(readerId);
    assert(writerId);
    assert(facade);

    guid = in_connectivityEntityFacadeGetGuid(in_connectivityEntityFacade(facade));
    do
    {
        timestamp = in_connectivityEntityFacadeGetTimestamp(in_connectivityEntityFacade(facade));
        allocTime.seconds = timestamp.tv_sec;
        allocTime.nanoseconds = timestamp.tv_nsec;

        nofOctets = in_ddsiSubmessageInfoDestinationSerializeInstantly(destGuidPrefix, serializer);
        if (nofOctets<0) break;
        total += nofOctets;

        nofOctets = in_ddsiSubmessageInfoTimestampSerializeInstantly(&allocTime, serializer);
        IN_BREAK_IF(nofOctets<0);
        total += nofOctets;

        nofOctets = in_ddsiSubmessageDataHeaderSerializeInstantly(
            inlineQosSize,
            serializedPayloadSize,
            &sequenceNumber,
            readerId,
            writerId,
            serializer);
        IN_BREAK_IF(nofOctets<0);
        total += nofOctets;

	    nofOctets = serializeGuid(
	            serializer,
	            IN_PID_KEY_HASH,
	            guid);
	    if (nofOctets<0) break;
	    total += nofOctets;

        nofOctets = serializeSentinel(serializer);
        if (nofOctets<0) break;
        total += nofOctets;

        if (serializedPayloadSize > 0)
        {
            os_ushort encapsKind;

            /* DATA submessage does not support anything larger than that */
            assert(serializedPayloadSize <= IN_USHORT_MAX);

            if(in_ddsiSerializerIsBigEndian(serializer))
            {
                encapsKind =IN_ENCAPSULATION_CDR_BE;
            } else
            {
                encapsKind = IN_ENCAPSULATION_CDR_LE;
            }

            /* add the encapsulation header*/
            nofOctets = in_ddsiSubmessageAppendEncapsulationHeaderInstantly(encapsKind, serializer);
            IN_BREAK_IF(nofOctets<0);

            tmpGuid = *guid;
            tmpGuid.entityId.entityKey[0] = 0;
            tmpGuid.entityId.entityKey[1] = 0;
            tmpGuid.entityId.entityKey[2] = 0;
            tmpGuid.entityId.entityKind = IN_PARTICIPANT_MESSAGE_DATA_KIND_AUTOMATIC_LIVELINESS_UPDATE;
             nofOctets = in_ddsiGuidSerialize(&tmpGuid, serializer);
             if (nofOctets<0) break;
             total += nofOctets;

            nofOctets = in_ddsiSerializerAppendUlong(serializer, 1);
            if (nofOctets<0) break;
            total += nofOctets;

            nofOctets = in_ddsiSerializerAppendOctet(serializer, 0);
            if (nofOctets<0) break;
            total += nofOctets;
            /* TODO: handle fragmented parameter list */
            /*
            nofOctets = in_ddsiParameterListForParticipantSerializeInstantly(facade, &(_this->serializer), discoveryData);
            IN_BREAK_IF(nofOctets<0);
            total += nofOctets;
			*/
            nofOctets = in_ddsiSerializerAlign(serializer, IN_DDSI_PARAMETER_HEADER_ALIGNMENT);
            if (nofOctets<0) break;
            total += nofOctets;
            /* Assumption was, that no fragmentation is required  so
             * all data fits into the current buffer. Futheron the number
             * of octets of serialized parameter list must be a multiple
             * of 4 (PARAMETER_HEADER_ALIGNMENT). The following verifies
             * valid alignment, return with error if otherwise. */
            IN_BREAK_IF(0!=in_ddsiSerializerAlign(serializer, IN_DDSI_PARAMETER_HEADER_ALIGNMENT));
        }
        /* finally assign result */
        result = total;
    } while(0); /* never repeat */

    return result;
}

/** non-fragmented transfer of data
 *
 * requries that all message items fit into the current buffer
 *
 * \return -1 in case of error, otherwise the number of octets
 * written in total */
static in_result
serializePublicationData(
        in_ddsiStreamWriterImpl _this,
        os_size_t serializedPayloadSize,
        in_ddsiEntityId readerId,
        in_endpointDiscoveryData discoveryData,
        in_connectivityWriterFacade facade)
{
    /* no inline Qos */
    const os_size_t inlineQosSize = 0;


    in_result result = IN_RESULT_ERROR;

    in_long nofOctets = 0;
    os_size_t total = 0;

    /* writer is the builtin writer */
   OS_STRUCT(in_ddsiEntityId) writerId =
        UI2ENTITYID(IN_ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER);

    do {
        const in_ddsiSequenceNumber sequenceNumber = in_connectivityWriterFacadeGetSequenceNumber(facade);

        os_time timestamp = in_connectivityEntityFacadeGetTimestamp(in_connectivityEntityFacade(facade));
        c_time allocTime;
        allocTime.seconds = timestamp.tv_sec;
        allocTime.nanoseconds = timestamp.tv_nsec;

        nofOctets =
            in_ddsiSubmessageInfoTimestampSerializeInstantly(
                    &allocTime,
                    &(_this->serializer));
        IN_BREAK_IF(nofOctets<0);
        total += nofOctets;

        nofOctets =
            in_ddsiSubmessageDataHeaderSerializeInstantly(
                    inlineQosSize,
                    serializedPayloadSize,
                    sequenceNumber,
                    readerId,
                    &writerId,
                    &(_this->serializer));
        IN_BREAK_IF(nofOctets<0);
        total += nofOctets;

        if (serializedPayloadSize > 0) {
            const os_ushort encapsKind =
                 in_ddsiSerializerIsBigEndian(&(_this->serializer))
                 ? IN_ENCAPSULATION_PL_CDR_BE
                 : IN_ENCAPSULATION_PL_CDR_LE;

            /* DATA submessage does not support anything larger than that */
            assert(serializedPayloadSize <= IN_USHORT_MAX);

            /* add the encapsulation header  */
            nofOctets =
                in_ddsiSubmessageAppendEncapsulationHeaderInstantly(
                        encapsKind, &(_this->serializer));
            IN_BREAK_IF(nofOctets<0);

            /* TODO: handle fragmented parameter list */
            nofOctets =
                in_ddsiParameterListForPublicationSerializeInstantly(
                        facade,
                        &(_this->serializer),
                        discoveryData);
            IN_BREAK_IF(nofOctets<0);
            total += nofOctets;

            /* Assumption was, that no fragmentation is required  so
             * all data fits into the current buffer. Futheron the number
             * of octets of serialized parameter list must be a multiple
             * of 4 (PARAMETER_HEADER_ALIGNMENT). The following verifies
             * valid alignment, return with error if otherwise. */
            IN_BREAK_IF(0!=in_ddsiSerializerAlign(
                    &(_this->serializer),
                    IN_DDSI_PARAMETER_HEADER_ALIGNMENT));
        }
        /* finally assign result */
        result = IN_RESULT_OK;
    } while(0); /* never repeat */

    return result;
}

/** non-fragmented transfer of data
 *
 * requries that all message items fit into the current buffer
 *
 * \return -1 in case of error, otherwise the number of octets
 * written in total */
static in_result
serializeSubscriptionData(
        in_ddsiStreamWriterImpl _this,
        os_size_t serializedPayloadSize,
        in_ddsiEntityId readerId,
        in_endpointDiscoveryData discoveryData,
        in_connectivityReaderFacade facade,
        in_ddsiGuidPrefix writerGuidPrefix)
{
    /* no inline Qos */
    const os_size_t inlineQosSize = 24U;
    const in_ddsiGuid guid =
         in_connectivityEntityFacadeGetGuid(
                 in_connectivityEntityFacade(facade));

    in_result result = IN_RESULT_ERROR;

    in_long nofOctets = 0;
    os_size_t total = 0;

    /* writer is the builtin writer */
   OS_STRUCT(in_ddsiEntityId) writerId =
        UI2ENTITYID(IN_ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER);

    do {
        in_ddsiSequenceNumber sequenceNumber = in_connectivityReaderFacadeGetSequenceNumber(facade);
        /* PW: shouldn't this be a builtin writer related seq counter instead of writerfacade related*/

        os_time timestamp = in_connectivityEntityFacadeGetTimestamp(in_connectivityEntityFacade(facade));
        c_time allocTime;
        allocTime.seconds = timestamp.tv_sec;
        allocTime.nanoseconds = timestamp.tv_nsec;

        if (writerGuidPrefix) {
            nofOctets = in_ddsiSubmessageInfoDestinationSerializeInstantly(
                writerGuidPrefix,
                &(_this->serializer));
            if (nofOctets<0) break;
            total += nofOctets;
        }

        nofOctets =
            in_ddsiSubmessageInfoTimestampSerializeInstantly(
                    &allocTime,
                    &(_this->serializer));
        IN_BREAK_IF(nofOctets<0);
        total += nofOctets;

        nofOctets = in_ddsiSubmessageDataHeaderSerializeInstantly(
                    inlineQosSize,
                    serializedPayloadSize,
                    sequenceNumber,
                    readerId,
                    &writerId,
                    &(_this->serializer));
        IN_BREAK_IF(nofOctets<0);
        total += nofOctets;

	    nofOctets = serializeGuid(
	            &(_this->serializer),
	            IN_PID_KEY_HASH,
	            guid);
	    if (nofOctets<0) break;
	    total += nofOctets;

        nofOctets = serializeSentinel(&(_this->serializer));
        if (nofOctets<0) break;
        total += nofOctets;
        /*
        {
        in_octet larData[] = {
            0x70, 0x00, 0x10, 0x00,
            0x29, 0xf0, 0x5c, 0xb7,
            0x00, 0x00, 0x00, 0x66,
            0x00, 0x00, 0x00, 0x01,
            0x00, 0x00, 0x70 ,0x04 ,
            0x01, 0x00, 0x00 ,0x00};
        IN_TRACE_1(Send, 2, "serializeSubscriptionData - ################################################################### %d", sizeof(larData));

        nofOctets = appendOctets(&(_this->serializer), larData, sizeof(larData));
        IN_BREAK_IF(nofOctets<0);
        total += nofOctets;
        }*/

        if (serializedPayloadSize > 0) {
            const os_ushort encapsKind =
                 in_ddsiSerializerIsBigEndian(&(_this->serializer))
                 ? IN_ENCAPSULATION_PL_CDR_BE
                 : IN_ENCAPSULATION_PL_CDR_LE;

            /* DATA submessage does not support anything larger than that */
            assert(serializedPayloadSize <= IN_USHORT_MAX);

            /* add the encapsulation header  */
            nofOctets =
                in_ddsiSubmessageAppendEncapsulationHeaderInstantly(
                        encapsKind, &(_this->serializer));
            IN_BREAK_IF(nofOctets<0);

            /* TODO: handle fragmented parameter list */
            nofOctets =
                in_ddsiParameterListForSubscriptionSerializeInstantly(
                        facade,
                        &(_this->serializer),
                        discoveryData);
            IN_BREAK_IF(nofOctets<0);
            total += nofOctets;

            /* Assumption was, that no fragmentation is required  so
             * all data fits into the current buffer. Futheron the number
             * of octets of serialized parameter list must be a multiple
             * of 4 (PARAMETER_HEADER_ALIGNMENT). The following verifies
             * valid alignment, return with error if otherwise. */
            IN_BREAK_IF(0!=in_ddsiSerializerAlign(
                    &(_this->serializer),
                    IN_DDSI_PARAMETER_HEADER_ALIGNMENT));
        }
        /* finally assign result */
        result = IN_RESULT_OK;
    } while(0); /* never repeat */

    return result;
}

/** non-fragmented transfer of data
 *
 * requries that all message items fit into the current buffer
 *
 * \return -1 in case of error, otherwise the number of octets
 * written in total */
static in_result
serializeData(
    in_ddsiStreamWriterImpl _this,
    os_size_t inlineQosSize,
    os_size_t serializedPayloadSize,
    in_ddsiEntityId readerId, /* may be UNKOWN */
    in_endpointDiscoveryData discoveryData,
    in_connectivityWriterFacade facade,
    v_message message,
    v_topic topic)
{
    const os_ushort encapsFlags = 0x0;
    c_octet* dataHeaderPosition, *endDataPosition;
    os_boolean keyHashAdded = OS_FALSE;
	in_result result = IN_RESULT_ERROR;
	in_result retState = IN_RESULT_OK;

	in_long nofOctets = 0;
	os_ushort octetsToNextHeader = 0;
	os_size_t total = 0;
    os_size_t serializedDataSize = 0;

	in_ddsiGuid writerGuid = in_connectivityEntityFacadeGetGuid(
	            in_connectivityEntityFacade(facade));

	in_ddsiEntityId writerId = &(writerGuid->entityId);
	OS_STRUCT(in_ddsiSequenceNumber) sequenceNumber;

	/* Init from native sequence number represenation */
	in_ddsiSequenceNumberInitNative(&sequenceNumber,
	        message->sequenceNumber);

	do {
		nofOctets = in_ddsiSubmessageInfoTimestampSerializeInstantly(
					&(message->writeTime),
					&(_this->serializer));
		IN_BREAK_IF(nofOctets<0);
		total += nofOctets;

		/* Must be written a second time when we know the real octet-size of
		 * payload, so keep track of current position.
		 */
		dataHeaderPosition = in_ddsiSerializerGetPosition(&(_this->serializer));

		nofOctets = in_ddsiSubmessageDataHeaderSerializeInstantly(
			        inlineQosSize,
			        serializedPayloadSize,
			        &sequenceNumber,
			        readerId,
			        writerId,
			        &(_this->serializer));
		IN_BREAK_IF(nofOctets<0);
		total += nofOctets;

		if (inlineQosSize > 0) {
		    nofOctets = in_ddsiParameterListForDataSerializeInstantly(
		                facade,
		                &(_this->serializer),
		                discoveryData,
		                message,
		                topic,
		                &keyHashAdded);
		    IN_BREAK_IF(nofOctets<0);
		    total += nofOctets;

		    /* must match alignment constraints */
		    assert((total%IN_DDSI_PARAMETER_HEADER_ALIGNMENT)==0);
		    assert(in_ddsiSerializerAlign(&(_this->serializer),IN_DDSI_PARAMETER_HEADER_ALIGNMENT)==0);
		}
		assert(P2UI(in_ddsiSerializerGetPosition(&(_this->serializer)))%4 == 0);

		if (serializedPayloadSize > 0) {
		    if (in_ddsiEncapsulationHeaderSerializeInstantly(
		                IN_MESSAGE_SERIALIZER_CODEC_ID,
		                encapsFlags,
                        &(_this->serializer))<0) {
		        /* error writing the encaps header */
		    } else {
                in_data firstOctetBehindDataHeader =
                    in_ddsiSerializerGetPosition(&(_this->serializer));
                in_data firstOctetBehindSerializedData = NULL;
                c_long topicDataOffset;


                /* serializedPayloadSize must be smaller than 64K*/
                os_uint32 bytesLeft =
                    (os_uint32) serializedPayloadSize;

                in_messageSerializerBegin(_this->messageSerializer,
                        firstOctetBehindDataHeader,
                        bytesLeft);
                /* serialize the userData encapsulated within the
                 * v_message object */
                topicDataOffset = v_topicDataOffset(topic);
                retState = in_messageSerializerWrite(
                    _this->messageSerializer,
                    message,
                    topicDataOffset,
                    &serializedDataSize);
                if(retState != IN_RESULT_OK)
                {
                    /* TODO report error and handle it!! */
                }
                IN_BREAK_IF(retState!=IN_RESULT_OK);


                /* For DATA it must not exceed the the buffer size */
                /* serializeData assumes that no fragmentation is done, so
                 * all data fits into the current buffer, so
                 * nofOctets==(firstOctetBehindSerializedData-firstOctetBehindDataHeader),
                 */
                firstOctetBehindSerializedData =
                    in_messageSerializerEnd(_this->messageSerializer);

                total += serializedDataSize;

                /* verify the buffer has not been  re-initialized */
                assert(firstOctetBehindDataHeader==in_ddsiSerializerGetPosition(&(_this->serializer)));

                /* seek the index of serializer to that position */
                nofOctets =
                    in_ddsiSerializerSeekTo(
                        &(_this->serializer),
                        firstOctetBehindSerializedData);
                IN_BREAK_IF(nofOctets<0);
                total += nofOctets;

                /* verify seek succeeded */
                assert(firstOctetBehindSerializedData ==
                    in_ddsiSerializerGetPosition(&(_this->serializer)));

                /* finally we must make sure that the end of the serialized data
                 * is aligned by 4 (SUBMESSAGE_HEADER_ALIGNMENT).
                 *
                 * Assuming that the serializedPayloadSize took this alignment already
                 * account we must calculate the difference and seek for the specific size */

                /* Note: bytesLeft may not be used the same way in case of
                 * fragmentation, as the serializer might have been re-initialized
                 * setting up a fresh buffer */

                /*
                alignmentDistance =
                    bytesLeft -
                    (firstOctetBehindSerializedData - firstOctetBehindDataHeader);


                nofOctets =
                    in_ddsiSerializerSeek(
                            &(_this->serializer),
                            alignmentDistance);
                IN_BREAK_IF(nofOctets < 0);
                total += nofOctets;
                */
                nofOctets = in_ddsiSerializerAlign(&(_this->serializer),
                    IN_DDSI_PARAMETER_HEADER_ALIGNMENT);

                if (nofOctets<0) break;
                total += nofOctets;

                endDataPosition = in_ddsiSerializerGetPosition(&(_this->serializer));
                octetsToNextHeader = (os_ushort)(endDataPosition -
                    (dataHeaderPosition + IN_DDSI_SUBMESSAGE_HEADER_SIZE));

                /* Set position of serializer to the location of
                 * octetsToNextHeader field. These are the last 2 bytes of
                 * the subMessage header. So add header size to start of the
                 * subMessage and subtract 2 bytes.
                 */
                in_ddsiSerializerSeekTo(&(_this->serializer),
                    dataHeaderPosition + IN_DDSI_SUBMESSAGE_HEADER_SIZE - 2);

                /* Set correct octetsToNextHeader*/
                in_ddsiSerializerAppendUshort(&(_this->serializer),
                    octetsToNextHeader);

                /* Reset serializer to endPosition */
                in_ddsiSerializerSeekTo(&(_this->serializer), endDataPosition);
            }
		}
		/* finally assign result */
        result = IN_RESULT_OK;
	} while(0); /* never repeat */

	return result;
}






static void
in_ddsiStreamWriterImplResetBuffer(
		in_ddsiStreamWriterImpl _this,
		const in_ddsiGuidPrefixRef guidPrefix)
{
	/* fixed protocol version and variable product version */
	OS_STRUCT(in_ddsiProtocolVersion) protocolVersion =
		IN_DDSI_PROTOCOL_VERSION_2_1;
	OS_STRUCT(in_ddsiVendor) vendor;

	assert(in_ddsiSerializerRemainingCapacity(&(_this->serializer)) > OS_SIZEOF(in_ddsiMessageHeader));

    vendor.vendorId[0] = IN_DDSI_VENDOR_PT_VAL1;
    vendor.vendorId[1] = IN_DDSI_VENDOR_PT_VAL2;
	in_ddsiSerializerInitWithDefaultEndianess(
			&(_this->serializer),
			_this->currentSendBuffer);

	in_ddsiMessageHeaderSerializeInstantly(
			&protocolVersion,
			&vendor,
			guidPrefix,
			&(_this->serializer));
}

static os_size_t
calculatePayloadSize(v_message message)
{
    /* TODO ask the serializer to calculate the size */

	/*Note: Here the effective length of serialized payload should be calculated,
	 * but so far we assume that users message will not exceed 800 bytes, and that the
	 * sendbuffer is large enough to hold these 800 plus the required space for headers
	 * and inline Qos.
	 * Plan is to implement a one-pass serialization strategy in future, so that this operation
	 * will become obsolete.
	 *  */
	os_size_t realSize = 800;
    os_size_t result;

    /* finally round up the value to match alignment constraints (just in
     * case realSize is not multiple of 4) */
    result = IN_ALIGN_UINT_CEIL(
            realSize,
            IN_DDSI_SUBMESSAGE_HEADER_ALIGNMENT);

    assert( (result >= realSize) &&
            (result % (IN_DDSI_SUBMESSAGE_HEADER_ALIGNMENT) == 0));

    return result;
}

static in_result
appendSentinelSubmessage(
        in_ddsiStreamWriterImpl _this)
{
    in_result result;
    const in_ddsiSubmessageKind kind = IN_SENTINEL;
    const in_ddsiSubmessageFlags flags = 0;
    const os_ushort octetsToNextHeader = 0;

    in_long nofOctets =
        in_ddsiSubmessageHeaderSerializeInstantly(
                kind, flags,
                octetsToNextHeader,
                &(_this->serializer));

    result = nofOctets < 0 ? IN_RESULT_ERROR : IN_RESULT_OK;
    return result;
}


static in_result
sendCurrentBufferToSingle(
        in_ddsiStreamWriterImpl _this,
        in_locator destLocator)
{
    const os_boolean isControl = OS_FALSE;
    const os_size_t length =
              in_ddsiSerializerNofOctets(&(_this->serializer));
    in_result result = IN_RESULT_OK;

    if (length < IN_DDSI_MESSAGE_HEADER_SIZE) {
        result = IN_RESULT_ERROR;
    } else {
        os_result osResult;
        assert(destLocator!=NULL);

        osResult = in_transportSenderSendTo(
                _this->transportSender,
                destLocator,
                _this->currentSendBuffer,
                length,
                isControl,
                &(_this->timeout));
        if(osResult == os_resultFail)
        {
            result = IN_RESULT_ERROR;
        }
    }

    return result;
}

static in_result
sendCurrentBufferToMultiple(
        in_ddsiStreamWriterImpl _this,
        Coll_List *locatorList)
{
    const os_boolean isControl = OS_FALSE;
    const os_size_t length = in_ddsiSerializerNofOctets(&(_this->serializer));
    in_result result = IN_RESULT_OK;
    if (length < IN_DDSI_MESSAGE_HEADER_SIZE)
    {
        result = IN_RESULT_ERROR;
    } else {
        Coll_Iter *iter = Coll_List_getFirstElement(locatorList);

        while (iter)
        {
            os_result osResult;
            in_locator destLocator = Coll_Iter_getObject(iter);

            assert(destLocator!=NULL);
            IN_TRACE_1(Send, 2, ">>> sendCurrentBufferToMultiple - calling transport layer %p", _this->transportSender);
            osResult = in_transportSenderSendTo(
                    _this->transportSender,
                    destLocator,
                    _this->currentSendBuffer,
                    length,
                    isControl,
                    &(_this->timeout));
            if(osResult == os_resultFail)
            {
                IN_TRACE_1(Send, 2, ">>> sendCurrentBufferToMultiple - calling transport layer gave error %p", _this->transportSender);
                result = IN_RESULT_ERROR;
            }
            iter = Coll_Iter_getNext(iter);
        }
    }

    return result;
}

static in_result
in_ddsiStreamWriterImplAppendData(
		in_ddsiStreamWriterImpl _this,
		v_message message,
		in_endpointDiscoveryData discoveryData,
		in_connectivityWriterFacade facade,
		os_boolean recipientExpectsInlineQos,
		Coll_List *locatorList)
{
	v_topic topic;
	struct v_publicationInfo * info;
    const os_size_t sentinelSize = 0U;
	in_ddsiGuid guid;
	os_boolean keyHashAdded = OS_FALSE; /* must be initialized as FALSE */
	os_size_t serializedPayloadSize, inlineQosSize, timestampSubmessageLength;
	os_size_t dataSubmessageLength, totalLength;
	in_result result = IN_RESULT_OK;

	info = in_connectivityWriterFacadeGetInfo(facade);
	/*TODO: lookup topic is very expensive. It could also be stored at the
	 * writerFacade on creation.
	 */
	topic = in_plugKernelLookupTopic(_this->plugKernel, info->topic_name);

	guid = in_connectivityEntityFacadeGetGuid(in_connectivityEntityFacade(
		facade));
	inlineQosSize = recipientExpectsInlineQos ?
		in_ddsiParameterListForDataCalculateSize(facade, discoveryData, message,
		topic, &keyHashAdded): 0U;

	if(!keyHashAdded)
	{
		serializedPayloadSize = calculatePayloadSize(message);
	} else
	{
		serializedPayloadSize = 0;
	}
	timestampSubmessageLength = IN_DDSI_INFOTIMESTAMP_SUBMESSAGE_SIZE;

	dataSubmessageLength = in_ddsiSubmessageDataSerializedSize(inlineQosSize,
		serializedPayloadSize);

	totalLength = timestampSubmessageLength + dataSubmessageLength +
	   /* always keep space at end for the terminating sentinel */ sentinelSize;

	assert(timestampSubmessageLength % 4 == 0);
	assert(dataSubmessageLength % 4 == 0);
	assert(dataSubmessageLength > 0);

	/* for now, one packet per message, no bundling of data items */
	in_ddsiStreamWriterImplResetBuffer(_this, guid->guidPrefix);

	if (totalLength >
        in_ddsiSerializerRemainingCapacity(&(_this->serializer))) {
		/* now we should turn to fragmentation, but
		 * this is not implemented yet */
	    result = IN_RESULT_ERROR;
        IN_TRACE_1(Send, 2, ">>> in_ddsiStreamWriterImplAppendData - remaining capacity exceeded %p", facade);
	} else {
        in_result  retState;

        do {
            IN_TRACE_1(Send, 2, ">>> in_ddsiStreamWriterImplAppendData - calling serializeData %p", facade);

            retState = serializeData(
                _this,
                inlineQosSize,
                serializedPayloadSize,
                &unknownId,
                discoveryData,
                facade,
                message,
                topic);
            IN_BREAK_IF(retState!=IN_RESULT_OK);
            /*IN_TRACE_1(Send, 2, ">>> in_ddsiStreamWriterImplAppendData - calling appendSentinelSubmessage %p", facade);
            retState = appendSentinelSubmessage(_this);
            IN_BREAK_IF(retState!=IN_RESULT_OK);*/
            IN_TRACE_1(Send, 2, ">>> in_ddsiStreamWriterImplAppendData - calling sendCurrentBufferToMultiple %p", facade);
            retState = sendCurrentBufferToMultiple(_this, locatorList);
            IN_BREAK_IF(retState!=IN_RESULT_OK);
            IN_TRACE_1(Send, 2, ">>> in_ddsiStreamWriterImplAppendData - done %p", facade);
            /* all succeeded */
            result = IN_RESULT_OK;
        } while(0);
	}
	c_free(topic);

	return result;
}

/**     */
static in_result
in_ddsiStreamWriterImplAppendParticipantData(
		in_ddsiStreamWriterImpl _this,
        in_endpointDiscoveryData discoveryData,
		in_connectivityParticipantFacade facade,
		Coll_List *locatorList)
{
    const os_size_t sentinelSize = 0U;

     const in_ddsiGuid guid =
         in_connectivityEntityFacadeGetGuid(
                 in_connectivityEntityFacade(facade));

     /* payload is encoded as PL_CDR_LE/BE */
     const os_size_t serializedPayloadSize =
         in_ddsiParameterListForParticipantCalculateSize(
                 facade,
                 discoveryData);

     /* no inlineQos */
     const os_size_t inlineQosSize = 24U;

     in_result result = IN_RESULT_ERROR;

     /* Note: do not store the locatorList permanently, it
      * might be modified or erased by owner */
     const os_size_t timestampSubmessageLength =
         IN_DDSI_INFOTIMESTAMP_SUBMESSAGE_SIZE;

     const os_size_t dataSubmessageLength =
         in_ddsiSubmessageDataSerializedSize(
                 inlineQosSize,
                 serializedPayloadSize);

     const os_size_t totalLength =
         timestampSubmessageLength +
         dataSubmessageLength +
         /* always keep space at end for the terminating sentinel */
         sentinelSize;

     assert(timestampSubmessageLength % 4 == 0);
     assert(dataSubmessageLength % 4 == 0);
     assert(dataSubmessageLength > 0);

     /* for now, one packet per message, no bundling of data items */
     in_ddsiStreamWriterImplResetBuffer(_this, guid->guidPrefix);

    if (totalLength > in_ddsiSerializerRemainingCapacity(&(_this->serializer)))
    {
         /* now we should turn to fragmentation, but
          * this is not implemented yet */
         result = IN_RESULT_ERROR;
    } else
    {
        in_result  retState;
        OS_STRUCT(in_ddsiEntityId) readerId = UI2ENTITYID(IN_ENTITYID_UNKNOWN);
        do
        {
            retState = serializeParticipantData(
                 _this,
                 serializedPayloadSize,
                 &readerId,
                 discoveryData,
                 facade);
            IN_BREAK_IF(retState != IN_RESULT_OK);

            /*
             retState = appendSentinelSubmessage(_this);
             IN_BREAK_IF(retState!=IN_RESULT_OK);
             */
            retState = sendCurrentBufferToMultiple(_this, locatorList);
            IN_BREAK_IF(retState!=IN_RESULT_OK);

            /* all succeeded */
            result = IN_RESULT_OK;
         } while(0);
     }
     return result;
}

static os_size_t
in_ddsiParameterListForParticipantMessageCalculateSize(
    in_ddsiStreamWriterImpl _this,
    in_connectivityParticipantFacade facade,
    in_ddsiGuidPrefixRef destGuidPrefix)
{
    OS_STRUCT(in_ddsiSerializer) nilSerializer;

    os_size_t result = 0;
    in_long nofOctets;
    OS_STRUCT(in_ddsiEntityId) readerId = UI2ENTITYID(IN_ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_READER);
    OS_STRUCT(in_ddsiEntityId) writerId = UI2ENTITYID(IN_ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_WRITER);
    os_size_t serializedPayloadSize = 200;

    in_ddsiSerializerInitNil(&nilSerializer);

    /**/
    nofOctets = serializeParticipantMessage(
                &nilSerializer,
                serializedPayloadSize,
                &readerId,
                &writerId,
                destGuidPrefix,
                facade);
    assert(nofOctets>=4);
    result = (os_size_t) nofOctets;

    in_ddsiSerializerDeinit(&nilSerializer);

    assert(result % 4 == 0);

    return 24;/* TODO: Calculate actual size.*/
}
static in_result
in_ddsiStreamWriterImplAppendParticipantMessage(
    in_ddsiStreamWriterImpl _this,
    in_connectivityParticipantFacade facade,
    in_ddsiGuidPrefixRef destGuidPrefix,
    in_locator locator)
{
    in_result result = IN_RESULT_ERROR;
    os_size_t sentinelSize = 0U;
    in_ddsiGuid guid;
    in_long nofOctets;
    os_size_t serializedPayloadSize;
    /* no inlineQos */
    os_size_t inlineQosSize = 24U;
    /* Note: do not store the locatorList permanently, it
     * might be modified or erased by owner
     */
    os_size_t timestampSubmessageLength = IN_DDSI_INFOTIMESTAMP_SUBMESSAGE_SIZE;
    os_size_t dataSubmessageLength;
    os_size_t totalLength;
    in_result  retState;
    OS_STRUCT(in_ddsiEntityId) readerId = UI2ENTITYID(IN_ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_READER);
    OS_STRUCT(in_ddsiEntityId) writerId = UI2ENTITYID(IN_ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_WRITER);

    assert(_this);
    assert(facade);
    assert(locator);

    guid = in_connectivityEntityFacadeGetGuid(in_connectivityEntityFacade(facade));

     /* payload of this message doesn't have an encoding, so don't add
      * encapsulation header size!
      */
    serializedPayloadSize = in_ddsiParameterListForParticipantMessageCalculateSize(_this, facade, destGuidPrefix);
    dataSubmessageLength = in_ddsiSubmessageDataSerializedSize(inlineQosSize, serializedPayloadSize);
    /* always keep space at end for the terminating sentinel */
    totalLength = timestampSubmessageLength + dataSubmessageLength + sentinelSize;

    assert(timestampSubmessageLength % 4 == 0);
    assert(dataSubmessageLength % 4 == 0);
    assert(dataSubmessageLength > 0);

    /* for now, one packet per message, no bundling of data items */
    in_ddsiStreamWriterImplResetBuffer(_this, guid->guidPrefix);

    if (totalLength > in_ddsiSerializerRemainingCapacity(&(_this->serializer)))
    {
         /* now we should turn to fragmentation, but this is not implemented yet */
         result = IN_RESULT_ERROR;
    } else
    {
        do
        {
            nofOctets = serializeParticipantMessage(
                &(_this->serializer),
                serializedPayloadSize,
                &readerId,
                &writerId,
                destGuidPrefix,
                facade);
            IN_BREAK_IF(nofOctets<0);

            /*retState = appendSentinelSubmessage(_this);
            IN_BREAK_IF(retState!=IN_RESULT_OK);*/

            retState = sendCurrentBufferToSingle(_this, locator);
            IN_BREAK_IF(retState!=IN_RESULT_OK);

        /* all succeeded */
        result = IN_RESULT_OK;
        } while(0);
    }
    return result;
}
/** */
static in_result
in_ddsiStreamWriterImplAppendPublicationData(
        in_ddsiStreamWriterImpl _this,
        in_endpointDiscoveryData discoveryData,
        in_connectivityWriterFacade facade,
        Coll_List *locatorList)
{
    const os_size_t sentinelSize = 0U;

    const in_ddsiGuid guid =
         in_connectivityEntityFacadeGetGuid(
                 in_connectivityEntityFacade(facade));

     /* payload is encoded as PL_CDR_LE/BE */
     const os_size_t serializedPayloadSize =
         in_ddsiParameterListForPublicationCalculateSize(
                 facade,
                 discoveryData);

     /* no inlineQos */
     const os_size_t inlineQosSize = 0U;

     in_result result = IN_RESULT_ERROR;

     /* Note: do not store the locatorList permanently, it
      * might be modified or erased by owner */
     const os_size_t timestampSubmessageLength =
         IN_DDSI_INFOTIMESTAMP_SUBMESSAGE_SIZE;

     const os_size_t dataSubmessageLength =
         in_ddsiSubmessageDataSerializedSize(
                 inlineQosSize,
                 serializedPayloadSize);

     const os_size_t totalLength =
         timestampSubmessageLength +
         dataSubmessageLength +
         /* always keep space at end for the terminating sentinel */
         sentinelSize;

     assert(timestampSubmessageLength % 4 == 0);
     assert(dataSubmessageLength % 4 == 0);
     assert(dataSubmessageLength > 0);

     /* for now, one packet per message, no bundling of data items */
     in_ddsiStreamWriterImplResetBuffer(_this, guid->guidPrefix);

     if (totalLength >
         in_ddsiSerializerRemainingCapacity(&(_this->serializer))) {
         /* now we should turn to fragmentation, but
          * this is not implemented yet */
         result = IN_RESULT_ERROR;
     } else {
         in_result  retState;
        OS_STRUCT(in_ddsiEntityId) readerId = UI2ENTITYID(IN_ENTITYID_SEDP_BUILTIN_PUBLICATIONS_READER);
         do {
             retState =
                 serializePublicationData(
                         _this,
                         serializedPayloadSize,
                         &readerId,
                         discoveryData,
                         facade);
             IN_BREAK_IF(retState!=IN_RESULT_OK);

            /* retState =
                 appendSentinelSubmessage(_this);
             IN_BREAK_IF(retState!=IN_RESULT_OK);*/

             retState =
                 sendCurrentBufferToMultiple(_this, locatorList);
             IN_BREAK_IF(retState!=IN_RESULT_OK);

             /* all succeeded */
             result = IN_RESULT_OK;
         } while(0);
     }
     return result;
}



/** */
static in_result
in_ddsiStreamWriterImplAppendSubscriptionData(
        in_ddsiStreamWriterImpl _this,
        in_ddsiGuidPrefixRef writerGuidPrefix,
        in_endpointDiscoveryData discoveryData,
        in_connectivityReaderFacade facade,
        Coll_List *locatorList)
{
    const os_size_t sentinelSize = 0U;

    const in_ddsiGuid guid =
         in_connectivityEntityFacadeGetGuid(
                 in_connectivityEntityFacade(facade));

     /* payload is encoded as PL_CDR_LE/BE */
     const os_size_t serializedPayloadSize =
         in_ddsiParameterListForSubscriptionCalculateSize(facade, discoveryData);

     /* no inlineQos */
     const os_size_t inlineQosSize = 24U;

     in_result result = IN_RESULT_ERROR;

     /* Note: do not store the locatorList permanently, it
      * might be modified or erased by owner */
     const os_size_t timestampSubmessageLength =
         IN_DDSI_INFOTIMESTAMP_SUBMESSAGE_SIZE;

     const os_size_t dataSubmessageLength =
         in_ddsiSubmessageDataSerializedSize(
                 inlineQosSize,
                 serializedPayloadSize);

     const os_size_t totalLength =
         timestampSubmessageLength +
         (writerGuidPrefix!=NULL ? IN_DDSI_INFODESTINATION_SUBMESSAGE_SIZE : 0L) +
         dataSubmessageLength +
         /* always keep space at end for the terminating sentinel */
         sentinelSize;

     assert(timestampSubmessageLength % 4 == 0);
     assert(dataSubmessageLength % 4 == 0);
     assert(dataSubmessageLength > 0);

     /* for now, one packet per message, no bundling of data items */
     in_ddsiStreamWriterImplResetBuffer(_this, guid->guidPrefix);

     if (totalLength >
         in_ddsiSerializerRemainingCapacity(&(_this->serializer))) {
         /* now we should turn to fragmentation, but
          * this is not implemented yet */
         result = IN_RESULT_ERROR;
     } else {
        in_result  retState;
        OS_STRUCT(in_ddsiEntityId) readerId = UI2ENTITYID(IN_ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_READER);
        do {
             retState =
                 serializeSubscriptionData(
                         _this,
                         serializedPayloadSize,
                         &readerId,
                         discoveryData,
                         facade,
                         writerGuidPrefix);
             IN_BREAK_IF(retState!=IN_RESULT_OK);
             /*
             retState =
                 appendSentinelSubmessage(_this);
             IN_BREAK_IF(retState!=IN_RESULT_OK);
             */
             retState =
                 sendCurrentBufferToMultiple(_this, locatorList);
             IN_BREAK_IF(retState!=IN_RESULT_OK);

             /* all succeeded */
             result = IN_RESULT_OK;
         } while(0);
     }
     return result;
}

/** \return  corresponding default multicast locator for data (also SDP traffic)
 *
 *  hook implementation of corresponding virtual method*/
static in_locator
in_streamWriterGetDataMulticastLocator_i(in_streamWriter _this)
{
    in_locator result = NULL;
    in_ddsiStreamWriterImpl narrowed =
        in_ddsiStreamWriterImpl(_this);

    result =
        in_transportSenderGetDataMulticastLocator(narrowed->transportSender);

    return result;
}

    static os_int32 count = 0; /* TODO: shoudl be parameter */
/** ingore "Final" flag for now
 *
 * If sending the first AckNack to this writerGuid, the streamWriter
 * must be flushed first to deliver previous data items to multiple
 * recipients. */
static in_result
in_ddsiStreamWriterImplAppendAckNack(
        in_ddsiStreamWriterImpl _this,
        in_ddsiGuid readerGuid, /* TODO: maybe it would be more efficient to split into entityId and GuidPrefix*/
        in_ddsiGuid writerGuid,
        in_ddsiSequenceNumberSet readerSNState,
        in_locator singleDestination)
{
    in_result result = IN_RESULT_OK;

    const in_ddsiEntityId readerId = &(readerGuid->entityId);
    const in_ddsiEntityId writerId = &(writerGuid->entityId);
    const in_ddsiGuidPrefixRef writerGuidPrefix = &(writerGuid->guidPrefix[0]);
    const os_size_t infoDestSize = IN_DDSI_INFODESTINATION_SUBMESSAGE_SIZE;
    const os_size_t ackNackSize = in_ddsiSubmessageAckNackSerializedSize(readerSNState);
    const os_size_t totalSize = infoDestSize + ackNackSize;
    const os_size_t remainingCapa = in_ddsiSerializerRemainingCapacity(&(_this->serializer));

    /* must be multiple of 4 */
    assert(ackNackSize % IN_DDSI_SUBMESSAGE_HEADER_ALIGNMENT == 0);
    /* increase count each time, so it basically starts at 1 */
    count++;
    if (totalSize > remainingCapa)
    {
        result = IN_RESULT_ERROR;
    } else
    {
        in_long total = 0;
        in_long nofOctets = 0;

        /* for now, one packet per message, no bundling of data items */
        in_ddsiStreamWriterImplResetBuffer(_this, readerGuid->guidPrefix);
        do
        {
            nofOctets = in_ddsiSubmessageInfoDestinationSerializeInstantly(
                writerGuidPrefix,
                &(_this->serializer));
            if (nofOctets<0) break;
            total += nofOctets;

            nofOctets = in_ddsiSubmessageAckNackSerializeInstantly(
                readerId,
                writerId,
                readerSNState,
                count,
                &(_this->serializer));
            if (nofOctets<0) break;
            total += nofOctets;

            /* TODO: verify "total" matches pre-calculated size */
            result = sendCurrentBufferToSingle(_this, singleDestination);
            IN_BREAK_IF(result!=IN_RESULT_OK);
        } while (0);
    }
    return result;
}

/** \return corresponding default unicast locator for data (also SDP traffic)
 *
 *   hook implementation of corresponding virtual method*/
static in_locator
in_streamWriterGetDataUnicastLocator_i(in_streamWriter _this)
{
    in_locator result = NULL;
    in_ddsiStreamWriterImpl narrow =
        in_ddsiStreamWriterImpl(_this);

    result =
        in_transportSenderGetDataUnicastLocator(narrow->transportSender);

    return result;
}


/** \return corresponding default unicast locator for control messages
 *
 *   hook implementation of corresponding virtual method*/
static in_locator
in_streamWriterGetCtrlUnicastLocator_i(in_streamWriter _this)
{
    in_locator result = NULL;
    in_ddsiStreamWriterImpl narrow =
        in_ddsiStreamWriterImpl(_this);

    result =
        in_transportSenderGetCtrlUnicastLocator(narrow->transportSender);

    return result;
}

/** ingore "Final" flag for now
 *
 * If sending the first AckNack to this writerGuid, the streamWriter
 * must be flushed first to deliver previous data items to multiple
 * recipients. */
in_result
in_streamWriterAppendAckNack_i(
        in_streamWriter _this,
        in_ddsiGuid readerGuid,
        in_ddsiGuid writerGuid,
        in_ddsiSequenceNumberSet readerSNState,
        in_locator singleDestination)
{
    in_ddsiStreamWriterImpl narrowed =
         in_ddsiStreamWriterImpl(_this);

    /* narrowing */
    return in_ddsiStreamWriterImplAppendAckNack(
            narrowed,
            readerGuid,
            writerGuid,
            readerSNState,
            singleDestination);
}

static in_result
in_streamWriterAppendHeartbeat_i(
                in_streamWriter super,
                in_ddsiGuidPrefixRef sourceGuidPrefix,
                in_ddsiGuidPrefixRef destGuidPrefix,
                in_ddsiEntityId readerId,
                in_ddsiEntityId writerId,
                in_ddsiSequenceNumber firstSN,
                in_ddsiSequenceNumber lastSN,
                os_ushort count,
                in_locator singleDestination)
{
    in_ddsiStreamWriterImpl _this =
         in_ddsiStreamWriterImpl(super);

    in_result result = IN_RESULT_OK;

    /* append the InfoDestination submessage */

    /* append the AckNack message
     *
     * ** calculate the message size
     * ** verify it fits into buffer
     * ** perform serialization  */
    OS_STRUCT(in_ddsiSequenceNumberSet)  sequenceNumberSet;

    const os_size_t infoDestSize =
        IN_DDSI_INFODESTINATION_SUBMESSAGE_SIZE;

    const os_size_t heartbeatSize =
        OS_SIZEOF(in_ddsiSubmessageHeartbeat);

    const os_size_t totalSize = infoDestSize + heartbeatSize;
    const os_size_t remainingCapa =
        in_ddsiSerializerRemainingCapacity(&(_this->serializer));

    /* must be multiple of 4 */
    assert(heartbeatSize % IN_DDSI_SUBMESSAGE_HEADER_ALIGNMENT == 0);

    if (totalSize > remainingCapa) {
        result = IN_RESULT_ERROR;
    } else {
        in_long total = 0;
        in_long nofOctets = 0;

        /* for now, one packet per message, no bundling of data items */
        in_ddsiStreamWriterImplResetBuffer(_this,
                sourceGuidPrefix);

        do {
            nofOctets =
                in_ddsiSubmessageInfoDestinationSerializeInstantly(
                        destGuidPrefix,
                        &(_this->serializer));
            if (nofOctets<0) break;
            total += nofOctets;

            nofOctets =
                in_ddsiSubmessageHeartbeatSerializeInstantly(
                        readerId,
                        writerId,
                        firstSN,
                        lastSN,
                        count,
                        &(_this->serializer));
            if (nofOctets<0) break;
            total += nofOctets;

            /* TODO: verify "total" matches pre-calculated size */

            result = sendCurrentBufferToSingle(_this, singleDestination);
            IN_BREAK_IF(result!=IN_RESULT_OK);
        } while (0);

    }

    in_ddsiSequenceNumberSetDeinit(&sequenceNumberSet);

    return result;

}

/**  hook  implementation of corresponding virtual method*/
static void
in_objectDeinit_i(in_object _this)
{
    /* narrow */
    in_ddsiStreamWriterImplDeinit(in_ddsiStreamWriterImpl(_this));
}


/** */
static in_result
in_ddsiStreamWriterImplFlush(
        in_ddsiStreamWriterImpl _this,
        Coll_List *locatorList)
{
    in_result result = IN_RESULT_OK;
    /* nop */
    return result;
}

/** */
static in_result
in_ddsiStreamWriterImplFlushSingle(
        in_ddsiStreamWriterImpl _this,
        in_locator locator)
{
    in_result result = IN_RESULT_OK;
    /* nop */
    return result;
}

/**  hook  implementation of corresponding virtual method*/
static void
in_streamWriterFlush_i(in_streamWriter _this,
        Coll_List *locatorList)
{
    /* narrow */
    in_ddsiStreamWriterImplFlush(
            in_ddsiStreamWriterImpl(_this),
            locatorList);
}

/**  hook  implementation of corresponding virtual method*/
static void
in_streamWriterFlushSingle_i(in_streamWriter _this,
        in_locator locator)
{
    /* narrow */
    in_ddsiStreamWriterImplFlushSingle(
            in_ddsiStreamWriterImpl(_this),
            locator);
}

/**  hook  implementation of corresponding virtual method*/
static in_result
in_streamWriterAppendData_i(
        in_streamWriter _this,
        v_message message,
        in_endpointDiscoveryData discoveryData,
        in_connectivityWriterFacade facade,
        os_boolean recipientExpectsInlineQos,
        Coll_List *locatorList)
{
    /* narrow */
    return in_ddsiStreamWriterImplAppendData(
            in_ddsiStreamWriterImpl(_this),
            message,
            discoveryData,
            facade,
            recipientExpectsInlineQos,
            locatorList);
}

/** hook  implementation of corresponding virtual method*/
static in_result
in_streamWriterAppendParticipantData_i(
            in_streamWriter _this,
            in_endpointDiscoveryData discoveryData,
            in_connectivityParticipantFacade facade,
            Coll_List *locatorList)
{   /* narrow */
    return in_ddsiStreamWriterImplAppendParticipantData(
            in_ddsiStreamWriterImpl(_this),
            discoveryData,
            facade,
            locatorList);
}
 in_result
in_streamWriterAppendParticipantMessage_i(
            in_streamWriter _this,
            in_connectivityParticipantFacade facade,
            in_ddsiGuidPrefixRef destGuidPrefix,
            in_locator locator)
{
    in_result result;

    assert(_this);
    assert(facade);
    assert(locator);

    result = in_ddsiStreamWriterImplAppendParticipantMessage(
        in_ddsiStreamWriterImpl(_this),
        facade,
        destGuidPrefix,
        locator);

    return result;
}

/*  hook  implementation of corresponding virtual method*/
static in_result
in_streamWriterAppendSubscriptionData_i(
            in_streamWriter _this,
            in_ddsiGuidPrefixRef guidPrefix,
            in_endpointDiscoveryData discoveryData,
            in_connectivityReaderFacade facade,
            Coll_List *locatorList)
{   /* narrow */
    return in_ddsiStreamWriterImplAppendSubscriptionData(
            in_ddsiStreamWriterImpl(_this),
            guidPrefix,
            discoveryData,
            facade,
            locatorList);
}

/*  hook  implementation of corresponding virtual method*/
static in_result
in_streamWriterAppendPublicationData_i(
            in_streamWriter _this,
            in_endpointDiscoveryData discoveryData,
            in_connectivityWriterFacade facade,
            Coll_List *locatorList)
{   /* narrow */
    return in_ddsiStreamWriterImplAppendPublicationData(
            in_ddsiStreamWriterImpl(_this),
            discoveryData,
            facade,
            locatorList);
}


static void
in_ddsiStreamWriterImplGetBuffer(
        in_ddsiStreamWriterImpl _this,
        in_data* bufferPtr,
        os_uint32* length)
{
    /* seek the index to current position */
    in_ddsiSerializerSeekTo(&(_this->serializer), *bufferPtr);

    /* verify the end aligns to SUBMESSAGE_LENGTH */

    /* append a terminating SUBMESSAGE SENTINEL */

    /* distribute to all recipients */

    /* reset the buffer */

    /* inrement the sequence-counter and
     * create message header and DataFrag submessage header
     */

    /* return new index and "bytesLeft" to callee */

    *length = 0;
    *bufferPtr = NULL;
    OS_REPORT(OS_ERROR, "ddsi", 0,
		"Message doesn't fit in one fragment, but fragmentation has not been implemented yet.");
    assert(!"Fragmentation is not implemented yet");
}

static void
in_messageTransformerGetBuffer_i(
        in_data* bufferPtr,
        os_uint32* length,
        c_voidp userData)
{
    /* narrow */
    in_ddsiStreamWriterImplGetBuffer(
            in_ddsiStreamWriterImpl(userData),
            bufferPtr,
            length);
}

/* **** public functions **** */

/* static constant memory area */
static OS_STRUCT(in_streamWriterPublicVTable)
staticPublicVTable = {
        in_streamWriterFlush_i,
        in_streamWriterFlushSingle_i,
        in_streamWriterAppendData_i,
        in_streamWriterAppendParticipantData_i,
        in_streamWriterAppendParticipantMessage_i,
        in_streamWriterAppendSubscriptionData_i,
        in_streamWriterAppendPublicationData_i,
        in_streamWriterAppendAckNack_i,
        in_streamWriterAppendHeartbeat_i,
        in_streamWriterGetDataUnicastLocator_i,
        in_streamWriterGetDataMulticastLocator_i,
        in_streamWriterGetCtrlUnicastLocator_i};


/**
 */
in_ddsiStreamWriterImpl
in_ddsiStreamWriterImplNew(
		in_configChannel config,
		in_transportSender sender,
		in_plugKernel plugKernel) /* ignore */
{
    /* config may be NULL for DBTs */
	os_time sendTimeout =
		in_configChannelGetIOTimeout(config);

	in_ddsiStreamWriterImpl result =
		os_malloc(sizeof(*result));

	if (result) {
		in_streamWriterInit(
			OS_SUPER(result),
			IN_OBJECT_KIND_STREAM_WRITER_BASIC,
	        in_objectDeinit_i,
			&staticPublicVTable);


		result->timeout = sendTimeout;
		result->transportSender =
		    in_transportSenderKeep(sender);

		result->currentSendBuffer =
			in_transportSenderGetBuffer(sender);

		result->messageSerializer =
		    in_messageSerializerNew(
		        in_messageTransformerGetBuffer_i,
		        (c_voidp) result);
        result->plugKernel = in_plugKernelKeep(plugKernel);
		if (result->currentSendBuffer==NULL ||
		    result->messageSerializer==NULL) {

		    /* error: abort init */

		    if (result->currentSendBuffer) {
		        in_transportSenderReleaseBuffer(
		                sender,
		                result->currentSendBuffer);
		    }
		    if (result->messageSerializer) {
		        in_messageSerializerFree(result->messageSerializer);
		    }
		    in_transportSenderFree(result->transportSender);

			in_streamWriterDeinit(OS_SUPER(result));
			os_free(result);
			result = NULL;
		} else {
			in_ddsiSerializerInitWithDefaultEndianess(
					&(result->serializer),
					result->currentSendBuffer);
		}
	}

    IN_TRACE_1(Construction,2,"in_ddsiStreamWriterImpl created = %x",result);

	return result;
}

/**
 *  */
void
in_ddsiStreamWriterImplFree(in_ddsiStreamWriterImpl _this)
{
	in_streamWriterFree(OS_SUPER(_this));
}


