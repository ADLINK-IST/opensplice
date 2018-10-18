/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

/** \file os/vxworks6.6/code/os_sharedmem.c
 *  \brief vxWorks shared memory management
 *
 * Implements shared memory management for vxWorks by
 * including the POSIX, SVR4 and heap implementation
 */

#include <sdLib.h>
#include <sdLibCommon.h>
#include "os_mutex.h"
#include "os_heap.h"
#include "os_report.h"
#include <string.h>
#include <assert.h>
#include "os_errno.h"
#include <strings.h>
#include "../common/code/os_sharedmem_handle.c"
#include "../common/code/os_sharedmem_heap.c"

typedef struct os_sdidMap {
    /** Next element in the list */
    struct os_sdidMap *next;
    /** Name of the shared memory */
    char              *name;
    /** Address of the shared memory */
    SD_ID              shmid;
    int                id;
    os_state           state;
} os_sdid;

/** Mutex for locking the shared memory data */
static os_mutex os_sdidAdminLock;

/** Pointer to the linked list of shared memory data */
static os_sdid *os_sdidAdmin = NULL;

static os_sdid *
os_sdid_find(
    const char *name)
{
    os_sdid *rv = NULL;
    os_sdid *sdid;

    os_mutexLock(&os_sdidAdminLock);
    sdid = os_sdidAdmin;

    while (sdid != NULL) {
        if (strcmp(sdid->name, name) == 0) {
            rv = sdid;
            sdid = NULL;
        } else {
            sdid = sdid->next;
        }
    }
    os_mutexUnlock(&os_sdidAdminLock);

    return rv;
}

static os_sdid *
os_sdid_find_by_id(
    const int id)
{
    os_sdid *rv = NULL;
    os_sdid *sdid;

    os_mutexLock(&os_sdidAdminLock);
    sdid = os_sdidAdmin;

    while (sdid != NULL) {
        if (sdid->id == id) {
            rv = sdid;
            sdid = NULL;
        } else {
            sdid = sdid->next;
        }
    }
    os_mutexUnlock(&os_sdidAdminLock);

    return rv;
}

static os_result
os_sdid_add(
    const char *name,
    SD_ID shmid,
    const int id)
{
    os_result osr;

    os_sdid *osd;

    osd = os_malloc(sizeof(os_sdid));
    if(osd){
        osd->name = os_malloc((unsigned int)(strlen (name) + 1));
        if (osd->name) {
            os_strcpy(osd->name, name);
            osd->shmid = shmid;
            osd->id = id;
            osd->state = OS_STATE_NONE;

            os_mutexLock(&os_sdidAdminLock);
            osd->next    = os_sdidAdmin;
            os_sdidAdmin = osd;
            os_mutexUnlock(&os_sdidAdminLock);
            osr = os_resultSuccess;
        } else {
            os_free(osd);
            osr = os_resultFail;
        }
    } else {
        osr = os_resultFail;
    }
    return osr;
}

static os_sdid *
os_sdid_remove(
    const char *name)
{
    os_sdid *sm = os_sdidAdmin;
    os_sdid *rsm = NULL;
    os_sdid *psm = NULL;

    os_mutexLock(&os_sdidAdminLock);

    while( (sm != NULL) && (rsm == NULL)){
        if (strcmp(sm->name, name) == 0) {
            rsm = sm;

            if(psm == NULL){
                os_sdidAdmin = sm->next;
            } else {
                psm->next = sm->next;
            }
        } else {
            psm = sm;
            sm = sm->next;
        }
    }
    os_mutexUnlock(&os_sdidAdminLock);

    return rsm;
}

void
os_sharedMemoryInit (void)
{
    os_mutexInit(&os_sdidAdminLock, NULL);
    os_heap_sharedMemoryInit();
}

void
os_sharedMemoryExit (void)
{
    os_mutexDestroy(&os_sdidAdminLock);
}

