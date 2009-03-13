
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
determineMasters(
    d_nameSpacesRequestListener listener)
{
    d_admin admin;
    d_durability durability;
    d_configuration config;
    c_long count, i; 
    d_nameSpace n;
    d_nameSpaces ns;
    d_networkAddress master;
    
    assert(d_listenerIsValid(d_listener(listener), D_NAMESPACES_REQ_LISTENER)); 
    
    admin      = d_listenerGetAdmin(d_listener(listener));
    durability = d_adminGetDurability(admin);
    config     = d_durabilityGetConfiguration(durability);
    count      = c_iterLength(config->nameSpaces);
    
    assert(count == c_iterLength(listener->nameSpaces));
    
    for(i=0; i<count; i++){
        n = d_nameSpace(c_iterObject(config->nameSpaces, i));
        ns = d_nameSpaces(c_iterObject(listener->nameSpaces, i));
        master = d_nameSpaceGetMaster(n);
        
        while(!master){
            master = d_nameSpaceGetMaster(n);
        }
        d_nameSpacesSetMaster(ns, master);
        d_networkAddressFree(master);
    }
    return;
}

void
d_nameSpacesRequestListenerInit(
    d_nameSpacesRequestListener listener,
    d_subscriber subscriber)
{
    d_admin admin;
    d_durability durability;
    d_configuration config;
    d_nameSpaces ns;
    d_nameSpace n;
    c_ulong count, i;
    d_networkAddress master;
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
    
    admin      = d_subscriberGetAdmin(subscriber);
    durability = d_adminGetDurability(admin);
    config     = d_durabilityGetConfiguration(durability);
    count      = c_iterLength(config->nameSpaces);
    
    listener->nameSpaces = c_iterNew(NULL);
    
    for(i=0; i<count; i++){
        n = d_nameSpace(c_iterObject(config->nameSpaces, i));
        ns = d_nameSpacesNew(admin, n, d_nameSpaceGetInitialQuality(n), count);
        master = d_nameSpaceGetMaster(n);
        d_nameSpacesSetMaster(ns, master);
        d_networkAddressFree(master);
        listener->nameSpaces = c_iterAppend(listener->nameSpaces, ns);
    }
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
    d_nameSpaces ns;
    d_nameSpacesRequestListener listener;
    
    assert(d_listenerIsValid(d_listener(object), D_NAMESPACES_REQ_LISTENER));
        
    if(object){
        listener = d_nameSpacesRequestListener(object);
        ns = d_nameSpaces(c_iterTakeFirst(listener->nameSpaces));
        
        while(ns){
            d_nameSpacesFree(ns);
            ns = d_nameSpaces(c_iterTakeFirst(listener->nameSpaces));
        }
        c_iterFree(listener->nameSpaces);
    }
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
            "Received nameSpacesRequest from fellow %d.\n", 
            message->senderAddress.systemId);
   
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
                "Fellow %d unknown, added to administration and requesting nameSpaces.\n", 
                message->senderAddress.systemId);
            request = d_nameSpacesRequestNew(admin);
            d_messageSetAddressee(d_message(request), addr);
            d_publisherNameSpacesRequestWrite(publisher, request, addr);
            d_nameSpacesRequestFree(request);
        }
    }
    d_fellowUpdateStatus(fellow, message->senderState, v_timeGet());
    determineMasters(d_nameSpacesRequestListener(listener));
    count = c_iterLength(d_nameSpacesRequestListener(listener)->nameSpaces);
    
    for(i=0; i<count; i++){
        ns = d_nameSpaces(c_iterObject(d_nameSpacesRequestListener(listener)->nameSpaces, i));
        d_messageInit(d_message(ns), admin);
        d_messageSetAddressee(d_message(ns), addr);
        d_publisherNameSpacesWrite(publisher, ns, addr);
    }
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
    
    assert(d_listenerIsValid(d_listener(listener), D_NAMESPACES_REQ_LISTENER));
    
    if(listener){
        addr = d_networkAddressUnaddressed();
        admin = d_listenerGetAdmin(d_listener(listener));
        publisher = d_adminGetPublisher(admin);
        determineMasters(listener);
        count = c_iterLength(listener->nameSpaces);
        
        for(i=0; i<count; i++){
            ns = d_nameSpaces(c_iterObject(listener->nameSpaces, i));
            d_messageInit(d_message(ns), admin);
            d_messageSetAddressee(d_message(ns), addr);
            d_publisherNameSpacesWrite(publisher, ns, addr);
        }        
        d_networkAddressFree(addr);
    }
    return;
}
