/*
 * in_ddsiStreamReaderImpl.c
 *
 *  Created on: Mar 5, 2009
 *      Author: frehberg
 */

/* **** interface headers **** */
#include "in__ddsiStreamReaderImpl.h"

/* **** implementation headers **** */
#include "in_abstractReceiveBuffer.h"
#include "in_assert.h"
#include "in_report.h"
#include "in_ddsiSubmessageTokenizer.h"
#include "in_ddsiSubmessageDeserializer.h"
#include "in_ddsiElements.h"
#include "in_ddsiSubmessage.h"
#include "in_ddsiParameterList.h"
#include "in_connectivityPeerParticipant.h"
#include "in_connectivityPeerWriter.h"
#include "in_connectivityParticipantFacade.h"

#include "in_ddsiPublication.h"  /* in_ddsiDiscoveredWriterData */
#include "in_ddsiSubscription.h" /* in_ddsiDiscoveredReaderData */
#include "in_ddsiParticipant.h"  /* in_ddsiDiscoveredParticipantData */
#include "in__messageDeserializer.h"
#include "in_messageQos.h"
#include "in_ddsiEncapsulationHeader.h"
#include "v_topic.h"

/* **** private functions **** */

#define WARN_SUBMESSAGE_IGNORED(_kind)             \
	IN_REPORT_INFO_1(0, "submessage-kind %0x ignored", (os_uint32)(_kind))
#define WARN_SUBMESSAGE_NOT_YET_IMPLEMENTED(_kind) \
	IN_REPORT_INFO_1(0, "submessage-kind %0x not yet implemented", (os_uint32)(_kind))
#define WARN_SUBMESSAGE_UNKNOWN(_kind)            \
	IN_REPORT_INFO_1(0, "submessage-kind %0x not yet implemented", (os_uint32)(_kind))

#define IN_WARN_ENTITYKIND_UNKNOWN(_kind) \
	IN_REPORT_INFO_1(0, "entity-kind %0x unknown", (os_uint32)(_kind))


/** */
static os_boolean
in_ddsiStreamReaderImplProcessData(
		in_ddsiStreamReaderImpl _this,
		in_ddsiSubmessageDeserializer submessageDeserializer);

/** */
static os_boolean
in_ddsiStreamReaderImplProcessHeartbeat(
		in_ddsiStreamReaderImpl _this,
		in_ddsiSubmessageDeserializer submessageDeserializer);

/** */
static in_result
in_ddsiStreamReaderImplScanBuffer(
		in_ddsiStreamReaderImpl _this);


/** */
static os_boolean
in_ddsiStreamReaderImplProcessAppdefData(
		in_ddsiStreamReaderImpl _this,
		in_ddsiSubmessageData submessage);


/** */
static os_boolean
in_ddsiStreamReaderImplProcessBuiltinParticipantData(
		in_ddsiStreamReaderImpl _this,
		in_ddsiSubmessageData submessage);


/** */
static os_boolean
in_ddsiStreamReaderImplProcessBuiltinReaderData(
		in_ddsiStreamReaderImpl _this,
		in_ddsiSubmessageData submessage);


/** */
static os_boolean
in_ddsiStreamReaderImplProcessBuiltinWriterData(
        in_ddsiStreamReaderImpl _this,
        in_ddsiSubmessageData submessage);


static v_topic
lookupTopic(
    in_ddsiStreamReaderImpl _this,
    in_connectivityPeerWriter peerWriter)
{
    v_topic result = NULL;

    if (peerWriter) {
        c_char* topicName =
            in_connectivityPeerWriterGetTopicName(peerWriter);

        if (topicName) {
            result =
                in_plugKernelLookupTopic(
                    _this->plugKernel,
                    topicName);
        }
    }

    return result;
}

static in_connectivityPeerWriter
getPeerWriter(in_ddsiStreamReaderImpl _this,
        in_ddsiGuidPrefixRef guidPrefix,
        in_ddsiEntityId writerId)
{
    const in_ddsiGuidPrefix UNKNOWN_PREFIX =
        IN_GUIDPREFIX_UNKNOWN;
    OS_STRUCT(in_ddsiGuid) writerGuid;
    in_connectivityAdmin admin =
        in_connectivityAdminGetInstance();
    in_connectivityPeerWriter result = NULL;

    assert(admin);

    if (memcmp(UNKNOWN_PREFIX, guidPrefix, sizeof(UNKNOWN_PREFIX))==0) {
        IN_REPORT_WARNING(IN_SPOT, "guid prefix invalid");
    }

    if (writerId->entityKey[0] == 0 &&
            writerId->entityKey[1] == 0 &&
            writerId->entityKey[2] == 0 &&
            writerId->entityKind == 0) {
        IN_REPORT_WARNING(IN_SPOT, "writerId invalid");
    }

    in_ddsiGuidInit(
                &writerGuid,
                guidPrefix,
                writerId);

    result =
        in_connectivityAdminGetPeerWriter(admin, &writerGuid);

    return result;
}

static void
releaseTopic(
    in_ddsiStreamReaderImpl _this,
    v_topic topic)
{
    c_free(topic);
}

/* *******************************************/
/* **** private function implementation **** */
/* *******************************************/


