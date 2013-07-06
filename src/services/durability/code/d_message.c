/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

#include "d_message.h"
#include "d_durability.h"
#include "d_admin.h"
#include "d_networkAddress.h"

void
d_messageInit(
    d_message message,
    d_admin admin)
{
    d_networkAddress addr, unAddr;
        
    if(message){
        addr = d_adminGetMyAddress(admin);
        unAddr = d_networkAddressUnaddressed();
        
        message->senderState               = d_durabilityGetState(d_adminGetDurability(admin));
        message->senderAddress.systemId    = addr->systemId;
        message->senderAddress.localId     = addr->localId;
        message->senderAddress.lifecycleId = addr->lifecycleId;
        message->addressee.systemId        = unAddr->systemId;
        message->addressee.localId         = unAddr->localId;
        message->addressee.lifecycleId     = unAddr->lifecycleId;
        
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
