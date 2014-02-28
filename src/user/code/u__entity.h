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
#ifndef U__ENTITY_H
#define U__ENTITY_H

#include "u_entity.h"
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

#define U_ECREATE_OWNED         (0x01U << 0)
#define U_ECREATE_NOTOWNED      (0x01U << 1)
#define U_ECREATE_INITIALISED   (0x01U << 2)

#define u_entityAlloc(participant,_type,entity,_owner) \
        _type(u_entityNew(v_entity(entity),participant,_owner))

void
u_entityDealloc (
    u_entity _this);

u_result
u_entityInit (
    u_entity _this,
    v_entity ke,
    u_participant p,
    c_bool owner);

u_result
u_entityDeinit (
    u_entity _this);

/**
 * \brief The User Layer Entity claim method for writing.
 *
 * This method will ask the kernel to get the reference to the kernel
 * entity associated to the User Layer entity that this method operates
 * on.
 * The kernel will register the kernel entity as being accessed and
 * therefore may not be deleted. If in case the requested kernel entity
 * doesn't exist anymore this method will return a proper return code
 * indicating the non-existence of the kernel entity.
 * In the event that shared memory has been exhausted, this operation will
 * return an OUT_OF_MEMORY result code.
 *
 * NOTE: This operation MUST be used when it is intended to write data into
 * shared memory, such as performing a write on a datawriter or creating new
 * entities. When the goal is to simple retrieve or read data then the
 * 'Read' variant of this operation should be used.
 *
 * \param _this The User layer Entity object where this method operates
 *              on.
 * \param ke The associated Kernel Entity object or NULL if the Kernel
 *         Entity object does not exist anymore.
 * \return the result value of the claim operation
 */
u_result
u_entityWriteClaim(
    u_entity _this,
    v_entity* ke);

/**
 * \brief The User Layer Entity claim method for writing.
 *
 * This method will ask the kernel to get the reference to the kernel
 * entity associated to the User Layer entity that this method operates
 * on.
 * The kernel will register the kernel entity as being accessed and
 * therefore may not be deleted. If in case the requested kernel entity
 * doesn't exist anymore this method will return a proper return code
 * indicating the non-existence of the kernel entity.
 * In the event that shared memory has been exhausted, this operation will
 * NOT return an OUT_OF_MEMORY result code.
 *
 * NOTE: This operation MUST be used when it is intended to read/remove data
 * from shared memory, such as performing a read on a datareader or deleting
 * entities. When the goal is to create or write data then the
 * 'Write' variant of this operation should be used.
 *
 * \param _this The User layer Entity object where this method operates
 *              on.
 * \param ke The associated Kernel Entity object or NULL if the Kernel
 *         Entity object does not exist anymore.
 * \return the result value of the claim operation
 */
OS_API u_result
u_entityReadClaim(
    u_entity e,
    v_entity* ke);

/**
 * \brief The User Layer Entity Release method.
 *
 * This method will notify the kernel that the kernel entity associated
 * to the User Layer entity that this method operates on is of no
 * interest anymore, the kernel will then deregister the associated
 * kernel entity and if not referenced anymore it will be deleted.
 *
 * \param _this The User layer Entity object where this method operates
 *              on.
 */
OS_API u_result
u_entityRelease(
    u_entity _this);

u_result
u_entityLock(
    u_entity e);

u_result
u_entityUnlock(
    u_entity e);

#undef OS_API

#endif

