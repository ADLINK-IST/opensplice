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

#ifndef V__LEASE_H
#define V__LEASE_H

#include "kernelModuleI.h"
#include "v__leaseTime.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * \brief The <code>v_lease</code> cast method.
 *
 * This method casts an object to a <code>v_lease</code> object.
 * Before the cast is performed, if compiled with the NDEBUG flag not set,
 * the type of the object is checked to be <code>v_lease</code> or
 * one of its subclasses.
 */
#define v_lease(o) (C_CAST(o,v_lease))

/**
 * \brief The <code>v_leaseGetKind</code> cast method.
 *
 * This method returns the kind of the lease which indicates
 * if the lease is associated with a MONOTONIC or an ELAPSED clock.
 */
#define v_leaseGetKind(l) (v_leaseTimeKind((l)->expiryTime))

/**
 * \brief This operation creates a new lease object and inits it.
 *
 * \param k The kernel pointer so we know in which shared memory segment to
 *          allocate the new lease in
 * \param kind The kind of the lease (MONONIC or ELAPSED)
 * \param leaseDuration The duration time that the lease is valid, used to
 *                      calculate the expiry time of the lease.
 *
 * \return The newly allocated lease object, or NULL if not enough
 *         memory was available to complete the operation.
 */
v_lease
v_leaseNew (
    v_kernel k,
    v_leaseKind kind,
    os_duration leaseDuration);

/**
 * \brief This operation creates a new lease object associated with
 * the MONOTONIC clock and inits it.
 *
 * \param k The kernel pointer so we know in which shared memory segment to
 *          allocate the new lease in
 * \param leaseDuration The duration time that the lease is valid, used to
 *                      calculate the expiry time of the lease.
 *
 * \return The newly allocated lease object, or NULL if not enough
 *         memory was available to complete the operation.
 */
v_lease
v_leaseMonotonicNew(
    v_kernel k,
    os_duration leaseDuration);

/**
 * \brief This operation creates a new lease object associated with
 * the ELAPSED clock and inits it.
 *
 * \param k The kernel pointer so we know in which shared memory segment to
 *          allocate the new lease in
 * \param leaseDuration The duration time that the lease is valid, used to
 *                      calculate the expiry time of the lease.
 *
 * \return The newly allocated lease object, or NULL if not enough
 *         memory was available to complete the operation.
 */
v_lease
v_leaseElapsedNew(
    v_kernel k,
    os_duration leaseDuration);

/**
 * \brief This operation deinits the lease
 *
 * \param _this The lease object to deinit
 */
void
v_leaseDeinit (
    v_lease _this);

/**
 * \brief This operation updates the expiry time of the lease with a (potential)
 * new duration and stores this new duration in the lease. If the expiryTime of
 * the lease now is earlier then the expiryTime before the renew, then the
 * observers will be notified of an earlier expiryTime. Otherwise no notification
 * will be done.
 *
 * \param _this The lease object of which to update the expiry time (!= NULL)
 * \param leaseDuration The new lease duration, if no new duration is needed
 *                      then this param may be NULL.
 */
void
v_leaseRenew (
    v_lease _this,
    os_duration leaseDuration);

/**
 * \brief This operation returns the expiry time of this lease.
 *
 * \param _this The lease object of which to get the expiry time . (!= NULL)
 */
OS_API v_leaseTime
v_leaseExpiryTime (
    v_lease _this);

/**
 * \brief This operation returns the expiry time of this lease. Before this
 * function is used the v_leaseLock operation must be called!
 *
 * \param _this The lease object of which to get the expiry time . (!= NULL)
 */
v_leaseTime
v_leaseExpiryTimeNoLock(
    v_lease _this);

/**
 * \brief This operation returns the lease duration.
 *
 * \param _this The lease object of which to get the duration. (!= NULL)
 */
os_duration
v_leaseDuration(
    v_lease _this);

/**
 * \brief This operation returns the lease duration. Before this
 * function is used the v_leaseLock operation must be called
 *
 * \param _this The lease object of which to get the duration. (!= NULL)
 */
os_duration
v_leaseDurationNoLock(
    v_lease _this);

/**
 * \brief This operation locks the mutex of the v_lease. This operation is
 * available to allow the lease to be kept in a consistent state while
 * the leaseManager updates it's administration with the lease information.
 *
 * \param _this The lease object to be locked. (!= NULL)
 */
void
v_leaseLock(
    v_lease _this);

/**
 * \brief This operation unlocks the mutex of the v_lease.
 *
 * \param _this The lease object to be unlocked. (!= NULL)
 */
void
v_leaseUnlock(
    v_lease _this);

/**
 * \brief This operation adds an observer to this lease. Before this function
 * is used the v_leaseLock operation must be called!
 *
 * \param _this The lease object on which to add the observer. (!= NULL)
 * \param observer The lease manager object to be added to the observer list
 *                 (!= NULL)
 *
 * \return TRUE if the observer was succesfully added, FALSE otherwise.
 */
c_bool
v_leaseAddObserverNoLock(
    v_lease _this,
    v_leaseManager observer);

/**
 * \brief This operation removes an observer from this lease. If the observer
 * was not registered, this operation is a no-op and will return FALSE. Before
 * this function is used the v_leaseLock operation must be called!
 *
 * \param _this The lease object on which to remove the observer. (!= NULL)
 * \param observer The lease manager object to be removed from the observer list
 *                 (!= NULL)
 *
 * \return TRUE if the observer was succesfully removed, FALSE otherwise.
 */
c_bool
v_leaseRemoveObserverNoLock(
    v_lease _this,
    v_leaseManager observer);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* V__LEASE_H */
