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
#ifndef U_SERVICE_H
#define U_SERVICE_H

#include "u_types.h"

#if defined (__cplusplus)
extern "C" {
#endif

#include "v_serviceState.h"
#include "kernelModule.h" /* because we need the v_serviceType enum */

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/*
 * ======   Services, application termination and user layer shut down   ======
 *
 * The next text has mainly to do with Single Process, but Shared Memory is
 * influenced a bit as well.
 *
 * When an application terminates, all functions registered at os_procAtExit()
 * are called. This functionality can be used by components to shut themselves
 * down properly and free their resources. The user layer registers itself at
 * os_procAtExit() to do just that (see u__userExit()).
 *
 * The services are tightly coupled with the user layer and the kernel. When
 * the services are still running when the user layer shuts down by the
 * u__userExit() call, a number of problems occur. This can be solved by making
 * sure that the services are terminated before the user layer.
 * To support that, the u_serviceAtExit() functionality is introduced.
 *
 * Just like with os_procAtExit(), a service can register itself at
 * u_serviceAtExit() and will receive a call when the application terminates.
 * There are a few advantages though:
 *    - Every registered service will receive a callback with
 *      the related u_service and private data.
 *    - Every registration will be removed automatically when
 *      the related u_service is de-initted.
 *    - The user layer will not shut down until all services
 *      have terminated.
 *
 * The user layer service component provides a function (u__serviceExit()) that
 * will call all the registered u_serviceAtExit callbacks and waits until all
 * related services are terminated (in other words; are de-initted, which means
 * they are removed from the u_serviceAtExit administration).
 *
 * When calling u__serviceExit() is the first thing that u__userExit() does,
 * you can be sure that the services are terminated before the user layer
 * continues to shut down.
 *
 * Note:
 * By design, the user layer service component will wait until the services are
 * terminated. This means that the services should just initiate their
 * termination in the callback function. They shouldn't do much more within that
 * callback.
 * The actual service termination should happen in a different context.
 *
 * Related API:
 * u_serviceAtExit        - Registration function
 * u_serviceAtExitAction  - Callback function declaration
 *
 */

/**
 * \brief The <code>u_service</code> cast method.
 *
 * This method casts an object to a <code>u_service</code> object.
 * Since objects of the user layer are allocated on heap, no type checking
 * is performed.
 */
#define u_service(o) ((u_service)(o))

/**
 * \brief The u_serviceAtExit action callback function.
 *
 * See the comment at the start of u_service.h for more information.
 *
 * It can be registered by calling u_serviceAtExit().
 */
typedef void (*u_serviceAtExitAction)(u_service service,  void *privateData);

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
 * \return the created proxy to the service if the creation succeeded,
 *         otherwise <code>NULL</code>
 */
OS_API u_service
u_serviceNew(
    const os_char *uri,
    const u_domainId_t id,
    const os_int32 timeout, /* in seconds */
    const os_char *name,
    v_serviceType serviceType,
    const u_participantQos qos,
    c_bool enable);

u_service
u_serviceNewSpecialized (
    void * (*v_new) (v_kernel kernel, const c_char *name, const c_char *extStateName, v_participantQos qos, c_bool enable, void *arg),
    const c_char *serviceConfigName,
    const os_char *uri,
    const u_domainId_t id,
    const os_int32 timeout, /* in seconds */
    const c_char *name,
    const u_participantQos qos,
    c_bool enable,
    void *arg);

/**
 * \brief Returns the name of the service.
 *
 * Each service is uniquely identified by a name and each service has
 * a state object.
 *
 * \param _this      the service proxy to operate on.
 *
 * \return a copy of the name of this service. It is the responsibility
 *         of the caller to free the copy of the name.
 */
OS_API os_char *
u_serviceGetName(
    const u_service _this);

/**
 * \brief Changes the state of a service object.
 *
 * The state kind indicates the state of the service. The following
 * table defines the state transitions that are allowed. The row of
 * the table defines the current state and the column represents the
 * new state (i.e. the state to transfer to).
 * <p>
 * <table BORDER COLS=8 WIDTH="100%" NOSAVE >
 * <tr ALIGN=CENTER NOSAVE>
 *   <td></td>
 *   <td>STATE_NONE</td>
 *   <td>STATE_INITIALISING</td>
 *   <td>STATE_OPERATIONAL</td>
 *   <td>STATE_INCOMPATIBLE_CONFIGURATION</td>
 *   <td>STATE_TERMINATING</td>
 *   <td>STATE_TERMINATED</td>
 *   <td>STATE_DIED</td>
 * </tr>
 * <tr ALIGN=CENTER NOSAVE>
 *   <td>STATE_NONE</td>
 *   <td align=center>x</td><td>x</td><td>-</td><td>-</td><td>-</td><td>-</td><td>-</td>
 * </tr>
 * <tr ALIGN=CENTER NOSAVE>
 *   <td>STATE_INITIALISING</td>
 *   <td>-</td><td>x</td><td>x</td><td>x</td><td>x</td><td>-</td><td>x</td>
 * </tr>
 * <tr ALIGN=CENTER NOSAVE>
 *   <td>STATE_OPERATIONAL</td>
 *   <td>-</td><td>-</td><td>x</td><td>-</td><td>x</td><td>-</td><td>x</td>
 * </tr>
 * <tr ALIGN=CENTER NOSAVE>
 *   <td>STATE_INCOMPATIBLE_CONFIGURATION</td>
 *   <td>-</td><td>-</td><td>-</td><td>x</td><td>x</td><td>-</td><td>x</td>
 * </tr>
 * <tr ALIGN=CENTER NOSAVE>
 *   <td>STATE_TERMINATING</td>
 *   <td>-</td><td>-</td><td>-</td><td>-</td><td>x</td><td>x</td><td>x</td>
 * </tr>
 * <tr ALIGN=CENTER NOSAVE>
 *   <td>STATE_TERMINATED</td>
 *   <td>-</td><td>-</td><td>-</td><td>-</td><td>-</td><td>x</td><td>x</td>
 * </tr>
 * <tr ALIGN=CENTER NOSAVE>
 *   <td>STATE_DIED</td>
 *   <td>-</td><td>x</td><td>-</td><td>-</td><td>-</td><td>-</td><td>x</td>
 * </tr>
 * </table>
 *
 *
 * \param _this     the service proxy to operate on.
 * \param newState  the new state for this service.
 *
 * \return TRUE,  if the state change is allowed and succeeded.<br>
 *         FALSE, if the state change is not allowed.
 */
OS_API u_bool
u_serviceChangeState(
    const u_service _this,
    const v_serviceStateKind newState);

/**
 * \brief Returns the state kind of the given service.
 *
 * The following kind of states are defined: <code>STATE_NONE</code>,
 * <code>STATE_INITIALISING</code>, <code>STATE_OPERATIONAL</code>,
 * <code>STATE_INCOMPATIBLE_CONFIGURATION</code>, <code>STATE_TERMINATING</code>,
 * <code>STATE_TERMINATED</code> and <code>STATE_DIED</code>.
 * The state <code>STATE_NONE</code> is returned when an error has occurred.
 *
 * \param _this   the service proxy to operate on.
 *
 * \return the state kind of the given service.
 */
OS_API v_serviceStateKind
u_serviceGetStateKind(
    const u_service _this);

/**
 * \brief Set/reset the listener that is called when the splice service terminates or dies.
 *
 * The given listener is called, when the splice service terminates or dies. With this
 * call the given user data (<code>usrData</code>) is also passed. When no listener is given
 * (<code>listener == NULL</code>) the listener previously set (if any) is reset.
 *
 * \param _this    The service proxy to operate on.
 * \param listener The listener that must be called.
 * \param usrData  Any pointer to data that is passed when the listener is called.
 *
 * \return U_RESULT_OK on a succesful operation <br>
 *         U_RESULT_ILL_PARAM if the specified proxy is incorrect.
 */
OS_API u_result
u_serviceWatchSpliceDaemon(
    const u_service _this,
    const u_serviceSplicedaemonListener listener,
    const void *usrData);

/**
 * \brief Renews the lease of the service.
 *
 * This method renews the lease of the service by adding the lease period
 * to the current time.
 *
 * \param _this       The service proxy to operate on.
 * \param leasePeriod Period to renew the lease with.
 *
 * \return U_RESULT_OK the service is disabled.<br>
 *         U_RESULT_ILL_PARAM if the specified service is incorrect.
 */
OS_API u_result
u_serviceRenewLease(
    const u_service _this,
    os_duration leasePeriod);

/**
 * \brief Registers u_serviceAtExitAction function.
 *
 * See the comment at the start of u_service.h for more information.
 *
 * \param _this   The service proxy to operate on.
 *
 * \return U_RESULT_OK the atExit action is registered.<br>
 *         U_RESULT_* otherwise.
 */
OS_API u_result
u_serviceAtExit(
    u_service _this,
    u_serviceAtExitAction action,
    void *privateData);

OS_API u_result
u_serviceFillNewGroups(
    const u_service _this);

typedef struct u_service_cmdopts
{
   char *uri;
   char *exeName;
   char *serviceName;
} u_service_cmdopts;

OS_API void
u_serviceCheckEnvURI(struct u_service_cmdopts *cmdopts);

OS_API u_domainId_t
u_serviceThreadGetDomainId(void);

OS_API void
u_serviceThreadSetDomainId(
     u_domainId_t domainId);

OS_API os_result
u_serviceThreadCreate(
    _Out_ os_threadId *threadId,
    _In_ const char *name,
    _In_ const os_threadAttr *threadAttr,
    _In_ os_threadRoutine start_routine,
    _In_ void *arg) __nonnull((1,2,3,4));

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
