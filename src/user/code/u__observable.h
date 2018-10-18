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
#ifndef U__OBSERVABLE_H
#define U__OBSERVABLE_H

#include "u_observable.h"
#include "u__types.h"
#include "u__handle.h"
#include "v_public.h"
/* exporting some functions from this header file is only needed,
 * since cmxml uses these functions
 */
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */


u_result
u_observableInit(
    const u_observable _this,
    const v_public vObject,
    const u_domain domain);

u_result
u__observableDeinitW(
    void *_this);

void
u__observableFreeW(
    void *_this);

u_result
u__observableProxyDeinitW(
    void *_this);

void
u__observableProxyFreeW(
    void *_this);

/**
 * \brief The User Layer Observable claim method for writing.
 *
 * This method will ask the kernel to get the reference to the kernel
 * observable associated to the User Layer observable that this method operates
 * on.
 * The kernel will register the kernel observable as being accessed and
 * therefore may not be deleted. If in case the requested kernel observable
 * doesn't exist anymore this method will return a proper return code
 * indicating the non-existence of the kernel observable.
 * In the event that shared memory has been exhausted, this operation will
 * return an OUT_OF_MEMORY result code.
 *
 * NOTE: This operation MUST be used when it is intended to write data into
 * shared memory, such as performing a write on a datawriter or creating new
 * entities. When the goal is to simple retrieve or read data then the
 * 'Read' variant of this operation should be used.
 *
 * \param _this The User layer Observable observable where this method operates
 *              on.
 * \param ke The associated Kernel Observable observable or NULL if the Kernel
 *         Observable observable does not exist anymore.
 * \return the result value of the claim operation
 */
OS_API u_result
u_observableWriteClaim(
    const u_observable _this,
          v_public* vObject,
          os_address memoryClaim);

/**
 * \brief The User Layer Observable claim method for reading.
 *
 * This method will ask the kernel to get the reference to the kernel
 * observable associated to the User Layer observable that this method operates
 * on.
 * The kernel will register the kernel observable as being accessed and
 * therefore may not be deleted. If in case the requested kernel observable
 * doesn't exist anymore this method will return a proper return code
 * indicating the non-existence of the kernel observable.
 * In the event that shared memory has been exhausted, this operation will
 * NOT return an OUT_OF_MEMORY result code.
 *
 * NOTE: This operation MUST be used when it is intended to read/remove data
 * from shared memory, such as performing a read on a datareader or deleting
 * entities. When the goal is to create or write data then the
 * 'Write' variant of this operation should be used.
 *
 * \param _this The User layer Observable where this method operates
 *              on.
 * \param ke The associated Kernel Observable or NULL if the Kernel
 *         Observable does not exist anymore.
 * \return the result value of the claim operation
 */
OS_API u_result
u_observableReadClaim(
    const u_observable _this,
          v_public* vObject,
          os_address memoryClaim);

/**
 * \brief The User Layer Observable claim method for triggering.
 *
 * This function requests a reference to the associated kernel observable.
 *
 * The kernel will mark the kernel observable as being used and therefore
 * cannot be deleted. If a reference to a non-existing observable is requested,
 * ALREADY_DELETED will be returned. In the event that shared memory has
 * been exhausted, this operation will NOT return OUT_OF_MEMORY. This operation
 * does NOT check if the splice daemon is running.
 *
 * NOTE: This operation currently MAY ONLY be used for triggering application
 * threads blocking on kernel conditions.
 *
 * \param _this The User Layer Observable to claim
 * \param vObject Associated kernel observable or NULL if already deleted
 * \return result value of the claim operation
 */
u_result
u_observableTriggerClaim(
    const u_observable _this,
    v_public *vObject,
    os_address memoryClaim);

/**
 * \brief The User Layer Observable Release method.
 *
 * This method will notify the kernel that the kernel observable associated
 * to the User Layer observable that this method operates on is of no
 * interest anymore, the kernel will then deregister the associated
 * kernel observable and if not referenced anymore it will be deleted.
 *
 * \param _this The User layer Observable where this method operates
 *              on.
 */
OS_API void
u_observableRelease(
    const u_observable _this,
    os_address memoryClaim);

u_domain
u_observableDomain (
    const u_observable _this);

u_handle
u_observableHandle (
    const u_observable _this);

#undef OS_API

#endif

