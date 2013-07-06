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
/** \file os/common/code/os_sharedmem_heap.c
 *  \brief Shared memory management - heap shared memory
 *
 * Implements shared memory management on heap
 */

#ifdef OS_SHAREDMEM_HEAP_DISABLE

os_result 
os_heap_sharedMemoryAttach (
    const char *name,
    const os_sharedAttr *sharedAttr,
    void **mapped_address)
{
    return os_resultFail;
}

os_result os_heap_sharedMemoryDestroy (const char *name)
{
    return os_resultFail;
}

os_result
os_heap_sharedMemoryCreate (
    const char *name,
    os_sharedAttr *sharedAttr,
    os_address size)
{
    return os_resultFail;
}

os_result os_heap_sharedMemoryDetach (const char *name, void *address)
{
    return os_resultFail;
}

os_result os_heap_sharedSize (const char *name, os_address *size)
{
    return os_resultFail;
}

#else

#include "os_mutex.h"
#include "os_heap.h"
#include "os_report.h"
#include "os_init.h"
#include "os_stdlib.h"

/** \brief Structure providing linked list for keeping
 *         shared memory data
 *
 * The structure defines an element of the linked list.
 */
typedef struct os_sm {
    /** Next element in the list */
    struct os_sm        *next;
    /** Name of the shared memory */
    char                *name;
    /** Address of the shared memory */
    void		*address;
    /** Size of the shared memory */
    os_uint32           size;
    /** Number of attachments to the shared memory */
    os_int32            nattach;
    /** Id of the shared memory */
    os_int32            id;
} os_sm;

/** Mutex for locking the shared memory data */
static os_mutex	os_smAdminLock;

/** Pointer to the linked list of shared memory data */
static os_sm    *os_smAdmin = NULL;

/** \brief Initialize the shared memory on heap data
 *
 * Initialize the mutex \b os_smAdminLock
 */
void
os_heap_sharedMemoryInit(void)
{
    os_mutexAttr mutexAttr;

    os_mutexAttrInit(&mutexAttr);
    mutexAttr.scopeAttr = OS_SCOPE_PRIVATE;
    os_mutexInit(&os_smAdminLock, &mutexAttr);
    return;
}

/** \brief Deinitialize the shared memory on heap data
 */
void
os_heap_sharedMemoryExit(void)
{
    /* It is assumed that the application has freed */
    /* all resources not required anymore before    */
    /* calling this function                        */
    os_mutexDestroy(&os_smAdminLock);
    return;
}

/** \brief Search an entry by name in the linked list
 *
 * Search an entry in the linked list of created named
 * shared memory entries by \b name. If found return the
 * address of the element else return \b NULL;
 *
 * It is assumed that the \b os_smAdminLock mutex is claimed
 * by the calling thread.
 */
static os_sm *
os_heap_search_entry(
    const char *name)
{
    os_sm *sm;
    os_sm *rv = NULL;

    sm = os_smAdmin;
    while (sm != NULL) {
        if (strcmp(sm->name, name) == 0) {
            rv = sm;
            sm = NULL;
        } else {
            sm = sm->next;
        }
    }
    return rv;
}

/** \brief Search an entry by name and id in the linked list
 *
 * Search an entry in the linked list of created named
 * shared memory entries by \b name and id. If found return the
 * address of the element else return \b NULL;
 *
 * It is assumed that the \b os_smAdminLock mutex is claimed
 * by the calling thread.
 */
static os_sm *
os_heap_search_entry_name_and_id(
    const char *name,
    const os_int32 id)
{
    os_sm *sm;
    os_sm *rv = NULL;

    sm = os_smAdmin;
    while (sm != NULL) {
        if ((strcmp(sm->name, name) == 0) && sm->id == id) {
            rv = sm;
            sm = NULL;
        } else {
            sm = sm->next;
        }
    }
    return rv;
}

os_result
os_heap_sharedMemoryGetNameFromId(
    os_int32 id,
    char **name)
{

    os_sm *sm;
    os_result rv = os_resultFail;
    sm = os_smAdmin;
    *name = NULL;
    while (sm != NULL) {
        if (sm->id == id) {
            *name =  os_strdup(sm->name);
            rv = os_resultSuccess;
	    break;
        } else {
            sm = sm->next;
        }
    }
    return rv;
}

/** \brief Add an entry by address to the linked list
 *
 * Add an entry \b sm at the front of the linked list.
 *
 * It is assumed that the \b os_smAdminLock mutex is claimed
 * by the calling thread.
 */
