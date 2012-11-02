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
/**
 * \file v_serviceManager.h
 * \brief
 *
 * Currently the service manager is passes all requests to the garbage 
 * collector. For A02 the lease mechanism will remain in the garbage collector,
 * while the service state management is moved to the service manager.
 */

#ifndef V_SERVICEMANAGER_H
#define V_SERVICEMANAGER_H

#include "kernelModule.h"

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
 * \brief The <code>v_serviceManager</code> cast method.
 *
 * This method casts an object to a <code>v_serviceManager</code> object.
 * Before the cast is performed, the type of the object is checked to
 * be <code>v_serviceManager</code>.
 */
#define v_serviceManager(o) (C_CAST(o,v_serviceManager))

/**
 * \brief The <code>v_serviceManager</code> constructor.
 * 
 * The constructor will create a new <code>v_serviceManager</code> object.
 *
 * \param kernel the kernel 
 *
 * \return NULL if memory allocation failed, otherwise a reference to a newly
 *         created  <code>v_serviceManager</code> object.
 */
OS_API v_serviceManager
v_serviceManagerNew(
    v_kernel kernel);

/**
 * \brief Retrieves the state kind of a given service.
 *
 * The state kind of the given service, identified by a name, is returned. When
 * the service is not known the state <code>STATE_NONE</code> is returned.
 *
 * \param serviceManager a reference to the service manager.
 * \param serviceName    the name of the service, which state kind is requested.
 *
 * \return The service state kind of the given service.
 */
OS_API v_serviceStateKind
v_serviceManagerGetServiceStateKind(
    v_serviceManager serviceManager,
    const c_char *serviceName);

/**
 * \brief Retrieves all known services with the given state kind.
 *
 * An iterator, containing references to all services with the given state
 * kind, is returned. When no services are found with the given state kind,
 * an empty iterator is returned. It is the responsibility of the caller to
 * free the references in the iterator appropriately.
 *
 * \param serviceManager a reference to the service manager.
 * \param kind           the state kind of interest.
 *
 * \return An iterator object containing the references to all services with
 *         the given state kind.
 */
OS_API c_iter
v_serviceManagerGetServices(
    v_serviceManager serviceManager,
    v_serviceStateKind kind);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* V_SERVICEMANAGER_H */
