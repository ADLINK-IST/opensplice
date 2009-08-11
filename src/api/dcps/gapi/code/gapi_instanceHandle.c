/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#include "gapi_instanceHandle.h"
#include "gapi_kernel.h"
#include "u_instanceHandle.h"
#include "u_dataView.h"
#include "u_dataReader.h"
#include "u_query.h"
#include "u_entity.h"
#include "v_public.h"
//#include "v_builtin.h"
#include "v_dataReader.h"
#include "v_topic.h"
#include "v_kernel.h"

typedef union {
    v_gid gid;
    struct {
        c_long lifecycleId;
        c_long localId;
    } lid;
    gapi_instanceHandle_t handle;
} gapi_id;

#define GAPI_INSTANCEHANDLE_ENTITYID_MASK (0x80000000)

gapi_instanceHandle_t
gapi_instanceHandleFromGID (
    v_gid gid)
{
    gapi_id id;
    gapi_instanceHandle_t handle;

    if( (gid.systemId == V_PUBLIC_ILLEGAL_GID) &&
        (gid.localId  == V_PUBLIC_ILLEGAL_GID) &&
        (gid.serial   == V_PUBLIC_ILLEGAL_GID))
    {
        handle = GAPI_HANDLE_NIL;
    } else {
        id.gid = gid;
        assert((id.gid.systemId & GAPI_INSTANCEHANDLE_ENTITYID_MASK) == 0);
        id.gid.systemId |= GAPI_INSTANCEHANDLE_ENTITYID_MASK;
        handle = id.handle;
    }
    return handle;
}

v_gid
gapi_instanceHandleToGID (
    gapi_instanceHandle_t handle)
{
    gapi_id id;

    id.handle = handle;
    id.gid.systemId &= ~GAPI_INSTANCEHANDLE_ENTITYID_MASK;

    return id.gid;
}

gapi_instanceHandle_t
gapi_instanceHandleFromHandle (
    u_instanceHandle handle)
{
    gapi_id id;

    id.lid.lifecycleId = handle.serial;
    id.lid.localId = handle.index;
    assert((id.gid.systemId & GAPI_INSTANCEHANDLE_ENTITYID_MASK) == 0);
    return id.handle;
}

gapi_returnCode_t
gapi_instanceHandleToHandle(
    gapi_instanceHandle_t _this,
    u_entity entity,
    u_instanceHandle *handle)
{
    gapi_id id;

    assert(handle != NULL);

    id.handle = _this;
    if ((id.gid.systemId & GAPI_INSTANCEHANDLE_ENTITYID_MASK) != 0) {
        return GAPI_RETCODE_BAD_PARAMETER;
    }

    if ((id.lid.localId == 0) && (id.lid.lifecycleId == 0)) {
        handle->server = 0;
    } else {
        /* ES 7/16/2009. Be aware that the user layer entity being used here
         * can also be a user layer entity without the userdata referring back
         * to the gapi object. This is a use case within DLRL which is maintained
         * for performance reasons.
         * Currently the fact that the user layer entity does not have a
         * reference back to the gapi object is not an issue, so this will work
         * fine. This comment is placed here merely to be made aware of this use
         * case as this scenario might not be directly obvious
         */
        handle->server = u_entitySystemId(entity);
    }
    handle->index = id.lid.localId;
    handle->serial = id.lid.lifecycleId;
    return GAPI_RETCODE_OK;
}

c_bool
gapi_instanceHandleIsGid (
    gapi_instanceHandle_t handle)
{
    gapi_id id;

    id.handle = handle;
    id.gid.systemId &= GAPI_INSTANCEHANDLE_ENTITYID_MASK;
    return (c_bool)(id.gid.systemId == GAPI_INSTANCEHANDLE_ENTITYID_MASK);
}

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

gapi_returnCode_t
gapi_instanceHandle_to_u_instanceHandle (
    gapi_instanceHandle_t p_handle,
    u_reader            reader,
    u_instanceHandle   *i_handle)
{
    struct v_publicationInfo pubInfo;
    gapi_returnCode_t result;
    u_result          uResult;

    if (p_handle != GAPI_HANDLE_NIL) {
        if (gapi_instanceHandleIsGid(p_handle)) {

            while (u_entityKind(u_entity(reader)) == U_QUERY) {
                reader = u_querySource(u_query(reader));
            }
            if (u_entityKind(u_entity(reader)) == U_DATAVIEW) {
                reader = u_dataViewSource(u_dataView(reader));
            }
            pubInfo.key = gapi_instanceHandleToGID(p_handle);
            uResult = u_dataReaderLookupInstance(u_dataReader(reader),
                                                 &pubInfo,
                                                 copyBuiltinTopicKey,
                                                 i_handle);

            if(u_instanceHandleIsNil(*i_handle)){
                result = GAPI_RETCODE_BAD_PARAMETER;
            } else {
                result = kernelResultToApiResult(uResult);
            }
        } else {
            result = gapi_instanceHandleToHandle(p_handle,
                                                 u_entity(reader),
                                                 i_handle);
        }
    } else {
        result = gapi_instanceHandleToHandle(p_handle,
                                             u_entity(reader),
                                             i_handle);
    }
    return result;
}