os_result
os_svr4_sharedMemoryGetNameFromId(
    int id,
    char **name)
{

    os_result rv = os_resultFail;
    *name = NULL;
    os_sdid *sdid;

    sdid = os_sdid_find_by_id(id);

    if (sdid) {
        *name =  os_strdup(sdid->name);
        rv = os_resultSuccess;
    }
    return rv;
}

os_result
os_svr4_sharedMemoryAttach (
    const char *name,
    os_sharedAttr *sharedAttr,
    void **mapped_address,
    const int id)
{
   SD_ID shmid;
   os_sdid *id2;
   os_result rv;
   void *map_address;
   /* We don't look at the sharedAttr at all so there is no control of the
      map address! */
   (void)sharedAttr;

   id2 = os_sdid_find(name);

   if (id2)
   {
      shmid = id2->shmid;
   }
   else
   {
      shmid = sdOpen((char *)name, SD_LINGER, 0, 1 /* ignored size */, 0,
                     (SD_ATTR_RW | SD_CACHE_COPYBACK), &map_address);
      if (shmid == (SD_ID) NULL)
      {
         if(os_getErrno() == S_sdLib_VIRT_ADDR_PTR_IS_NULL)
         {
            OS_REPORT_WID (OS_ERROR,
                         "os_svr4_sharedMemoryAttach", 1, id,
                         "sdOpen failed because the requested virtual "
                         "address is NULL. (%s)", name);
         }
         else if(os_getErrno() == S_sdLib_ADDR_NOT_ALIGNED)
         {
            OS_REPORT_WID (OS_ERROR,
                         "os_svr4_sharedMemoryAttach", 1, id,
                         "sdOpen failed because the physical address "
                         "is not properly aligned (%s)", name);
         }
         else if(os_getErrno() == S_sdLib_SIZE_IS_NULL)
         {
            OS_REPORT_WID (OS_ERROR,
                         "os_svr4_sharedMemoryAttach", 1, id,
                         "sdOpen failed because the requested size "
                         "is NULL (%s)", name);
         }
         else if(os_getErrno() == S_sdLib_INVALID_OPTIONS)
         {
            OS_REPORT_WID (OS_ERROR,
                         "os_svr4_sharedMemoryAttach", 1, id,
                         "sdOpen failed because invalid options "
                         "were provided (%s)", name);
         }
         else if(os_getErrno() == S_sdLib_VIRT_PAGES_NOT_AVAILABLE)
         {
            OS_REPORT_WID (OS_ERROR,
                         "os_svr4_sharedMemoryAttach", 1, id,
                         "sdOpen failed because there is not enough "
                         "virtual space left in system (%s)", name);
         }
         else if(os_getErrno() == S_sdLib_PHYS_PAGES_NOT_AVAILABLE)
         {
            OS_REPORT_WID (OS_ERROR,
                         "os_svr4_sharedMemoryAttach", 1, id,
                         "sdOpen failed because there is not "
                         "enough physical memory left in system (%s)", name);
         }
         else if(os_getErrno() == ENOSYS)
         {
            OS_REPORT_WID (OS_ERROR,
                         "os_svr4_sharedMemoryAttach", 1, id,
                         "sdOpen failed because INCLUDE_SHARED_DATA "
                         "has not been configured into the kernel (%s)", name);
         }
         else if(os_getErrno() != S_objLib_OBJ_NOT_FOUND)
         {
            OS_REPORT_WID (OS_ERROR,
                         "os_svr4_sharedMemoryAttach", 1, id,
                         "sdOpen failed with unknown error %d (%s)",
                         os_getErrno(), name);
         }

         rv = os_resultFail;
      }
      else
      {
         os_sdid_add (name, shmid, id);
      }
   }

   if (shmid != (SD_ID) NULL)
   {
      *mapped_address = (void *)sdMap(
         shmid,                      /* ID of shared data region to map */
         (SD_ATTR_RW | SD_CACHE_COPYBACK),/* MMU attr used to map region */
         0                           /* reserved - use zero */);
      if (*mapped_address != NULL) {
         if ( sharedAttr->needsErase ) {
            sharedAttr->needsErase=0;
            /* Erase enough of the shared memory to ensure we don't find
               the signature for the mm subsystem if is happens to be
               in memory from a previous shared segment */
            bzero( *mapped_address, 1024 );
         }
         rv = os_resultSuccess;
      } else {
         if (os_getErrno() == S_sdLib_INVALID_SD_ID) {
            OS_REPORT_WID (OS_ERROR,
                       "os_svr4_sharedMemoryAttach", 1, id,
                       "sdMap failed because the provided id is invalid.");
         } else if (os_getErrno() == S_sdLib_SD_IS_PRIVATE) {
            OS_REPORT_WID (OS_ERROR,
                         "os_svr4_sharedMemoryAttach", 1, id,
                         "sdMap failed because the requested segment "\
                         "is private to another application. (%s)", name);
         } else if(os_getErrno() == ENOSYS){
            OS_REPORT_WID (OS_ERROR,
                         "os_svr4_sharedMemoryAttach", 1, id,
                         "sdMap failed because INCLUDE_SHARED_DATA "\
                         "has not been configured into the kernel (%s)", name);
         } else {
            OS_REPORT_WID (OS_ERROR,
                         "os_svr4_sharedMemoryAttach", 1, id,
                         "sdMap failed with unknown error %d (%s)",
                         os_getErrno(), name);
         }
         rv = os_resultFail;
      }
   } else {
      rv = os_resultFail;
   }

   return rv;
}

