/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
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
 *    void          u_entityFree          (u_entity e);
 *    void          u_entityEnable        (u_entity e);
 *    c_voidp       u_entityGetUserData   (u_entity e);
 *    c_voidp       u_entitySetUserData   (u_entity e, c_voidp userData);
 *    u_participant u_entityParticipant   (u_entity e);
 *    c_bool        u_entityAction        (u_entity e,
 *                                         c_bool (*action)(v_entity e,
 *                                                          c_voidp arg),
 *                                         c_voidp arg);
 *    c_bool        u_entityWalkEntities  (u_entity e,
 *                                         c_bool (*action)(v_entity e,
 *                                                          c_voidp arg),
 *                                         c_voidp arg);
 *    v_qos         u_entityQoS           (u_entity entity);
 *    u_result      u_entitySetQoS        (u_entity entity, v_qos qos);
 *    c_type        u_entityResolveType   (u_entity e);
 */

#if defined (__cplusplus)
extern "C" {
#endif

#include "u_types.h"
#include "u_handle.h"
#include "u_instanceHandle.h"
#include "os_if.h"

#ifdef OSPL_BUILD_USER
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

typedef c_voidp (*u_entityCopy)(v_entity e, c_voidp copyArg);

#ifdef NDEBUG
#define u_entityCheckType(_this, kind) _this
#else
OS_API u_entity
u_entityCheckType (
    u_entity _this,
    u_kind kind);
#endif
                                    
#define u_entity(e) ((u_entity)(e))

/**
 * \brief The User Layer Entity Abstract constructor.
 *
 * This method will allocate the resources required by the derived classes.
 * Memory is allocated, the Kernel Handle is set and the participant scope
 * wherein the entity is created is set.
 * The Kernel Handle is either newly created or copied depending on the
 * ownership.
 * If the created User Layer Entity is the owner proxy the owner parameter
 * must be set to TRUE indicating that the Kernel Entity must be destroyed
 * when the User Layer Entity is destroyed.
 * When the created User Layer Entity is not the owner proxy (e.g. if it is
 * created as result of a lookup action) the owner parameter must be set to
 * FALSE, destruction of the User Layer Entity will then not destroy the
 * Kernel Entity.
 */
OS_API u_entity
u_entityNew (
    v_entity ke,
    u_participant p,
    c_bool owner);

OS_API u_entity
u_entityKeep (
    u_entity _this);

/**
 * \brief The Entity Destructor.
 *
 * The User Layer Entity destructor deletes the given u_entity object and
 * informs the kernel that the associated kernel entity can be deleted as
 * soon as it is no longer referenced.
 * Once the kernel is informed to delete the kernel entity the kernel will
 * prohibit any new attempt to access the entity.
 *
 * \param e The User layer Entity object where this method operates on.
 */
OS_API u_result
u_entityFree (
    u_entity _this);

OS_API c_bool
u_entityDereference (
    u_entity _this);

/**
 * \brief The User Layer Entity enable method.
 *
 * This method will enable the given entity object making it fully functional.
 * This method is currently not implemented.
 *
 * \param _this The User layer Entity object where this method operates on.
 */
OS_API void
u_entityEnable (
    u_entity _this);

/**
 * \brief The User Layer Entity get UserData method.
 *
 * This method returns the user data associated to the given entity.
 * User data is available to processes to store a pointer to process specific
 * data.
 *
 * \param _this The User layer Entity object where this method operates on.
 * \return the UserData associated to the specified entity.
 */
OS_API c_voidp
u_entityGetUserData (
    u_entity _this);

/**
 * \brief  The User Layer Entity set UserData method.
 *
 * This method assigns user data to the given entity.
 * Processes can assign a pointer to process specific data to User Layer
 * entities e.g. API's will add pointers to API specific bindings of User
 * Layer entities to make it easy to find these API bindings if only the
 * User Layer entity is available.
 * This method is not MT-safe; in fact it is preferred to set the value of
 * user data only once.
 * NOTE: this method is deprecated and will be replaced by the enable method.
 *
 * \param _this The User layer Entity object where this method operates on.
 * \param userData The UserData that will be associated to the given entity
 *                 object.
 */
OS_API c_voidp
u_entitySetUserData (
    u_entity _this,
    c_voidp userData);

/**
 * \brief The User Layer entity get participant method.
 *
 * This method returns the participant that defines the scope in which the
 * given entity is created.
 *
 * \param _this The User layer Entity object where this method operates on.
 * \return The User Layer Participant Entity object associated to the given
 *         entity.
 */
OS_API u_participant
u_entityParticipant (
    u_entity _this);

/**
 * \brief The User Layer Entity Action method.
 *
 * This method provides a generic access method to the associated Kernel
 * Entity object.
 * The given action method will be executed upon the associated Kernel
 * Entity object in a safe manner with respect to the liveliness of the
 * Kernel Entity object and calling process.
 *
 * \param _this The User layer Entity object where this method operates on.
 * \param action The action method that will be executed upon the associated
 *               Kernel Entity object.
 * \param arg The argument that will be passed to the action method upon
 *            execution.
 * \return The execution result of the action method.
 */
OS_API u_result
u_entityAction (
    u_entity _this,
    void (*action)(v_entity e, c_voidp arg),
    c_voidp arg);

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
 */
OS_API u_result
u_entityWalkEntities (
    u_entity _this,
    c_bool (*action)(v_entity e, c_voidp arg),
    c_voidp arg);

OS_API u_result
u_entityWalkDependantEntities (
    u_entity _this,
    c_bool (*action)(v_entity e, c_voidp arg),
    c_voidp arg);

/**
 * \brief Resolves the QoS policy of the supplied entity.
 * 
 * \param _this The entity to resolve the QoS policy from.
 * \result The QoS policy of the supplied entity.
 */
OS_API u_result
u_entityQoS(
    u_entity _this,
    v_qos* qos);

/**
 * \brief Applies the supplied QoS policy to the supplied entity.
 *
 * \param _this The entity to apply the QoS policy to.
 * \result U_RESULT_OK if the QoS policy was successfully applied. If not
 * successfully applied, any other result can be expected depending on the
 * reason of the failure.
 */
OS_API u_result
u_entitySetQoS(
    u_entity _this,
    v_qos qos);

/**
 * \brief The User Layer Entity resolve type method.
 *
 * This method is ambiguous and will be replaced by two other methods.
 */
OS_API c_type
u_entityResolveType (
    u_entity _this);

OS_API c_bool
u_entityOwner (
    u_entity _this);

OS_API c_bool
u_entityEnabled (
    u_entity _this);

OS_API u_instanceHandle
u_entityGetInstanceHandle(
    u_entity _this);

OS_API u_kind
u_entityKind(
    u_entity _this);

OS_API u_handle
u_entityHandle (
    u_entity _this);

OS_API v_gid
u_entityGid (
    u_entity _this);

OS_API c_long
u_entitySystemId(
    u_entity _this);

/* This method returns a copy of the kernel entity name that must
 * be freed using os_free when no longer needed.
 * If the name is not defined then 'No Name' is returned.
 * If an invalid entity is passed 'Invalid Entity' is returned.
 */
OS_API c_char *
u_entityName(
    u_entity _this);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