static in_result
in_ddsiStreamReaderImplProcessAppdefDataPayload(
        in_ddsiStreamReaderImpl _this,
        in_ddsiSubmessageData submessage,
        in_connectivityPeerWriter peerWriter,
        v_message *messageObject)

{
    in_result result = IN_RESULT_OK;
    v_topic topic = NULL;
    in_ddsiSerializedData serializedPayload = NULL;
    OS_STRUCT(in_ddsiEncapsulationHeader) encapsHeader;
    in_long encapsHeaderSize = 0;
    in_octet *firstOctetBehindMessage = NULL; /* dummy */


    assert(peerWriter);
    assert(messageObject);
    assert(submessage);

    *messageObject = NULL;
    serializedPayload =
        &submessage->serializedPayload;

    topic = lookupTopic(_this, peerWriter);

    /* paranoid checks */
    if (!topic || !(topic->dataField) || !(topic->dataField->type)) {
        IN_REPORT_WARNING_1(IN_SPOT, "topic lookup failed for %s",
                in_connectivityPeerWriterGetTopicName(peerWriter));
    } else {
        encapsHeaderSize =
            in_ddsiEncapsulationHeaderInit(
                &encapsHeader,
                serializedPayload->begin,
                serializedPayload->length);

        if (encapsHeaderSize<0) {
            /* TODO report error, invalid payload */
            result = IN_RESULT_ERROR;
        } else {
            const c_type type = v_topicMessageType(topic);
            const c_long offset = v_topicDataOffset(topic);
            const os_boolean isBigEndian =
                in_ddsiEncapsulationHeaderIsBigEndian(&encapsHeader);

            in_messageDeserializerBegin(
                    _this->messageDeserializer,
                    serializedPayload->begin + (os_size_t) encapsHeaderSize,
                    serializedPayload->length - encapsHeaderSize);

            result =
                in_messageDeserializerRead(
                        _this->messageDeserializer,
                        type,
                        offset,
                        isBigEndian,
                        messageObject);

            /* value is ignored */
            firstOctetBehindMessage =
                in_messageDeserializerEnd(_this->messageDeserializer);

            if (result==IN_RESULT_OK && *messageObject) {
                const v_gid debugGuid = {0,0,0};

                v_message m = *messageObject;

                OS_SUPER(m)->nodeState = 0;
                m->allocTime = _this->receiver.haveTimestamp ?
                        _this->receiver.timestamp : C_TIME_ZERO;
                m->sequenceNumber = (submessage->writerSN.low);
                m->writeTime = _this->receiver.haveTimestamp ?
                        _this->receiver.timestamp : C_TIME_ZERO;
                m->writerGID = debugGuid; /* tbd */
                m->writerInstanceGID = debugGuid; /* tbd */
                m->qos = in_messageQos_new(peerWriter, c_getBase(topic));
            }
        }
    }
    if(topic)
    {
        /* decrement refcount */
        releaseTopic(_this, topic);
    }

    return result;
}

/** */
static os_boolean
in_ddsiStreamReaderImplProcessAppdefData(
		in_ddsiStreamReaderImpl _this,
		in_ddsiSubmessageData submessage)
{
	os_boolean result = OS_FALSE;

    /* if the callback is undefined, avoid further parsing and processing,
     * but return with TRUE, so that scanner continues with following
     * submessages */
 	if (!(_this->callbackTable->processData)) {
	    IN_TRACE(Receive,2,"callback 'processData' not defined, AppeDef Data ignored");
	    result = OS_TRUE; /* continue buffer scan */
	} else {
        /*  peerWriter's refcounter is not incremented  */
        in_connectivityPeerWriter peerWriter =
            getPeerWriter(
                    _this,
                    &(_this->receiver.sourceGuidPrefix[0]),
                    &(submessage->writerId));

        v_message messageObject = NULL;

        if (!peerWriter) {
            /* TODO, maybe the inlineQos contains the topic-name we could use to fetch the
             * topic and type objects */
            IN_REPORT_WARNING(IN_SPOT, "ignoring message from unknwon peer writer");
            result = OS_TRUE; /* continue with succeeding messages in same buffer */
        } else {
            in_result payloadResult =
                in_ddsiStreamReaderImplProcessAppdefDataPayload(_this,
                        submessage,
                        peerWriter,
                        &messageObject);
             if (payloadResult!=IN_RESULT_OK) {
                 IN_REPORT_WARNING(IN_SPOT, "message deserialization failed");
                 result = OS_FALSE; /* abort processing this buffer */
             } else if (messageObject==NULL) { /* paranoid check */
                 IN_REPORT_WARNING(IN_SPOT, "message parsing succeeded, but message object empty");
                 result = OS_FALSE; /* abort processing this buffer */
             } else {
                 /* messageObject points to vali v_message object */

                 /* TODO process inlineQos */

                 in_result callbackResult =
                       _this->callbackTable->processData(
                           _this->callbackArg,
                           messageObject, /* handing over ownership */
                           peerWriter,
                           &(_this->receiver));

                 result = (callbackResult==IN_RESULT_OK)
                         ? OS_TRUE
                         : OS_FALSE;
             }
        }
	}
	return result;
}