static void
os_heap_add_entry(
    os_sm *sm)
{
    sm->next = os_smAdmin;
    os_smAdmin = sm;
    return;
}

/** \brief Remove an entry by name from the linked list
 *
 * Remove an entry by name from thethnked list by first
 * searching the entry. When found remove the entry from
 * the list by correcting the pointers of the list.
 * The address of the entry is returned to the calling
 * thread, which is expected to release all claimed
 * resources related to the entry.
 *
 * It is assumed that there are no attachments to
 * the entry and the \b os_smAdminLock mutex is claimed
 * by the calling thread.
 */
static os_sm *
os_heap_remove_entry(
    const char *name)
{
    os_sm *sm = os_smAdmin;
    os_sm *psm = NULL;

    if (sm->next == NULL) {
        if (strcmp(sm->name, name) == 0) {
            psm = sm;
            os_smAdmin = NULL;
        } else {
            psm = NULL;
        }
    } else {
        psm = sm;
        sm = sm->next;
        while (sm != NULL) {
            if (strcmp(sm->name, name) == 0) {
                psm->next = sm->next;
                sm->next = NULL;
                psm = sm;
                sm = NULL;
            } else {
                psm = sm;
                sm = sm->next;
            }
        }
    }
    return psm;
}

/** \brief Create named shared memory on the heap
 *
 * Create named shared memory on the heap by \b name.
 * The \b sharedAttr attributes are ignored for this
 * implementation. The claimed memory is expected
 * to be of size \b size.
 *
 * First lock the shared memory data list by claiming
 * \b os_smAdminLock. Then search in the list to find
 * an entry with the same \b name by calling \b os_heap_search_entry.
 * If not found, create an entry \b sm and set \b sm->nattach
 * to 0, \b sm->name to \b name and claim \b size bytes
 * from heap for the named shared memory and set \b sm->address.
 * If this all succeeds, add the entry to the list by calling
 * \b os_heap_add_entry and release the list by releasing
 * \b os_smAdminLock. \b os_resultSuccess is returned
 * to the calling thread.
 *
 * If an entry with the same name already exists,
 * or there are no sufficient resources to create
 * the named shared memory, \b os_resultFail is returned
 * to the calling thread and no resources are allocated.
 */
os_result
os_heap_sharedMemoryCreate(
    const char *name,
    const os_sharedAttr *sharedAttr,
    os_address size,
    const os_int32 id)
{
    os_sm *sm;
    os_result rv = os_resultFail;
    (void)sharedAttr;

    OS_UNUSED_ARG(sharedAttr);

    os_mutexLock(&os_smAdminLock);
    sm = os_heap_search_entry_name_and_id(name,id);
    if (sm == NULL) {
        sm = (os_sm *)os_malloc(sizeof(os_sm));
        if (sm != NULL) {
            sm->nattach = 0;
            sm->size = size;
            sm->name = os_malloc((unsigned int)(strlen (name) + 1));
            sm->id = id;
            if (sm->name) {
                os_strcpy(sm->name, name);
                sm->address = os_malloc(size);
                if (sm->address) {
                    os_heap_add_entry(sm);
                    rv = os_resultSuccess;
                } else {
                    os_free(sm->name);
                    os_free(sm);
                    OS_REPORT_1(OS_ERROR, "os_heap_sharedMemoryCreate", 1, "Out of heap memory (%s)", name);
                }
            } else {
                os_free(sm);
                OS_REPORT_1(OS_ERROR, "os_heap_sharedMemoryCreate", 1, "Out of heap memory (%s)", name);
            }
        } else {
            OS_REPORT_1(OS_ERROR, "os_heap_sharedMemoryCreate", 1, "Out of heap memory (%s)", name);
        }
    }
    os_mutexUnlock(&os_smAdminLock);
    return rv;
}

/** \brief Destroy named shared memory on the heap
 *
 * Destroy named shared memory on the heap by \b name
 * and release all related resources.
 *
 * First lock the shared memory data list by claiming
 * \b os_smAdminLock. Then search in the list to find
 * an entry with the same \b name by calling \b os_heap_search_entry.
 * If not found, return \b os_resultFail indicating a failure
 * after releasing the list.
 * If \b sm-nattach is > 0 there are still attachements,
 * return \b os_resultFail indicating a failure
 * after releasing the list.
 * If the entry is found, remove the entry from the
 * list and release the list by calling \b os_remove_entry.
 * Then free the shared memory (\b sm->address), the
 * name (\b sm->name) and the entry itself (\b sm).
 */
