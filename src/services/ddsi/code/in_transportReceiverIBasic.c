/*
 * in_transportReceiverIBasic.c
 *
 *  Created on: Feb 10, 2009
 *      Author: frehberg
 */

/* interfaces */
#include "in_transportReceiverIBasic.h"
#include "in__transportReceiver.h"
#include "in_transportReceiver.h"

/* implementation */
#include "os_heap.h"
#include "in_commonTypes.h"
#include "in_socket.h"
#include "in__object.h"
#include "in__receiveBuffer.h"
#include "in__config.h"
#include "in__configDdsiService.h"
#include "in__configPartitioning.h"
#include "in__configChannel.h"
#include "in_report.h"

/** \brief abstract class  */
OS_STRUCT(in_transportReceiverIBasic)
{
	OS_EXTENDS(in_transportReceiver);
	in_socket socket;
	os_uint32 fragmentLength;
	in_receiveBuffer nextReceiveBuffer;
};

/* **** declare private operations **** */

static in_abstractReceiveBuffer
in_transportReceiverReceive_i(
		in_transportReceiver _this,
		os_boolean *isControl,
		os_time *timeout,
        in_result *errorState);

static void
in_transportReceiverPeriodicAction_i(
		in_transportReceiver reader,
		os_time *timeout);

static void
in_transportReceiverReleaseBuffer_i(
		in_transportReceiver _this,
		in_abstractReceiveBuffer iBuffer);

static OS_STRUCT(in_locator)*
in_transportReceiverGetUnicastDataLocator_i(
		in_transportReceiver _this);

static OS_STRUCT(in_locator)*
in_transportReceiverGetMulticastDataLocator_i(
		in_transportReceiver _this);

static OS_STRUCT(in_locator)*
in_transportReceiverGetUnicastControlLocator_i(
		in_transportReceiver _this);

static OS_STRUCT(in_locator)*
in_transportReceiverGetMulticastControlLocator_i(
        in_transportReceiver _this);

/* **** implement private operations **** */

static in_abstractReceiveBuffer
in_transportReceiverReceive_i(
		in_transportReceiver _this,
		os_boolean *isControl,
		os_time  *timeout,
		in_result *errorState)
{
	in_long nofOctets = 0;
	in_locator sender;
	in_transportReceiverIBasic obj =
		in_transportReceiverIBasic(_this);

	/* TODO fetch from pool */
	in_receiveBuffer result = NULL;

	assert(_this!=NULL);
	assert(obj->socket!=NULL);
	assert(obj!=NULL);

	result = obj->nextReceiveBuffer;
	if (!result) {
		result =
			in_receiveBufferNew(obj->fragmentLength);
	}

	sender = in_abstractReceiveBufferGetSender(in_abstractReceiveBuffer(result));
	nofOctets =
		in_socketReceive(obj->socket,
			sender, /* in/out */
			in_abstractReceiveBufferBegin(in_abstractReceiveBuffer(result)),
			in_abstractReceiveBufferLength(in_abstractReceiveBuffer(result)),
			isControl, /* out */
			timeout);
	in_locatorFree(sender);

	if (nofOctets <= 0) {
		/* nothing read, or error. Store the data-buffer for next read
		 * and return NULL to callee. */
		obj->nextReceiveBuffer = result;
		result = NULL;
	} else {
		in_receiveBufferSetLength(result, nofOctets);
		/* ownership to callee */
		obj->nextReceiveBuffer = NULL;
	}

	return (in_abstractReceiveBuffer) result;
}

static void
in_transportReceiverPeriodicAction_i(
		in_transportReceiver _this,
		os_time *timeout)
{
	/* nop */
}

static void
in_transportReceiverReleaseBuffer_i(
		in_transportReceiver _this,
		in_abstractReceiveBuffer buffer)
{
	in_transportReceiverIBasic obj =
		in_transportReceiverIBasic(_this);

	assert(_this);
	assert(buffer);

	/* reset length and check the refcounter of embedded locator */
	in_receiveBufferReset(in_receiveBuffer(buffer));

	/* TODO return to buffer pool */
	if (obj->nextReceiveBuffer==NULL) {
		obj->nextReceiveBuffer = (in_receiveBuffer) buffer;
	} else {
		in_receiveBufferFree(in_receiveBuffer(buffer));
	}
}

static in_locator
in_transportReceiverGetUnicastDataLocator_i(
		in_transportReceiver _this)
{
	in_transportReceiverIBasic obj =
		in_transportReceiverIBasic(_this);

	in_locator result = NULL;

	assert(_this);
	result = in_socketGetUnicastDataLocator(obj->socket);
	return result;
}


static in_locator
in_transportReceiverGetMulticastDataLocator_i(
		in_transportReceiver _this)
{
	in_transportReceiverIBasic obj =
		in_transportReceiverIBasic(_this);

	in_locator result = NULL;

	assert(_this);
	result = in_socketGetMulticastDataLocator(obj->socket);
	return result;
}