os_result
os_svr4_sharedMemoryDestroy (
    const char *name)
{
    os_sdid *id;
    os_result rv;
    STATUS s;

    id = os_sdid_remove(name);

    if(id){
        s = sdDelete(id->shmid, 0);
        if(s != OK){
            if(os_getErrno() == S_sdLib_INVALID_SD_ID){
                OS_REPORT (OS_ERROR,
                     "os_svr4_sharedMemoryDestroy", 1,
                     "sdDelete failed because the requested id "\
                     "is invalid (%s)", name);
            } else if(os_getErrno() == S_sdLib_CLIENT_COUNT_NOT_NULL){
                OS_REPORT (OS_ERROR,
                     "os_svr4_sharedMemoryDestroy", 1,
                     "sdDelete failed because the requested id "\
                     "is still mapped by an application (%s)", name);
            } else if(os_getErrno() == ENOSYS){
                OS_REPORT (OS_ERROR,
                     "os_svr4_sharedMemoryDestroy", 1,
                     "sdDelete failed because INCLUDE_SHARED_DATA "\
                     "has not been configured into the kernel (%s)", name);
            } else {
                OS_REPORT (OS_ERROR,
                     "os_svr4_sharedMemoryDestroy", 1,
                     "sdDelete failed with unknown error %d (%s)",
                     os_getErrno(), name);
            }
            rv = os_resultFail;
        } else {
            rv = os_resultSuccess;
        }
    } else {
        OS_REPORT (OS_ERROR,
                     "os_svr4_sharedMemoryDestroy", 1,
                     "The requested segment is unknown (%s)", name);
        rv = os_resultFail;
    }
    return rv;
}

