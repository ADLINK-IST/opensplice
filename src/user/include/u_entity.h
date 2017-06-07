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
#ifndef U_ENTITY_H
#define U_ENTITY_H

/** \file u_entity.h
 *
 * The User Layer Entity class implements the base class of all User Layer
 * classes that provide an interface to a kernel entity class.
 * This class manages the access to kernel objects, i.e. it will protect
 * calling threads against process termination, inform the kernel to delay
 * any deletion of the kernel entity during access and inform callers if the
 * kernel entity is already deleted.
 * The User Layer Entity class also provides status information of it and
 * supports navigation over contained entities.
 *
 * The following methods are supported:
 *
 *    u_entityEnable
 *    u_entityEnabled
 *    u_entityGetInstanceHandle
 *    u_entitySetListener
 *    u_entityWalkEntities          [DEPRECATED]
 *    u_entityWalkDependantEntities [DEPRECATED]
 *    u_entityGetEventState
 *    u_entitySetXMLQos
 *    u_entityGetXMLQos
 */

#include "u_types.h"
#include "u_object.h"
#include "u_instanceHandle.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define u_entity(e) ((u_entity)(e))

/**
 * \brief The User Layer Entity enable method.
 *
 * This method will enable the given entity object making it fully functional.
 * This method is currently not implemented.
 *
 * \param _this The User layer Entity object where this method operates on.
 */
OS_API u_result
u_entityEnable (
    const u_entity _this);

OS_API u_bool
u_entityEnabled (
    const u_entity _this);

/**
 * \brief Returns the corresponding builtin DataReader instance handle that holds
 *        the data published for this entity.
 */
OS_API u_instanceHandle
u_entityGetInstanceHandle(
    const u_entity _this);

OS_API u_result
u_entitySetListener(
    const u_entity _this,
    u_listener listener,
    void *listenerData,
    u_eventMask interest);

/**
 * \brief The User Layer Entity Walk all related Entities method.
 *
 * This method will execute the given action method upon all kernel entities
 * owned by the kernel entity that is associated to the given User Layer
 * entity.
 * The execution of the action method is performed in a safe manner, i.e. the
 * executing thread is protected against termination and the action method is
 * protected against the deletion of the kernel entity.
 * This method will return U_RESULT_OK if all entities are successfully
 * visited. An entity is considered successfully visited if the action method
 * returns TRUE. If the action method returns FALSE the method is aborted,
 * the rest of the contained kernel entities will not be visited anymore and
 * this method will return U_RESULT_INTERRUPTED.
 * The given arg parameter is passed to the action method.
 *
 * \param _this The User layer Entity object where this method operates on.
 * \param action The action method that will be executed upon the associated
 *               Kernel Entity object.
 * \param arg The argument that will be passed to the action method upon
 *            execution.
 * \return U_RESULT_OK if all owned Kernel Entities are visited and
 *         U_RESULT_INTERRUPTED if the walk is terminated by the action
 *         method. The action method will terminate the walk when it returns
 *         FALSE.
 *
 * \note This function is DEPRECATED. It is currently still used by the C&M
 *       API, but should be removed in the future.
 */
OS_API u_result
u_entityWalkEntities (
    const u_entity _this,
    u_bool (*action)(v_entity e, void *arg),
    void *arg);

/*
 * Like u_entityWalkEntities, this function is also DEPRECATED. It is
 * currently still used by the C&M API, but should be removed in the future.
 */
OS_API u_result
u_entityWalkDependantEntities (
    const u_entity _this,
    u_bool (*action)(v_entity e, void *arg),
    void *arg);

/* This method returns a copy of the kernel entity name that must
 * be freed using os_free when no longer needed.
 * If the name is not defined then 'No Name' is returned.
 * If an invalid entity is passed 'Invalid Entity' is returned.
 */
OS_API os_char *
u_entityName(
    const u_entity _this);

/**
 * \brief Returns a mask with the event status flags that are set since the last reset.
 *
 * This operation will not change the value of the flags.
 */
OS_API u_result
u_entityGetEventState (
    const u_entity _this,
    u_eventMask *eventState);

OS_API u_result
u_entityGetXMLQos (
    const u_entity _this,
    os_char **xml);

OS_API u_result
u_entitySetXMLQos (
    const u_entity _this,
    const os_char *xml);

OS_API u_result
u_entityReleaseLoan(
    u_entity _this,
    v_objectLoan loan);

OS_API u_bool
u_entityDisableCallbacks (
    const u_entity _this);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
