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

/** \file os/common/code/os_sharedmem_handle.c
 *  \brief sommon shared memory handle implementation
 *
 * Implements common and platform independent shared
 * memory handle functions
 */

#include "os_heap.h"

#include <assert.h>

struct os_sharedHandle_s {
    os_sharedAttr attr;
    void *mapped_address;
    char *name;
};

/** \brief Create a handle for shared memory operations
 *
 * The identified \b name and \b sharedAttr values
 * are applied during creation of the shared memory.
 * The requested map address in \b sharedAttr is applied
 * during the attach function.
 */
os_sharedHandle
os_sharedCreateHandle (
    const char *name,
    const os_sharedAttr *sharedAttr)
{
    os_sharedHandle sh;

    assert(name != NULL);
    assert(sharedAttr != NULL);
    sh = os_malloc (sizeof (struct os_sharedHandle_s));
    if (sh != NULL) {
	sh->name = os_malloc (strlen(name) + 1);
	if (sh->name != NULL) {
	    os_strcpy (sh->name, name);
	    sh->attr = *sharedAttr;
	    sh->mapped_address = (void *)0;
	} else {
	    os_free (sh);
	    sh = NULL;
	}
    }
    return sh;
}

/** \brief Destroy a handle for shared memory operations
 */
void
os_sharedDestroyHandle (
    os_sharedHandle sharedHandle)
{
    assert (sharedHandle != NULL);
    assert (sharedHandle->name != NULL);
    os_free (sharedHandle->name);
    sharedHandle->name = NULL;
    os_free (sharedHandle);
    return;
}

/** \brief Return the address of the attached shared memory
 *         related to the handle
 */
void *
os_sharedAddress (
    os_sharedHandle sharedHandle)
{
    assert (sharedHandle != NULL);
    return sharedHandle->mapped_address;
}