/** */
static os_boolean
in_ddsiStreamReaderImplProcessBuiltinParticipantData(
		in_ddsiStreamReaderImpl _this,
		in_ddsiSubmessageData submessage)
{
    os_boolean result = OS_FALSE;
    /* if the callback is undefined, avoid further parsing and processing,
     * but return with TRUE, so that scanner continues with following
     * submessages */
    if (!(_this->callbackTable->processPeerEntity)) {
        IN_TRACE(Receive,2,"callback 'processPeerEntity' not defined, BuiltinParticipantData ignored");
        result = OS_TRUE; /* continue buffer scan */
    } else {
        in_ddsiDiscoveredParticipantData discoveredData = NULL;

        in_ddsiSerializedData seriallizedPayload =
            &(submessage->serializedPayload);

        /** codec must be either PL_CDR_BE or PL_CDR_LE */
        OS_STRUCT(in_ddsiParameterList) parameterList;
        IN_TRACE(Receive,2,"in_ddsiStreamReaderImplProcessBuiltinParticipantData ");
        if (!in_ddsiParameterListInitFromEncapsulation(
                &parameterList,
                seriallizedPayload)) {
            IN_REPORT_WARNING_1(IN_SPOT, "unknown codec %d",
                    seriallizedPayload->codecId);
            result = OS_FALSE;
        } else {
            /* TODO, no fragmentation supported yet */
            discoveredData =
                in_ddsiDiscoveredParticipantDataNew();
            if (!discoveredData) {
                /* TODO handle out-of-memory */
                result = OS_FALSE;
            } else {
                /* TODO, to handle fragments, add
                 * newBuffer-callback to decode operation */
                const in_result parseResult =
                    in_ddsiParameterListForParticipantParse(
                                &parameterList,
                                discoveredData);
                if (parseResult!=IN_RESULT_OK) {
                    /* TODO handle out-of-memory */
                    result = OS_FALSE;
                } else {
                    in_connectivityPeerParticipant newPeer;

                    newPeer = in_connectivityPeerParticipantNew(discoveredData);

                    /* in any case release the data object, which should be
                     * refcounted by peer-object now  */
                    in_ddsiDiscoveredParticipantDataFree(discoveredData);

                    if (!newPeer) {
                        /* TODO handle out-of-memory */
                        result = OS_FALSE;
                    } else {
                        in_result processResult;
                        OS_STRUCT(in_discoveredPeer) discoveredPeer;
                        assert(sizeof(in_ddsiGuidPrefix) == 12);

                         memcpy(discoveredPeer.writerGuidPrefix,
                                 _this->receiver.sourceGuidPrefix,
                                 sizeof(in_ddsiGuidPrefix));
                         memcpy(discoveredPeer.readerGuidPrefix,
                                 _this->receiver.destGuidPrefix,
                                 sizeof(in_ddsiGuidPrefix));
                         discoveredPeer.writerId = &(submessage->writerId);
                         discoveredPeer.readerId = &(submessage->readerId);
                         discoveredPeer.sequenceNumber = &(submessage->writerSN);
                         discoveredPeer.discoveredPeerEntity =
                             in_connectivityPeerEntity(newPeer);

                         /* TODO verify if builtin-topic writer is reliable and
                         * requires ackNacks */
                         processResult =
                            _this->callbackTable->processPeerEntity(
                                _this->callbackArg, /* object */
                                &discoveredPeer); /* method parameter */
                         /* cleanup the dynamic object, decrement refcount */
                         in_connectivityPeerParticipantFree(newPeer);

                         if (processResult!=IN_RESULT_OK && processResult!=IN_RESULT_ALREADY_EXISTS) {
                            result = OS_FALSE;
                         } else {
                            result = OS_TRUE;
                         }
                    }
                }
            }
        }
	}
	return result;
}


/** */
static os_boolean
in_ddsiStreamReaderImplProcessBuiltinReaderData(
        in_ddsiStreamReaderImpl _this,
        in_ddsiSubmessageData submessage)
{
    os_boolean result = OS_FALSE;
    /* if the callback is undefined, avoid further parsing and processing,
     * but return with TRUE, so that scanner continues with following
     * submessages */
    if (!(_this->callbackTable->processPeerEntity)) {
        IN_TRACE(Receive,2,"callback 'processPeerEntity' not defined, BuiltinReaderData ignored");
        result = OS_TRUE; /* continue buffer scan */
    } else {
        in_ddsiDiscoveredReaderData discoveredData = NULL;

        in_ddsiSerializedData seriallizedPayload =
            &(submessage->serializedPayload);

        /** codec must be either PL_CDR_BE or PL_CDR_LE */
        OS_STRUCT(in_ddsiParameterList) parameterList;

        if (!in_ddsiParameterListInitFromEncapsulation(
                &parameterList,
                seriallizedPayload)) {
            IN_REPORT_WARNING_1(IN_SPOT, "unknown codec %d",
                    seriallizedPayload->codecId);
            result = OS_FALSE;
        } else {
            /* TODO, no fragmentation supported yet */
            discoveredData =
                in_ddsiDiscoveredReaderDataNew();

            if (!discoveredData) {
                /* TODO handle out-of-memory */
                result = OS_FALSE;
            } else {
                const in_result parseResult =
                    /* TODO, to handle fragments, add
                     * newBuffer-callback to decode operation */
                    in_ddsiParameterListForReaderParse(
                        &parameterList,
                        discoveredData,
                        in_plugKernelGetBase(_this->plugKernel));

                if (parseResult!=IN_RESULT_OK) {
                    /* TODO handle out-of-memory */
                    result = OS_FALSE;
                } else {
                    in_connectivityPeerReader newPeer;

                    newPeer = in_connectivityPeerReaderNew(discoveredData);

                    /* in any case release the data object, which should be
                     * refcounted by peer-object now  */
                    in_ddsiDiscoveredReaderDataFree(discoveredData);

                    if (!newPeer) {
                        /* TODO handle out-of-memory */
                        result = OS_FALSE;
                    } else {
                        in_result processResult;

                        OS_STRUCT(in_discoveredPeer) discoveredPeer;
                        discoveredPeer.readerId = &(submessage->readerId);
                        discoveredPeer.writerId = &(submessage->writerId);
                        discoveredPeer.sequenceNumber = &(submessage->writerSN);
                        discoveredPeer.discoveredPeerEntity =
                            in_connectivityPeerEntity(newPeer);

                        /* TODO verify if builtin-topic writer is reliable and
                         * requires ackNacks */
                        processResult =
                            _this->callbackTable->processPeerEntity(
                                _this->callbackArg, /* object */
                                &discoveredPeer);           /* method parameter */
                        /* cleanup the dynamic object, decrement refcount */
                          in_connectivityPeerReaderFree(newPeer);

                          if (processResult!=IN_RESULT_OK && processResult!=IN_RESULT_ALREADY_EXISTS) {
                            result = OS_FALSE;
                        } else {
                            result = OS_TRUE;
                        }
                    }
                }
            }
        }
    }

    return result;
}