os_result
os_svr4_sharedMemoryCreate (
    const char *name,
    os_sharedAttr *sharedAttr,
    os_address size,
    const int id)
{
    SD_ID shmid;
    os_result rv;

    shmid = sdCreate((char *)name, 0, size, 0,
                (SD_ATTR_RW | SD_CACHE_COPYBACK),
                &sharedAttr->map_address);
    if (shmid == (SD_ID) NULL)
    {
        if(os_getErrno() == S_sdLib_VIRT_ADDR_PTR_IS_NULL){
            OS_REPORT (OS_ERROR,
                       "os_svr4_sharedMemoryCreate", 1,
                       "sdCreate failed because the requested virtual "\
                       "address is NULL. (%s)", name);
        } else if(os_getErrno() == S_sdLib_ADDR_NOT_ALIGNED){
            OS_REPORT (OS_ERROR,
                       "os_svr4_sharedMemoryCreate", 1,
                       "sdCreate failed because the physical address "\
                       "is not properly aligned (%s)", name);
        } else if(os_getErrno() == S_sdLib_SIZE_IS_NULL){
            OS_REPORT (OS_ERROR,
                       "os_svr4_sharedMemoryCreate", 1,
                       "sdCreate failed because the requested size "\
                       "is NULL (%s)", name);
        } else if(os_getErrno() == S_sdLib_INVALID_OPTIONS){
            OS_REPORT (OS_ERROR,
                       "os_svr4_sharedMemoryCreate", 1,
                       "sdCreate failed because invalid options "\
                       "were provided (%s)", name);
        } else if(os_getErrno() == S_sdLib_VIRT_PAGES_NOT_AVAILABLE){
            OS_REPORT (OS_ERROR,
                       "os_svr4_sharedMemoryCreate", 1,
                       "sdCreate failed because there is not enough "\
                       "virtual space left in system (%s)", name);
        } else if(os_getErrno() == S_sdLib_PHYS_PAGES_NOT_AVAILABLE){
            OS_REPORT (OS_ERROR,
                       "os_svr4_sharedMemoryCreate", 1,
                       "sdCreate failed because there is not "\
                       "enough physical memory left in system (%s)", name);
        } else if(os_getErrno() == ENOSYS){
            OS_REPORT (OS_ERROR,
                       "os_svr4_sharedMemoryCreate", 1,
                       "sdCreate failed because INCLUDE_SHARED_DATA "\
                       "has not been configured into the kernel (%s)", name);
        } else {
            OS_REPORT (OS_ERROR,
                       "os_svr4_sharedMemoryCreate", 1,
                       "sdCreate failed with unknown error %d (%s)",
                       os_getErrno(), name);
        }
        rv = os_resultFail;
    } else {
        sharedAttr->needsErase = 1;
        rv = os_sdid_add(name, shmid,id);

        if(rv != os_resultSuccess){
            sdDelete(shmid, 0);

            OS_REPORT (OS_ERROR,
                 "os_svr4_sharedMemoryCreate", 1,
                 "Unable to administrate created segment. (%s)", name);
        }
    }
    return rv;
}

os_result
os_svr4_sharedMemoryDetach (
    const char *name,
    void *address,
    os_int32 domainId)
{
    os_sdid *id;
    os_result rv;
    STATUS s;
    (void)address;

    id = os_sdid_find(name);

    if(id)
    {
        s = sdUnmap(id->shmid, 0);

        if(s != OK)
        {
            if(os_getErrno() == S_sdLib_INVALID_SD_ID)
            {
                OS_REPORT_WID (OS_ERROR,
                     "os_svr4_sharedMemoryDetach", 1, domainId,
                     "sdUnmap failed because the requested id "\
                     "is invalid (%s)", name);
            }
            else if(os_getErrno() == S_sdLib_NOT_MAPPED)
            {
                OS_REPORT_WID (OS_ERROR,
                     "os_svr4_sharedMemoryDetach", 1, domainId,
                     "sdUnmap failed because the requested id "\
                     "is not mapped in the application (%s)", name);
            }
            else if(os_getErrno() == ENOSYS)
            {
                OS_REPORT_WID (OS_ERROR,
                     "os_svr4_sharedMemoryDetach", 1, domainId,
                     "sdUnmap failed because INCLUDE_SHARED_DATA "\
                     "has not been configured into the kernel (%s)", name);
            }
            else
            {
                OS_REPORT_WID (OS_ERROR,
                     "os_svr4_sharedMemoryDetach", 1, domainId,
                     "sdUnmap failed with unknown error %d (%s)",
                     os_getErrno(), name);
            }

            rv = os_resultFail;
        }
        else
        {
            rv = os_resultSuccess;
        }
    }
    else
    {
        OS_REPORT_WID (OS_ERROR,
                 "os_svr4_sharedMemoryDetach", 1, domainId,
                 "The requested segment is unknown (%s)", name);
        rv = os_resultFail;
    }

    return rv;
}

