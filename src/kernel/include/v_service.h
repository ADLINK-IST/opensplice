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
#ifndef V_SERVICE_H
#define V_SERVICE_H

#include "kernelModule.h"
#include "v_participantQos.h"

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
 * \brief The <code>v_service</code> cast method.
 *
 * This method casts an object to a <code>v_service</code> object.
 * Before the cast is performed, the type of the object is checked to
 * be <code>v_service</code> or one of its subclasses.
 */
#define v_service(o) (C_CAST(o,v_service))

/**
 * \brief The <code>v_service</code> constructor.
 *
 * The constructor will create a new <code>v_service</code> object and a
 * corresponding <code>v_serviceState</code> object.
 * The <code>extStateName</code> parameter can be used to create the instance
 * of a subclass of <code>v_serviceState</code>. This can be used by services
 * to store their own state information in the kernel. Only the attributes
 * defined in the <code>v_serviceState</code> class are initialised. It is
 * up to the service to initialise all attributes of the subclass.
 * The given lease period is a contract between the service and the
 * kernel. This contract specifies that during the given period the kernel
 * will keep all resources. When the lease/contract is not renewed, all
 * resources used by the service are removed from the kernel.
 *
 * \param manager     The serviceManager
 * \param name        The name of the service.
 * \param extSateName The name of a subclass of <code>v_serviceState</code>
 * \param leasePeriod The duration of the validity of this service.
 *
 * \return NULL if memory allocation failed, otherwise
 *         a reference to a newly created <code>v_service</code> object.
 */
OS_API v_service
v_serviceNew(
    v_serviceManager manager,
    const c_char *name,
    const c_char *extStateName,
    v_participantQos qos,
    v_statistics s);

/**
 * \brief The initialisation function, which can be used by classes that
 *        extend from this class.
 *
 * \param service     The service object to operate on.
 */
OS_API void
v_serviceInit(
    v_service service,
    v_serviceManager manager,
    const c_char *name,
    const c_char *extStateName,
    v_participantQos qos,
    v_statistics s);

/**
 * \brief The <code>v_service</code> destructor.
 *
 * The object is really freed, if no more references to this object exist.
 * The function <code>v_serviceDeinit</code> is called to free all used
 * resources by this object.
 *
 * \param service the service object to operate on.
 */
OS_API void
v_serviceFree(
    v_service service);

/**
 * \brief The de-initialisation function, that frees all used resources
 *        by this object.
 *
 * This function is called by <code>v_serviceFree</code>, but can
 * also be called by destructor's of subclasses. All used resources by this
 * object are freed.
 *
 * \param service the service object to operate on.
 */
OS_API void
v_serviceDeinit(
    v_service service);

/**
 * \brief Returns the state of the service.
 *
 * Each service has a state, whose validity must be confirmed periodically.
 *
 * \param service the service object to operate on.
 *
 * \return a reference to the <code>v_serviceState</code> object of
 *         this service.
 */
OS_API v_serviceState
v_serviceGetState(
    v_service service);

/**
 * \brief Returns the registered name of the service.
 *
 * Each service is uniquely identified by a name.
 *
 * \param service the service object to operate on.
 *
 * \return a reference to the name of the service.
 */
OS_API const c_char *
v_serviceGetName(
    v_service service);

/**
 * \brief Changes the state of a service object.
 *
 * The state kind indicates the state of the service. The following
 * table defines the state transitions that are allowed. The row of
 * the table defines the current state and the column represents the
 * new state (i.e. the state to transfer to).
 * <p>
 * <table BORDER COLS=7 WIDTH="100%" NOSAVE >
 * <tr ALIGN=CENTER NOSAVE>
 *   <td></td>
 *   <td>STATE_NONE</td>
 *   <td>STATE_INITIALISING</td>
 *   <td>STATE_OPERATIONAL</td>
 *   <td>STATE_TERMINATING</td>
 *   <td>STATE_TERMINATED</td>
 *   <td>STATE_DIED</td>
 * </tr>
 * <tr ALIGN=CENTER NOSAVE>
 *   <td>STATE_NONE</td>
 *   <td align=center>x</td><td>x</td><td>-</td><td>-</td><td>-</td><td>-</td>
 * </tr>
 * <tr ALIGN=CENTER NOSAVE>
 *   <td>STATE_INITIALISING</td>
 *   <td>-</td><td>x</td><td>x</td><td>x</td><td>-</td><td>x</td>
 * </tr>
 * <tr ALIGN=CENTER NOSAVE>
 *   <td>STATE_OPERATIONAL</td>
 *   <td>-</td><td>-</td><td>x</td><td>x</td><td>-</td><td>x</td>
 * </tr>
 * <tr ALIGN=CENTER NOSAVE>
 *   <td>STATE_TERMINATING</td>
 *   <td>-</td><td>-</td><td>-</td><td>x</td><td>x</td><td>x</td>
 * </tr>
 * <tr ALIGN=CENTER NOSAVE>
 *   <td>STATE_TERMINATED</td>
 *   <td>-</td><td>-</td><td>-</td><td>-</td><td>x</td><td>x</td>
 * </tr>
 * <tr ALIGN=CENTER NOSAVE>
 *   <td>STATE_DIED</td>
 *   <td>-</td><td>x</td><td>-</td><td>-</td><td>-</td><td>x</td>
 * </tr>
 * </table>
 *
 *
 * \param service   the service object to operate on.
 * \param newState  the new state for this service.
 *
 * \return TRUE,  if the state change is allowed and succeeded.<br>
 *         FALSE, if the state change is not allowed.
 */
OS_API c_bool
v_serviceChangeState(
    v_service service,
    v_serviceStateKind newState);


OS_API void
v_serviceFillNewGroups(
    v_service service);


OS_API c_iter
v_serviceTakeNewGroups(
    v_service service);

/**
 * \brief Renews the lease of the service.
 *
 * This method renews the lease of the service by adding the
 * lease period to the current time.
 *
 * \param service The service to operate on.
 */
OS_API void
v_serviceRenewLease(
    v_service service,
    v_duration leasePeriod);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* V_SERVICE_H */
