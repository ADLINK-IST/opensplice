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
 *    c_bool         v_entityWalkEntities(v_entity _this,
 *                                        v_entityAction action,
 *                                        c_voidp arg);
 */

#include "v_kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_KERNEL
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
 *         The caller is resposible to free the returned status object
 *         when it is no longer required.
 */
OS_API v_status
v_entityStatus(
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
 */
OS_API c_bool
v_entityWalkEntities (
    v_entity _this,
    v_entityAction action,
    c_voidp arg);
                            
OS_API c_bool
v_entityWalkDependantEntities(
    v_entity _this,
    v_entityAction action,
    c_voidp arg);

OS_API v_qos
v_entityGetQos(
    v_entity _this);

OS_API v_result
v_entitySetQos(
    v_entity _this,
    v_qos qos);

OS_API v_statistics
v_entityStatistics(
    v_entity _this);

OS_API v_result
v_entityEnable (
    v_entity _this);

OS_API c_bool
v_entityEnabled (
    v_entity _this);

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

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
