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

#include "d__nameSpacesRequestListener.h"
#include "d__readerListener.h"
#include "d__admin.h"
#include "d__fellow.h"
#include "d__listener.h"
#include "d__nameSpace.h"
#include "d__publisher.h"
#include "d__admin.h"
#include "d__configuration.h"
#include "d__fellow.h"
#include "d__table.h"
#include "d__misc.h"
#include "d__durability.h"
#include "d__thread.h"
#include "d_nameSpaces.h"
#include "d_nameSpacesRequest.h"
#include "d_message.h"
#include "d_networkAddress.h"
#include "v_time.h"
#include "os_heap.h"

/**
 * Macro that checks the d_nameSpacesRequestListener validity.
 * Because d_nameSpacesRequestListener is a concrete class typechecking is required.
 */
#define             d_nameSpacesRequestListenerIsValid(_this)   \
    d_listenerIsValidKind(d_listener(_this), D_NAMESPACES_REQ_LISTENER)

/**
 * \brief The <code>d_nameSpacesRequestListener</code> cast macro.
 *
 * This macro casts an object to a <code>d_nameSpacesRequestListener</code> object.
 */
#define d_nameSpacesRequestListener(_this) ((d_nameSpacesRequestListener)(_this))

C_STRUCT(d_nameSpacesRequestListener){
    C_EXTENDS(d_readerListener);
};

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

static void
d_nameSpacesRequestListenerAction(
    d_listener listener,
    d_message message)
{
    d_durability durability;
    d_admin admin;
    d_configuration config;
    d_fellow fellow;
    d_publisher publisher;
    c_ulong i, count;
    d_nameSpaces ns;
    d_networkAddress fellowAddr;
    c_iter nameSpaces;
    c_bool sendNow = FALSE;

    assert(d_nameSpacesRequestListenerIsValid(listener));

    admin      = d_listenerGetAdmin(listener);
    durability = d_adminGetDurability(admin);
    config     = d_durabilityGetConfiguration(durability);
    fellowAddr = d_networkAddressNew(message->senderAddress.systemId,
                                     message->senderAddress.localId,
                                     message->senderAddress.lifecycleId);
    publisher  = d_adminGetPublisher(admin);

    if (d_networkAddressIsUnaddressed(fellowAddr)) {
        /* A nameSpacesRequest from (0,0,0) is received. This is
         * interpreted as a request to send all my nameSpaces to
         * everybody. */
        d_printTimedEvent(durability, D_LEVEL_FINE,
                "Received a request to broadcast my nameSpaces.\n");
        sendNow = TRUE;
    } else {
        d_printTimedEvent(durability, D_LEVEL_FINE,
                "Received nameSpacesRequest from fellow %u.\n",
                fellowAddr->systemId);
        /* This might be the first message received from the fellow
         * Look up the fellow and create it if it did not exist */
        fellow = d_durabilityGetOrCreateFellowFromMessage(admin, fellowAddr, message);
        /* At this point the fellow must be created unless it has recently
         * terminated or is terminating/terminated. If the fellow is responsive we
         * can answer the request and send the namespaces back.
         * Otherwise we need to cache the request and sent the namespaces when the fellow
         * becomes responsive. There is no need to lock the fellow because this is the only
         * thread that can set has_requested_namespaces to TRUE */
        if (fellow) {
            sendNow = d_fellowIsResponsive(fellow, config->waitForRemoteReaders);
            if ((!sendNow) && (!fellow->has_requested_namespaces)) {
                fellow->has_requested_namespaces = TRUE;
                d_printTimedEvent(durability, D_LEVEL_FINEST,
                        "Unable to send my namespace(s) to fellow %u because I have not yet received his capabilities, caching the request\n",
                        fellowAddr->systemId);
            }
            d_fellowFree(fellow);
        }
    }
    if (sendNow) {
        /* Update nameSpaces list for listener */
        nameSpaces = updateNameSpaces(d_nameSpacesRequestListener(listener));
        count = c_iterLength(nameSpaces);
        d_printTimedEvent(durability, D_LEVEL_FINE,
                "Sending %d namespace(s) to fellow %u.\n",
                count, fellowAddr->systemId);
        for(i=0; i<count; i++){
            ns = d_nameSpaces(c_iterObject(nameSpaces, i));
            d_messageInit(d_message(ns), admin);
            d_messageSetAddressee(d_message(ns), fellowAddr);
            d_publisherNameSpacesWrite(publisher, ns, fellowAddr);
        }
        cleanNameSpaces (nameSpaces);
    }
    d_networkAddressFree(fellowAddr);
    return;
}