/** */
static os_boolean
in_ddsiStreamReaderImplProcessBuiltinWriterData(
        in_ddsiStreamReaderImpl _this,
        in_ddsiSubmessageData submessage)
{
    os_boolean result = OS_FALSE;
    /* if the callback is undefined, avoid further parsing and processing,
     * but return with TRUE, so that scanner continues with following
     * submessages */
    if (!(_this->callbackTable->processPeerEntity)) {
         IN_TRACE(Receive,2,"callback 'processPeerEntity' not defined, BuiltinWriterData ignored");
         result = OS_TRUE; /* continue buffer scan */
     } else {
        in_ddsiDiscoveredWriterData discoveredData = NULL;

        in_ddsiSerializedData seriallizedPayload =
            &(submessage->serializedPayload);

        /** codec must be either PL_CDR_BE or PL_CDR_LE */
        OS_STRUCT(in_ddsiParameterList) parameterList;

        if (!in_ddsiParameterListInitFromEncapsulation(
                &parameterList,
                seriallizedPayload)) {
            IN_REPORT_WARNING_1(IN_SPOT, "unknown codec %d",
                    seriallizedPayload->codecId);
            result = OS_FALSE;
        } else {
            /* TODO, no fragmentation supported yet */
            discoveredData =
                in_ddsiDiscoveredWriterDataNew();

            if (!discoveredData) {
                /* TODO handle out-of-memory */
                result = OS_FALSE;
            } else {
                const in_result parseResult =
                    /* TODO, to handle fragments, add
                     * newBuffer-callback to decode operation */
                    in_ddsiParameterListForWriterParse(
                        &parameterList,
                        discoveredData,
                        in_plugKernelGetBase(_this->plugKernel));

                if (parseResult!=IN_RESULT_OK) {
                    /* TODO handle out-of-memory */
                    result = OS_FALSE;
                } else {
                    in_connectivityPeerWriter newPeer =
                        in_connectivityPeerWriterNew(discoveredData);

                    /* in any case release the data object, which should be
                     * refcounted by peer-object now  */
                    in_ddsiDiscoveredWriterDataFree(discoveredData);

                    if (!newPeer) {
                        /* TODO handle out-of-memory */
                        result = OS_FALSE;
                    } else {
                        in_result processResult;

                        OS_STRUCT(in_discoveredPeer) discoveredPeer;
                        discoveredPeer.readerId = &(submessage->readerId);
                        discoveredPeer.writerId = &(submessage->writerId);
                        discoveredPeer.sequenceNumber = &(submessage->writerSN);
                        discoveredPeer.discoveredPeerEntity =
                            in_connectivityPeerEntity(newPeer);

                        /* TODO verify if builtin-topic writer is reliable and
                         * requires ackNacks */
                        processResult =
                            _this->callbackTable->processPeerEntity(
                                _this->callbackArg, /* object */
                                &discoveredPeer);           /* method parameter */
                        /* cleanup the dynamic object, decrement refcount */
                        in_connectivityPeerWriterFree(newPeer);

                        if (processResult!=IN_RESULT_OK && processResult!=IN_RESULT_ALREADY_EXISTS) {
                            result = OS_FALSE;
                        } else {
                            result = OS_TRUE;
                        }
                    }
                }
            }
        }
    }

    return result;
}

