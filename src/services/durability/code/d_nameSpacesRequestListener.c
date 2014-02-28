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

#include "d__nameSpacesRequestListener.h"
#include "d_nameSpacesRequestListener.h"
#include "d_readerListener.h"
#include "d__readerListener.h"
#include "d_listener.h"
#include "d_nameSpace.h"
#include "d_nameSpaces.h"
#include "d_nameSpacesRequest.h"
#include "d_table.h"
#include "d_message.h"
#include "d_publisher.h"
#include "d_networkAddress.h"
#include "d_admin.h"
#include "d_configuration.h"
#include "d_misc.h"
#include "d_fellow.h"
#include "v_time.h"
#include "os_heap.h"

d_nameSpacesRequestListener
d_nameSpacesRequestListenerNew(
    d_subscriber subscriber)
{
    d_nameSpacesRequestListener listener;

    listener = NULL;

    if(subscriber){
        listener = d_nameSpacesRequestListener(os_malloc(C_SIZEOF(d_nameSpacesRequestListener)));
        d_listener(listener)->kind = D_NAMESPACES_REQ_LISTENER;
        d_nameSpacesRequestListenerInit(listener, subscriber);
    }
    return listener;
}

static void
cleanNameSpaces (
    c_iter nameSpaces)
{
    d_nameSpaces ns;

    /* Clean old nameSpaces list if exists */
    if (nameSpaces) {
        ns = d_nameSpaces(c_iterTakeFirst(nameSpaces));

        while(ns){
            d_nameSpacesFree(ns);
            ns = d_nameSpaces(c_iterTakeFirst(nameSpaces));
        }

        c_iterFree (nameSpaces);
    }
}

struct updateNsWalkData
{
    c_iter nameSpaces;
    d_nameSpacesRequestListener listener;
};

static void
updateNameSpacesWalk (
    d_nameSpace n,
    c_iterActionArg userData)
{
    d_nameSpacesRequestListener listener;
    d_admin admin;
    d_nameSpaces ns;
    d_networkAddress master;
    struct updateNsWalkData* walkData;

    walkData = (struct updateNsWalkData*)userData;
    listener = walkData->listener;
    admin = d_listenerGetAdmin(d_listener(listener));

    /* Create nameSpaces object from namespace, update total later */
    ns = d_nameSpacesNew(admin, n, d_nameSpaceGetInitialQuality(n), 0);
    master = d_nameSpaceGetMaster(n);
    d_nameSpacesSetMaster(ns, master);
    d_networkAddressFree(master);

    /* Add namespaces object to listener */
    walkData->nameSpaces = c_iterAppend (walkData->nameSpaces, ns);
}

static void
updateNameSpacesTotalWalk (
    void* o,
    c_iterActionArg userData)
{
    c_ulong total;
    d_nameSpaces ns;

    ns = d_nameSpaces(o);

    total = *((c_ulong*)userData);

    d_nameSpacesSetTotal(ns, total);
}

/* TODO: NOT THREAD SAFE */
static c_iter
updateNameSpaces(
    d_nameSpacesRequestListener listener)
{
    d_admin admin;
    c_ulong total;
    c_iter nameSpaces;
    struct updateNsWalkData walkData;

    admin = d_listenerGetAdmin (d_listener(listener));

    /* Update nameSpaces */
    nameSpaces = c_iterNew (NULL);
    walkData.listener = listener;
    walkData.nameSpaces = nameSpaces;

    d_adminNameSpaceWalk (admin, updateNameSpacesWalk, &walkData);
    total = c_iterLength (walkData.nameSpaces);

    /* Update totals */
    c_iterWalk (walkData.nameSpaces, updateNameSpacesTotalWalk, &total);

    return walkData.nameSpaces;
}

void
d_nameSpacesRequestListenerInit(
    d_nameSpacesRequestListener listener,
    d_subscriber subscriber)
{
    os_threadAttr attr;

    os_threadAttrInit(&attr);
    d_readerListenerInit(d_readerListener(listener),
                         d_nameSpacesRequestListenerAction, subscriber,
                         D_NAMESPACES_REQ_TOPIC_NAME,
                         D_NAMESPACES_REQ_TOP_NAME,
                         V_RELIABILITY_RELIABLE,
                         V_HISTORY_KEEPALL,
                         V_LENGTH_UNLIMITED,
                         attr,
                         d_nameSpacesRequestListenerDeinit);
}

void
d_nameSpacesRequestListenerFree(
    d_nameSpacesRequestListener listener)
{
    assert(d_listenerIsValid(d_listener(listener), D_NAMESPACES_REQ_LISTENER));

    if(listener){
        d_readerListenerFree(d_readerListener(listener));
    }
}

