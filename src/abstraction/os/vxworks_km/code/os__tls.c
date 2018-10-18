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
#include <pthread.h>
#include <stdlib.h>
#include <assert.h>
#include "os__tls.h"

typedef struct tlsKey_s
{
  void *key;
  void *value;
  struct tlsKey_s *next;
} tlsKey;

typedef struct taskKey_s
{
  int taskId;
  tlsKey *tlsk;
  struct taskKey_s *next;
} taskKey;

static taskKey *taskKeys = NULL;

pthread_mutex_t os__tlsLock = PTHREAD_MUTEX_INITIALIZER;

static taskKey *os__findTaskKey( int taskId )
{
   taskKey *tk;
   for ( tk = taskKeys; tk != NULL && tk->taskId != taskId ; tk = tk->next )
   {
   }
   return( tk );
}

static tlsKey *os__findTlsKey( tlsKey *tlskstart, void *key )
{
   tlsKey *tlsk;
   for ( tlsk = tlskstart; tlsk != NULL && tlsk->key != key ; tlsk = tlsk->next )
   {
   }
   return( tlsk );
}

int os__tlsKeyCreate( int taskId, void *key )
{
   taskKey *tk;
   tlsKey *newtlsk = NULL;

   pthread_mutex_lock( &os__tlsLock );

   tk = os__findTaskKey(taskId);
   if ( tk == NULL )
   {
      tk = (taskKey *)malloc( sizeof(taskKey) );
      if ( tk != NULL )
      {
         tk->next = taskKeys;
         tk->tlsk = NULL;
         tk->taskId = taskId;
         taskKeys = tk;
      }
   }

   if ( tk != NULL )
   {
      newtlsk = (tlsKey *)malloc ( sizeof(tlsKey) );
      newtlsk -> next = tk->tlsk;
      tk->tlsk = newtlsk;
      newtlsk->value = NULL;
      newtlsk->key = key;
   }

   pthread_mutex_unlock( &os__tlsLock );
   return( newtlsk != NULL );
}

int os__tlsSet( int taskId, void *key, void *value )
{
   taskKey *tk;
   tlsKey *tlsk = NULL;

   pthread_mutex_lock( &os__tlsLock );

   tk = os__findTaskKey(taskId);
   if ( tk != NULL )
   {
      tlsk = os__findTlsKey( tk->tlsk, key );
   }
   tlsk->value = value;

   pthread_mutex_unlock( &os__tlsLock );
   return( tlsk != NULL );
}

void *os__tlsGet( int taskId, void *key )
{
   taskKey *tk;
   tlsKey *tlsk = NULL;
   void * result = NULL;

   pthread_mutex_lock( &os__tlsLock );

   tk = os__findTaskKey(taskId);
   if ( tk != NULL )
   {
      tlsk = os__findTlsKey( tk->tlsk, key );
   }
   if ( tlsk != NULL )
   {
      result = tlsk->value;
   }

   pthread_mutex_unlock( &os__tlsLock );
   return( result );
}

int os__tlsKeyDestroy( int taskId, void *key )
{
   taskKey **tkPtr;
   taskKey *tk;
   tlsKey **tlskPtr;
   tlsKey *tlsk = NULL;

   pthread_mutex_lock( &os__tlsLock );

   for ( tkPtr = &taskKeys;
	 *tkPtr != NULL && (*tkPtr)->taskId != taskId ;
	 tkPtr = &((*tkPtr)->next) )
   {
   }
   if ( *tkPtr != NULL )
   {
      for ( tlskPtr = &((*tkPtr)->tlsk);
            *tlskPtr != NULL && (*tlskPtr)->key != key;
	    tlskPtr = &((*tlskPtr)->next) )
      {
      }
      if ( *tlskPtr != NULL )
      {
         tlsk = *tlskPtr;
         assert ( tlsk != NULL );
         *tlskPtr = tlsk->next;
         tlsk->next = NULL;
         tlsk->value = NULL;
         free ( tlsk );
      }
      if ( (*tkPtr)->tlsk == NULL )
      {
         /* No keys left, remove struct for task */
	tk = *tkPtr;
	*tkPtr = tk->next;
	tk->tlsk = NULL;
	tk->next = NULL;
	free( tk );
      }
   }

   pthread_mutex_unlock( &os__tlsLock );
   return(tlsk != NULL);
}

#if 0
#include <stdio.h>
int main( int argc, char **argv )
{
  int key1;
  int key2;
   os__tlsKeyCreate( 1, &key1 );
   os__tlsSet( 1, &key1, "Task 1 Key A" );
   os__tlsKeyCreate( 2, &key1 );
   os__tlsKeyCreate( 2, &key2 );
   os__tlsKeyCreate( 1, &key2 );
   os__tlsSet( 2, &key1, "Task 2 Key A" );
   os__tlsSet( 2, &key2, "Task 2 Key B" );
   os__tlsSet( 1, &key2, "Task 1 Key B" );

   printf("%s\n", (char *)os__tlsGet( 1, &key1 ));
   printf("%s\n", (char *)os__tlsGet( 1, &key2 ));
   printf("%s\n", (char *)os__tlsGet( 2, &key1 ));
   printf("%s\n", (char *)os__tlsGet( 2, &key2 ));

   os__tlsKeyDestroy( 1, &key1 );
   os__tlsKeyDestroy( 1, &key2 );
   os__tlsKeyDestroy( 2, &key1 );
   os__tlsKeyDestroy( 2, &key2 );

}
#endif
