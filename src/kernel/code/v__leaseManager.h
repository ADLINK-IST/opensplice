/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */

#ifndef V__LEASEMANAGER_H
#define V__LEASEMANAGER_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "kernelModule.h"
#include "v_leaseManager.h"
#include "v_kernel.h"

/**
 * \brief This operation creates a new leaseManager object and inits it.
 *
 * \param k The kernel pointer so we know in which shared memory segment to
 *          allocate the new leaseManager in
 *
 * \return The newly allocated leaseManager object, or NULL if not enough
 *         memory was available to complete the operation.
 */
v_leaseManager
v_leaseManagerNew(
    v_kernel k);

/**
 * \brief This operation frees resources used by the lease manager.
 *
 * \param _this The lease manager to free.
 */
void
v_leaseManagerFree(
    v_leaseManager _this);

/**
 * \brief This operation will register a lease object to the set of observed
 * leases. The lease manager will determine if the expiry time of this newly
 * registered lease is earlier then already known leases, and if so it will
 * ensure that it only waits until the new lease is set to expire.
 * When a lease is registered an actionId is provided which indicates what the
 * lease manager should do when the lease expires. And an actionObject is also
 * provided which will be used when the action upon expiry is performed.
 * The lease manager can also be set to automatically repeat the lease once it
 * detects that it expires. Ideally only one lease manager should auto manage
 * a lease.
 *
 * \param _this The leaseManager object to register the lease object to.
 * \param lease The lease object to register to the set of observed leases
 * \param actionId The ID indicating what action to perform when the lease
 *                 expires.
 * \param actionObject The object to be used in the action executed when the
 *                     lease expires
 * \param repeatLease A boolean to indicate if the lease should be
 *                    repeated(TRUE) or not (FALSE).
 */
v_result
v_leaseManagerRegister(
    v_leaseManager  _this,
    v_lease         lease,
    v_leaseActionId actionId,
    v_public        actionObject,
    c_bool          repeatLease);

/* \brief This operation will remove a lease object from the set of observed
 * leases.
 *
 * \param _this The leaseManager object to remove the lease object from.
 * \param lease The lease object to remove from the set of observed leases
 */
void
v_leaseManagerDeregister(
    v_leaseManager _this,
    v_lease lease);

#if defined (__cplusplus)
}
#endif

#endif /* V__LEASEMANAGER_H */
