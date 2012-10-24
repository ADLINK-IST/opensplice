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
#ifndef IN__TRANSPORTPAIR_H_
#define IN__TRANSPORTPAIR_H_


#include "in__object.h"
#include "in_commonTypes.h"
#include "in_transportPair.h"
#include "in_transportReceiver.h"
#include "in_transportSender.h"
/**
 * Allow usage of this C code from C++ code. Always include this in a header
 * file.
 */
#if defined (__cplusplus)
extern "C" {
#endif

/** \brief virtual operations */
typedef OS_STRUCT(in_transportReceiver)* (*in_transportPairGetReceiverFunc)(in_transportPair pair);
typedef OS_STRUCT(in_transportSender)* (*in_transportPairGetSenderFunc)(in_transportPair pair);

/** \brief abstract class  */
OS_STRUCT(in_transportPair)
{
	OS_EXTENDS(in_object);
	in_transportPairGetReceiverFunc getReceiver;
	in_transportPairGetSenderFunc getSender;
};

/** \brief init */
void
in_transportPairInitParent(
		in_transportPair _this,
		in_objectKind kind,
		in_objectDeinitFunc deinit,
		in_transportPairGetReceiverFunc getReceiver,
		in_transportPairGetSenderFunc getSender);

/** \brief deinit */
void
in_transportPairDeinit(
		in_transportPair _this);


/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif

#endif /* IN__TRANSPORTPAIR_H_ */
