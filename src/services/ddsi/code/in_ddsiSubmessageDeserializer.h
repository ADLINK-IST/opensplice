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
#ifndef IN_DDSISUBMESSAGEDESERIALIZER_H_
#define IN_DDSISUBMESSAGEDESERIALIZER_H_

#include "in__ddsiDeserializer.h"
#include "in_ddsiSubmessageToken.h"
#include "in_ddsiElements.h"

#if defined (__cplusplus)
extern "C" {
#endif

OS_STRUCT(in_ddsiSubmessageDeserializer)
{
	OS_EXTENDS(in_ddsiDeserializer);
	/* this is a side-product of the next-token operation*/
	OS_STRUCT(in_ddsiSubmessageHeader) submessageHeader;
};

/** \brief init
 */
os_boolean
in_ddsiSubmessageDeserializerInit(
		in_ddsiSubmessageDeserializer _this,
		in_ddsiSubmessageToken   token,
		os_size_t  maxNofOctets);


/** \brief deinit
 */
void
in_ddsiSubmessageDeserializerDeinit(
		in_ddsiSubmessageDeserializer _this);


#if defined (__cplusplus)
}
#endif


#endif /* IN_DDSISUBMESSAGEDESERIALIZER_H_ */
