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
#ifndef IN_STREAMWRITER_H_
#define IN_STREAMWRITER_H_

#include "in_commonTypes.h"
#include "in_connectivityWriterFacade.h"
#include "in_connectivityReaderFacade.h"
#include "in_connectivityParticipantFacade.h"
#include "Coll_List.h"
#include "in_result.h"
#include "in_endpointDiscoveryData.h"

#if defined (__cplusplus)
extern "C" {
#endif



/** */
#define in_streamWriter(_obj) \
	((in_streamWriter)_obj)

/**
 * Macro to make the in_objectKeep operation type specific
 */
#define in_streamWriterKeep(_this) in_streamWriter(in_objectKeep(in_object(_this)))


#define in_streamWriterIsValid(c) in_objectIsValid(in_object(c))


/** */
void
in_streamWriterFree(
		in_streamWriter _this);

/** must be invoked if destination (locatorList changes,
 * in this sense the object is stateless */
void
in_streamWriterFlush(
		in_streamWriter _this,
		Coll_List* locatorList);

/** must be invoked if destination (locator) changes,
 * in this sense the object is stateless */
void
in_streamWriterFlushSingle(
        in_streamWriter _this,
        in_locator locator);

/** */
in_result
in_streamWriterAppendData(
        in_streamWriter _this,
        v_message message,
        in_endpointDiscoveryData discoveryData,
        in_connectivityWriterFacade facade,
        os_boolean recipientExpectsInlineQos,
        Coll_List *locatorList);


/** */
in_result
in_streamWriterAppendHeartbeat(
        in_streamWriter _this,
        in_ddsiGuidPrefixRef sourceGuidPrefix,
        in_ddsiGuidPrefixRef destGuidPrefix,
        in_ddsiEntityId readerId,
        in_ddsiEntityId writerId,
        in_ddsiSequenceNumber firstSN,
        in_ddsiSequenceNumber lastSN,
        os_ushort count,
        in_locator singleDestination);

/** */
in_result
in_streamWriterAppendParticipantData(
		in_streamWriter _this,
        in_endpointDiscoveryData discoveryData,
		in_connectivityParticipantFacade facade,
		Coll_List *locatorList);

in_result
in_streamWriterAppendParticipantMessage(
    in_streamWriter _this,
    in_connectivityParticipantFacade facade,
    in_ddsiGuidPrefixRef destGuidPrefix,
    in_locator locator);

/** */
in_result
in_streamWriterAppendReaderData(
		in_streamWriter _this,
        in_ddsiGuidPrefixRef writerGuidPrefix,
        in_endpointDiscoveryData discoveryData,
		in_connectivityReaderFacade facade,
		Coll_List *locatorList);

/** */
in_result
in_streamWriterAppendWriterData(
		in_streamWriter _this,
        in_endpointDiscoveryData discoveryData,
		in_connectivityWriterFacade facade,
		Coll_List *locatorList);

/**
 * If sending the first AckNack to this writerGuid, the streamWriter
 * must be flushed first to deliver previous data items to multiple
 * recipients. */
in_result
in_streamWriterAppendAckNack(
        in_streamWriter _this,
        in_ddsiGuid readerGuid,
        in_ddsiGuid writerGuid,
        in_ddsiSequenceNumberSet readerSNState,
        in_locator singleDestination);

/** \return corresponding default multicast locator for data (also SDP traffic) */
in_locator
in_streamWriterGetDataMulticastLocator(in_streamWriter _this);

/** \return corresponding default uniicast locator for data (also SDP traffic) */
in_locator
in_streamWriterGetDataUnicastLocator(in_streamWriter _this);

/** \return corresponding default unicast locator for control messages */
in_locator
in_streamWriterGetCtrlUnicastLocator(in_streamWriter _this);


#if defined (__cplusplus)
}
#endif


#endif /* IN_STREAMWRITER_H_ */