/** */
static os_boolean
in_ddsiStreamReaderImplProcessBuiltinData(
        in_ddsiStreamReaderImpl _this,
        in_ddsiSubmessageData submessage)
{
    os_boolean result = OS_FALSE;

    os_uint32 writerIdAsUint32 =
        in_ddsiEntityIdAsUint32(&(submessage->writerId));

    switch (writerIdAsUint32) {
    case IN_ENTITYID_SEDP_BUILTIN_TOPIC_WRITER: /* 0x000002c2 */        /* Was: ENTITYID_BUILTIN_TOPIC_WRITER */
        IN_REPORT_WARNING_1(IN_SPOT, "entity %0x ignored", writerIdAsUint32);
        break;

    case IN_ENTITYID_SEDP_BUILTIN_PUBLICATIONS_WRITER: /* 0x000003c2 */        /* Was: ENTITYID_BUILTIN_PUBLICATIONS_WRITER */
        /* only process submessage if the specific callback
         * function has been defined, otherwise ignore */
        result =
            in_ddsiStreamReaderImplProcessBuiltinWriterData(
                    _this,
                    submessage);
        break;

    case IN_ENTITYID_SEDP_BUILTIN_SUBSCRIPTIONS_WRITER: /* 0x000004c2 */        /* Was: ENTITYID_BUILTIN_SUBSCRIPTIONS_WRITER */
        /* only process submessage if the specific callback
         * function has been defined, otherwise ignore */
         result =
            in_ddsiStreamReaderImplProcessBuiltinReaderData(
                    _this,
                    submessage);
        break;


    case IN_ENTITYID_SPDP_BUILTIN_PARTICIPANT_WRITER: /* 0x000100c2 */        /* Was: ENTITYID_BUILTIN_SDP_PARTICIPANT_WRITER */
        /* only process submessage if the specific callback
         * function has been defined, otherwise ignore */
        result =
            in_ddsiStreamReaderImplProcessBuiltinParticipantData(
                    _this,
                    submessage);
        break;

    case IN_ENTITYID_P2P_BUILTIN_PARTICIPANT_MESSAGE_WRITER: /* 0x000200c2 */
        IN_REPORT_WARNING_1(IN_SPOT, "entity %0x ignored", writerIdAsUint32);
        break;

    default:
        IN_REPORT_WARNING_1(IN_SPOT, "entity %0x unexpected", writerIdAsUint32);
        break;
    }
    return result;
}

/** */
static os_boolean
in_ddsiStreamReaderImplProcessInfoTimestamp(
        in_ddsiStreamReaderImpl _this,
        in_ddsiSubmessageDeserializer submessageDeserializer)
{
    os_boolean result = OS_FALSE;
    OS_STRUCT(in_ddsiTime) timestamp;

    in_long nofOctets =
        in_ddsiTimeInitFromBuffer(
                &timestamp,
                OS_SUPER(submessageDeserializer));
   if (nofOctets<0) {
       result = OS_FALSE;
   } else {
       result = OS_TRUE;
       _this->receiver.haveTimestamp = OS_TRUE;
       in_ddsiTimeAsCTime(
               &timestamp,
               &(_this->receiver.timestamp),
               OS_FALSE);
   }
    return result;
}

/** */
static os_boolean
in_ddsiStreamReaderImplProcessInfoDestination(
        in_ddsiStreamReaderImpl _this,
        in_ddsiSubmessageDeserializer submessageDeserializer)
{
    os_boolean result = OS_FALSE;

    in_long nofOctets =
        in_ddsiGuidPrefixInitFromBuffer(
                &(_this->receiver.destGuidPrefix[0]),
                OS_SUPER(submessageDeserializer));
    result = (nofOctets>=0);

    return result;
}
/** */
static os_boolean
in_ddsiStreamReaderImplProcessHeartbeat(
        in_ddsiStreamReaderImpl _this,
        in_ddsiSubmessageDeserializer submessageDeserializer)
{
    os_boolean result = OS_TRUE;
    OS_STRUCT(in_ddsiHeartbeat) heartbeat;


    if (!_this->callbackTable->processHeartbeat) {
        IN_TRACE(Receive,2,"callback 'processHeartbeat' not defined, heartbeat will be ignored");
        result = OS_TRUE; /* continue buffer scan */
    } else
    {

        in_ddsiDeserializer deserializer;
        in_ddsiSubmessageHeader preparsedHeader;
        in_long nofOctets;

        deserializer = OS_SUPER(submessageDeserializer);
        preparsedHeader = &(submessageDeserializer->submessageHeader);

        nofOctets = in_ddsiSubmessageHeartbeatInitFromBuffer(
                    OS_SUPER(&heartbeat),
                    preparsedHeader,
                    deserializer);
        if (nofOctets < 0)
        {
            result = OS_FALSE;
        } else
        {
            in_result retVal;

            retVal = _this->callbackTable->processHeartbeat(
                    _this->callbackArg,
                    &heartbeat,
                    &(_this->receiver));
            result = (retVal==IN_RESULT_OK);
        }

    }
    return result;
}

/** */
static os_boolean
in_ddsiStreamReaderImplProcessAckNack(
        in_ddsiStreamReaderImpl _this,
        in_ddsiSubmessageDeserializer submessageDeserializer)
{
    os_boolean result = OS_TRUE;
    OS_STRUCT(in_ddsiAckNack) ackNack; /* extends from in_ddsiSubmessageAckNack*/
    if (!_this->callbackTable->processAckNack) {
        IN_TRACE(Receive,2,"callback 'processAckNack' not defined, AckNack will be ignored");
        result = OS_TRUE; /* continue buffer scan */
    } else {
        IN_TRACE(Receive,2,"callback 'processAckNack' will be called");
        in_ddsiDeserializer deserializer =
            OS_SUPER(submessageDeserializer);
        in_ddsiSubmessageHeader preparsedHeader =
            &(submessageDeserializer->submessageHeader);

        in_long nofOctets =
            in_ddsiSubmessageAckNackInitFromBuffer(
                    OS_SUPER(&ackNack),
                    preparsedHeader,
                    deserializer);
        if (nofOctets < 0) {
            IN_TRACE(Receive,2,"callback 'processAckNack' will be called === FALSE");
            result = OS_FALSE;
        } else {
            IN_TRACE(Receive,2,"callback 'processAckNack' will be called === TRUE");
            in_result retVal =
                _this->callbackTable->processAckNack(
                    _this->callbackArg,
                    &ackNack,
                    &(_this->receiver));
            result = (retVal==IN_RESULT_OK);
        }
    }
    return result;
}

