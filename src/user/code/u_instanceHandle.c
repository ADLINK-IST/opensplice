/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#include "u__instanceHandle.h"
#include "u__handle.h"
#include "u__types.h"
#include "u__domain.h"
#include "u__user.h"
#include "v_dataView.h"
#include "v_handle.h"
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
        c_ulong lifecycleId;
        c_ulong localId;
    } lid;
    u_instanceHandle handle;
} u_instanceHandleTranslator;

u_instanceHandle
u_instanceHandleNew(
    v_public object)
{
    v_handle handle;
    u_instanceHandleTranslator translator;
    c_ulong id;

    if (object) {
        handle = v_publicHandle(object);
        if (handle.serial != (handle.serial & HANDLE_SERIAL_MASK)) {
            handle.serial = (handle.serial & HANDLE_SERIAL_MASK);
            OS_REPORT(OS_CRITICAL,"u_instanceHandleNew", U_RESULT_ILL_PARAM,
                      "handle.serial exceeds HANDLE_SERIAL_MASK");
        }
        id = u_userServerId(v_public(object));
        if (id != (id & HANDLE_SERVER_MASK)) {
            id = (id & HANDLE_SERVER_MASK);
            OS_REPORT(OS_CRITICAL,"u_instanceHandleNew", U_RESULT_ILL_PARAM,
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
    void *instance)
{
    u_result result;
    v_handleResult vresult;
    v_handle handle;
    u_instanceHandleTranslator translator;

    if (instance == NULL) {
        result = U_RESULT_ILL_PARAM;
        OS_REPORT(OS_ERROR, "u_instanceHandleClaim", result,
                  "Bad parameter: instance = NULL");
    } else if (_this == 0) {
        result = U_RESULT_ILL_PARAM;
        OS_REPORT(OS_ERROR, "u_instanceHandleClaim", result,
                  "Bad Parameter: instance handle = DDS_HANDLE_NIL");
    } else {
        translator.handle = _this;

        handle.serial = (translator.lid.lifecycleId & HANDLE_SERIAL_MASK);
        handle.index  = translator.lid.localId;
        handle.server = u_userServer(translator.lid.lifecycleId & HANDLE_SERVER_MASK);

        vresult = v_handleClaim(handle, instance);
        result = u__handleResult(vresult);
        if ((result != U_RESULT_OK) && (result != U_RESULT_HANDLE_EXPIRED)) {
            OS_REPORT(OS_WARNING, "u_instanceHandleClaim", U_RESULT_ILL_PARAM,
                      "Bad parameter: Instance handle is invalid");
        }
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
        if (result != U_RESULT_OK) {
            OS_REPORT(OS_WARNING, "u_instanceHandleRelease", result,
                      "Bad parameter: Instance handle is invalid");
        }
    }

    return result;
}

u_bool
u_instanceHandleIsNil (
    u_instanceHandle _this)
{
    return (_this == 0);
}

u_bool
u_instanceHandleIsEqual (
    u_instanceHandle h1,
    u_instanceHandle h2)
{
    return (h1 == h2);
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
            c_free(reader);
        }
        while (v_objectKind(v_entity(reader)) == K_DATAVIEW) {
            reader = v_collection(v_dataViewGetReader(v_dataView(reader)));
        }
        topic = v_dataReaderGetTopic(v_dataReader(reader));
        message = v_topicMessageNew(topic);
        data = (void *) (message + 1);
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
