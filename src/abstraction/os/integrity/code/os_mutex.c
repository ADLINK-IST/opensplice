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

/** \file os/int509/code/os_mutex.c
 *  \brief Integrity mutual exclusion semaphores
 *
 * Implements mutual exclusion semaphores for Integrity */

#include "os_mutex.h"
#include "include/ResourceStore.h"
#include "include/os_getRSObjects.h"
#include <assert.h>
#include "os_errno.h"
#include <INTEGRITY.h>
#include <INTEGRITY_types.h>
#include "os_time.h"
#include "os_signature.h"
#include "os_init.h"


#include "../common/code/os_mutex_attr.c"

/* Address space local cache of semaphore links */
typedef struct mapEntry
{
   Semaphore semaphore;
   uint64_t uid;
#ifndef NDEBUG
   os_mutex *mutex;
#endif
} mapEntry;

static mapEntry *semMap = NULL;

#ifndef NDEBUG
static int semMapIndexCeiling;
#endif

/* Connection to the ResourceStore and associated lock - also used by os_cond */
Semaphore os_connLock;
Connection os_conn;


Error rsReceiveReturn(void)
{
   Error rv;
   Error err;
   Address lr;
   err = Receive( os_conn, (Address)&rv, sizeof(rv), &lr );
   assert( lr == sizeof(rv) );
   return ( err == Success ? rv : err );
}

static Error rsGetSemaphore( Semaphore *sem, int index )
{
   Error err;
   Error err2;
   static const enum Command cmd = RSGetSemaphore;

   err = WaitForSemaphore( os_connLock );
   assert ( err == Success );
   if ( err == Success )
   {
      err = Send( os_conn, (Address)&cmd, sizeof(cmd) );
      assert ( err == Success );
      if ( err == Success )
      {
         err = Send( os_conn, (Address)&index, sizeof(int) );
         assert ( err == Success );
         if ( err == Success )
         {
            err == rsReceiveReturn();
            if ( err == Success )
            {
               err = ReceiveObject( os_conn, (Object *)sem );
               assert( err == Success );
            }
         }
      }

      err2 = ReleaseSemaphore(os_connLock);
      assert ( err2 == Success );
      (void)err2;
   }
   return (err);
}

static Error rsRemoveSemaphore( int index )
{
   Error err;
   Error err2;
   static const enum Command cmd = RSRemoveSemaphore;

   err = WaitForSemaphore( os_connLock );
   assert ( err == Success );
   if ( err == Success )
   {
      err = Send( os_conn, (Address)&cmd, sizeof(cmd) );
      assert ( err == Success );
      if ( err == Success )
      {
         err = Send( os_conn, (Address)&index, sizeof(int) );
         assert ( err == Success );
         if ( err == Success )
         {
            err == rsReceiveReturn();
         }
      }

      err2 = ReleaseSemaphore(os_connLock);
      assert ( err2 == Success );
      (void)err2;
   }
   return (err);
}


static Error rsNewSemaphore( Semaphore *sem, int *index, uint64_t *objectID )
{
   Error err;
   Error err2;
   static const enum Command cmd = RSCreateSemaphore;

   Address lr;

   err = WaitForSemaphore( os_connLock );
   assert ( err == Success );
   if ( err == Success )
   {
      err = Send( os_conn, (Address)&cmd, sizeof(cmd) );
      assert( err == Success );
      if ( err == Success )
      {
         err  = rsReceiveReturn();
         if ( err == Success )
         {
            err = ReceiveObject( os_conn, (Object *)sem );
            assert( err == Success );
            if ( err == Success )
            {
               err = Receive(os_conn, (Address)objectID, sizeof(uint64_t), &lr);
               assert( lr == sizeof(uint64_t) && err == Success );
               if ( err == Success )
               {
                  err = Receive( os_conn, (Address)index, sizeof(int), &lr );
                  assert( lr == sizeof(int) && err == Success );
               }
            }
         }
      }

      err2 = ReleaseSemaphore(os_connLock);
      assert ( err2 == Success );
      (void)err2;
   }
   return (err);
}

static Error rsGetSysResource( Object *obj, const char *resname )
{
   Error err;
   int tries;
   os_duration delay = 250*OS_DURATION_MILLISECOND;

   for ( tries = 0; tries < DDS_MAX_TRIES_TO_LOCATE_RESOURCESTORE; tries++ )
   {
      err = RequestResource(obj,
                            resname,
                            DDS_RES_PASSWD);
      if ( err != ResourceNotAvailable )
      {
         break;
      }
      else
      {
         ospl_os_sleep(delay);
      }
   }
   return( err );
}