/** */
static os_boolean
in_ddsiStreamReaderImplProcessData(
		in_ddsiStreamReaderImpl _this,
		in_ddsiSubmessageDeserializer submessageDeserializer)
{
	os_boolean result = OS_TRUE;
    in_long length;

	OS_STRUCT(in_ddsiSubmessageData) submessage;
	in_ddsiDeserializer deserializer = NULL;
	in_ddsiSubmessageHeader submessageHeader = NULL;

	assert(_this);

	deserializer =
		OS_SUPER(submessageDeserializer);
	submessageHeader =
		&(submessageDeserializer->submessageHeader);

	assert(deserializer);
	assert(submessageHeader);
	assert(submessageHeader->kind==IN_RTPS_DATA);

	/* pre: parsing of submessage header succeed already,
	 * the deserializer is pointing to first octet behind the
	 * submessage-header. The deserializer prevents parsing
	 * beyond submessage end */
    length = in_ddsiSubmessageDataInitFromBuffer(
        &submessage,
        submessageHeader,
        deserializer);
 	if (length < 0) {
		result = OS_FALSE;
	} else {
		in_ddsiEntityId writerId = &(submessage.writerId);

	switch (writerId->entityKind) {
        case IN_ENTITYKIND_APPDEF_WRITER_WITH_KEY:    /* (0x02) */
		case IN_ENTITYKIND_APPDEF_WRITER_NO_KEY:      /* (0x03) */
		    /* only process submessage if the specific callback
		     * function has been defined, otherwise ignore */
		    result =
		        in_ddsiStreamReaderImplProcessAppdefData(
		                _this,
		                &submessage);
		    break;

        case IN_ENTITYKIND_BUILTIN_WRITER_WITH_KEY:   /* (0xc2) */
		case IN_ENTITYKIND_BUILTIN_WRITER_NO_KEY:     /* (0xc3) */
		    result = in_ddsiStreamReaderImplProcessBuiltinData(
		            _this,
		            &submessage);
			break;

		default:
			IN_WARN_ENTITYKIND_UNKNOWN(writerId->entityKind);
			result = OS_FALSE;
			break;
		}
	}
	return result;
}