void
os_sharedAttrInit (
    os_sharedAttr *sharedAttr)
{
    assert (sharedAttr != NULL);
    sharedAttr->lockPolicy = OS_LOCK_DEFAULT;
    sharedAttr->sharedImpl = OS_MAP_ON_SEG;
    sharedAttr->userCred.uid = 0;
    sharedAttr->userCred.gid = 0;
    sharedAttr->map_address = (void *)0x60000000;
    sharedAttr->needsErase = 0;
}

os_result
os_sharedMemoryCreate (
    os_sharedHandle sharedHandle,
    os_address size)
{
    os_result result = os_resultFail;

    assert (sharedHandle != NULL);
    assert (sharedHandle->name != NULL);
    assert (size > 0);

    switch (sharedHandle->attr.sharedImpl)
    {
       case OS_MAP_ON_FILE:
          result = os_resultFail;
          break;
       case OS_MAP_ON_SEG:
          result = os_svr4_sharedMemoryCreate (sharedHandle->name, &sharedHandle->attr, size,sharedHandle->id);
          break;
       case OS_MAP_ON_HEAP:
          result = os_heap_sharedMemoryCreate (sharedHandle->name, &sharedHandle->attr, size,sharedHandle->id);
          break;
    }
    return result;
}

os_result
os_sharedMemoryDestroy (
    os_sharedHandle sharedHandle)
{
    os_result result = os_resultFail;

    assert (sharedHandle != NULL);
    assert (sharedHandle->name != NULL);
    switch (sharedHandle->attr.sharedImpl)
    {
       case OS_MAP_ON_FILE:
          result = os_resultFail;
          break;
       case OS_MAP_ON_SEG:
          result = os_svr4_sharedMemoryDestroy (sharedHandle->name);
          break;
       case OS_MAP_ON_HEAP:
          result = os_heap_sharedMemoryDestroy (sharedHandle->name);
          break;
    }
    return result;
}

os_result
os_sharedMemoryGetNameFromId(
    os_sharedHandle sharedHandle,
    char **name)
{
    os_result result = os_resultFail;

    assert(sharedHandle != NULL);

    switch (sharedHandle->attr.sharedImpl) {
    case OS_MAP_ON_FILE:
        result = os_resultFail;
    break;
    case OS_MAP_ON_SEG:
        result = os_svr4_sharedMemoryGetNameFromId(sharedHandle->id, name);
    break;
    case OS_MAP_ON_HEAP:
        result = os_heap_sharedMemoryGetNameFromId(sharedHandle->id, name);
    break;
    }
    return result;
}

os_result
os_sharedMemoryAttach (
    os_sharedHandle sharedHandle)
{
    os_result result = os_resultFail;

    assert (sharedHandle != NULL);
    assert (sharedHandle->name != NULL);
    assert (sharedHandle->mapped_address == NULL);
    switch (sharedHandle->attr.sharedImpl)
    {
       case OS_MAP_ON_FILE:
          result = os_resultFail;
          break;
       case OS_MAP_ON_SEG:
          result = os_svr4_sharedMemoryAttach (sharedHandle->name, &sharedHandle->attr, &sharedHandle->mapped_address, sharedHandle->id);
          break;
       case OS_MAP_ON_HEAP:
          result = os_heap_sharedMemoryAttach (sharedHandle->name, &sharedHandle->mapped_address);
          break;
    }
    return result;
}

os_result
os_sharedMemoryDetach (
    os_sharedHandle sharedHandle)
{
    os_result result = os_resultFail;

    assert (sharedHandle != NULL);
    assert (sharedHandle->name != NULL);
    assert (sharedHandle->mapped_address != NULL);
    switch (sharedHandle->attr.sharedImpl)
    {
       case OS_MAP_ON_FILE:
          result = os_resultFail;
          break;
       case OS_MAP_ON_SEG:
          result = os_svr4_sharedMemoryDetach (sharedHandle->name, sharedHandle->mapped_address, sharedHandle->id);
          break;
       case OS_MAP_ON_HEAP:
          result = os_heap_sharedMemoryDetach (sharedHandle->name, sharedHandle->mapped_address, sharedHandle->id);
          break;
    }
    if (result == os_resultSuccess)
    {
       sharedHandle->mapped_address = NULL;
    }
    return result;
}

