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
#ifndef U_OBJECT_H
#define U_OBJECT_H

/** \file u_object.h
 *
 * The User Layer Object class implements the base class of all User Layer
 * classes that provide an interface to a kernel public class.
 * This class manages the access to kernel objects, i.e. it will protect
 * calling threads against process termination, inform the kernel to delay
 * any deletion of the kernel object during access and inform callers if the
 * kernel object is already deleted.
 *
 * The following methods are supported:
 *
 *    void          u_objectFree          (u_object o);
 *    u_participant u_objectParticipant   (u_object o);
 */

#include "u_types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/** \brief The u_object cast method.
 */
#define u_object(_this) ((u_object)(_this))

/** \brief Runtime type check function for debug purpose.
 *
 * All type cast macro's should be implemented using this function.
 * When NDEBUG is not set each cast will verify if the given object is of the expected type.
 * The function will return the given object if correct or otherwise return NULL.
 * The 'kind' parameter specifies the expected type.
 */
OS_API u_object u__objectCheckType (const void *_this, const u_kind kind);
#ifdef NDEBUG
#define u_objectCheckType(_this, kind) _this
#else
#define u_objectCheckType(_this, kind) u__objectCheckType(_this, kind)
#endif

/**
 * \brief The Object Destructor.
 *
 * The User Layer Object destructor deletes the given u_object object.
 * For a normal object the kernel is informed to delete the associated
 * shared object, for proxy objects only the proxy is deleted.
 * Once the kernel is informed to delete the shared object the kernel will
 * prohibit any new attempt to access the object and wait until all
 * ongoing operations accessing the object have finished before deleting
 * the object.
 *
 * \param _this The User layer Object object where this method operates on.
 */
OS_API void
u_objectFree(
    void *_this);

OS_API u_result
u_objectFree_s(
    void *_this);

OS_API u_result
u_objectClose(
    void *_this);

/**
 * \brief The User Layer object get kind method.
 *
 * This method returns the kind of an object i.e. a enumeration value that
 * defines the actual object type.
 *
 * \param _this The User layer object where this method operates on.
 * \return The User Layer kind of the given object.
 */
OS_API u_kind
u_objectKind(
    const void *_this);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
