/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
/*
 * in__ddsiStreamReaderImpl.h
 *
 *  Created on: Mar 5, 2009
 *      Author: frehberg
 */

#include "in_ddsiStreamReaderImpl.h"

#include "in__streamReader.h"
#include "in_connectivityAdmin.h"
#include "in_transportReceiver.h"
#include "in_abstractReceiveBuffer.h"
#include "in_ddsiSubmessageTokenizer.h"
#include "in__ddsiReceiver.h"

#ifndef IN__DDSISTREAMREADERIMPL_H_
#define IN__DDSISTREAMREADERIMPL_H_


#if defined (__cplusplus)
extern "C" {
#endif

/** */
OS_STRUCT(in_ddsiStreamReaderImpl)
{
	OS_EXTENDS(in_streamReader);
	in_connectivityAdmin connectivityAdmin;
	in_transportReceiver transport;
	/* current receive buffer being scanned */
	in_abstractReceiveBuffer currentReceiveBuffer;
	OS_STRUCT(in_ddsiSubmessageTokenizer) currentReceiveBufferTokenizer;
	OS_STRUCT(in_ddsiReceiver) receiver; /* holds the state of previously parsed submessage */

    /* operations that are re-implemented by derived classes */
    in_streamReaderCallbackTable callbackTable;
    in_streamReaderCallbackArg   callbackArg;
};


/** called by deriving classes */
os_boolean
in_ddsiStreamReaderImplInit(
        in_ddsiStreamReaderImpl _this,
        in_objectKind kind,
        in_objectDeinitFunc deinit,
        in_configChannel config,
        in_transportReceiver receiver,
        in_connectivityAdmin connectivityAdmin);


/** called by deriving classes */
void
in_ddsiStreamReaderImplDeinit(
        in_ddsiStreamReaderImpl _this);


#if defined (__cplusplus)
}
#endif


#endif /* IN__DDSISTREAMREADERIMPL_H_ */