static in_result
in_ddsiStreamReaderImplScanBuffer(
		in_ddsiStreamReaderImpl _this)
{
	in_result result = IN_RESULT_OK;
	in_ddsiSubmessageTokenizer tokenizer =
		&(_this->currentReceiveBufferTokenizer);
	in_ddsiSubmessageToken token = NULL;
	OS_STRUCT(in_ddsiSubmessageDeserializer) deserializer;
	os_boolean continueScan = OS_TRUE;

	assert(_this->currentReceiveBuffer != NULL);

	/* iterate all submessages in buffer */
	for (token=in_ddsiSubmessageTokenizerGetNext(tokenizer, &deserializer);
		 token!=NULL && continueScan;
		 token=in_ddsiSubmessageTokenizerGetNext(tokenizer, &deserializer)) {

		const in_ddsiSubmessageKind kind =
			deserializer.submessageHeader.kind;

		switch (kind) {
		case  IN_SENTINEL:          /* (0x0) */
		    /* end of buffer reached */
			continueScan=OS_FALSE;
			break;

		case  IN_PAD:               /* (0x01) */
		    /* just skip the specified amount of octets */
		    break;

		case  IN_DATA:              /* (0x02) */
		case  IN_NOKEY_DATA:        /* (0x03) */
			WARN_SUBMESSAGE_IGNORED(kind);
			break;

		case  IN_ACKNACK:           /* (0x06) */
            /* only process submessage if the specific callback
             * function has been defined, otherwise ignore */
            if (!in_ddsiStreamReaderImplProcessAckNack(
                    _this,
                    &deserializer)) {
                /* interrupt scan either if parsing failed or result points to valid
                 * to new v_message object */
                continueScan = OS_FALSE;
            }
            break;

		case  IN_HEARTBEAT:         /* (0x07) */
		    /* only process submessage if the specific callback
		     * function has been defined, otherwise ignore */
			if (!in_ddsiStreamReaderImplProcessHeartbeat(
			        _this,
			        &deserializer)) {
			    /* interrupt scan either if parsing failed or result points to valid
			     * to new v_message object */
			    continueScan = OS_FALSE;
			}
			break;

		case  IN_GAP:           	/* (0x08) */
			WARN_SUBMESSAGE_NOT_YET_IMPLEMENTED(kind);
			break;

		case  IN_INFO_TS:           /* (0x09) */
		    /* submessage modifying receiver state only, processed always */
		    if (!in_ddsiStreamReaderImplProcessInfoTimestamp(
		                        _this,
		                        &deserializer)) {
		        /* interrupt scan either if parsing failed or result points to valid
		         * to new v_message object */
		        continueScan = OS_FALSE;
		    }
		    break;

		case  IN_INFO_SRC:          /* (0x0c) */
			WARN_SUBMESSAGE_NOT_YET_IMPLEMENTED(kind);
			break;

		case  IN_INFO_REPLY_IP4:    /* (0x0d) */
			WARN_SUBMESSAGE_NOT_YET_IMPLEMENTED(kind);
			break;

		case  IN_INFO_DST:          /* (0x0e) */
            /* submessage modifying receiver state only, processed always */
		    if (!in_ddsiStreamReaderImplProcessInfoDestination(
		            _this,
		            &deserializer)) {
		        /* interrupt scan either if parsing failed or result points to valid
		         * to new v_message object */
		        continueScan = OS_FALSE;
		    }
			break;

		case  IN_INFO_REPLY:        /* (0x0f) */
			WARN_SUBMESSAGE_NOT_YET_IMPLEMENTED(kind);
			break;

		case  IN_DATA_FRAG:   		/* (0x10)  RTPS 2.0 Only */
			WARN_SUBMESSAGE_IGNORED(kind);
			break;

		case  IN_NOKEY_DATA_FRAG:   /* (0x11)  RTPS 2.0 Only */
			WARN_SUBMESSAGE_IGNORED(kind);
			break;

		case  IN_NACK_FRAG:   		/* (0x12)  RTPS 2.0 Only */
		    WARN_SUBMESSAGE_IGNORED(kind);
			break;

		case  IN_HEARTBEAT_FRAG:    /* (0x13)  RTPS 2.0 Only */
		    WARN_SUBMESSAGE_IGNORED(kind);
			break;

		case  IN_RTPS_DATA:   		/* (0x15)  RTPS 2.1 only */
			if (!in_ddsiStreamReaderImplProcessData(
					_this,
					&deserializer)) {
				/* interrupt scan either if parsing failed or result points to valid
				 * to new v_message object */
				continueScan = OS_FALSE;
			}
			break;

		case  IN_RTPS_DATA_FRAG:    /* (0x16)  RTPS 2.1 only */
			WARN_SUBMESSAGE_NOT_YET_IMPLEMENTED(kind);
			break;

		case  IN_ACKNACK_BATCH:     /* (0x17)  RTPS 2.1 only */
			WARN_SUBMESSAGE_NOT_YET_IMPLEMENTED(kind);
			break;

		case  IN_RTPS_DATA_BATCH:   /* (0x18)  RTPS 2.1 Only */
			WARN_SUBMESSAGE_NOT_YET_IMPLEMENTED(kind);
			break;

		case  IN_HEARTBEAT_BATCH:   /* (0x19)  RTPS 2.1 only */
			WARN_SUBMESSAGE_NOT_YET_IMPLEMENTED(kind);
			break;
		default:
			WARN_SUBMESSAGE_UNKNOWN(kind);
		}
	}

	return result;
}


static void
in_ddsiStreamReaderImplReleaseBuffer(
		in_ddsiStreamReaderImpl _this)
{
	if (_this->currentReceiveBuffer) {
		in_ddsiSubmessageTokenizerDeinit(
				&(_this->currentReceiveBufferTokenizer));
		in_ddsiReceiverDeinit(&(_this->receiver));

		in_transportReceiverReleaseBuffer(
				_this->transport,
				_this->currentReceiveBuffer);

		_this->currentReceiveBuffer = NULL;
	}
}
static os_boolean
in_ddsiStreamReaderImplFetchBuffer(
		in_ddsiStreamReaderImpl _this,
		os_time *timeout,
		in_result *errorState)
{
	os_boolean result = OS_TRUE;
	os_boolean isControl = OS_FALSE;
	*errorState = IN_RESULT_OK;

	if (_this->currentReceiveBuffer==NULL) {

		_this->currentReceiveBuffer =
			in_transportReceiverReceive(_this->transport,
					&isControl,
					timeout,
					errorState);
		/* currentReceiveBuffe!=NULL || errorState!=Ok */
		/* the transportReceiverReceive operation is only returned from
		 * if a message has been received, or an error occured. In the latter
		 * case the scan-operation shall be exited. */
		if (_this->currentReceiveBuffer) {
		    if (!in_ddsiReceiverInit(&(_this->receiver),
			        _this->currentReceiveBuffer) ||
			    !in_ddsiSubmessageTokenizerInit(
					&(_this->currentReceiveBufferTokenizer),
					_this->currentReceiveBuffer)) {
                /* invalid message buffer, not valid RTPS headers */
		        in_transportReceiverReleaseBuffer(
							_this->transport,
							_this->currentReceiveBuffer);
		        _this->currentReceiveBuffer = NULL;
		        result = OS_FALSE;
		    } else {
		        result = OS_TRUE;
		    }
		} else {
		    /* no new message buffer */
		    result = OS_FALSE;
		}
	}
	return result;
}

static void
in_ddsiStreamReaderImplSetCallbackTable(
        in_ddsiStreamReaderImpl _this,
        in_streamReaderCallbackTable callbackTable,
        in_streamReaderCallbackArg   callbackArg)
{
    assert(_this->callbackTable == NULL);
    assert(_this->callbackArg == NULL);

    _this->callbackTable = callbackTable;
    _this->callbackArg = callbackArg;
}

