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
 * Interface definition for memory mapped file of SPLICE-DDS    *
 ****************************************************************/

#ifndef OS_MEMMAPFILE_H_
#define OS_MEMMAPFILE_H_

/** \file os_memMapFile.h
 *  \brief memory mapped file management - create, open, attach,
 *         detach and destroy memory mapped file
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


/** \brief Handle for memory mapped file operations
 */
typedef struct os_mmfHandle_s *os_mmfHandle;


/** \brief Definition of the memory mapped file attributes
 */
typedef struct os_mmfAttr {
    /** User credentials */
    os_userCred  userCred;
    /** Preferred mapping address */
    void         *map_address;
} os_mmfAttr;


/** \brief Set the default memory mapped file attributes
 *
 * Postcondition:
 * - user credentials is set platform dependent
 * - mapping address is platform dependent
 *
 * Possible Results:
 * - assertion failure: sharedAttr = NULL
 * - returns os_resultSuccess
 */
OS_API os_result
os_mmfAttrInit(
		os_mmfAttr *mmfAttr);

/** \brief Create a handle for memory mapped file operations
 *
 * The identified \b name and \b mmfAttr values
 * are applied during creation of the memory mapped file.
 * The requested map address in \b mmfAttr is applied
 * during the attach function.
 */

OS_API os_mmfHandle
os_mmfCreateHandle(
    const char *filename,
    const os_mmfAttr *mmfAttr);

/** \brief Destroy a handle for memory mapped file operations
 */
OS_API void
os_mmfDestroyHandle(
		os_mmfHandle mmfHandle);

/** \brief Return the filename of the attached memory mapped file
 *         related to the handle
 */
OS_API const char *
os_mmfFilename(
		os_mmfHandle mmfHandle);

/** \brief Return the address of the attached memory mapped file
 *         related to the handle
 */
OS_API void *
os_mmfAddress(
		os_mmfHandle mmfHandle);

/** \brief Return the size of the attached memory mapped file
 *         related to the handle
 */
OS_API os_size_t
os_mmfSize(
		os_mmfHandle mmfHandle);

/** \brief Return true if memory mapped file related
 *         to the handle exists
 */
OS_API os_boolean
os_mmfFileExist (
		os_mmfHandle mmfHandle);


/** \brief Create and open a memory mapped file with the specified initial size.
 *
 * If the file already exists, the creation fails and this operation
 * returns an error status
 */
OS_API os_result
os_mmfCreate(
    os_mmfHandle mmfHandle,
    os_uint32 size);

/** \brief Open an existing memory mapped file.
 *
 * If the file doesn't exist, the open fails and this operation
 * returns an error status
 */
OS_API os_result
os_mmfOpen(
    os_mmfHandle mmfHandle);

/** \brief Close an open memory mapped file.
 */
OS_API os_result
os_mmfClose(
    os_mmfHandle mmfHandle);

/** \brief Resize the memory mapped file.
 */
OS_API os_result
os_mmfResize(
    os_mmfHandle mmfHandle,
    os_uint32 new_size);

/** \brief Attach the memory mapped file in memory.
 */
OS_API os_result
os_mmfAttach(
    os_mmfHandle mmfHandle);

/** \brief Detach the memory mapped file from memory.
 */
OS_API os_result
os_mmfDetach(
    os_mmfHandle mmfHandle);

/** \brief Force system to synchronize the attached memory
 *         to the memory mapped file.
 */
OS_API os_result
os_mmfSync(
    os_mmfHandle mmfHandle);

#undef OS_API
#if defined (__cplusplus)
}
#endif

#endif /* OS_MEMMAPFILE_H_ */