static void
d_nameSpacesRequestListenerDeinit(
    d_nameSpacesRequestListener listener)
{
    assert(d_nameSpacesRequestListenerIsValid(listener));

    /* Stop the listener */
    d_nameSpacesRequestListenerStop(listener);
    /* Nothing to deallocate, call super-deinit */
    d_readerListenerDeinit(d_readerListener(listener));
}

static void
d_nameSpacesRequestListenerInit(
    d_nameSpacesRequestListener listener,
    d_subscriber subscriber)
{
    os_threadAttr attr;

    /* Do not assert the listener because the initialization
     * of the listener has not yet completed */

    assert(d_subscriberIsValid(subscriber));
    assert(listener);

    /* Call super-init */
    os_threadAttrInit(&attr);
    d_readerListenerInit(d_readerListener(listener),
                         D_NAMESPACES_REQ_LISTENER,
                         d_nameSpacesRequestListenerAction,
                         subscriber,
                         D_NAMESPACES_REQ_TOPIC_NAME,
                         D_NAMESPACES_REQ_TOP_NAME,
                         V_RELIABILITY_RELIABLE,
                         V_HISTORY_KEEPALL,
                         V_LENGTH_UNLIMITED,
                         attr,
                         (d_objectDeinitFunc)d_nameSpacesRequestListenerDeinit);
}

d_nameSpacesRequestListener
d_nameSpacesRequestListenerNew(
    d_subscriber subscriber)
{
    d_nameSpacesRequestListener listener;

    assert(d_subscriberIsValid(subscriber));

    /* Allocate nameSpacesRequestListener object */
    listener = d_nameSpacesRequestListener(os_malloc(C_SIZEOF(d_nameSpacesRequestListener)));
    if (listener) {
        /* Initialize the nameSpacesRequestListener */
        d_nameSpacesRequestListenerInit(listener, subscriber);
    }
    return listener;
}

void
d_nameSpacesRequestListenerFree(
    d_nameSpacesRequestListener listener)
{
    assert(d_nameSpacesRequestListenerIsValid(listener));

    d_objectFree(d_object(listener));
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
d_nameSpacesRequestListenerReportNameSpaces(
    d_nameSpacesRequestListener listener)
{
    d_admin admin;
    d_durability durability;
    d_publisher publisher;
    d_networkAddress myAddr, addr;
    d_nameSpacesRequest request;

    assert(d_nameSpacesRequestListenerIsValid(listener));

    if(listener){
        admin = d_listenerGetAdmin(d_listener(listener));
        assert (admin);
        publisher = d_adminGetPublisher(admin);
        durability = d_adminGetDurability(admin);
        myAddr = d_adminGetMyAddress(admin);
        addr = d_networkAddressUnaddressed();

        /* Create a request to broadcast namesSpaces */
        d_printTimedEvent(durability, D_LEVEL_FINER, "Trigger a request to broadcast nameSpaces.\n");
        request = d_nameSpacesRequestNew(admin);
        d_messageSetAddressee(d_message(request), myAddr);    /* Send the nameSpacesRequest to myself. */
        d_messageSetSenderAddress(d_message(request), addr);  /* Pretend that the request to send the nameSpaces came from (0,0,0). */
        d_publisherNameSpacesRequestWrite(publisher, request, addr, d_durabilityGetState(durability));
        d_nameSpacesRequestFree(request);
        d_networkAddressFree(addr);
        d_networkAddressFree(myAddr);
    }
    return;
}
