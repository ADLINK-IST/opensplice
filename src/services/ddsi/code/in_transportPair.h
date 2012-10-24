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
#ifndef IN_TRANSPORTPAIR_H_
#define IN_TRANSPORTPAIR_H_


#include "in__object.h"
#include "in_commonTypes.h"
#include "in_transportReceiver.h"
#include "in_transportSender.h"

/**
 * Allow usage of this C code from C++ code. Always include this in a header
 * file.
 */
#if defined (__cplusplus)
extern "C" {
#endif


#define in_transportPair(_derivedObj) \
	((in_transportPair)(_derivedObj))

/** \brief destructor */
void
in_transportPairFree(in_transportPair _this);

/** \brief getter */
in_transportReceiver
in_transportPairGetReceivcer(in_transportPair _this);


/** \brief getter */
in_transportSender
in_transportPairGetSender(in_transportPair _this);

/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif


#endif /* IN_TRANSPORTPAIR_H_ */
