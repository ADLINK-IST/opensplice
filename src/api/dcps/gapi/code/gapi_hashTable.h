/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#ifndef GAPI_HASHTABLE_H
#define GAPI_HASHTABLE_H

#include "gapi_common.h"

#define INVALID_HASH_HANDLE   ((HashHandle) -1)
#define INVALID_HASH_KEY      ((HashKey) -1)

typedef unsigned long HashKey;
typedef unsigned long HashHandle;

typedef int     (*HashCompareFunc)(void *o1, void *o2);
typedef HashKey (*HashKeyFunc)(void *o);
typedef void *  (*HashCopyFunc)(void *o);
typedef void    (*HashFreeFunc)(void *o);

C_CLASS(gapi_hashTable);

gapi_hashTable
gapi_hashTableNew (
    unsigned long   maxSize,
    unsigned long   keySize,
    HashCompareFunc compareFunc,
    HashCopyFunc    copyFunc,
    HashFreeFunc    freeFunc);

void
gapi_hashTableFree (
    gapi_hashTable hashTable);


HashHandle
gapi_hashTableInsert (
    gapi_hashTable hashTable,
    HashKey        key,
    void           *object);

void
gapi_hashTableRemoveAt (
    gapi_hashTable table,
    HashHandle     handle);

void
gapi_hashTableRemove (
    gapi_hashTable  table,
    HashKey         key,
    void           *object);


void *
gapi_hashTableAt (
    gapi_hashTable hashTable,
    HashHandle     handle);

HashKey
gapi_hashTableKey (
    gapi_hashTable hashTable,
    HashHandle     handle);

HashHandle
gapi_hashTableLookup (
    gapi_hashTable  table,
    HashKey         key,
    void           *object);


#endif
