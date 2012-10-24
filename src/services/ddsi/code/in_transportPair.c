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
/*
 * in_transportPair.c
 *
 *  Created on: Feb 10, 2009
 *      Author: frehberg
 */

 /* interface */
#include "in__transportPair.h"

/* implementation */
#include "in_commonTypes.h"
#include "in__object.h"


/** \brief init */
void
in_transportPairInitParent(
		in_transportPair _this,
		in_objectKind kind,
		in_objectDeinitFunc deinit,
		in_transportPairGetReceiverFunc getReceiver,
		in_transportPairGetSenderFunc getSender)
{
	in_objectInit(OS_SUPER(_this), kind, deinit);

	_this->getReceiver = getReceiver;
	_this->getSender = getSender;
}

/** \brief deinit */
void
in_transportPairDeinit(
		in_transportPair _this)
{
	in_objectDeinit(OS_SUPER(_this));
}

/** \brief getter */
in_transportReceiver
in_transportPairGetReader(in_transportPair _this)
{
	return _this->getReceiver(_this);
}


/** \brief getter */
in_transportSender
in_transportPairGetWriter(in_transportPair _this)
{
	return _this->getSender(_this);
}

/** \brief destructor */
void
in_transportPairFree(in_transportPair _this)
{
	in_objectFree(in_object(_this));
}
