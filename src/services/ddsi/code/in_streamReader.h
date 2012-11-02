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
#ifndef IN_STREAMREADER_H_
#define IN_STREAMREADER_H_

#include "in__object.h"
#include "in_ddsiSubmessage.h"
#include "in_ddsiParticipant.h"
#include "in_ddsiSubscription.h"
#include "in_ddsiPublication.h"
#include "in_result.h"
#include "kernelModule.h"
#include "in_ddsiReceiver.h"

#if defined (__cplusplus)
extern "C" {
#endif

/** */
typedef void* in_streamReaderCallbackArg;

OS_CLASS(in_discoveredPeer);


OS_STRUCT(in_discoveredPeer)
{
    in_ddsiGuidPrefix writerGuidPrefix; /* source Guid */
    in_ddsiGuidPrefix readerGuidPrefix; /* destination Guid, may be UNKNOWN */
    in_ddsiEntityId writerId;

    in_ddsiEntityId readerId;
    in_ddsiSequenceNumber sequenceNumber;

    /* callback becomes owner of this object */
    in_connectivityPeerEntity discoveredPeerEntity;
};

/** declaring heartbeat event structure  */
OS_STRUCT(in_ddsiHeartbeat)
{
    OS_EXTENDS(in_ddsiSubmessageHeartbeat);
};

/** declaring heartbeat event structure  */
OS_STRUCT(in_ddsiAckNack)
{
    OS_EXTENDS(in_ddsiSubmessageAckNack);
};

/** */
typedef in_result (*in_streamReaderProcessPeerEntityFunc)(
        in_streamReaderCallbackArg _this,
        in_discoveredPeer discoveredPeer);

/*  will be invoked in case data has been received
 *
 * the writerGuid and sequenceNumber can be used to AckNack reception of
 * data items, This operation is not invoked for builtin data objects. */
typedef in_result (*in_streamReaderProcessDataFunc)(
        in_streamReaderCallbackArg _this,
        v_message message,
        in_connectivityPeerWriter peerWriter,
        in_ddsiReceiver receiver);

typedef in_result (*in_streamReaderProcessDataFragFunc)(
        in_streamReaderCallbackArg _this,
        in_ddsiDataFrag event,
        in_ddsiReceiver receiver);

typedef in_result (*in_streamReaderProcessHeartbeatFunc)(
        in_streamReaderCallbackArg _this,
        in_ddsiHeartbeat event,
        in_ddsiReceiver receiver);

typedef in_result (*in_streamReaderProcessAckNackFunc)(
        in_streamReaderCallbackArg _this,
        in_ddsiAckNack event,
        in_ddsiReceiver receiver);

typedef in_result (*in_streamReaderProcessNackFragFunc)(
        in_streamReaderCallbackArg _this,
        in_ddsiNackFrag event,
        in_ddsiReceiver receiver);

typedef in_result (*in_streamReaderRequestNackFragFunc)(
        in_streamReaderCallbackArg _this,
        in_ddsiNackFragRequest request);

/* to check incoming messages for relevance, 
 * 
 * The operation can be used for two purposes, first to detect incoming messages 
 * as loopback messages the process is the originator itself, and second 
 * to ignore messages whose destined entity is not a local one. 
 * 
 * \param guidPrefixRef refering the participant GuidPrefix 
 */
typedef os_boolean (*in_streamReaderIsLocalEntityFunc)(
		in_streamReaderCallbackArg _this,
        in_ddsiGuidPrefixRef guidPrefixRef);

/** */
OS_STRUCT(in_streamReaderCallbackTable)
{
    /** discovery related operation */
    in_streamReaderProcessPeerEntityFunc processPeerEntity;

    /* common data transfer operations */
    in_streamReaderProcessDataFunc        processData;
    in_streamReaderProcessDataFragFunc    processDataFrag;
    in_streamReaderProcessAckNackFunc     processAckNack;
    in_streamReaderProcessNackFragFunc    processNackFrag;
    in_streamReaderProcessHeartbeatFunc   processHeartbeat;
    in_streamReaderRequestNackFragFunc    requestNackFrag;
    in_streamReaderIsLocalEntityFunc      isLocalEntity;
};


#define in_streamReaderKeep(s) in_streamReader(in_objectKeep(in_object(s)))

#define in_streamReaderFree(s) in_objectFree(in_object(s))

#define in_streamReaderIsValid(c) in_objectIsValid(in_object(c))

/** */
#define in_streamReader(_o) \
	((in_streamReader)_o)

/**
 * \param timeout if exceeded the operation will return with IN_RESULT_TIMEOUT
 *
 * The timeout shall be used to return from scan-operation
 * and check for termination. Or perform periodic actions.
 */
in_result
in_streamReaderScan(in_streamReader _this,
        in_streamReaderCallbackTable callbackTable,
        in_streamReaderCallbackArg   callbackArg,
		os_time *timeout);


#if defined (__cplusplus)
}
#endif


#endif /* IN_STREAMREADER_H_ */
