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
#include "v_public.h"
#include "c_base.h"
#include "v_entity.h"
#include "v_handle.h"
#include "v_participant.h"
#include "v_publisher.h"
#include "v_subscriber.h"
#include "v_writer.h"
#include "v__dataReader.h"
#include "v__networkReader.h"
#include "v__reader.h"
#include "v_partition.h"
#include "v__group.h"
#include "v__groupQueue.h"
#include "v__topic.h"
#include "v_query.h"
#include "v_status.h"
#include "v_serviceManager.h"
#include "v_service.h"
#include "v_serviceState.h"
#include "v__spliced.h"
#include "v__waitset.h"
#include "v_writerInstance.h"
#include "os_report.h"
#include "v__dataReaderInstance.h"
#include "v__dataView.h"
#include "v__deliveryService.h"
#include "v_dataReaderQuery.h"
#include "v_dataViewQuery.h"
#include "v_dataViewInstance.h"

c_bool
v_publicInit(
    v_public o)
{
    v_kernel kernel;

    assert(C_TYPECHECK(o,v_public));

    kernel = v_objectKernel(o);
    o->handle = v_handleServerRegister(kernel->handleServer,o);
    o->userDataPublic = NULL;
    return TRUE;
}

void
v_publicDeinit(
    v_public o)
{
    OS_UNUSED_ARG(o);
    assert(C_TYPECHECK(o,v_public));
}

void
v_publicDispose(
    v_public o)
{
    assert(C_TYPECHECK(o,v_public));

    if (o == NULL) {
        return;
    }
    switch(v_objectKind(o)) {
    case K_PARTICIPANT:    v_participantDeinit(v_participant(o));       break;
    case K_PUBLISHER:      v_publisherDeinit(v_publisher(o));           break;
    case K_SUBSCRIBER:     v_subscriberDeinit(v_subscriber(o));         break;
    case K_WRITER:         v_writerDeinit(v_writer(o));                 break;
    case K_DATAREADER:     v_dataReaderDeinit(v_dataReader(o));         break;
    case K_DELIVERYSERVICE:v_deliveryServiceDeinit(v_deliveryService(o)); break;
    case K_NETWORKREADER:  v_networkReaderDeinit(v_networkReader(o));   break;
    case K_READER:         v_readerDeinit(v_reader(o));                 break;
    case K_GROUPQUEUE:     v_groupQueueDeinit(v_groupQueue(o));         break;
    case K_TOPIC:          v_topicDeinit(v_topic(o));                   break;
    case K_ENTITY:                                                      break;
    case K_DOMAIN:         v_partitionDeinit(v_partition(o));           break;
    case K_GROUP:          v_groupDeinit(v_group(o));                   break;
    case K_SERVICEMANAGER: /* Is never freed! */                        break;
    case K_SPLICED:        v_splicedDeinit(v_spliced(o));               break;
    case K_NETWORKING:
    case K_DURABILITY:
    case K_CMSOAP:
    case K_RNR:
    case K_SERVICE:        v_serviceDeinit(v_service(o));               break;
    case K_SERVICESTATE:   /* Is never freed! */                        break;
    case K_CONFIGURATION:                                               break;
    case K_QUERY:
        OS_REPORT(OS_ERROR, "v_publicDispose failure",
                  0, "deinit of abstract class K_QUERY");
    break;
    case K_DATAREADERQUERY:
        v_dataReaderQueryDeinit(v_dataReaderQuery(o));
    break;
    case K_DATAVIEWQUERY:
        v_dataViewQueryDeinit(v_dataViewQuery(o));
    break;
    case K_DATAVIEW:       v_dataViewDeinit(v_dataView(o));             break;
    case K_WAITSET:        v_waitsetDeinit(v_waitset(o));               break;
    case K_WRITERINSTANCE:
        v_writerInstanceDeinit(v_writerInstance(o));
    break;
    case K_DATAREADERINSTANCE:
        v_dataReaderInstanceDeinit(v_dataReaderInstance(o));
    break;
    case K_DATAVIEWINSTANCE:
        v_dataViewInstanceDeinit(v_dataViewInstance(o));
    break;
    default:
        OS_REPORT_1(OS_ERROR,"v_publicDispose failed",0,
                    "illegal entity kind (%d) specified",v_objectKind(o));
        assert(FALSE);
    break;
    }
    c_free(o);
}

