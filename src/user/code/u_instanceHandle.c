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
#include "u__types.h"
#include "u__domain.h"
#include "u__user.h"
#include "v_dataView.h"
#include "v_public.h"
#include "v_dataReader.h"
#include "v_reader.h"
#include "v_query.h"
#include "v_topic.h"
#include "os_report.h"

#define HANDLE_SERVER_MASK (0x7f000000)
#define HANDLE_SERIAL_MASK (0x00ffffff)
#define HANDLE_GLOBAL_MASK (0x80000000)

typedef union {
    struct {
        c_long lifecycleId;
        c_long localId;
    } lid;
    u_instanceHandle handle;
} u_instanceHandleTranslator;

static int u__handleResult (v_handleResult result)
{
    return ((result == V_HANDLE_OK) ? U_RESULT_OK :
            (result == V_HANDLE_EXPIRED) ? U_RESULT_ALREADY_DELETED :
            U_RESULT_ILL_PARAM);
}

u_instanceHandle
u_instanceHandleNew(
    v_public object)
{
    v_handle handle;
    u_instanceHandleTranslator translator;
    c_long id;

    if (object) {
        handle = v_publicHandle(object);
        if (handle.serial != (handle.serial & HANDLE_SERIAL_MASK)) {
            handle.serial = (handle.serial & HANDLE_SERIAL_MASK);
            OS_REPORT(OS_ERROR,"u_instanceHandleNew",0,
                      "handle.serial exceeds HANDLE_SERIAL_MASK");
        }
        id = u_userServerId(v_public(object));
        if (id != (id & HANDLE_SERVER_MASK)) {
            id = (id & HANDLE_SERVER_MASK);
            OS_REPORT(OS_ERROR,"u_instanceHandleNew",0,
                      "ServerId exceeds HANDLE_SERVER_MASK");
        }
        translator.lid.lifecycleId = (handle.serial | id);
        translator.lid.localId = handle.index;
    } else {
        translator.handle = 0;
    }
    return translator.handle;
}

u_instanceHandle
u_instanceHandleFromGID (
    v_gid gid)
{
    u_instanceHandleTranslator translator;

    if( (gid.systemId == V_PUBLIC_ILLEGAL_GID) &&
        (gid.localId  == V_PUBLIC_ILLEGAL_GID) &&
        (gid.serial   == V_PUBLIC_ILLEGAL_GID))
    {
        translator.handle = 0;
    } else {
        translator.lid.lifecycleId = (gid.systemId | HANDLE_GLOBAL_MASK);
        translator.lid.localId = gid.localId;
    }
    return translator.handle;
}

v_gid
u_instanceHandleToGID (
    u_instanceHandle handle)
{
    u_instanceHandleTranslator translator;
    v_gid gid;

    translator.handle = handle;

    gid.systemId = translator.lid.lifecycleId & ~HANDLE_GLOBAL_MASK;
    gid.localId = translator.lid.localId;
    gid.serial = 0;

    return gid;
}

u_result
u_instanceHandleClaim (
    u_instanceHandle _this,
    c_voidp instance)
{
    u_result result;
    v_handleResult vresult;
    v_handle handle;
    u_instanceHandleTranslator translator;

    if (instance == NULL) {
        result = U_RESULT_ILL_PARAM;
    } else if (_this == 0) {
        result = U_RESULT_ILL_PARAM;
    } else {
        translator.handle = _this;

        handle.serial = (translator.lid.lifecycleId & HANDLE_SERIAL_MASK);
        handle.index  = translator.lid.localId;
        handle.server = u_userServer(translator.lid.lifecycleId & HANDLE_SERVER_MASK);

        vresult = v_handleClaim(handle, instance);
        result = u__handleResult(vresult);
    }

    return result;
}

u_result
u_instanceHandleRelease (
    u_instanceHandle _this)
{
    u_result result;
    v_handle handle;
    u_instanceHandleTranslator translator;

    if (_this == 0) {
        result = U_RESULT_ILL_PARAM;
    } else {
        translator.handle = _this;

        handle.serial = (translator.lid.lifecycleId & HANDLE_SERIAL_MASK);
        handle.index  = translator.lid.localId;
        handle.server = u_userServer(translator.lid.lifecycleId & HANDLE_SERVER_MASK);

        result = u__handleResult(v_handleRelease(handle));
    }

    return result;
}

c_bool
u_instanceHandleIsNil (
    u_instanceHandle _this)
{
    return (_this == 0);
}

c_bool
u_instanceHandleIsEqual (
    u_instanceHandle h1,
    u_instanceHandle h2)
{
    return (h1 == h2);
}

/* Depricated : only for GID publication_handle legacy. */

static void
copyBuiltinTopicKey(
    c_type type,
    void *data,
    void *to)
{
    struct v_publicationInfo *from = (struct v_publicationInfo *)data;
    struct v_publicationInfo *copyIn = (struct v_publicationInfo *)to;

    copyIn->key = from->key;
}

u_instanceHandle
u_instanceHandleFix(
    u_instanceHandle _this,
    v_collection reader)
{
    u_instanceHandleTranslator translator;
    struct v_publicationInfo *data;
    v_topic topic;
    v_message message;
    v_public instance;

    translator.handle = _this;
    if (translator.lid.lifecycleId & HANDLE_GLOBAL_MASK) {
        /* Is a GID therefore fix handle by lookup. */
        while (v_objectKind(v_entity(reader)) == K_QUERY ||
               v_objectKind(v_entity(reader)) == K_DATAREADERQUERY ||
               v_objectKind(v_entity(reader)) == K_DATAVIEWQUERY) {
            /* If the entity derives from a query entity it can be cast to a v_query */
            reader = v_querySource(v_query(reader));
        }
        while (v_objectKind(v_entity(reader)) == K_DATAVIEW) {
            reader = v_collection(v_dataViewGetReader(v_dataView(reader)));
        }
        topic = v_dataReaderGetTopic(v_dataReader(reader));
        message = v_topicMessageNew(topic);
        data = (c_voidp)C_DISPLACE(message, v_topicDataOffset(topic));
        data->key = u_instanceHandleToGID(_this);
        instance = (v_public)v_dataReaderLookupInstance(v_dataReader(reader),
                                                        message);
        translator.handle = u_instanceHandleNew(instance);
        c_free(instance);
        c_free(topic);
        c_free(message);
    }
    return translator.handle;
}
