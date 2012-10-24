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
#ifndef IN_DDSISUBMESSAGETOKENIZER_H_
#define IN_DDSISUBMESSAGETOKENIZER_H_

#include "in_commonTypes.h"
#include "in_ddsiElements.h"
#include "in_ddsiSubmessageToken.h"
#include "in_abstractReceiveBuffer.h"
#include "in_ddsiSubmessageDeserializer.h"

#if defined (__cplusplus)
extern "C" {
#endif


/**
 */
OS_STRUCT(in_ddsiSubmessageTokenizer)
{
	in_ddsiSubmessageToken nextToken;
	in_ulong  remainingLength;
};

/** skips the ddsi-message header and points to first submessage
 */
os_boolean
in_ddsiSubmessageTokenizerInit(in_ddsiSubmessageTokenizer _this,
		in_abstractReceiveBuffer receiveBuffer);

/** skips the ddsi-message header and points to first submessage
 */
void
in_ddsiSubmessageTokenizerDeinit(in_ddsiSubmessageTokenizer _this);


/** Get next token
 *
 * Is Parsing the next submessage header, and verifying the length.
 *  \return NULL on parse error
 */
in_ddsiSubmessageToken
in_ddsiSubmessageTokenizerGetNext(in_ddsiSubmessageTokenizer _this,
		in_ddsiSubmessageDeserializer deserializer /* in/out parameter */ );


#if defined (__cplusplus)
}
#endif


#endif /* IN_DDSISUBMESSAGETOKENIZER_H_ */