static void
in_ddsiStreamReaderImplResetCallbackTable(
        in_ddsiStreamReaderImpl _this)
{
    _this->callbackTable = NULL;
    _this->callbackArg = NULL;
}

static in_result
in_ddsiStreamReaderImplScan(
		in_ddsiStreamReaderImpl _this,
        in_streamReaderCallbackTable callbackTable,
        in_streamReaderCallbackArg   callbackArg,
		os_time *timeout)
{
	in_result result = IN_RESULT_OK;
	os_time now = os_timeGet();
	os_time deadline = os_timeAdd(now, *timeout);
	os_time recvTimeout;
	os_boolean continueLoop = OS_TRUE;
	os_boolean deadlineNotPassed = OS_TRUE;

	/* init */

	in_ddsiStreamReaderImplSetCallbackTable(_this,
	        callbackTable,
	        callbackArg);

	/* iterate: wait for messages until timeout exceeded */
	do {
		/* each iteration the receiveTimeout decreases, "now" guarantees
		 * positive receiveTimeout */
		recvTimeout = os_timeSub(deadline, now);
		if (!in_ddsiStreamReaderImplFetchBuffer(_this, &recvTimeout, &result)) {
			/* either timeout or criticial error, such as network down */
			continueLoop = OS_FALSE;
		} else {
			result = in_ddsiStreamReaderImplScanBuffer(_this);

			/* all messages have been parsed from the buffer */
			in_ddsiStreamReaderImplReleaseBuffer(_this);

			/* update 'now' for next iteration */
			now = os_timeGet();
		}
		deadlineNotPassed = os_timeCompare(now, deadline) == OS_LESS;
	} while (continueLoop && deadlineNotPassed);


    in_ddsiStreamReaderImplResetCallbackTable(_this);

    return result;
}



static in_result
in_streamReaderScan_i(
		in_streamReader _this,
		in_streamReaderCallbackTable callbackTable,
		in_streamReaderCallbackArg   callbackArg,
		os_time *timeout)
{
	/* narrow _this and invoke scan operation */
	return in_ddsiStreamReaderImplScan(
			in_ddsiStreamReaderImpl(_this),
			callbackTable,
			callbackArg,
			timeout);
}

/* **** public functions **** */

/* static vtable */
static  OS_STRUCT(in_streamReaderPublicVTable) staticPublicVTable = {
        in_streamReaderScan_i
};

static void
in_messageTransformerGetBufferFunc_i(
    in_data* bufferPtr,
    os_uint32* length,
    c_voidp userData)
{


    /* fragments not supported currently */
    *length = 0;
    *bufferPtr = NULL;

    return;
}

/** called by deriving classes */
os_boolean
in_ddsiStreamReaderImplInit(
        in_ddsiStreamReaderImpl _this,
        in_objectKind kind,
        in_objectDeinitFunc deinit,
        in_configChannel config,
        in_transportReceiver receiver,
        in_plugKernel plug)
{
    os_boolean result = OS_TRUE;

    in_streamReaderInit(OS_SUPER(_this),
            kind,
            deinit,
            &staticPublicVTable);

    _this->transport = in_transportReceiverKeep(receiver);
    _this->plugKernel = in_plugKernelKeep(plug);

    _this->currentReceiveBuffer = NULL;
    _this->messageDeserializer =
        in_messageDeserializerNew(
                in_messageTransformerGetBufferFunc_i,
                _this);

    if (!_this->messageDeserializer) {
        /* out of memory */
        in_transportReceiverFree(_this->transport);
        in_plugKernelFree(_this->plugKernel);

        in_streamReaderDeinit(OS_SUPER(_this));
        result = OS_FALSE;
    }

    in_ddsiStreamReaderImplResetCallbackTable(_this);

    return result;
}

/** */
void
in_ddsiStreamReaderImplDeinit(
        in_ddsiStreamReaderImpl _this)
{
    in_transportReceiverFree(_this->transport);
    in_plugKernelFree(_this->plugKernel);
    in_messageDeserializerFree(_this->messageDeserializer);

    _this->transport = NULL;
    _this->plugKernel = NULL;


    in_objectDeinit(OS_SUPER(_this));
}



in_ddsiStreamReaderImpl
in_ddsiStreamReaderImplNew(
        in_configChannel config,
        in_transportReceiver receiver,
        in_plugKernel plug)
{
    in_ddsiStreamReaderImpl result =
        os_malloc(sizeof(*result));



    if (result) {
        if (!in_ddsiStreamReaderImplInit(
                result,
                IN_OBJECT_KIND_STREAM_READER_BASIC,
                (in_objectDeinitFunc) in_ddsiStreamReaderImplDeinit,
                config,
                receiver,
                plug)) {
            os_free(result);
            result = NULL;
        }
    }

    IN_TRACE_1(Construction,2,"in_ddsiStreamReaderImpl created = %x",result);

    return result;
}



#undef WARN_SUBMESSAGE_NOT_YET_IMPLEMENTED
#undef WARN_SUBMESSAGE_UNKNOWN
#undef WARN_SUBMESSAGE_IGNORED
#undef WARN_ENTITY_KIND_UNKNOWN