void os_mutexModuleInit()
{
   if ( !os_getIsSingleProcess() )
   {
      semMap = (mapEntry *)malloc( sizeof(mapEntry) * DDS_MAX_SEMAPHORES );
      memset( semMap, '\0', sizeof(mapEntry) * DDS_MAX_SEMAPHORES );

      os_connLock = os_getResourceStoreSemaphore();
      os_conn = os_getResourceStoreConnection();
   }
#ifndef NDEBUG
   semMapIndexCeiling=0;
#endif
}

void os_mutexModuleExit()
{
   if ( !os_getIsSingleProcess() )
   {
      free(semMap);
   }
}

#ifndef NDEBUG
static void checkMutexCache( os_mutex *mutex )
{
#ifdef OPENSPLICE_IS_FIXED
   int i;
   for ( i=0; i<semMapIndexCeiling; i++ )
   {
      assert( semMap[i].uid == 0 || semMap[i].mutex != mutex );
   }
#endif
}
#endif

static pthread_mutex_t mapMutex = PTHREAD_MUTEX_INITIALIZER;

static Semaphore os_mutexMapFind(os_mutex *mutex)
{
   int res;
   Semaphore result = 0;
   mapEntry *current;

   res = pthread_mutex_lock(&mapMutex);
   assert(res == 0);

#ifndef NDEBUG
   if ( mutex->index > semMapIndexCeiling )
   {
      assert( mutex->index < DDS_MAX_SEMAPHORES );
      semMapIndexCeiling = mutex->index;
   }
#endif

   current = &semMap[mutex->index];
   if ( current->uid != mutex->uid )
   {
      /* The index has been reused and we have a stale link */
      CloseLink( (Link)current->semaphore );
      current->uid = 0;
      current->semaphore = 0;
#ifndef NDEBUG
      current->mutex = NULL;
#endif
   }

   if (!current->uid)
   {
      /* We have no local entry in cache */
      Error err;

      err = rsGetSemaphore( &current->semaphore, mutex->index);
      if ( err == Success )
      {
         result = current->semaphore;
         current->uid = mutex->uid;
#ifndef NDEBUG
         current->mutex = mutex;
#endif
      }
   }
   else
   {
      result = current->semaphore;
   }

   res = pthread_mutex_unlock( &mapMutex );
   assert(res == 0);
   (void) res;
   assert( result != 0 );

   return (result);
}

Semaphore os_os_mutexGetSem( os_os_mutex *mutex )
{
   assert(mutex != NULL);
   assert(mutex->signature == OS_MUTEX_MAGIC_SIG);
   return (mutex->localsem ? mutex->localsem : os_mutexMapFind(mutex));
}

static void os_mutexAddToMap( os_mutex *mutex, Semaphore sem )
{
   int res;
   struct mapEntry *current;

   res = pthread_mutex_lock(&mapMutex);
   assert( res == 0 );

#ifndef NDEBUG
   if ( mutex->index > semMapIndexCeiling )
   {
      assert( mutex->index < DDS_MAX_SEMAPHORES );
      semMapIndexCeiling = mutex->index;
   }
#endif
   current = &semMap[mutex->index];

   if ( current->uid != 0 )
   {
      /* Slot previously used, free old link */
      CloseLink( (Link)current->semaphore );
   }
   current->semaphore = sem;
   current->uid = mutex->uid;
#ifndef NDEBUG
   current->mutex = mutex;
#endif

   res = pthread_mutex_unlock(&mapMutex);
   assert( res == 0 );
   (void) res;
}

static Error os_mutexMapFindAndRemove( os_mutex *mutex )
{
   int res;
   mapEntry *toDestroy;
   Error err;

   res = pthread_mutex_lock(&mapMutex);
   assert(res == 0);

   toDestroy = &semMap[mutex->index];

   assert( toDestroy->semaphore != 0 && toDestroy->uid != 0 );
#ifndef NDEBUG
   assert( toDestroy->mutex == mutex );
#endif

   err = CloseLink((Link)toDestroy->semaphore);
   assert(err == Success);

   toDestroy->semaphore = 0;
   toDestroy->uid = 0;
#ifndef NDEBUG
   toDestroy->mutex = NULL;
#endif

   res = pthread_mutex_unlock(&mapMutex);
   assert(res == 0);
   (void) res;

   return (err);
}

