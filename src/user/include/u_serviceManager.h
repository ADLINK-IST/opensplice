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
#ifndef U_SERVICEMANAGER_H
#define U_SERVICEMANAGER_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "u_types.h"
#include "u_dispatcher.h"
#include "v_serviceManager.h"
#include "os_if.h"

#ifdef OSPL_BUILD_USER
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
 * \brief Listeners of the service manager are called when one or more of
 *        the known services have changed their state.
 *
 * \param manager A proxy to the kernel service manager.
 * \param usrData Is the same pointer as given with the addition of the
 *                listener.
 */
typedef u_dispatcherListener u_serviceManagerListener;

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
    u_participant participant);


OS_API u_result
u_serviceManagerInit(
    u_serviceManager sm);

/** \brief The <code>u_serviceManager</code> destructor.
 *
 * The destructor frees the proxy to the kernel service manager.
 *
 * \param serviceManager The proxy to the kernel service manager.
 * \return U_RESULT_OK on a succesful operation or <br>
 *         U_RESULT_ILL_PARAM if the specified service manager is incorrect.
 */
OS_API u_result
u_serviceManagerFree(
    u_serviceManager serviceManager);

/**
 * \brief Retrieves the state kind of a given service.
 *
 * The state kind of the given service, identified by a name, is returned. When
 * the service is not known the state <code>STATE_NONE</code> is returned.
 *
 * \param serviceManager the proxy to the kernel service manager.
 * \param serviceName    the name of the service, which state kind is requested.
 *
 * \return The service state kind of the given service.
 */
OS_API v_serviceStateKind
u_serviceManagerGetServiceStateKind(
    u_serviceManager serviceManager,
    const c_char *serviceName);

/**
 * \brief Retrieves all known services with the given state kind.
 *
 * An iterator, containing all services with the given state kind, is returned.
 * When no services are found with the given state kind, an empty iterator is
 * returned.
 *
 * \param serviceManager the proxy to the kernel service manager.
 * \param kind           the state kind of interest.
 *
 * \return An iterator object containing all services with the given state kind.
 */
OS_API c_iter
u_serviceManagerGetServices(
    u_serviceManager serviceManager,
    v_serviceStateKind kind);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* U_SERVICE_H */
