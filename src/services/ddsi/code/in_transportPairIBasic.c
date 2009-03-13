/*
 * in_transportPairIBasic.c
 *
 *  Created on: Feb 10, 2009
 *      Author: frehberg
 */

/* interfaces */
#include "in_transportPairIBasic.h"
#include "in__transportPair.h"

/* implemenation */
#include "in_transportReceiverIBasic.h"
#include "in_transportSenderIBasic.h"
#include "os_heap.h"

OS_STRUCT(in_transportPairIBasic)
{
	OS_EXTENDS(in_transportPair);
	in_transportReceiverIBasic receiver;
	in_transportSenderIBasic   sender;
};

static in_transportReceiver
in_transportPairGetReceiverImpl(
		in_transportPair _this)
{
	in_transportReceiver result;

	/* narrow interface */
	in_transportPairIBasic obj =
		in_transportPairIBasic(_this);

	/* generalize */
	result = in_transportReceiver(obj->receiver);

	return result;
}

static in_transportSender
in_transportPairGetSenderImpl(
		in_transportPair _this)
{
	in_transportSender result;

	/* narrow interface */
	in_transportPairIBasic obj =
		in_transportPairIBasic(_this);

	/* generalize */
	result = in_transportSender(obj->sender);

	return result;
}

static void
in_transportPairIBasicDeinit(in_object _this)
{
	/* narrow interface */
	in_transportPairIBasic obj =
		in_transportPairIBasic(_this);

	in_transportReceiverIBasicFree(obj->receiver);
	in_transportSenderIBasicFree(obj->sender);
	obj->receiver = NULL;
	obj->sender = NULL;

	in_transportPairDeinit(OS_SUPER(obj));
}

static os_boolean
in_transportPairIBasicInit(
		in_transportPairIBasic _this,
		in_configChannel configChannel)
{
	os_boolean result = OS_TRUE;
	in_transportReceiverIBasic receiver = NULL;
	in_transportSenderIBasic sender = NULL;

	assert(_this);
	assert(configChannel);

	receiver = in_transportReceiverIBasicNew(configChannel);
	sender = in_transportSenderIBasicNew(configChannel);
	if (!sender || !receiver) {
		if (sender) {
			in_transportSenderIBasicFree(sender);
		}
		if (receiver) {
			in_transportReceiverIBasicFree(receiver);
		}
	} else {
		/* now as sender/receiver have been
		 * instanciated successfully, initialize the parent */
		in_transportPairInitParent(
				OS_SUPER(_this),
				IN_OBJECT_KIND_TRANSPORT_PAIR_BASIC,
				in_transportPairIBasicDeinit,
				in_transportPairGetReceiverImpl,
				in_transportPairGetSenderImpl);
		_this->receiver = receiver;
		_this->sender   = sender;
	}

	return result;
}

OS_STRUCT(in_transportPairIBasic)*
in_transportPairIBasicNew(
		const in_configChannel configChannel)
{
	in_transportPairIBasic result =
		in_transportPairIBasic(os_malloc(OS_SIZEOF(in_transportPairIBasic)));

	if (result) {
		if (!in_transportPairIBasicInit(result, configChannel)) {
			os_free(result);
			result = NULL;
		}
	}

	return result;
}

/** \brief decrease refcount */
void
in_transportPairIBasicFree(in_transportPairIBasic _this)
{
	in_objectFree(in_object(_this));
}
