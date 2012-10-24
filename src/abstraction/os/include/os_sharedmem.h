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
/****************************************************************
 * Interface definition for named shared memory of SPLICE-DDS   *
 ****************************************************************/

#ifndef OS_SHAREDMEM_H
#define OS_SHAREDMEM_H

/** \file os_sharedmem.h
 *  \brief Shared memory management - create, attach, detach and
 *         destroy shared memory
 */

#if defined (__cplusplus)
extern "C" {
#endif

#include "os_defs.h"
#include "os_if.h"

#ifdef OSPL_BUILD_OS
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/** \brief Handle for shared memory operations
 */
typedef struct os_sharedHandle_s *os_sharedHandle;

/** \brief Definition of implementation selection options
 *         for the shared memory
 */
typedef enum os_sharedImpl {
    /** Map the shared memory on a file */
    OS_MAP_ON_FILE,
    /** Map the shared memory on SVR4 shared memory segment */
    OS_MAP_ON_SEG,
    /** Map the shared memory on heap */
    OS_MAP_ON_HEAP
} os_sharedImpl;

/** \brief Definition of the shared memory attributes
 */
typedef struct os_sharedAttr {
    /** Policy for locking pages in physical memory */
    os_lockPolicy	lockPolicy;
    /** Implementation selection */
    os_sharedImpl	sharedImpl;
    /** User credentials */
    os_userCred		userCred;
    /** Preferred mapping address */
    void		*map_address;
} os_sharedAttr;

/** \brief Create a handle for shared memory operations
 *
 * The identified \b name and \b sharedAttr values
 * are applied during creation of the shared memory.
 * The requested map address in \b sharedAttr is applied
 * during the attach function.
 *
 * Possible Results:
 * - assertion failure: name = NULL || sharedAttr = NULL
 * - returns NULL if insufficient resources are available
 * - returns != NULL if a valid handle is created
 */
OS_API os_sharedHandle
os_sharedCreateHandle(
    const char *name,
    const os_sharedAttr *sharedAttr);

/** \brief Destroy a handle for shared memory operations
 *
 * Precondition:
 * - sharedHandle is previously created with \b os_sharedCreateHandle
 *
 * Possible Results:
 * - assertion failure: sharedHandle = NULL ||
 *     sharedHandle is inconsistent
 */
OS_API void
os_sharedDestroyHandle(
    os_sharedHandle sharedHandle);

/** \brief Return the address of the attached shared memory
 *         related to the handle
 *
 * Possible Results:
 * - assertion failure: sharedHandle = NULL ||
 *     sharedHandle is inconsistent
 * - returns NULL if sharedHandle relates to unattached shared memory
 * - returns address of the shared memory if sharedHandle relates to
 *     attached shared memory
 */
OS_API void *
os_sharedAddress(
    os_sharedHandle sharedHandle);

/** \brief Return the size of the attached shared memory
 *         related to the handle
 *
 * Possible Results:
 * - assertion failure: sharedHandle = NULL ||
 *     sharedHandle is inconsistent
 * - returns os_resultFail if sharedHandle relates to unattached shared memory
 * - returns os_resultSuccess if sharedHandle relates to attached shared memory
 *                            and stores the size in the size parameter.
 */
OS_API os_result
os_sharedSize(
    os_sharedHandle sharedHandle,
    os_address *size);

/** \brief Create shared memory
 *
 * Shared memory is created according the attributes related
 * to \b sharedHandle
 *
 * Precondition:
 * - sharedHandle is previously created with \b os_sharedCreateHandle
 *
 * Possible Results:
 * - assertion failure: sharedHandle = NULL ||
 *     sharedHandle is inconsistent
 * - returns os_resultSuccess if
 *     The shared memory is created successfuly
 * - returns os_resultFail if
 *     The shared memory is not created because of a failure
 */
OS_API os_result
os_sharedMemoryCreate(
    os_sharedHandle sharedHandle,
    os_address size);

/** \brief Destroy shared memory
 *
 * Shared memory is destroyed and related resources are freed.
 * Depending on the operating system and situation the kernel
 * may decide to keep the resources until the last process is
 * detached.
 *
 * Precondition:
 * - sharedHandle is previously created with \b os_sharedCreateHandle
 *
 * Possible Results:
 * - assertion failure: sharedHandle = NULL ||
 *     sharedHandle is inconsistent
 * - returns os_resultSuccess if
 *     The shared memory is destroyed successfuly
 * - returns os_resultFail if
 *     The shared memory is not detroyed because of a failure
 */
OS_API os_result
os_sharedMemoryDestroy(
    os_sharedHandle sharedHandle);

/** \brief Attach to the named shared memory
 *
 * Attach to the identified shared memory referenced by name
 * and sharedAttr. The address it is mapped to is returned
 * in address.
 *
 * Precondition:
 * - sharedHandle is previously created with \b os_sharedCreateHandle
 *
 * Possible Results:
 * - assertion failure: sharedHandle = NULL ||
 *     sharedHandle is inconsistent
 * - returns os_resultSuccess if
 *     The shared memory is successfuly attached
 * - returns os_resultFail if
 *     The shared memory is not attached because of a failure
 */
OS_API os_result
os_sharedMemoryAttach(
    os_sharedHandle sharedHandle);

/** \brief Detach from the named shared memory
 *
 * Detach from the identified shared memory referenced by name
 * and sharedAttr and address.
 *
 * Precondition:
 * - sharedHandle is previously created with \b os_sharedCreateHandle
 *
 * Possible Results:
 * - assertion failure: sharedHandle = NULL ||
 *     sharedHandle is inconsistent
 * - returns os_resultSuccess if
 *     The shared memory is successfuly detached
 * - returns os_resultFail if
 *     The shared memory is not detached because of a failure
 */
OS_API os_result
os_sharedMemoryDetach(
    os_sharedHandle sharedHandle);

/** \brief Set the default shared memory attributes
 *
 * Postcondition:
 * - locking policy is set platform dependent
 * - implementation selection is set platform dependent
 * - user credentials is set platform dependent
 * - mapping address is platform dependent
 *
 * Possible Results:
 * - assertion failure: sharedAttr = NULL
 * - returns os_resultSuccess
 */
OS_API os_result
os_sharedAttrInit(
    os_sharedAttr *sharedAttr);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* OS_SHAREDMEM_H */