os_result
os_sharedMemoryDetachUnclean(
    os_sharedHandle sharedHandle)
{
    return os_sharedMemoryDetach(sharedHandle);
}

os_result
os_sharedSize(
    os_sharedHandle sharedHandle,
    os_address *size)
{
    assert(sharedHandle != NULL);
    assert(sharedHandle->name != NULL);

    os_sdid *id;
    STATUS s;
    SD_DESC sharedDataInfo;

    os_result rv = os_resultFail;

    id = os_sdid_find(sharedHandle->name);

    if(id)
    {
       s = sdInfoGet(id->shmid, &sharedDataInfo);

       if (s == OK)
       {
          *size = sharedDataInfo.size;
          rv = os_resultSuccess;
       }
    }

    return rv;
}

void
os_shmClientFree(
    os_shmClient client)
{
    (void) client;
}

os_result
os_sharedMemoryWaitForClientChanges(
    os_sharedHandle sharedHandle,
    os_duration maxBlockingTime,
    os_shmClient* changedClients)
{
    (void) sharedHandle;
    (void) changedClients;
    ospl_os_sleep(maxBlockingTime);
    return os_resultTimeout;
}

char *
os_findKeyFile(
    const char * name)
{
    (void) name;
    return NULL;
}

char *
os_findKeyFileById(
    const os_int32 domainId)
{
    (void) domainId;
    return NULL;
}

os_int32
os_destroyKeyFile(
    const char * name)
{
    (void) name;
    return 0;
}

os_int32
os_sharedMemorySegmentFree(
    const char * fname)
{
    (void) fname;
    return 0;
}

os_state
os_sharedMemoryGetState(
    os_sharedHandle sharedHandle)
{
    os_state state = OS_STATE_NONE;
    os_sdid *sdid;

    assert(sharedHandle);

    switch (sharedHandle->attr.sharedImpl)
    {
        case OS_MAP_ON_FILE:
            break;
       case OS_MAP_ON_SEG:
           sdid = os_sdid_find_by_id(sharedHandle->id);
           if (sdid) {
               state = sdid->state;
           }
           break;
       case OS_MAP_ON_HEAP:
           state = os_heap_sharedMemoryGetState(sharedHandle->id);
           break;
    }

    return state;
}

os_result
os_sharedMemorySetState(
    os_sharedHandle sharedHandle,
    os_state state)
{
    os_result rv = os_resultFail;
    os_sdid *sdid;

    assert(sharedHandle);

    switch (sharedHandle->attr.sharedImpl)
    {
        case OS_MAP_ON_FILE:
            break;
       case OS_MAP_ON_SEG:
           sdid = os_sdid_find_by_id(sharedHandle->id);
           if (sdid) {
               sdid->state = state;
               rv = os_resultSuccess;
           } else {
               rv = os_resultUnavailable;
           }
           break;
       case OS_MAP_ON_HEAP:
           rv = os_heap_sharedMemorySetState(sharedHandle->id, state);
           break;
    }

    return rv;
}

os_result
os_sharedMemoryRegisterServerDiedCallback(
    os_sharedHandle sharedHandle,
    os_onSharedMemoryManagerDiedCallback onServerDied,
    void *args)
{
    (void) sharedHandle;
    (void) onServerDied;
    (void) args;
    return os_resultUnavailable;
}

os_result os_sharedMemoryLock(os_sharedHandle sharedHandle) {
    OS_UNUSED_ARG(sharedHandle);
    return os_resultUnavailable;
}

void os_sharedMemoryUnlock(os_sharedHandle sharedHandle) {
    OS_UNUSED_ARG(sharedHandle);
    return;
}

void
os_sharedMemoryImplDataCreate(
    os_sharedHandle sharedHandle)
{
    OS_UNUSED_ARG(sharedHandle);
}

void
os_sharedMemoryImplDataDestroy(
    os_sharedHandle sharedHandle)
{
    OS_UNUSED_ARG(sharedHandle);
}
