/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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

#ifndef V__LEASEMANAGER_H
#define V__LEASEMANAGER_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "kernelModuleI.h"
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
