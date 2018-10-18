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

/** \file os/common/code/os_sharedmem_handle.c
 *  \brief sommon shared memory handle implementation
 *
 * Implements common and platform independent shared
 * memory handle functions
 */

#include "os_heap.h"
#include "os_stdlib.h"
#include "os_classbase.h"

#include <assert.h>

OS_CLASS(os_implData);

struct os_sharedHandle_s {
    os_sharedAttr attr;
    void *mapped_address;
    char *name;
    os_int32 id;
    char *keyfile;
    os_implData data;
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
    const os_sharedAttr *sharedAttr,
    const os_int32 id)
{
    os_sharedHandle sh;

    assert(name != NULL);
    assert(sharedAttr != NULL);
    sh = os_malloc (sizeof (struct os_sharedHandle_s));
    if (sh != NULL) {
        sh->name = os_strdup (name);
        if (sh->name != NULL) {
            sh->attr = *sharedAttr;
            sh->mapped_address = (void *)0;
            sh->id = id;
            sh->keyfile = NULL;
            os_sharedMemoryImplDataCreate(sh);
        } else {
            os_free (sh);
            sh = NULL;
        }
    }
    return sh;
}

void
os_sharedMemoryRegisterUserProcess(
    const os_char* domainName,
    os_procId pid)
{
    OS_UNUSED_ARG(domainName);
    OS_UNUSED_ARG(pid);
    /* does nothing */
    return;
}

/** \brief Destroy a handle for shared memory operations
 */
void
os_sharedDestroyHandle (
    os_sharedHandle sharedHandle)
{
    assert (sharedHandle != NULL);
    assert (sharedHandle->name != NULL);
    os_sharedMemoryImplDataDestroy(sharedHandle);
    os_free (sharedHandle->name);
    sharedHandle->name = NULL;
    os_free (sharedHandle->keyfile);
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

