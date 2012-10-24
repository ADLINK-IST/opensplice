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
#ifndef U_SERVICE_H
#define U_SERVICE_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "u_types.h"
#include "v_serviceState.h"
#include "os_if.h"

#ifdef OSPL_BUILD_USER
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * \brief The <code>u_service</code> cast method.
 *
 * This method casts an object to a <code>u_service</code> object.
 * Since objects of the user layer are allocated on heap, no type checking
 * is performed.
 */
#define u_service(o) ((u_service)(o))

/**
 * \brief A splice daemon listener is called, when the splice service terminates or dies.
 *
 * \param state the state of the splice service
 * \param usrData Is the same pointer as given at setting the listener.
 */
typedef void (*u_serviceSplicedaemonListener)(v_serviceStateKind spliceDaemonState, c_voidp usrData);

/**
 * \brief Creates a new service in the kernel and returns a proxy to that service.
 *
 * A service is uniquely identified by its name. The given lease period is a
 * contract between the service and the kernel. This contract specifies that
 * during the given period the kernel will keep all resources. When the
 * lease/contract is not renewed, all resources used by the service are removed
 * from the kernel.
 *
 *
 * \param kernel       the kernel.
 * \param name         the unique name of the service
 * \param extStateName the name of the subclass of <code>v_serviceState</code>
                       (optional).
 * \param leasePeriod the duration of the validity of this service.
 * \param king the service kind.
 *
 * \return the created proxy to the service if the creation succeeded,
 *         otherwise <code>NULL</code>
 */
OS_API u_service
u_serviceNew(
    const c_char *uri,
    c_long timeout,
    const c_char *name,
    const c_char *extStateName,
    u_serviceKind kind,
    v_qos qos);

/**
 * \brief Initialises the proxy object to a service object.
 *
 * \param service The service proxy to operate on.
 */
OS_API u_result
u_serviceInit(
    u_service _this,
    u_serviceKind kind,
    u_domain kernel);


OS_API u_result
u_serviceDeinit(
    u_service service);

/** \brief The <code>u_service</code> destructor.
 *
 * The destructor frees the kernel service associated with this proxy
 * and then frees this proxy.
 *
 * \param service The service proxy to operate on.
 * \return U_RESULT_OK on a succesful operation or<br>
 *         U_RESULT_ILL_PARAM if the specified service state is incorrect.
 */
OS_API u_result
u_serviceFree(
    u_service service);

/**
 * \brief Returns the name of the service.
 *
 * Each service is uniquely identified by a name and each service has
 * a state object.
 *
 * \param service      the service proxy to operate on.
 *
 * \return a copy of the name of this service. It is the responsibility
 *         of the caller to free the copy of the name.
 */
OS_API c_char *
u_serviceGetName(
    u_service service);

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
 * \param service   the service proxy to operate on.
 * \param newState  the new state for this service.
 *
 * \return TRUE,  if the state change is allowed and succeeded.<br>
 *         FALSE, if the state change is not allowed.
 */
OS_API c_bool
u_serviceChangeState(
    u_service service,
    v_serviceStateKind newState);

/**
 * \brief Returns the state kind of the given service.
 *
 * The following kind of states are defined: <code>STATE_NONE</code>,
 * <code>STATE_INITIALISING</code>, <code>STATE_OPERATIONAL</code>,
 * <code>STATE_TERMINATING</code>, <code>STATE_TERMINATED</code> and
 * <code>STATE_DIED</code>.
 * The state <code>STATE_NONE</code> is returned when an error has occurred.
 *
 * \param service the service proxy to operate on.
 *
 * \return the state kind of the given service.
 */
OS_API v_serviceStateKind
u_serviceGetStateKind(
    u_service service);

/**
 * \brief Set/reset the listener that is called when the splice service terminates or dies.
 *
 * The given listener is called, when the splice service terminates or dies. With this
 * call the given user data (<code>usrData</code>) is also passed. When no listener is given
 * (<code>listener == NULL</code>) the listener previously set (if any) is reset.
 *
 * \param service The service proxy to operate on.
 * \param listener The listener that must be called.
 * \param usrData Any pointer to data that is passed when the listener is called.
 *
 * \return U_RESULT_OK on a succesful operation <br>
 *         U_RESULT_ILL_PARAM if the specified proxy is incorrect.
 */
OS_API u_result
u_serviceWatchSpliceDaemon(
    u_service service,
    u_serviceSplicedaemonListener listener,
    c_voidp usrData);

OS_API u_result
u_serviceEnableStatistics(
    u_service service,
    const char *categoryName);

/**
 * \brief Renews the lease of the service.
 *
 * This method renews the lease of the service by adding the lease period
 * to the current time.
 *
 * \param _this The service proxy to operate on.
 *
 * \return U_RESULT_OK the service is disabled.<br>
 *         U_RESULT_ILL_PARAM if the specified service is incorrect.
 */
OS_API u_result
u_serviceRenewLease(
    u_service _this,
    v_duration leasePeriod);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* U_SERVICE_H */
