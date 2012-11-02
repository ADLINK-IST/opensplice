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

#include "d__statusRequestListener.h"
#include "d_statusRequestListener.h"
#include "d_readerListener.h"
#include "d__readerListener.h"
#include "d_listener.h"
#include "d_status.h"
#include "d_publisher.h"
#include "d_networkAddress.h"
#include "d_admin.h"
#include "d_message.h"
#include "d_misc.h"
#include "os_heap.h"

d_statusRequestListener
d_statusRequestListenerNew(
    d_subscriber subscriber)
{
    d_statusRequestListener listener;

    listener = NULL;

    if(subscriber){
        listener = d_statusRequestListener(os_malloc(C_SIZEOF(d_statusRequestListener)));
        d_listener(listener)->kind = D_STATUS_REQ_LISTENER;
        d_statusRequestListenerInit(listener, subscriber);
    }
    return listener;
}

void
d_statusRequestListenerInit(
    d_statusRequestListener listener,
    d_subscriber subscriber)
{
    os_threadAttr attr;

    os_threadAttrInit(&attr);

    d_readerListenerInit(   d_readerListener(listener),
                            d_statusRequestListenerAction, subscriber,
                            D_STATUS_REQ_TOPIC_NAME, D_STATUS_REQ_TOP_NAME,
                            V_RELIABILITY_RELIABLE,
                            V_HISTORY_KEEPALL,
                            V_LENGTH_UNLIMITED, attr,
                            d_statusRequestListenerDeinit);

}

void
d_statusRequestListenerFree(
    d_statusRequestListener listener)
{
    assert(d_listenerIsValid(d_listener(listener), D_STATUS_REQ_LISTENER));

    if(listener){
        d_readerListenerFree(d_readerListener(listener));
    }
}

void
d_statusRequestListenerDeinit(
    d_object object)
{
    assert(d_listenerIsValid(d_listener(object), D_STATUS_REQ_LISTENER));

    return;
}

c_bool
d_statusRequestListenerStart(
    d_statusRequestListener listener)
{
    return d_readerListenerStart(d_readerListener(listener));
}

c_bool
d_statusRequestListenerStop(
    d_statusRequestListener listener)
{
    return d_readerListenerStop(d_readerListener(listener));
}

void
d_statusRequestListenerAction(
    d_listener listener,
    d_message message)
{
    d_admin admin;
    d_publisher publisher;
    d_status status;
    d_networkAddress addr;

    assert(d_listenerIsValid(d_listener(listener), D_STATUS_REQ_LISTENER));

    admin = d_listenerGetAdmin(listener);
    publisher = d_adminGetPublisher(admin);
    status = d_statusNew(admin);
    addr = d_networkAddressNew (message->senderAddress.systemId,
                                message->senderAddress.localId,
                                message->senderAddress.lifecycleId);

    d_messageSetAddressee(d_message(status), addr);
    d_publisherStatusWrite(publisher, status, addr);
    d_statusFree(status);
    d_networkAddressFree(addr);
    return;
}
