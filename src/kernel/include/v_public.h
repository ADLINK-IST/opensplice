/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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
#ifndef V_PUBLIC_H
#define V_PUBLIC_H

#include "v_kernel.h"
#include "v_handle.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define V_PUBLIC_ILLEGAL_GID (0)

#define V_SYSTEMID_MASK    (0x7fffff00) /* MSB is reserved! */
#define V_LIFECYCLEID_MASK (0x000000ff)
#define V_SYSTEMID_SHIFT   (8)
#define V_ENTITYID_MASK    (0x80000000)

#define  v_public(o)       (C_CAST(o,v_public))

OS_API c_bool   v_publicInit      (v_public o);
OS_API void     v_publicDeinit    (v_public o);
OS_API v_handle v_publicHandle    (v_public o);
OS_API v_gid    v_publicGid       (v_public o);
OS_API void     v_publicFree      (v_public o);
OS_API void     v_publicDispose   (v_public o);
OS_API c_voidp  v_publicSetUserData (v_public o, c_voidp userData);
OS_API c_voidp  v_publicGetUserData (v_public o);

#define v_gidEqual(id1,id2) \
        (((id1).systemId==(id2).systemId)&& \
         ((id1).localId==(id2).localId)&& \
         ((id1).serial==(id2).serial))

#define v_gidIsValid(id) \
        (((id).systemId != V_PUBLIC_ILLEGAL_GID) &&     \
         ((id).localId != V_PUBLIC_ILLEGAL_GID) && \
         ((id).serial != V_PUBLIC_ILLEGAL_GID))

#define v_gidIsNil(id) \
        (((id).systemId == V_PUBLIC_ILLEGAL_GID) &&     \
         ((id).localId == V_PUBLIC_ILLEGAL_GID) && \
         ((id).serial == V_PUBLIC_ILLEGAL_GID))

#define v_gidSetNil(id)                         \
  (((id).systemId = V_PUBLIC_ILLEGAL_GID),      \
   ((id).localId = V_PUBLIC_ILLEGAL_GID),       \
   ((id).serial = V_PUBLIC_ILLEGAL_GID),        \
   ((void) 0))

#define v_gidIsFromKernel(id,kernel) \
    (((id).systemId) == ((kernel)->GID.systemId))

#define v_gidLocalId(id) ((id).localId)

#define v_gidSystemId(id) ((id).systemId)

#define v_gidLifecycleId(id) ((id).serial)

OS_API v_public       v_gidClaim        (v_gid id, v_kernel k);
OS_API void           v_gidRelease      (v_gid id, v_kernel k);
OS_API c_equality     v_gidCompare      (v_gid id1, v_gid id2);

OS_API v_handleResult v_gidClaimChecked  (v_gid id, v_kernel k, v_public *p);
OS_API v_handleResult v_gidReleaseChecked(v_gid id, v_kernel k);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
