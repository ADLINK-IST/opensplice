/*
 * in_ddsiReceiver.c
 *
 *  Created on: Mar 9, 2009
 *      Author: frehberg
 */

/* **** interface headers **** */
#include "in__ddsiReceiver.h"

/* **** implementation headers **** */

#include <assert.h>

#include "in_ddsiDefinitions.h"
#include "in__object.h"
#include "in_abstractReceiveBuffer.h"
#include "in__ddsiDeserializer.h"

/* **** private functions **** */

/* **** public functions **** */
os_boolean
in_ddsiReceiverInit(in_ddsiReceiver _this,
        in_abstractReceiveBuffer receiveBuffer)
{
    const c_time zeroTime = {0,0};
    const in_ddsiGuidPrefix zeroPrefix = IN_GUIDPREFIX_UNKNOWN;
    os_boolean result = OS_TRUE;
    OS_STRUCT(in_ddsiDeserializer) deserializer;
    OS_STRUCT(in_ddsiMessageHeader) messageHeader;

    in_long nofOctets = 0;
    in_locator sender = NULL;
    os_boolean isBigE = OS_TRUE; /* does not matter, the header
                                    is endianess invariant*/


    /**
     *  OS_STRUCT(in_ddsiProtocolVersion) sourceVersion;
     *  in_ddsiVendorId sourceVendorId;
     *  in_ddsiGuidPrefix sourceGuidPrefix;
     *  in_ddsiGuidPrefix destGuidPrefix;
     *  in_locatorList unicastReplyLocatorList;
     *  in_locatorList multicastReplyLocatorList;
     *  os_boolean haveTimestamp;
     *  os_time timestamp;
     *  */

    assert(sizeof(zeroPrefix) == 12);
    assert(sizeof(in_ddsiGuidPrefix) == 12); /* in_octet[12] */
    in_ddsiDeserializerInit(&deserializer, receiveBuffer, isBigE);
    /* the following operation parses and validates the header. It returns
     * with error if the protocol id or version do not match. */
    nofOctets =
        in_ddsiMessageHeaderInitFromBuffer(&messageHeader, &deserializer);
    if (nofOctets < 0 || nofOctets != IN_DDSI_MESSAGE_HEADER_SIZE) {
        /* error parsing message header:
         * maybe buffer to short, or mal formed header */
        result = OS_FALSE;
    } else {
        _this->sourceVersion = messageHeader.version;
        _this->sourceVendorId[0] = messageHeader.vendor.vendorId[0];
        _this->sourceVendorId[1] = messageHeader.vendor.vendorId[1];
        memcpy(_this->sourceGuidPrefix, messageHeader.guidPrefix, sizeof(in_ddsiGuidPrefix));
        memcpy(_this->destGuidPrefix, zeroPrefix, sizeof(in_ddsiGuidPrefix));

        in_locatorListInit(&(_this->unicastReplyLocatorList));
        in_locatorListInit(&(_this->multicastReplyLocatorList));

        /* get constant reference to sender's locator */
        sender = in_abstractReceiveBufferGetSender(receiveBuffer);
        if (sender) {
            /* deep copy */
        	if(in_locatorIsValid(sender))
        	{
        		in_locatorListPushBack(&(_this->unicastReplyLocatorList), sender);
				/* TODO: So far this reference is not a valid reply locator for OpenSplice, we must
				 * share a single socket between transportSend and transportReceiver */
        	}
        	in_locatorFree(sender);
        }
        _this->haveTimestamp = OS_FALSE;
        _this->timestamp = zeroTime;

        if (result==OS_FALSE) {
            /* out of memory, free any resources */
            in_ddsiReceiverDeinit(_this);
        }
    }

    in_ddsiDeserializerDeinit(&deserializer);

    return result;
}

void
in_ddsiReceiverDeinit(in_ddsiReceiver _this)
{
    in_locatorListDeinit(&(_this->unicastReplyLocatorList));
    in_locatorListDeinit(&(_this->multicastReplyLocatorList));
}
