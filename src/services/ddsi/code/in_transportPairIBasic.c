/*
 * in_transportPairIBasic.c
 *
 *  Created on: Feb 10, 2009
 *      Author: frehberg
 */

/* interfaces */
#include "in_transportPairIBasic.h"
#include "in_transport.h"
#include "in_report.h"

/* implemenation */
#include "in_transportReceiverIBasic.h"
#include "in_transportSenderIBasic.h"
#include "os_heap.h"
#include "in_socket.h"

OS_STRUCT(in_transportPairIBasic)
{
	OS_EXTENDS(in_transport);
};

static void
in_transportPairIBasicDeinit(in_object _this)
{
	/* narrow interface */

	in_transportDeinit(_this);
}

static os_boolean
in_transportPairIBasicInit(
		in_transportPairIBasic _this,
		in_configChannel configChannel)
{
	os_boolean result = OS_TRUE;
	in_transportReceiverIBasic receiver = NULL;
	in_transportSenderIBasic sender = NULL;
	in_socket duplexSock =
	    in_socketDuplexNew(
	            configChannel,
	            OS_FALSE);

	assert(_this);
	assert(configChannel);
	if (!duplexSock) {
	    result = OS_FALSE;
	} else {
        receiver =
            in_transportReceiverIBasicNewDuplex(
                    configChannel,
                    duplexSock);
        sender =
            in_transportSenderIBasicNewDuplex(
                    configChannel,
                    duplexSock);
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
            in_transportInit(
                    OS_SUPER(_this),
                    IN_OBJECT_KIND_TRANSPORT_PAIR_BASIC,
                    in_transportPairIBasicDeinit,
                    in_transportReceiver(receiver),
                    in_transportSender(sender));

            in_transportReceiverIBasicFree(receiver);
            in_transportSenderIBasicFree(sender);
        }
        /*  decrement refcounter */
        in_socketFree(duplexSock);
	}
	return result;
}

in_transportPairIBasic
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
    IN_TRACE_1(Construction,2,"in_transportPairIBasic created = %x",result);

	return result;
}

/** \brief decrease refcount */
void
in_transportPairIBasicFree(in_transportPairIBasic _this)
{
	in_objectFree(in_object(_this));
}
