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

/** \file kernel/include/v_participant.h
 *  \brief This file defines the interface of the kernel participant class.
 *
 *  The kernel participant implements an application specific container that
 *  will hold all kernel entities created by the application and where the
 *  application remains the owner.
 *  The existence of these entities depend on the existence of the participant.
 *  When an application deletes a participant either explicit or implicit via
 *  e.g. an exit handler all contained entities are also deleted.
 *  However if entities are in use i.e. are referenced by a process it will
 *  remain until the process releases the reference.
 */

#ifndef V_PARTICIPANT_H
#define V_PARTICIPANT_H

#include "v_kernel.h"
#include "v_entity.h"
#include "v_event.h"

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
 * \brief The <code>v_participant</code> cast method.
 *
 * This method casts an object to a <code>v_participant</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_participant</code> or
 * one of its subclasses.
 */
#define v_participant(o) (C_CAST(o,v_participant))
#define v_participantName(o) (v_entityName(v_participant(o)))

/** \fn v_participant v_participantNew(v_kernel k, const c_char *name, v_qos qos)
 *  \brief Creates a new participant object.
 *
 *  \param k The kernel scope wherein the participant shall be defined.
 *  \param name An optional name that is associated to the created participant.
 *  \param qos The Quality of service values to be assigned to the created
 *             participant.
 *  \return The newly created participant.
 */
OS_API v_participant
v_participantNew(
    v_kernel k,
    const c_char *name,
    v_qos qos,
    v_statistics s,
    c_bool enable);

/** \fn void v_participantInit (v_participant _this, const c_char *name, v_qos qos);
 *  \brief Initializes an existing participant object. This function will
 *  override all existing values therefore this function should only be performed
 *  on newly created participant objects.
 *
 *  \param _this The participant object to be initialized.
 *  \param name An optional name that is associated to the created participant.
 *  \param qos The Quality of service values to be assigned to the created participant.
 */
OS_API void
v_participantInit(
    v_participant _this,
    const c_char *name,
    v_participantQos qos,
    v_statistics s,
    c_bool enable);

/** \fn void v_participantFree (v_participant _this)
 *  \brief This function will free all resources claimed by the participant or any
 *  of its contained entities.
 *
 *  \param _this The participant object to be freed.
 */
OS_API void
v_participantFree(
    v_participant _this);

/** \fn void v_participantDeinit (v_participant _this)
 *  \brief This function will de-initialize the participant. De-initialisation
 *  will perform those actions required that are not performed by the c_free operation.
 *  E.g: unregistering its existence in containing factory scopes.
 *
 *  \param _this The participant object to be de-initialized.
 */
OS_API void
v_participantDeinit(
    v_participant _this);

/** \fn void v_participantAdd (v_participant _this, v_entity e)
 *  \brief This function will add the given entity to the participant, i.e. will
 *  add a reference from participant to the entity.
 *
 *  \param _this The participant.
 *  \param e The entity that is add to the participant.
 */
OS_API void
v_participantAdd(
    v_participant _this,
    v_entity e);

/** \fn void v_participantRemove (v_participant _this, v_entity e);
 *  \brief This function will remove the given entity from the participant.
 *  If the given entity is not associated to the participant this function will
 *  do nothing.
 *
 *  \param _this The participant.
 *  \param e The entity that is removed from the participant.
 */
OS_API void
v_participantRemove(
    v_participant _this,
    v_entity e);

OS_API void
v_participantNotify(
    v_participant _this,
    v_event e,
    c_voidp userData);

/** \fn void v_participantConnectNewGroup( v_participant _this, v_event event);
 *  \brief This function will connect all DataWriters and DataReaders of
 *         intrest to the new group (indecated by the 'new group' event.
 *  Note that for now the event is passed but eventually the group should be
 *  passed.
 */
OS_API void
v_participantConnectNewGroup (
    v_participant _this,
    v_event event);

OS_API void
v_participantAssertLiveliness(
    v_participant _this);

OS_API void
v_participantDeleteHistoricalData(
    v_participant _this,
    const c_char* partitionExpr,
    const c_char* topicExpr);

OS_API v_subscriber
v_participantGetBuiltinSubscriber(
    v_participant _this);

OS_API v_leaseManager
v_participantGetLeaseManager(
    v_participant _this);

OS_API void
v_participantResendManagerMain(
    v_participant _this);

OS_API void
v_participantResendManagerQuit(
    v_participant _this);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
