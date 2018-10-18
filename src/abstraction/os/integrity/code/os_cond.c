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
/** \file os/int509/code/os_cond.c
 *  \brief Integrity condition variables
 *
 * Implements condition variables for Integrity, the condition variable
 * is mapped on the Integrity Condition variable
 */

#include <INTEGRITY_types.h>
#include "os_cond.h"
#include <assert.h>
#include "os_errno.h"
#include <pthread.h>
#include "os_mutex.h"
#include "include/ResourceStore.h"
#include "os_signature.h"
#include "os_init.h"

#include "../common/code/os_cond_attr.c"

os_boolean os_getIsSingleProcess();
typedef struct mapEntry
{
   Condition condition;
   uint64_t uid;
#ifndef NDEBUG
   os_cond *cond;
#endif
} mapEntry;

static mapEntry *condMap = NULL;
static pthread_mutex_t mapMutex = PTHREAD_MUTEX_INITIALIZER;

#ifndef NDEBUG
static int condMapIndexCeiling = 0;
#endif

static Error rsNewCondition( int semIndex, Condition *cond, int *index, uint64_t *objectID )
{
   Error err;
   Error err2;
   static const enum Command cmd = RSCreateCondition;

   Address lr;

   err = WaitForSemaphore( os_connLock );
   assert ( err == Success );

   if (err == Success)
   {
      err = Send( os_conn, (Address)&cmd, sizeof(cmd) );
      assert( err == Success );
      if (err == Success)
      {
         err = Send( os_conn, (Address)&semIndex, sizeof(int) );
         assert( err == Success );
         if ( err == Success )
         {
            err = rsReceiveReturn();
            if ( err == Success )
            {
               err = ReceiveCondition( os_conn, cond );
               assert( err == Success );
               if ( err == Success )
               {
                  err = Receive( os_conn, (Address)objectID, sizeof(uint64_t), &lr );
                  assert( lr == sizeof(uint64_t) && err == Success );
                  if ( err == Success )
                  {
                     err = Receive( os_conn, (Address)index, sizeof(int), &lr );
                     assert( lr == sizeof(int) && err == Success );
                  }
               }
            }
         }
      }
      err2 = ReleaseSemaphore(os_connLock);
      assert ( err2 == Success );
      (void) err2;
   }
   return (err);
}

static Error rsGetCondition( Condition *cond, int index )
{
   Error err;
   Error err2;
   static const enum Command cmd = RSGetCondition;

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
            err = rsReceiveReturn();
            if ( err == Success )
            {
               err = ReceiveCondition( os_conn, cond );
               assert( err == Success );
            }
         }
      }

      err2 = ReleaseSemaphore(os_connLock);
      assert ( err2 == Success );
      (void) err2;
   }
   return (err);
}

static Error rsRemoveCondition( int index )
{
   Error err;
   Error err2;
   static const enum Command cmd = RSRemoveCondition;

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
      (void) err2;
   }
   return (err);
}

void os_condModuleInit()
{
  if ( !os_getIsSingleProcess() )
  {
     condMap=(mapEntry *)malloc( sizeof(mapEntry) * DDS_MAX_CONDITIONS );
     memset( condMap, '\0', sizeof(mapEntry) * DDS_MAX_CONDITIONS );
  }
#ifndef NDEBUG
   condMapIndexCeiling=0;
#endif
}

void os_condModuleExit()
{
   if ( !os_getIsSingleProcess() )
   {
      free(condMap);
   }
}

#ifndef NDEBUG
static void checkCondCache( os_cond *cond )
{
   int i;
   for ( i=0; i<condMapIndexCeiling; i++ )
   {
      assert( condMap[i].uid == 0 || condMap[i].cond != cond );
   }
}
#endif

static void os_condAddToMap(os_cond *cond, Condition condition)
{
   int res;
   mapEntry *current;
   res = pthread_mutex_lock(&mapMutex);
   assert (res == 0);

#ifndef NDEBUG
   if ( cond->index > condMapIndexCeiling )
   {
      assert( cond->index < DDS_MAX_CONDITIONS );
      condMapIndexCeiling = cond->index;
   }
#endif
   current = &condMap[cond->index];

   if ( current->uid != 0 )
   {
      /* Slot previously used, free old link */
      CloseLink( (Link)current->condition );
   }

   current->condition = condition;
   current->uid = cond->uid;

#ifndef NDEBUG
   current->cond = cond;
#endif

   res = pthread_mutex_unlock(&mapMutex);
   assert(res == 0);
}

static Condition os_condMapFind(os_cond *cond)
{
   int res;
   Condition result = NULL;
   mapEntry *current;

   res = pthread_mutex_lock(&mapMutex);
   assert(res == 0);

#ifndef NDEBUG
   if ( cond->index > condMapIndexCeiling )
   {
      assert( cond->index < DDS_MAX_CONDITIONS );
      condMapIndexCeiling = cond->index;
   }
#endif
   current = &condMap[cond->index];
   if ( current->uid != cond->uid )
   {
      /* The index has been reused and we have a stale link */
      CloseLink( (Link)current->condition );
      current->uid = 0;
      current->condition = NULL;
#ifndef NDEBUG
      current->cond = NULL;
#endif
   }

   if (!current->uid)
   {
      /* We have no local entry in cache */
      Error err;

      err = rsGetCondition( &current->condition, cond->index);
      if ( err == Success )
      {
         result = current->condition;
         current->uid = cond->uid;
#ifndef NDEBUG
         current->cond = cond;
#endif
      }
   }
   else
   {
      result = current->condition;
   }

   res = pthread_mutex_unlock(&mapMutex);
   assert(res == 0);

   assert( result != 0 );

   return (result);
}