os_result
os_heap_sharedMemoryDestroy(
    const char *name)
{
    os_sm *sm;
    os_result rv;

    os_mutexLock(&os_smAdminLock);
    sm = os_heap_search_entry(name);
    if (sm == NULL) {
        os_mutexUnlock(&os_smAdminLock);
        rv = os_resultFail;
        OS_REPORT_1(OS_ERROR, "os_heap_sharedMemoryDestroy", 2, "Entry not found by name (%s)", name);
    } else if (sm->nattach > 0) {
        os_mutexUnlock(&os_smAdminLock);
        rv = os_resultFail;
        OS_REPORT_1(OS_ERROR, "os_heap_sharedMemoryDestroy", 3, "Still users attached (%s)", name);
    } else {
        sm = os_heap_remove_entry(name);
        os_mutexUnlock(&os_smAdminLock);
        os_free(sm->address);
        os_free(sm->name);
        os_free(sm);
        rv = os_resultSuccess;
    }
    return rv;
}

/** \brief Attach named shared memory on the heap
 *
 * Attach to the named shared memory on the heap by \b name
 * and return the address of the memory in \b mapped_address.
 *
 * First lock the shared memory data list by claiming
 * \b os_smAdminLock. Then search in the list to find
 * an entry with name \b name by calling \b os_heap_search_entry.
 * If not found, return \b os_resultFail indicating a failure
 * after releasing the list.
 * If the entry is found, return the address in *mapped_address,
 * increase \b sm->nattach to indicate the number of attachments
 * and release the list before returning \b os_resultSuccess.
 */
os_result
os_heap_sharedMemoryAttach(
    const char *name,
    void **mapped_address)
{
    os_sm *sm;
    os_result rv;

    os_mutexLock(&os_smAdminLock);
    sm = os_heap_search_entry(name);
    if (sm == NULL) {
        os_mutexUnlock(&os_smAdminLock);
        rv = os_resultFail;
	/* OS_REPORT_1(OS_ERROR, "os_heap_sharedMemoryAttach", 2, "Entry not found by name (%s)", name); */
    } else {
        *mapped_address = sm->address;
        sm->nattach++;
        os_mutexUnlock(&os_smAdminLock);
        rv = os_resultSuccess;
    }
    return rv;
}

/** \brief Detach named shared memory on the heap
 *
 * Detach from the named shared memory on the heap by \b name.
 *
 * First lock the shared memory data list by claiming
 * \b os_smAdminLock. Then search in the list to find
 * an entry with name \b name by calling \b os_heap_search_entry.
 * If not found, return \b os_resultFail indicating a failure
 * after releasing the list.
 * If the entry is found, decrease \b sm->nattach to indicate
 * the number of attachemnts and release the list before
 * returning \b os_resultSuccess.
 */
os_result
os_heap_sharedMemoryDetach(
    const char *name,
    void *address)
{
    os_sm *sm;
    os_result rv;
    (void)address;

    OS_UNUSED_ARG(address);

    os_mutexLock(&os_smAdminLock);
    sm = os_heap_search_entry(name);
    if (sm == NULL) {
        os_mutexUnlock(&os_smAdminLock);
        rv = os_resultFail;
        OS_REPORT_1(OS_ERROR, "os_heap_sharedMemoryAttach", 2, "Entry not found by name (%s)", name);
    } else {
        sm->nattach--;
        os_mutexUnlock(&os_smAdminLock);
        rv = os_resultSuccess;
    }
    return rv;
}

os_result
os_heap_sharedSize(
    const char *name,
    os_address *size)
{
    os_sm *sm;
    os_result rv = os_resultSuccess;

    /* The os_sharedmem_heap abstraction is not used exclusively for single
     * process deployments, as some operating systems may use this as their
     * default memory configuration despite not being explicitly configured as
     * a single process deployment.
     */

    if (os_serviceGetSingleProcess()) {
#ifdef __x86_64__
        *size = 0xFFFFFFFFFFFFFFFF; /* maximal address on 64bit systems */
#else
        *size = 0xFFFFFFFF; /* maximal address on 32bit systems */
#endif
    } else {
        os_mutexLock(&os_smAdminLock);
        sm = os_heap_search_entry(name);
        if (sm == NULL) {
            os_mutexUnlock(&os_smAdminLock);
            rv = os_resultFail;
            OS_REPORT_1(OS_ERROR, "os_heap_sharedSize", 2, "Entry not found by name (%s)", name);
        } else {
            *size = sm->size;
            os_mutexUnlock(&os_smAdminLock);
            rv = os_resultSuccess;
        }
    }

    return rv;
}

#endif /* OS_SHAREDMEM_HEAP_DISABLE */
