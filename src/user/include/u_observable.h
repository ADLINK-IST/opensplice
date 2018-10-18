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
#ifndef U_OBSERVABLE_H
#define U_OBSERVABLE_H

/** \file u_observable.h
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
 *    void          u_observableFree          (u_observable o);
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

/** \brief The u_observable cast method.
 */
#define u_observable(_this) ((u_observable)(_this))

/**
 * \brief The User Layer Observable get UserData method.
 *
 * This method returns the user data associated to the given observable.
 * User data is available to processes to store a pointer to process specific
 * data. This data will be passed to listeners.
 *
 * \param _this The User layer Observable object where this method operates on.
 * \return the UserData associated to the specified observable.
 */
OS_API void *
u_observableGetUserData (
    const u_observable _this);

/**
 * \brief  The User Layer Observable set UserData method.
 *
 * This method assigns user data to the given observable.
 * Processes can assign a pointer to specific data to User Layer that will be
 * passed to listeners.
 *
 * \param _this The User layer Observable object where this method operates on.
 * \param userData The UserData that will be associated to the given observable
 *                 object.
 */
OS_API void *
u_observableSetUserData (
    const u_observable _this,
    void *userData);

/**
 * \brief The User Layer Object Action method.
 *
 * This method provides a generic access method to the associated Kernel
 * object.
 * The given action method will be executed upon the associated Kernel
 * object in a safe manner with respect to the liveliness of the
 * Kernel Object and calling process.
 *
 * The action is regarded as a 'read'-action within the scope of protecting
 * memory-resources. If the action allocates shared-memory, u_observableWriteAction
 * should be used instead.
 *
 * \param _this The User layer object where this method operates on.
 * \param action The action method that will be executed upon the associated
 *               Kernel object.
 * \param arg The argument that will be passed to the action method upon
 *            execution.
 * \return The execution result of the action method.
 */
OS_API u_result
u_observableAction (
    const u_observable _this,
    void (*action)(v_public p, void *arg),
    void *arg);

/**
 * \brief The User Layer Object Action method for actions allocating SHM.
 *
 * This method provides a generic access method to the associated Kernel
 * Object.
 * The given action method will be executed upon the associated Kernel
 * Object in a safe manner with respect to the liveliness of the
 * Kernel Object and calling process.
 *
 * The action is regarded as a 'write'-action within the scope of protecting
 * memory-resources.
 *
 * \param _this The User layer Object where this method operates on.
 * \param action The action method that will be executed upon the associated
 *               Kernel Object.
 * \param arg The argument that will be passed to the action method upon
 *            execution.
 * \return The execution result of the action method.
 */
OS_API u_result
u_observableWriteAction(
    const u_observable _this,
    void (*action)(v_public p, void *arg),
    void *arg);

/**
 * \brief The User Layer object get the GID (global ID) method.
 *
 * This method returns the GID that is a system wide unique identification
 * of the object.
 *
 * \param _this The User layer Object where this method operates on.
 * \return The User Layer object's GID.
 */
OS_API v_gid
u_observableGid (
    const u_observable _this);

/** \brief The following operations provide listener support for objects.
 *
 * A listener can be associated to an object and interest for event cani
 * be set on object, listeners will be called when events of interest occur.
 */
OS_API u_result
u_observableAddListener(
    const u_observable _this,
    const u_eventMask eventMask,
    const u_observableListener listener,
    void *userData);

OS_API u_result
u_observableRemoveListener(
    const u_observable _this,
    const u_observableListener listener);

OS_API u_result
u_observableNotify(
    const u_observable _this);

OS_API u_result
u_observableSetListenerMask(
    const u_observable _this,
    const u_eventMask eventMask);

OS_API u_result
u_observableGetListenerMask(
    const u_observable _this,
          u_eventMask *eventMask);

/**
 * \brief The User Layer object proxy constructor.
 *
 * This method will create a proxy object to an existing object.
 * This object provides access to the shared Domain object but has no
 * control over the lifecycle, i.e. deleting this object will not free
 * the shared object and when the owner deletes the shared object it
 * will not be kept alive by this proxy object.
 *
 * This operation is currently only used by the cm xml api and is subject
 * to changes in the near future.
 */
OS_API u_observable
u_observableCreateProxy (
    const v_public vObject,
    const u_participant p);

OS_API u_bool
u_observableGetY2038Ready(
    u_observable _this);

OS_API u_domainId_t
u_observableGetDomainId(
    u_observable _this);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