static Error os_condMapFindAndRemove(os_cond *cond)
{
   int res;
   mapEntry *toDestroy;
   Error err;

   res = pthread_mutex_lock(&mapMutex);
   assert( res == 0 );

   toDestroy = &condMap[cond->index];

   assert( toDestroy->condition != NULL && toDestroy->uid != 0 );
#ifndef NDEBUG
   assert( toDestroy->cond == cond );
#endif

   err = CloseCondition(toDestroy->condition);
   assert(err == Success);

   toDestroy->condition = 0;
   toDestroy->uid = 0;
#ifndef NDEBUG
   toDestroy->cond = NULL;
#endif

   res = pthread_mutex_unlock(&mapMutex);
   assert(res == 0);

   return (err);
}

/** \brief Initialize the condition variable taking the conition
 *         attributes into account
 */
os_result os_condInit(os_cond *cond, os_mutex *mtx, const os_condAttr *condAttr)
{
   Semaphore sem;
   Error err;
   os_condAttr defAttr;

   assert (cond != NULL);

   sem = os_os_mutexGetSem(mtx);
   assert(sem);

#ifndef INTEGRITY
   assert(cond->signature != OS_COND_MAGIC_SIG);
#endif
   assert(mtx->signature == OS_MUTEX_MAGIC_SIG);

   if(!condAttr) {
       os_condAttrInit(&defAttr);
       condAttr = &defAttr;
   }

   if (condAttr->scopeAttr==OS_SCOPE_SHARED)
   {
      Condition newcond; /* Link to condition for map in this process */
      cond->localcond = NULL;

#ifndef NDEBUG
      checkCondCache(cond);
#endif

      err = rsNewCondition(mtx->index, &newcond, &cond->index, &cond->uid);
      if ( err == Success )
      {
         os_condAddToMap(cond, newcond);
      }
   }
   else
   {
      err = CreateCondition(&cond->localcond, sem, false);
   }

#ifndef NDEBUG
   if ( err == Success )
   {
      cond->signature = OS_COND_MAGIC_SIG;
   }
#endif

   return (err == Success ? os_resultSuccess : os_resultFail);
}

/** \brief Destory the condition variable
 *
 * \b os_condDestroy calls \b pthread_cond_destroy to destroy the
 * posix condition variable.
 */
void os_condDestroy (os_cond *cond)
{
   Error err = Success;

   assert (cond != NULL);

#ifndef NDEBUG
   assert( cond->signature == OS_COND_MAGIC_SIG );
#endif

   if (cond->localcond != 0 )
   {
      err = CloseCondition(cond->localcond);
   }
   else
   {
      assert( cond->uid != 0 );
      err = rsRemoveCondition(cond->index);
      if ( err == Success )
      {
         err = os_condMapFindAndRemove(cond);
      }
   }

#ifndef NDEBUG
   if ( err == Success ) {
      cond->signature = 0;
   }
#endif

   if (err != Success) {
       abort ();
   }
}

/** \brief Wait for the condition
 *
 * \b os_condWait calls \b pthread_cond_wait to wait
 * for the condition.
 */
void os_condWait (os_cond *cond, os_mutex *mutex)
{
   Error err;
   Condition condition;

   assert(cond != NULL);
   assert(mutex != NULL);
   assert(cond->signature == OS_COND_MAGIC_SIG);
   assert(mutex->signature == OS_MUTEX_MAGIC_SIG);

   condition = cond->localcond ? cond->localcond : os_condMapFind(cond);
   err = WaitForCondition(condition, NULL, false);

   if (err != Success) {
       abort ();
   }
}

/** \brief Wait for the condition but return when the specified
 *         time has expired before the condition is triggered
 */
os_result os_condTimedWait (os_cond *cond, os_mutex *mutex, os_duration timeout)
{
    Time t;
    Condition condition;
    Error err;
    os_result rv;

    assert(cond != NULL);
    assert(mutex != NULL);
    assert(OS_DURATION_ISPOSITIVE(timeout));
    assert(cond->signature == OS_COND_MAGIC_SIG);
    assert(mutex->signature == OS_MUTEX_MAGIC_SIG);

    t.Seconds = OS_DURATION_GET_SECONDS(timeout);
    /* Convert from nanoseconds to fixed point binary fractions of a second */
    t.Fraction = ((uint64_t)0x800000ULL * OS_DURATION_GET_NANOSECONDS(timeout))/1953125ULL;

    condition = cond->localcond ? cond->localcond : os_condMapFind(cond);
    err = WaitForCondition(condition, &t, false);

    switch (err)
    {
       case Success:
       {
          rv = os_resultSuccess;
          break;
       }
       case  OperationTimedOut:
       {
          rv = os_resultTimeout;
          break;
       }
       default:
       {
          rv = os_resultFail;
          abort ();
       }
    }
    return rv;
}

/** \brief Signal the condition and wakeup one thread waiting
 *         for the condition
 */
void os_condSignal(os_cond *cond)
{
   Error err;
   Condition condition;

   assert(cond != NULL);
   assert(cond->signature == OS_COND_MAGIC_SIG);

   condition = cond->localcond ? cond->localcond : os_condMapFind(cond);
   err = SignalCondition (condition);
   if (err != Success) {
       abort ();
   }
}

/** \brief Signal the condition and wakeup all thread waiting
 *         for the condition
 */
void os_condBroadcast(os_cond *cond)
{
   Error err;
   Condition condition;

   assert(cond != NULL);
   assert(cond->signature == OS_COND_MAGIC_SIG);

   condition = cond->localcond ? cond->localcond : os_condMapFind(cond);
   err = BroadcastCondition(condition);

   if (err != Success) {
       abort ();
   }
}
