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
#include "os_time.h"
#include "os_process.h"
#include "os_signal.h"
#include "os_iterator.h"

#ifdef OSPL_BUILD_CORE
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
#ifdef VXWORKS_RTP
    int needsErase;
#endif
} os_sharedAttr;

/** \brief Definition of states of os modules (like shared memory)
 */
typedef enum os_state {
    OS_STATE_NONE,
    OS_STATE_INITIALIZING,
    OS_STATE_OPERATIONAL,
    OS_STATE_TERMINATING,
    OS_STATE_TERMINATED
} os_state;


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
    const os_sharedAttr *sharedAttr,
    const os_int32 id);

OS_API void
os_sharedMemoryRegisterUserProcess(
    const os_char* domainName,
    os_procId pid);

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
 *     The shared memory is created successfully
 * - returns os_resultFail if
 *     The shared memory is not created because of a failure
 */
OS_API os_result
os_sharedMemoryCreate(
    os_sharedHandle sharedHandle,
    os_address size);


typedef enum {
    OS_SHM_PROC_ATTACHED,
    OS_SHM_PROC_DETACHED,
    OS_SHM_PROC_TERMINATED
} os_shmProcState;

OS_CLASS(os_shmClient);
OS_STRUCT(os_shmClient){
    os_procId procId;
    os_shmProcState state;
    os_shmClient next;
};

OS_API void
os_shmClientFree(os_shmClient client);

OS_API os_result
os_sharedMemoryWaitForClientChanges(
    os_sharedHandle sharedHandle,
    os_duration maxBlockingTime,
    os_shmClient* changedClients);

typedef void (*os_onSharedMemoryManagerDiedCallback)(os_sharedHandle sharedHandle, void *args);

OS_API os_result
os_sharedMemoryRegisterServerDiedCallback(
    os_sharedHandle sharedHandle,
    os_onSharedMemoryManagerDiedCallback onServerDied,
    void *args);

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
 *     The shared memory is destroyed successfully
 * - returns os_resultFail if
 *     The shared memory is not destroyed because of a failure
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
 *     The shared memory is successfully attached
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
 *     The shared memory is successfully detached
 * - returns os_resultFail if
 *     The shared memory is not detached because of a failure
 */
OS_API os_result
os_sharedMemoryDetach(
    os_sharedHandle sharedHandle);

/** \brief Detach from the named shared memory without cleanup
 *
 * Detach from the identified shared memory referenced by name
 * and sharedAttr and address. This function is to be used when
 * the process has to detach from shared memory but did not
 * cleanup it's shared memory resources. In this case the
 * shared memory monitor is not signaled that the process has
 * detached from shared memory and at the moment the process
 * terminates the shared memory monitor will be signaled and
 * will try to cleanup these resources.
 *
 * Precondition:
 * - sharedHandle is previously created with \b os_sharedCreateHandle
 *
 * Possible Results:
 * - assertion failure: sharedHandle = NULL ||
 *     sharedHandle is inconsistent
 * - returns os_resultSuccess if
 *     The shared memory is successfully detached
 * - returns os_resultFail if
 *     The shared memory is not detached because of a failure
 */
OS_API os_result
os_sharedMemoryDetachUnclean(
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
 */
OS_API void
os_sharedAttrInit(
        os_sharedAttr *sharedAttr)
    __nonnull_all__;


/** \brief retrieve the domain name belonging to a domain id
 *
 * Precondition:
 * - sharedHandle is previously created with \b os_sharedCreateHandle
 *
 * Possible Results:
 * - assertion failure: sharedHandle = NULL
 *     sharedHandle is inconsistent
 * - returns os_resultSuccess if
 *     The name is returned successful
 * - returns os_resultFail if
 *     The name could not be resolved
 */
OS_API os_result
os_sharedMemoryGetNameFromId(
    os_sharedHandle sharedHandle,
    char **name);

OS_API char *
os_findKeyFile(
    const char * name);

OS_API char *
os_findKeyFileById(
    os_int32 domainId);

OS_API char *
os_findKeyFileByNameAndId(
    const char *name,
    const os_int32 id);

OS_API void
os_cleanSharedMemAndOrKeyFiles(
    void);

OS_API os_int32
os_sharedMemoryListUserProcesses(
    os_iter pidList,
    const char * fileName);

OS_API os_int32
os_sharedMemoryListUserProcessesFree(
    os_iter pidList);

OS_API os_int32
os_sharedMemoryListDomainNames(
    os_iter nameList);

OS_API void
os_sharedMemoryListDomainNamesFree(
    os_iter nameList);

OS_API os_int32
os_sharedMemoryListDomainIds(
    os_int32 **idList,
    os_int32  *listSize);

OS_API os_int32
os_destroyKeyFile(
    const char * name);

OS_API os_int32
os_sharedMemorySegmentFree(
    const char * fname);

OS_API os_state
os_sharedMemoryGetState(
    os_sharedHandle sharedHandle);

OS_API os_result
os_sharedMemorySetState(
    os_sharedHandle sharedHandle,
    os_state state);

/*
 * Some platforms can have an race condition between creating and attaching the shared memory
 * There is implemented a lock file mechanism to make creation, attaching and filling the domain
 * in a locked action. (OSPL-9085)
 */
OS_API os_result
os_sharedMemoryLock(
    os_sharedHandle sharedHandle);

OS_API void
os_sharedMemoryUnlock(
    os_sharedHandle sharedHandle);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* OS_SHAREDMEM_H */
