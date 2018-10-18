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
#ifndef U_SERVICEMANAGER_H
#define U_SERVICEMANAGER_H

#include "u_types.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "v_serviceManager.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define SERVICE_SPLICE_NAME     "Splicedaemon"
#define SERVICE_NETWORK_NAME    "Network"
#define SERVICE_TRANSIENT_NAME  "Transient"
#define SERVICE_PERSISTENT_NAME "Persistent"

/**
 * \brief The <code>u_serviceManager</code> cast method.
 *
 * This method casts an object to a <code>u_serviceManager</code> object.
 * Since objects of the user layer are allocated on heap, no type checking
 * is performed.
 */
#define u_serviceManager(o) ((u_serviceManager)(o))

/**
 * \brief Creates a proxy to the service manager in the kernel.
 *
 * For creating a proxy object representing to the service manager, a
 * participant object has to be given. The participant defines a connection
 * to the kernel, which is used to create a proxy to the service manager
 * of that kernel.
 *
 * \param participant the participant defining the kernel connection.
 *
 * \return the created proxy to the service manager if the creation succeeded,
 *         otherwise <code>NULL</code>
 */
OS_API u_serviceManager
u_serviceManagerNew(
    const u_participant participant);


/**
 * \brief Retrieves the state kind of a given service.
 *
 * The state kind of the given service, identified by a name, is returned. When
 * the service is not known the state <code>STATE_NONE</code> is returned.
 *
 * \param _this        the proxy to the kernel service manager.
 * \param serviceName  the name of the service, which state kind is requested.
 *
 * \return The service state kind of the given service.
 */
OS_API v_serviceStateKind
u_serviceManagerGetServiceStateKind(
    const u_serviceManager _this,
    const os_char *serviceName);

/**
 * \brief Retrieves all known services with the given state kind.
 *
 * An iterator, containing all services with the given state kind, is returned.
 * When no services are found with the given state kind, an empty iterator is
 * returned.
 *
 * \param _this  the proxy to the kernel service manager.
 * \param kind   the state kind of interest.
 *
 * \return An iterator object containing all services with the given state kind.
 */
OS_API c_iter
u_serviceManagerGetServices(
    const u_serviceManager _this,
    const v_serviceStateKind kind);

/**
 * \brief Removes the service with a given name from the servicemanager set.
 *
 * A service with the given name will be removed from the serviceSet.
 * This function should be called when the service kind of the service is changed to STATE_DIED
 *
 * \param _this  a reference to the service manager.
 * \param name   the name of the service.
 *
 * \return a boolean with the value TRUE if the remove was successful and FALSE if not.
 */
OS_API c_bool
u_serviceManagerRemoveService(
    const u_serviceManager _this,
    const c_char *serviceName);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