/** \brief Sets the priority inheritance mode for mutexes
 *   that are created after this call.
 *
 * Not (yet) supported on this platform
 */
os_result
os_mutexSetPriorityInheritanceMode(
    os_boolean enabled)
{
    /* Priority Inheritance is not supported on this platform (yet) */
    return os_resultSuccess;
}



/** \brief Initialize the mutex taking the mutex attributes
 *         into account
 */
os_result os_mutexInit (os_mutex *mutex, const os_mutexAttr *mutexAttr)
{
   Error err;
   os_mutexAttr defAttr;

   assert (mutex != NULL);

#if !defined NDEBUG && !defined INTEGRITY
   assert(mutex->signature != OS_MUTEX_MAGIC_SIG);
#endif

   if(!mutexAttr) {
       os_mutexAttrInit(&defAttr);
       mutexAttr = &defAttr;
   }

   if ( mutexAttr->scopeAttr == OS_SCOPE_SHARED)
   {
      Semaphore newsem;
      mutex->localsem = NULL;

#ifndef NDEBUG
      checkMutexCache(mutex);
#endif

      err = rsNewSemaphore(&newsem, &mutex->index, &mutex->uid);
      if ( err == Success )
      {
         os_mutexAddToMap(mutex, newsem);
      }
   }
   else
   {
      err = CreateBinarySemaphore(&mutex->localsem);
   }

#ifndef NDEBUG
   if ( err == Success )
   {
      mutex->signature = OS_MUTEX_MAGIC_SIG;
   }
#endif

   return ( err == Success ? os_resultSuccess : os_resultFail );
}

/** \brief Destroy the mutex
 */
void
os_mutexDestroy (os_mutex *mutex)
{
   Error err = Success;

   assert(mutex != NULL);

   if ( mutex->localsem != 0 )
   {
      err = CloseSemaphore(mutex->localsem);
   }
   else
   {
      assert( mutex->uid != 0 );
      err = rsRemoveSemaphore(mutex->index);
      if ( err == Success )
      {
         err = os_mutexMapFindAndRemove(mutex);
      }
   }

#ifndef NDEBUG
   if ( err == Success )
   {
      mutex->signature = 0;
   }
#endif

   if (err != Success) {
       abort ();
   }
}

/** \brief Acquire the mutex
 *
 */
os_result os_mutexLock_s (os_mutex *mutex)
{
   Error err;
   Semaphore semaphore;
   SignedValue svalue;

   semaphore = os_os_mutexGetSem(mutex);
   err = WaitForSemaphore(semaphore);

   assert (err != ActivityIsAlreadyInUse);

   if ( err == Success )
   {
      err = GetSemaphoreValue(semaphore, &svalue);
      assert( err == Success );
      if ( svalue > 1 )
      {
         /* This is not a recursive mutex */
         err = ReleaseSemaphore(semaphore);
         assert( err == Success );
         err = Failure;
      }
   }

   return (err == Success ? os_resultSuccess : os_resultFail);
}

void os_mutexLock (os_mutex *mutex)
{
    if (os_mutexLock_s (mutex) != os_resultSuccess) {
        abort ();
    }
}

/** \brief Try to acquire the mutex, immediately return if the mutex
 *         is already acquired by another thread
 */
os_result os_mutexTryLock (os_mutex *mutex)
{
   Error err;
   Semaphore semaphore;
   os_result rv;

   semaphore = os_os_mutexGetSem(mutex);
   err = TryToObtainSemaphore(semaphore);

   switch (err )
   {
      case Success:
      {
         SignedValue svalue;
         err = GetSemaphoreValue(semaphore, &svalue);
         assert( err == Success );
         if ( svalue > 1 )
         {
            /* This is not a recursive mutex */
            err = ReleaseSemaphore(semaphore);
            rv = os_resultBusy;
         }
         else
         {
            rv = os_resultSuccess;
         }
         break;
      }
      case ResourceNotAvailable:
      {
         rv = os_resultBusy;
         break;
      }
      default:
      {
         abort ();
         rv = os_resultFail;
      }
   }
   return (rv);
}

/** \brief Release the acquired mutex
 *
 */
void os_mutexUnlock(os_mutex *mutex)
{
   Error err;
   Semaphore semaphore;

   semaphore = os_os_mutexGetSem(mutex);
   err = ReleaseSemaphore(semaphore);
   if (err != Success) {
       abort ();
   }
}
