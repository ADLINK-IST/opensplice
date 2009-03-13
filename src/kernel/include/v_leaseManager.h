#ifndef V_LEASEMANAGER_H
#define V_LEASEMANAGER_H

/** \file kernel/include/v_leaseManager.h
 *  \brief This file defines the interface
 *
 */

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"
#include "v_event.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * \brief The <code>v_leaseManager</code> cast method.
 *
 * This method casts an object to a <code>v_leaseManager</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_leaseManager</code> or
 * one of its subclasses.
 */
#define v_leaseManager(o) (C_CAST(o,v_leaseManager))

OS_API void
v_leaseManagerMain(
    v_leaseManager lm);
    
OS_API c_bool
v_leaseManagerNotify(
    v_leaseManager lm,
    v_lease lease,
    v_eventKind event);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* V_LEASEMANAGER_H */
