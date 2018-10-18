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
#ifndef V_ENTITY_H
#define V_ENTITY_H

/** \file v_entity.h
 *  \brief The abstract base class of all Kernel objects that are directly
 *         accessible via the User Layer.
 *
 * The Kernel Entity class is the abstract base class of all Kernel objects
 * that are created and owned by User processes being either applications
 * processes or Splice Services.
 * From a User process perspective Kernel Entity objects will exist until the
 * owner destroys the Entity object or the owner itself is destroyed.
 * Once destroyed an Entity object will no longer be accessible by any User
 * process. The actual destruction, i.e. freeing all allocated resources,
 * may be delayed because entities are in use at the moment of destruction.
 *
 * All Kernel Entity objects live in the Splice object database and the
 * database automatically handles the destruction of entities when the last
 * reference is removed. Access to Kernel Entity objects is managed by the
 * Kernels HandleServer.
 * The initialisation of an Entity object (v_entityInit) will register the
 * Entity object at the Kernel's HandleServer. The HandleServer will maintain
 * a reference to the registered Entity object and keep it alive.
 * The Entity destructor will invalidate the associated handle and destroy the
 * Entity object.
 *
 * The following methods are provided by this class:
 *
 *    v_entityResult v_entityInit        (v_entity _this, const c_char *name);
 *    void           v_entityDeinit      (v_entity _this);
 *    v_handle       v_entityHandle      (v_entity _this);
 *    void           v_entityFree        (v_entity _this);
 *
 *    v_status       v_entityStatus      (v_entity _this);
 *
 *    c_voidp        v_entityGetUserData (v_entity _this);
 *    c_voidp        v_entitySetUserData (v_entity _this, c_voidp userData);
 *
 */

#include "v_kernel.h"
#include "v_event.h"

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

/**
 * \brief The <code>v_entity</code> cast method.
 *
 * This method casts an object to a <code>v_entity</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_entity</code> or
 * one of its subclasses.
 */
#define v_entity(e) (C_CAST(e,v_entity))

#define v_entityName(e) (v_entity(e)->name)
#define v_entityName2(e) (v_entityName(e) != NULL ? \
                          v_entityName(e) : "<Unknown>")

typedef c_bool (*v_entityAction)(v_entity e, c_voidp arg);

/**
 * \brief The Entity's get status method.
 *
 * This method will return a reference to the status object aggregated
 * by the given entity object.
 *
 * \param _this The entity object where this method will operate on.
 * \return The entity's status object.
 *         The caller is responsible to free the returned status object
 *         when it is no longer required.
 */
OS_API v_status
v_entityStatus(
    v_entity _this);

/**
 * \brief Resets the status bits set in mask on this entities status.
 *
 * This method invokes v_statusReset(_this->status, mask).
 *
 * \see v_statusReset
 * \param _this The entity object where this method will operate on.
 * \param mask The mask of bits to be reset.
 */
OS_API void
v_entityStatusReset(
    v_entity _this,
    c_ulong mask);

/**
 * \brief Retrieves the status mask of this entity.
 *
 * This method returns v_statusGetMask(_this->status).
 *
 * \see v_statusGetMask
 * \param _this The entity object where this method will operate on.
 * \return The entity's status mask.
 */
OS_API c_ulong
v_entityStatusGetMask(
    v_entity _this);

/**
 * \brief The Entity get UserData method.
 *
 * This method retrieves the UserData associated to the given entity.
 * The UserData can be any kind of data that is set via the
 * v_entitySetUserData method.
 * This data is intended to be a reference to the owners process specific
 * data, the data reference is not interpreted by the system and therefore
 * can have any value.
 *
 * \param _this The entity object where this method will operate on.
 * \return The entity's UserData.
 */
OS_API c_voidp
v_entityGetUserData(
    v_entity _this);

/**
 * \brief The Entity set UserData method.
 *
 * This method sets the UserData for the given entity.
 * This method is deprecated and will be replaced by the enable method in
 * the near future.
 * The UserData can be any kind of data that can be retrieved via the
 * v_entityGetUserData method. This data is intended to be a reference to
 * the owners process specific data, the data reference is not interpreted
 * by the system and therefore can have any value.
 *
 * \param _this    The entity object where this method will operate on.
 * \param userData The UserData given to be associated to the given entity
 *                 where this method
 *                 operates on.
 * \return The entity's previous set UserData.
 */
OS_API c_voidp
v_entitySetUserData (
    v_entity _this,
    c_voidp userData);

