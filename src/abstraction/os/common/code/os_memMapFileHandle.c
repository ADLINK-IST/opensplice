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

/** \file os/common/code/os_memMapFileHandle.c
 *  \brief common memory mapped file handle implementation
 *
 * Implements common and platform independent
 * memory mapped file handle functions
 */

#include "os_heap.h"
#include <assert.h>

/** \brief Handle structure declaration
 *
 */
struct os_mmfHandle_s {
    os_mmfAttr attr;              /* platform specific configuration attributes */
    void       *mapped_address;   /* memory mapped address */
    char       *filename;         /* filename of memory mapped file */
    int        fd;                /* file descriptor of memory mapped file */
    os_size_t  size;              /* size of memory mapped file */
};

/** \brief Create a handle for memory mapped file operations
 *
 * The identified \b name and \b mmfAttr values
 * are applied during creation of the memory mapped file.
 * The requested map address in \b mmfAttr is applied
 * during the attach function.
 */
os_mmfHandle
os_mmfCreateHandle (
    const char *filename,
    const os_mmfAttr *mmfAttr)
{
	os_mmfHandle sh;
    assert(filename != NULL);
    assert(mmfAttr != NULL);

    sh = os_malloc (sizeof (struct os_mmfHandle_s));
    if (sh != NULL) {
		sh->filename = os_malloc (strlen(filename) + 1);
		if (sh->filename != NULL) {
			strcpy (sh->filename, filename);
			sh->attr = *mmfAttr;
			sh->mapped_address = (void *)0;
			sh->fd = 0;
			sh->size = 0;
		} else {
			os_free (sh);
			sh = NULL;
		}
    }
    return sh;
}

/** \brief Destroy a handle for memory mapped file operations
 */
void
os_mmfDestroyHandle (
    os_mmfHandle mmfHandle)
{
    assert (mmfHandle != NULL);
    assert (mmfHandle->filename != NULL);

    os_free (mmfHandle->filename);
    mmfHandle->filename = NULL;
    os_free (mmfHandle);

    return;
}

/** \brief Return the filename of the attached memory mapped file
 *         related to the handle
 */
const char *
os_mmfFilename (
		os_mmfHandle mmfHandle)
{
    assert (mmfHandle != NULL);

    return mmfHandle->filename;
}

/** \brief Return the address of the attached memory mapped file
 *         related to the handle
 */
void *
os_mmfAddress (
		os_mmfHandle mmfHandle)
{
    assert (mmfHandle != NULL);

    return mmfHandle->mapped_address;
}

/** \brief Return the size of the attached memory mapped file
 *         related to the handle
 */
os_size_t
os_mmfSize (
		os_mmfHandle mmfHandle)
{
    assert (mmfHandle != NULL);

    return mmfHandle->size;
}

/** \brief Return true if memory mapped file related
 *         to the handle exists
 */
os_boolean
os_mmfFileExist (
		os_mmfHandle mmfHandle)
{
    struct os_stat_s stat;
    assert (mmfHandle != NULL);
    assert (mmfHandle->filename != NULL);

    if (os_stat(mmfHandle->filename, &stat) == os_resultSuccess) {
    	return OS_ISREG(stat.stat_mode);
    }

    return OS_FALSE;
}