void
d_nameSpacesRequestListenerDeinit(
    d_object object)
{
    OS_UNUSED_ARG(object);
    assert(d_listenerIsValid(d_listener(object), D_NAMESPACES_REQ_LISTENER));

    /* Obsolete ? */
}

c_bool
d_nameSpacesRequestListenerStart(
    d_nameSpacesRequestListener listener)
{
    return d_readerListenerStart(d_readerListener(listener));
}

c_bool
d_nameSpacesRequestListenerStop(
    d_nameSpacesRequestListener listener)
{
    return d_readerListenerStop(d_readerListener(listener));
}

void
d_nameSpacesRequestListenerAction(
    d_listener listener,
    d_message message)
{
    d_durability durability;
    d_admin admin;
    d_fellow fellow;
    d_publisher publisher;
    c_ulong i, count;
    d_nameSpaces ns;
    d_networkAddress addr;
    d_nameSpacesRequest request;
    c_bool added;
    c_iter nameSpaces;

    assert(d_listenerIsValid(d_listener(listener), D_NAMESPACES_REQ_LISTENER));

    admin      = d_listenerGetAdmin(listener);
    durability = d_adminGetDurability(admin);
    addr       = d_networkAddressNew(message->senderAddress.systemId,
                                     message->senderAddress.localId,
                                     message->senderAddress.lifecycleId);
    fellow     = d_adminGetFellow(admin, addr);
    publisher  = d_adminGetPublisher(admin);

    d_printTimedEvent(durability, D_LEVEL_FINE,
            D_THREAD_NAMESPACES_REQUEST_LISTENER,
            "Received nameSpacesRequest from fellow %u.\n",
            message->senderAddress.systemId);

    /* Update nameSpaces list for listener */
    nameSpaces = updateNameSpaces(d_nameSpacesRequestListener(listener));

    if(!fellow){
        fellow = d_fellowNew(addr, message->senderState);
        d_fellowUpdateStatus(fellow, message->senderState, v_timeGet());
        added = d_adminAddFellow(admin, fellow);

        if(added == FALSE){
            d_fellowFree(fellow);
            fellow = d_adminGetFellow(admin, addr);
            assert(fellow);
        } else {
            fellow = d_adminGetFellow(admin, addr);
            d_printTimedEvent(durability, D_LEVEL_FINE,
                D_THREAD_NAMESPACES_REQUEST_LISTENER,
                "Fellow %u unknown, added to administration and requesting nameSpaces.\n",
                message->senderAddress.systemId);
            request = d_nameSpacesRequestNew(admin);
            d_messageSetAddressee(d_message(request), addr);
            d_publisherNameSpacesRequestWrite(publisher, request, addr);
            d_nameSpacesRequestFree(request);
        }
    }
    d_fellowUpdateStatus(fellow, message->senderState, v_timeGet());

    count = c_iterLength(nameSpaces);

    for(i=0; i<count; i++){
        ns = d_nameSpaces(c_iterObject(nameSpaces, i));
        d_messageInit(d_message(ns), admin);
        d_messageSetAddressee(d_message(ns), addr);
        d_publisherNameSpacesWrite(publisher, ns, addr);
    }
    cleanNameSpaces (nameSpaces);
    d_fellowFree(fellow);
    d_networkAddressFree(addr);

    return;
}

void
d_nameSpacesRequestListenerReportNameSpaces(
    d_nameSpacesRequestListener listener)
{
    c_long count, i;
    d_networkAddress addr;
    d_nameSpaces ns;
    d_admin admin;
    d_publisher publisher;
    c_iter nameSpaces;

    assert(d_listenerIsValid(d_listener(listener), D_NAMESPACES_REQ_LISTENER));

    if(listener){
        addr = d_networkAddressUnaddressed();
        admin = d_listenerGetAdmin(d_listener(listener));

        assert (admin);

        publisher = d_adminGetPublisher(admin);

        /* Get list of namespaces */
        nameSpaces = updateNameSpaces(listener);

        count = c_iterLength(nameSpaces);
        for(i=0; i<count; i++){
            ns = d_nameSpaces(c_iterObject(nameSpaces, i));
            d_messageInit(d_message(ns), admin);
            d_messageSetAddressee(d_message(ns), addr);

            d_publisherNameSpacesWrite(publisher, ns, addr);
        }
        d_networkAddressFree(addr);

        /* Free namespace list */
        cleanNameSpaces (nameSpaces);
    }
    return;
}