static in_locator
in_transportReceiverGetUnicastControlLocator_i(
		in_transportReceiver _this)
{
	in_transportReceiverIBasic obj =
		in_transportReceiverIBasic(_this);

	in_locator result = NULL;

	assert(_this);
	result = in_socketGetUnicastControlLocator(obj->socket);
	if (!result) {
		/* control locator not supported */
		result = in_socketGetUnicastDataLocator(obj->socket);
		assert(result!=NULL);
	}
	return result;
}


static in_locator
in_transportReceiverGetMulticastControlLocator_i(
        in_transportReceiver _this)
{
    in_transportReceiverIBasic obj =
        in_transportReceiverIBasic(_this);

    in_locator result = NULL;

    assert(_this);
    result = in_socketGetMulticastControlLocator(obj->socket);
    if (!result) {
        /* control locator not supported */
        result = in_socketGetMulticastDataLocator(obj->socket);
        assert(result!=NULL);
    }
    return result;
}

/* **** implement init/deinit operations **** */

static void
in_objectDeinit_i(in_object _this)
{
	in_transportReceiverIBasic impl = in_transportReceiverIBasic(_this);

	if(impl->nextReceiveBuffer)
	{
		in_receiveBufferFree(in_receiveBuffer(impl->nextReceiveBuffer));
	}
	if(impl->socket)
	{
		in_socketFree(impl->socket);
	}

    /* narrow */
	in_transportReceiverDeinit(
	        OS_SUPER(in_transportReceiverIBasic(_this)));
}

static
OS_STRUCT(in_transportReceiverPublicVTable) staticPublicVTable = {
        in_transportReceiverReceive_i,
        in_transportReceiverPeriodicAction_i,
        in_transportReceiverReleaseBuffer_i,
        in_transportReceiverGetUnicastDataLocator_i,
        in_transportReceiverGetMulticastDataLocator_i,
        in_transportReceiverGetUnicastControlLocator_i,
        in_transportReceiverGetMulticastControlLocator_i
};

/* \return may return OS_FALSE on error */
static os_boolean
in_transportReceiverIBasicInit(
		in_transportReceiverIBasic _this,
		in_configChannel configChannel,
		in_socket sock)
{
	os_boolean result = OS_TRUE;
	os_uint32 fragmentLength = 0;

	fragmentLength =
		in_configChannelGetFragmentSize(configChannel);

	assert(fragmentLength <= 0x00ffffff); /* anything else is not plausible */

	in_transportReceiverInitParent(OS_SUPER(_this),
			IN_OBJECT_KIND_TRANSPORT_RECEIVER_BASIC,
			in_objectDeinit_i,
			&staticPublicVTable);

	_this->socket = in_socketKeep(sock);

	_this->fragmentLength = fragmentLength;
	_this->nextReceiveBuffer = NULL;

	if (!(_this->socket)) {
		/* error occured, faulty channel configuration */
		result = OS_FALSE;
		in_transportReceiverDeinit(OS_SUPER(_this));
	}

	return result;
}

/** \brief constructor */
in_transportReceiverIBasic
in_transportReceiverIBasicNew(in_configChannel config)
{
    os_boolean supportsControl =
        in_configChannelSupportsControl(config);

	in_transportReceiverIBasic result =
		(in_transportReceiverIBasic) os_malloc(OS_SIZEOF(in_transportReceiverIBasic));
    in_socket sock =
            in_socketReceiveNew(
                 config,
                 supportsControl);

	if (!result || !sock) {
	    if (result) {
	        os_free(result);
	    }
	    if (sock) {
	        in_socketFree(sock);
	    }
	} else {
		if (!in_transportReceiverIBasicInit(result, config, sock)) {
			os_free(result);
			result = NULL;
		}
		/* decrement refcount */
		in_socketFree(sock);
	}

    IN_TRACE_1(Construction,2,"in_transportReceiverIBasic created = %x",result);

	return result;
}

/** \brief constructor */
in_transportReceiverIBasic
in_transportReceiverIBasicNewDuplex(
        in_configChannel config,
        in_socket duplexSocket)
{
    in_transportReceiverIBasic result =
        (in_transportReceiverIBasic) os_malloc(OS_SIZEOF(in_transportReceiverIBasic));

    assert(duplexSocket);

    if (result) {

        if (!in_transportReceiverIBasicInit(
                result,
                config,
                duplexSocket)) {
            os_free(result);
            result = NULL;
        }
    }

    IN_TRACE_1(Construction,2,"in_transportReceiverIBasic created = %x",result);

    return result;
}


/** \brief destructor */
void
in_transportReceiverIBasicFree(
		in_transportReceiverIBasic _this)
{
	in_objectFree(OS_SUPER(OS_SUPER(_this)));
}

