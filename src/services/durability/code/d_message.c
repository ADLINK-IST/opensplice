/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "d_message.h"
#include "d__durability.h"
#include "d__admin.h"
#include "d__configuration.h"
#include "d_networkAddress.h"

void
d_messageInit(
    d_message message,
    d_admin admin)
{
    d_networkAddress addr, unAddr;
    d_configuration config;

    if (message) {
        config = d_durabilityGetConfiguration(d_adminGetDurability(admin));
        addr = d_adminGetMyAddress(admin);
        unAddr = d_networkAddressUnaddressed();

        message->senderState               = d_durabilityGetState(d_adminGetDurability(admin));
        message->senderAddress.systemId    = addr->systemId;
        message->senderAddress.localId     = addr->localId;
        message->senderAddress.lifecycleId = addr->lifecycleId;
        message->addressee.systemId        = unAddr->systemId;
        message->addressee.localId         = unAddr->localId;
        message->addressee.lifecycleId     = unAddr->lifecycleId;
        message->productionTimestamp.seconds = 0l;
        message->productionTimestamp.nanoseconds = 0ul;
        /* Initialize bits 30 and 31 of the productionTimestamp according to the capabilities */
        if (config->capabilitySupport) {
            message->productionTimestamp.nanoseconds = message->productionTimestamp.nanoseconds | (1ul << 31);
        }
        if (config->capabilityY2038Ready) {
            message->productionTimestamp.nanoseconds = message->productionTimestamp.nanoseconds | (1ul << 30);
        }
        d_networkAddressFree(addr);
        d_networkAddressFree(unAddr);
    }
}

void
d_messageSetAddressee(
    d_message message,
    d_networkAddress addressee)
{
    if(message){
        message->addressee.systemId    = addressee->systemId;
        message->addressee.localId     = addressee->localId;
        message->addressee.lifecycleId = addressee->lifecycleId;
    }
}

d_networkAddress
d_messageGetAddressee(
    d_message message)
{
    d_networkAddress result;

    result = NULL;

    if (message) {
        result = &(message->addressee);
    }

    return result;
}

void
d_messageSetSenderAddress(
    d_message message,
    d_networkAddress address)
{
    if(message){
        message->senderAddress.systemId     = address->systemId;
        message->senderAddress.localId      = address->localId;
        message->senderAddress.lifecycleId  = address->lifecycleId;
    }
}


void
d_messageDeinit(
    d_message message)
{
    OS_UNUSED_ARG(message);
    assert(message);
    return;
}

/**
 * \brief Indicates whether the sender of the message indicates capability support
 *
 * Capability support is enabled if the most-significant byte of the nanoseconds
 * field of the productionTimestamp is set.
*/
c_bool
d_messageHasCapabilitySupport(
    d_message message)
{
    c_bool result = FALSE;

    if (message) {
        result = (((message->productionTimestamp.nanoseconds) & (1U << 31)) != 0);
    }
    return result;
}