void
v_publicFree (
    v_public o)
{
    assert(C_TYPECHECK(o,v_public));

    v_handleDeregister(o->handle);
}

static v_handle
gidToHandle(
    v_gid id,
    v_kernel kernel)
{
    v_handle handle;

    handle.server = (c_address)kernel->handleServer;
    handle.index  = id.localId;
    handle.serial = id.serial;

    return handle;
}

v_handle
v_publicHandle (
    v_public o)
{
    assert(C_TYPECHECK(o,v_public));
    assert(o != NULL);

    return o->handle;
}

v_gid
v_publicGid (
    v_public o)
{
    v_gid id;

    assert(C_TYPECHECK(o,v_public));

    if (o == NULL) {
        id.systemId    = V_PUBLIC_ILLEGAL_GID;
        id.localId     = V_PUBLIC_ILLEGAL_GID;
        id.serial      = V_PUBLIC_ILLEGAL_GID;
    } else {
        id.systemId = v_objectKernel(o)->GID.systemId;
        id.localId  = o->handle.index;
        id.serial   = o->handle.serial;
    }
    return id;
}

v_public
v_gidClaim (
    v_gid id,
    v_kernel kernel)
{
    v_handle handle;
    v_object o = NULL;

    if (v_gidIsFromKernel(id, kernel)) {
        handle = gidToHandle(id,kernel);
        v_handleClaim(handle, &o); /* Ignore result */
        assert(C_TYPECHECK(o,v_public));
    }
    return v_public(o);
}

v_handleResult
v_gidClaimChecked(
    v_gid id,
    v_kernel kernel,
    v_public *public)
{
    v_handle handle;
    v_handleResult result;

    if (v_gidIsValid(id)) {
        handle = gidToHandle(id,kernel);
        result = v_handleClaim(handle, (v_object *)public);
        assert(C_TYPECHECK((*public),v_public));
    } else {
        result = V_HANDLE_ILLEGAL;
        *public = NULL;
    }
    return result;
}


void
v_gidRelease (
    v_gid id,
    v_kernel kernel)
{
    v_handle handle;

    if (v_gidIsValid(id)) {
        handle = gidToHandle(id,kernel);
        v_handleRelease(handle);
    }
}


v_handleResult
v_gidReleaseChecked(
    v_gid id,
    v_kernel kernel)
{
    v_handle handle;
    v_handleResult result;

    if (v_gidIsValid(id)) {
        handle = gidToHandle(id,kernel);
        result = v_handleRelease(handle);
    } else {
        result = V_HANDLE_ILLEGAL;
    }
    return result;
}

c_equality
v_gidCompare(
    v_gid id1,
    v_gid id2)
{
    if (id1.systemId > id2.systemId) {
        return C_GT;
    }
    if (id1.systemId < id2.systemId) {
        return C_LT;
    }
    if (id1.localId > id2.localId) {
        return C_GT;
    }
    if (id1.localId < id2.localId) {
        return C_LT;
    }
    if (id1.serial > id2.serial) {
        return C_GT;
    }
    if (id1.serial < id2.serial) {
        return C_LT;
    }
    return C_EQ;
}


c_voidp
v_publicSetUserData (
    v_public o,
    c_voidp userDataPublic)
{
    c_voidp old;

    assert(C_TYPECHECK(o,v_public));

    old = o->userDataPublic;
    o->userDataPublic = userDataPublic;

    return old;
}


c_voidp
v_publicGetUserData (
    v_public o)
{
    assert(C_TYPECHECK(o,v_public));

    return o->userDataPublic;
}


