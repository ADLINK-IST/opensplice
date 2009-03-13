#ifndef GAPI_INSTANCEHANDLE_H
#define GAPI_INSTANCEHANDLE_H

#include "gapi.h"
#include "u_entity.h"
#include "os_if.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_DCPSGAPI
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define gapi_instanceHandle_t(h) (GAPI_HANDLE_NIL)

gapi_instanceHandle_t
gapi_instanceHandleFromGID (
    v_gid gid);

v_gid
gapi_instanceHandleToGID (
    gapi_instanceHandle_t handle);

OS_API gapi_instanceHandle_t
gapi_instanceHandleFromHandle (
    u_instanceHandle handle);

OS_API gapi_returnCode_t
gapi_instanceHandleToHandle(
    gapi_instanceHandle_t _this,
    u_entity entity,
    u_instanceHandle *handle);

c_bool
gapi_instanceHandleIsGid (
    gapi_instanceHandle_t handle);

gapi_returnCode_t
gapi_instanceHandle_to_u_instanceHandle (
    gapi_instanceHandle_t p_handle,
    u_reader            reader,
    u_instanceHandle   *i_handle);

#undef OS_API


#if defined (__cplusplus)
}
#endif

#endif /* GAPI_INSTANCEHANDLE_H */
