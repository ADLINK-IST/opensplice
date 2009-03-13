/*
 * in__streamWriter.h
 *
 *  Created on: Mar 3, 2009
 *      Author: frehberg
 */

#ifndef IN__STREAMWRITER_H_
#define IN__STREAMWRITER_H_

#include "in_streamWriter.h"
#include "in__object.h"
#include "Coll_List.h"
#include "in_result.h"
#include "in_locator.h"
#include "in_ddsiElements.h"


#if defined (__cplusplus)
extern "C" {
#endif

/** must be invoked if destination, the locatorList will change,
 * in this sense the object is stateless */
typedef	void (*in_streamWriterFlushFunc)(
			in_streamWriter _this,
			Coll_List *locatorList);


/** must be invoked if destination, the locatorList will change,
 * in this sense the object is stateless */
typedef void (*in_streamWriterFlushSingleFunc)(
            in_streamWriter _this,
            in_locator locator);

/** ingore "Final" flag for now
 *
 * If sending the first AckNack to this writerGuid, the streamWriter
 * must be flushed first to deliver previous data items to multiple
 * recipients. */
typedef in_result (*in_streamWriterAppendAckNackFunc)(
        in_streamWriter _this,
        in_ddsiGuid readerGuid,
        in_ddsiGuid writerGuid,
        in_ddsiSequenceNumber writerSN,
        in_locator singleDestination);

/** */
typedef	in_result (*in_streamWriterAppendDataFunc)(
			in_streamWriter _this,
			v_message message,
			in_connectivityWriterFacade facade,
			os_boolean recipientExpectsInlineQos,
			Coll_List *locatorList);

/** */
typedef	in_result (*in_streamWriterAppendParticipantDataFunc)(
			in_streamWriter _this,
			in_connectivityParticipantFacade facade,
			Coll_List *locatorList);

/** */
typedef	in_result (*in_streamWriterAppendReaderDataFunc)(
			in_streamWriter _this,
			in_connectivityReaderFacade facade,
			Coll_List *locatorList);

/** */
typedef in_result (*in_streamWriterAppendWriterDataFunc)(
			in_streamWriter _this,
			in_connectivityWriterFacade facade,
			Coll_List *locatorList);

/** */
typedef OS_STRUCT(in_locator)* (*in_streamWriterGetDataUnicastLocatorFunc)(
            in_streamWriter _this);

/** */
typedef OS_STRUCT(in_locator)* (*in_streamWriterGetDataMulticastLocatorFunc)(
            in_streamWriter _this);

/** */
typedef OS_STRUCT(in_locator)* (*in_streamWriterGetCtrlUnicastLocatorFunc)(
            in_streamWriter _this);


OS_CLASS(in_streamWriterPublicVTable);
OS_STRUCT(in_streamWriterPublicVTable)
{
    in_streamWriterFlushFunc flush;
    in_streamWriterFlushSingleFunc flushSingle;
    in_streamWriterAppendDataFunc appendData;
    in_streamWriterAppendParticipantDataFunc appendParticipantData;
    in_streamWriterAppendReaderDataFunc appendReaderData;
    in_streamWriterAppendWriterDataFunc appendWriterData;

    /* meta data */
    in_streamWriterAppendAckNackFunc    appendAckNack;

    /*
    in_streamWriterAppendHeartbeatFunc  appendHeartbeat;
    in_streamWriterAppendNackFragFunc   appendNackFrag;
    in_streamWriterAppendGapFunc        appendGap;
    in_streamWriterAppendPadFunc        appendPad;*/

    /* return corresponding locators */
    in_streamWriterGetDataUnicastLocatorFunc   getDataUnicastLocator;
    in_streamWriterGetDataMulticastLocatorFunc getDataMulticastLocator;
    in_streamWriterGetCtrlUnicastLocatorFunc   getCtrlUnicastLocator;
};


OS_STRUCT(in_streamWriter)
{
	OS_EXTENDS(in_object);
	/* referencing static memory area */
	in_streamWriterPublicVTable publicVTable;
};

/** */
os_boolean
in_streamWriterInit(
		in_streamWriter _this,
		in_objectKind kind,
		in_objectDeinitFunc deinit,
		in_streamWriterPublicVTable staticPublicVTable);

/** */
void
in_streamWriterDeinit(
		in_streamWriter _this);

#if defined (__cplusplus)
}
#endif


#endif /* IN__STREAMWRITER_H_ */