/**
 * \brief The entity walk over all related entities method.
 *
 * This method visites all entities that are associated to the given entity
 * object.
 * This method will walk over the attributes of the derived class and in
 * case the value of an attribute is an entity the given action routine is
 * executed on the encountered entity.
 * The specified arg parameter is passed to the action routine.
 *
 * \param _this  The entity object where this method will operate on.
 * \param action The action routine that will be executed upon the Entity's
 *               associated entities.
 * \param arg    The argument that will be passed to the action routine when
 *               executed.
 *
 * return A boolean value that indicates if the walk has finished visiting
 *        all associated entities or not. The value TRUE indicates all
 *        associated entities are visited otherwise the walk is terminated
 *        by the action routine.
 *        The walk will terminate as soon as the action routine returns FALSE.
 *
 * \note This function is DEPRECATED. It is currently still used by the C&M
 *       API, but should be removed in the future.
 */
OS_API c_bool
v_entityWalkEntities (
    v_entity _this,
    v_entityAction action,
    c_voidp arg);

/*
 * Like v_entityWalkEntities, this function is also DEPRECATED. It is
 * currently still used by the C&M API, but should be removed in the future.
 */
OS_API c_bool
v_entityWalkDependantEntities(
    v_entity _this,
    v_entityAction action,
    c_voidp arg);

/**
 * \brief Tries to enable the entity.
 *
 * An entity has state V_ENTITYSTATE_DISABLED after creation. Calling v_entityEnable
 * will try to transition the entity to V_ENTITYSTATE_ENABLED. The state diagram
 * for an entity is as follows.
 *
 *  --> V_ENTITYSTATE_DISABLED --> v_entityEnable(...) --> V_ENTITYSTATE_ENABLING -->
 *           V_RESULT_OK: --> V_ENTITYSTATE_ENABLED
 *          !V_RESULT_OK: --> V_ENTITYSTATE_DISABLED
 *
 *  --> V_ENTITYSTATE_ENABLING --> v_entityEnable(...) -->
 *           V_RESULT_OK: --> V_ENTITYSTATE_ENABLED
 *          !V_RESULT_OK: --> V_ENTITYSTATE_DISABLED
 *
 *  --> V_ENTITYSTATE_ENABLED --> v_entityEnable(...) --> V_ENTITYSTATE_ENABLED
 *
 *  So calling v_entityEnable on an already enabled entity will be a no-op.
 *
 *  \return V_RESULT_OK if successfully enabled.
 */
OS_API v_result
v_entityEnable (
    _Inout_ v_entity _this);

/**
 * \brief Returns true if the entity is enabled
 *
 * Once this function returns true, it will always be true for that entity.
 *
 * Use this function to check whether the entity is neither disabled nor enabling.
 *
 * \param _this The entity to check the state for
 * \return true if the entity is enabled (so neither disabled or enabling)
 */
OS_API c_bool
v_entityEnabled (
    _In_ v_entity _this);

/**
 * \brief Returns true if the entity is in disabled state
 *
 * Use this function to check whether the entity is neither enabled nor enabling.
 *
 * \param _this The entity to check the state for
 * \return true if the entity is disabled (so neither enabled or enabling)
 */
OS_API c_bool
v_entityDisabled (
    _In_ v_entity _this);

/**
 * \brief The Entity getName method.
 *
 * This method will return the name of the given entity.
 * This method will allocate the name on heap and the callee is responsible
 * for the claimed resources. If an entity has no name this method will
 * generate a unique id (string representation of the entities GID).
 *
 * \param _this    The entity object where this method will operate on.
 * \return The entity's name or string representation of it's GID.
 */
OS_API c_char *
v_entityGetName(
    v_entity _this);

OS_API v_result
v_entitySetListener(
    v_entity _this,
    v_listener listener,
    void *listenerData,
    v_eventMask interest);

OS_API c_bool
v_entityNotifyListener(
    v_entity _this,
    v_event event);

#define v_entityGetListenerInterest(_this) (v_entity(_this)->listenerInterest)

OS_API c_char *
v_entityGetXMLQos(
    v_entity e);

OS_API v_result
v_entitySetXMLQos(
    v_entity e,
    const c_char *xml);

OS_API v_objectLoan
v_entityLoan(
    v_entity _this,
    c_bool subLoan);

OS_API void
v_entityReleaseLoan(
    v_entity _this);

OS_API c_bool
v_entityDisableCallbacks(
    v_entity _this);

OS_API v_entity
v_entityOwner (
    v_entity _this);

OS_API
c_longlong
v_entityGetProcessId(
    v_entity _this);

OS_API v_state
v_entityGetStatusFlags(
    v_entity e);

OS_API v_result
v_entityGetProperty(
        const v_entity _this,
        const os_char * property,
        os_char ** value);

OS_API v_result
v_entitySetProperty(
        const v_entity _this,
        const os_char * property,
        const os_char * value);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
