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

#include "in__ddsiParticipant.h"
#include "in__ddsiSubscription.h"
#include "in__ddsiPublication.h"
#include "in_align.h"

/* ***** constants ***** */

#define IN_DDSI_TIMESTAMP_SUBMESSAGE_SIZE \
	(IN_DDSI_SUBMESSAGE_HEADER_SIZE + \
	 IN_DDSI_SUBMESSAGE_INFOTIMESTAMP_BODY_SIZE)


/* **** private functions **** */

static void
in_ddsiStreamWriterImplDeinit(in_ddsiStreamWriterImpl _this)
{
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
serializeData(
        in_ddsiStreamWriterImpl _this,
        os_size_t inlineQosSize,
        os_size_t serializedPayloadSize,
        in_ddsiEntityId readerId, /* may be UNKOWN */
        in_connectivityWriterFacade facade,
        v_message message)
{
	in_result result = IN_RESULT_ERROR;
	in_result retState = IN_RESULT_OK;

	in_long nofOctets = 0;
	os_size_t total = 0;
    os_size_t serializedDataSize = 0;

	in_ddsiGuid writerGuid =
	    in_connectivityEntityFacadeGetGuid(
	            in_connectivityEntityFacade(facade));

	in_ddsiEntityId writerId =
	    &(writerGuid->entityId);

	do {
		nofOctets =
			in_ddsiSubmessageInfoTimestampSerializeInstantly(
					&(message->allocTime),
					&(_this->serializer));
		if (nofOctets < 0) break;
		total += nofOctets;

		nofOctets =
			in_ddsiSubmessageDataHeaderSerializeInstantly(
			        inlineQosSize,
			        serializedPayloadSize,
			        message->sequenceNumber, /* OSPL native representation */
			        readerId,
			        writerId,
			        &(_this->serializer));
		if (nofOctets < 0) break;
		total += nofOctets;

		if (inlineQosSize > 0) {
		    nofOctets =
		        in_ddsiParameterListForPublicationSerializeInstantly(
		                facade,
		                &(_this->serializer));
		    if (nofOctets < 0) break;
		    total += nofOctets;

		    /* must match alignment constraints */
		    assert( total % IN_DDSI_PARAMETER_HEADER_ALIGNMENT == 0);
		    assert(in_ddsiSerializerAlign(
		            &(_this->serializer),
		            IN_DDSI_PARAMETER_HEADER_ALIGNMENT)==0);
		}
		assert(P2UI(in_ddsiSerializerGetPosition(&(_this->serializer))) % 4 == 0);

		if (serializedPayloadSize > 0) {
		    in_data firstOctetBehindDataHeader =
		        in_ddsiSerializerGetPosition(&(_this->serializer));
		    in_data firstOctetBehindSerializedData = NULL;
		    os_size_t alignmentDistance = 0;

		    /* serializedPayloadSize must be smaller than 64K*/
            os_uint32 bytesLeft =
		        (os_uint32) serializedPayloadSize;

            /* DATA submessage does not support anything larger than that */
            assert(serializedPayloadSize <= IN_USHORT_MAX);

		    /* assuming that the message serializer will
		     * add the encapsulation header itself */
		    in_messageSerializerBegin(_this->messageSerializer,
		            firstOctetBehindDataHeader,
		            bytesLeft);
		    /* serialize the userData encapsulated within the
		     * v_message object */
		    retState =
		        in_messageSerializerWrite(
		            _this->messageSerializer,
		            message,
		            &serializedDataSize);

		    /* For DATA it must not exceed the the buffer size */
		    /* serializeData assumes that no fragmentation is done, so
		     * all data fits into the current buffer, so
		     * nofOctets==(firstOctetBehindSerializedData-firstOctetBehindDataHeader),
		     */
		    firstOctetBehindSerializedData =
		        in_messageSerializerEnd(_this->messageSerializer);
            if (retState!=IN_RESULT_OK) break;
            total += serializedDataSize;

            /* verify the buffer has not been  re-initialized */
            assert(firstOctetBehindDataHeader ==
                in_ddsiSerializerGetPosition(&(_this->serializer)));

		    /* seek the index of serializer to that position */
		    nofOctets =
		        in_ddsiSerializerSeekTo(
		            &(_this->serializer),
		            firstOctetBehindSerializedData);
		    if (nofOctets < 0) break;
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
		    alignmentDistance =
		        bytesLeft -
		        (firstOctetBehindSerializedData - firstOctetBehindDataHeader);
		    nofOctets =
		        in_ddsiSerializerSeek(
		                &(_this->serializer),
		                alignmentDistance);

		    if (nofOctets < 0) break;
		    total += nofOctets;
		}
		/* finally assign result */
		result = total;
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
	in_ddsiVendorId vendorId =
		IN_DDSI_VENDOR_PT;

	assert(in_ddsiSerializerRemainingCapacity(&(_this->serializer)) > OS_SIZEOF(in_ddsiMessageHeader));

	in_ddsiSerializerInitWithDefaultEndianess(
			&(_this->serializer),
			_this->currentSendBuffer);

	in_ddsiMessageHeaderSerializeInstantly(
			&protocolVersion,
			vendorId,
			guidPrefix,
			&(_this->serializer));
}

static os_size_t
calculatePayloadSize(v_message message)
{
    /* TODO ask the serializer to calculate the size */
    os_size_t realSize =
        400;
    os_size_t result;

    /* finally round up the value to match alignment constraints */
    result = IN_ALIGN_UINT_CEIL(
            realSize,
            IN_DDSI_SUBMESSAGE_HEADER_ALIGNMENT);

    assert( (result >= realSize) &&
            (result % (IN_DDSI_SUBMESSAGE_HEADER_ALIGNMENT) == 0));

    return result;
}

static OS_STRUCT(in_ddsiEntityId) unknownId =
    UINT32_TO_ENTITYID(IN_ENTITYID_UNKNOWN);

static in_result
in_ddsiStreamWriterImplAppendData(
		in_ddsiStreamWriterImpl _this,
		v_message message,
		in_connectivityWriterFacade facade,
		os_boolean recipientExpectsInlineQos,
		Coll_List *locatorList)
{
    const os_size_t sentinelSize =
        IN_DDSI_SUBMESSAGE_HEADER_SIZE;

	const in_ddsiGuid guid =
        in_connectivityEntityFacadeGetGuid(in_connectivityEntityFacade(facade));

	const os_size_t serializedPayloadSize =
	    calculatePayloadSize(message);

	const os_size_t inlineQosSize =
	    recipientExpectsInlineQos
	    ? in_ddsiParameterListForParticipantCalculateSize(facade)
	    : 0U;

	in_result result = IN_RESULT_OK;

	/* Note: do not store the locatorList permanently, it
	 * might be modified or erased by owner */
	const os_size_t timestampSubmessageLength =
		IN_DDSI_TIMESTAMP_SUBMESSAGE_SIZE;

	const os_size_t dataSubmessageLength =
		in_ddsiSubmessageDataSerializedSize(
		        inlineQosSize,
		        serializedPayloadSize);

	const os_size_t totalLength =
		timestampSubmessageLength +
		dataSubmessageLength +
		sentinelSize;

	const os_boolean isControl = OS_FALSE;

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
		const in_result  retState=
			serializeData(
				_this,
				inlineQosSize,
				serializedPayloadSize,
				&unknownId,
				facade,
				message);

		/* TODO append sentinel */

		if (retState!=IN_RESULT_OK) {
			/* serialization error */
			result = IN_RESULT_ERROR;
		} else {
		    const os_size_t length =
		            in_ddsiSerializerNofOctets(&(_this->serializer));

		    assert(length > 0);

			Coll_Iter *iter =
				Coll_List_getFirstElement(locatorList);

			while (iter) {
				in_locator destLocator =
					Coll_Iter_getObject(iter);

				assert(destLocator!=NULL);

				in_transportSenderSendTo(
						_this->transportSender,
						destLocator,
						_this->currentSendBuffer,
						length,
						isControl,
						&(_this->timeout));

				iter = Coll_Iter_getNext(iter);
			}
		}
	}

	return result;
}


/** */
static in_result
in_ddsiStreamWriterImplAppendParticipantData(
		in_ddsiStreamWriterImpl _this,
		in_connectivityParticipantFacade facade,
		Coll_List *locatorList)
{
    const os_size_t sentinelSize =
        IN_DDSI_SUBMESSAGE_HEADER_SIZE;

	const in_ddsiGuid guid =
        in_connectivityEntityFacadeGetGuid(in_connectivityEntityFacade(facade));

	in_result result = IN_RESULT_OK;

#if 0
	/* Note: do not store the locatorList permanently, it
	 * might be modified or erased by owner */
	const os_size_t timestampSubmessageLength =
		IN_DDSI_TIMESTAMP_SUBMESSAGE_SIZE;

	const os_size_t serializedPayloadSize =
	    IN_DDSI_ENCAPSULATION_HEADER_SIZE +
		in_ddsiParameterListForParticipantSerializedSized(facade);

	const os_size_t timestampSubmessageLength =
	    in_ddsiSubmessageDataSerializedSize(
	            inlineQosSize,
	            serializedPayloadSize);

	const os_size_t totalLength =
		timestampSubmessageLength +
		dataSubmessageLength +
		+ sentinelSize;

	const os_boolean isControl = OS_FALSE;

	assert(timestampSubmessageLength % 4 == 0);
	assert(dataSubmessageLength % 4 == 0);
	assert(dataSubmessageLength > 0);

	/* for now, one packet per message, no bundling of data items */

	in_ddsiStreamWriterImplResetBuffer(_this, guid->guidPrefix);

	if (totalLength > in_ddsiSerializerRemainingCapacity(&(_this->serializer))) {
	    /* now we should turn to fragmentation, but
	     * this is not implemented yet */
	    result = IN_RESULT_ERROR;
	} else {
		const in_long nofOctets =
		    serializeParticipantData(
		            _this,
		            serializedPayloadSize,
		            &unknownId, /* readerId */
		            facade);

		if (nofOctets  < 0) {
			/* serialization error */
			result = IN_RESULT_ERROR;
		} else {
		    const os_size_t length =
	            in_ddsiSerializerNofOctets(&(_this->serializer));
			Coll_Iter *iter =
				Coll_List_getFirstElement(locatorList);

			while (iter) {
				in_locator destLocator =
					Coll_Iter_getObject(iter);
				assert(destLocator!=NULL);

				in_transportSenderSendTo(
						_this->transportSender,
						destLocator,
						_this->currentSendBuffer,
						length,
						isControl,
						&(_this->timeout));

				iter = Coll_Iter_getNext(iter);
			}
		}
	}

#endif
	return result;
}

/** */
static in_result
in_ddsiStreamWriterImplAppendPublicationData(
        in_ddsiStreamWriterImpl _this,
        in_connectivityWriterFacade facade,
        Coll_List *locatorList)
{
    const in_ddsiGuid guid =
        in_connectivityEntityFacadeGetGuid(in_connectivityEntityFacade(facade));

    in_result result = IN_RESULT_OK;

    return result;
}

/** ingore "Final" flag for now
 *
 * If sending the first AckNack to this writerGuid, the streamWriter
 * must be flushed first to deliver previous data items to multiple
 * recipients. */
in_result
in_ddsiStreamWriterImplAppendAckNack(
        in_ddsiStreamWriterImpl _this,
        in_ddsiGuid readerGuid, /* TODO: maybe it would be more efficient to split into entityId and GuidPrefix*/
        in_ddsiGuid writerGuid,
        /* single sequence number, but will be transformed
         * to sequencenumber-set on the wire */
        in_ddsiSequenceNumber writerSN,
        in_locator singleDestination)
{
    in_result result = IN_RESULT_OK;
    /* TBD */
    return result;
}


/** */
static in_result
in_ddsiStreamWriterImplAppendSubscriptionData(
        in_ddsiStreamWriterImpl _this,
        in_connectivityReaderFacade facade,
        Coll_List *locatorList)
{
    const in_ddsiGuid guid =
        in_connectivityEntityFacadeGetGuid(in_connectivityEntityFacade(facade));

    in_result result = IN_RESULT_OK;

    return result;
}

/** \return  corresponding default multicast locator for data (also SDP traffic)
 *
 *  hook implementation of corresponding virtual method*/
static in_locator
in_streamWriterGetDataMulticastLocator_i(in_streamWriter _this)
{
    in_locator result = NULL;
    in_ddsiStreamWriterImpl narrow =
        in_ddsiStreamWriterImpl(_this);

    result =
        in_transportSenderGetDataMulticastLocator(&(narrow->transportSender));

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
        in_transportSenderGetDataUnicastLocator(&(narrow->transportSender));

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
        in_transportSenderGetCtrlUnicastLocator(&(narrow->transportSender));

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
        in_ddsiSequenceNumber writerSN,
        in_locator singleDestination)
{
    in_ddsiStreamWriterImpl narrowed =
         in_ddsiStreamWriterImpl(_this);

    /* narrowing */
    return in_ddsiStreamWriterImplAppendAckNack(
            narrowed,
            readerGuid,
            writerGuid,
            writerSN,
            singleDestination);
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
        in_connectivityWriterFacade facade,
        os_boolean recipientExpectsInlineQos,
        Coll_List *locatorList)
{
    /* narrow */
    return in_ddsiStreamWriterImplAppendData(
            in_ddsiStreamWriterImpl(_this),
            message,
            facade,
            recipientExpectsInlineQos,
            locatorList);
}

/** hook  implementation of corresponding virtual method*/
static in_result
in_streamWriterAppendParticipantData_i(
            in_streamWriter _this,
            in_connectivityParticipantFacade facade,
            Coll_List *locatorList)
{   /* narrow */
    return in_ddsiStreamWriterImplAppendParticipantData(
            in_ddsiStreamWriterImpl(_this),
            facade,
            locatorList);
}

/*  hook  implementation of corresponding virtual method*/
static in_result
in_streamWriterAppendSubscriptionData_i(
            in_streamWriter _this,
            in_connectivityReaderFacade facade,
            Coll_List *locatorList)
{   /* narrow */
    return in_ddsiStreamWriterImplAppendSubscriptionData(
            in_ddsiStreamWriterImpl(_this),
            facade,
            locatorList);
}

/*  hook  implementation of corresponding virtual method*/
static in_result
in_streamWriterAppendPublicationData_i(
            in_streamWriter _this,
            in_connectivityWriterFacade facade,
            Coll_List *locatorList)
{   /* narrow */
    return in_ddsiStreamWriterImplAppendPublicationData(
            in_ddsiStreamWriterImpl(_this),
            facade,
            locatorList);
}


static void
in_ddsiStreamReaderImplGetBuffer(
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
    assert(!"never reached");
}

static void
in_messageTransformerGetBuffer_i(
        in_data* bufferPtr,
        os_uint32* length,
        c_voidp userData)
{
    /* narrow */
    in_ddsiStreamReaderImplGetBuffer(
            in_ddsiStreamWriterImpl(userData),
            bufferPtr,
            length);
}

/* **** public functions **** */

/* static memory area */
static const  OS_STRUCT(in_streamWriterPublicVTable)
staticPublicVTable = {
        in_streamWriterFlush_i,
        in_streamWriterFlushSingle_i,
        in_streamWriterAppendData_i,
        in_streamWriterAppendParticipantData_i,
        in_streamWriterAppendSubscriptionData_i,
        in_streamWriterAppendPublicationData_i,
        in_streamWriterAppendAckNack_i,
        in_streamWriterGetDataUnicastLocator_i,
        in_streamWriterGetDataMulticastLocator_i,
        in_streamWriterGetCtrlUnicastLocator_i};


/**
 */
in_ddsiStreamWriterImpl
in_ddsiStreamWriterImplNew(
		in_configChannel config,
		in_transportSender sender,
		in_connectivityAdmin connectivityAdmin)
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
		result->transportSender = sender;
		result->currentSendBuffer =
			in_transportSenderGetBuffer(sender);
		result->connectivityAdmin = connectivityAdmin;

		result->messageSerializer =
		    in_messageSerializerNew(
		        in_messageTransformerGetBuffer_i,
		        (c_voidp) result);

		if (result->currentSendBuffer==NULL ||
		    result->messageSerializer==NULL) {
		    if (result->currentSendBuffer) {
		        in_transportSenderReleaseBuffer(
		                sender,
		                result->currentSendBuffer);
		    }
		    if (result->messageSerializer) {
		        in_objectFree(result->messageSerializer);
		    }

			in_streamWriterDeinit(OS_SUPER(result));
			os_free(result);
			result = NULL;
		} else {
			in_ddsiSerializerInitWithDefaultEndianess(
					&(result->serializer),
					result->currentSendBuffer);
		}
	}

	return result;
}

/**
 *  */
void
in_ddsiStreamWriterImplFree(in_ddsiStreamWriterImpl _this)
{
	in_streamWriterFree(OS_SUPER(_this));
}


