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
#include "gapi_common.h"
#include "gapi_hashTable.h"

#include "os_heap.h"


#define ALLOC_UNITS  256

typedef struct HashTableEntry {
    HashKey                key;
    struct HashTableEntry *next;
    struct HashTableEntry *prev;
    void                  *object;
} HashTableEntry;
 
C_STRUCT(gapi_hashTable) {
    unsigned long    keySize;
    unsigned long    maxSize;  
    unsigned long    size;
    unsigned long    increment;
    gapi_boolean     owns;
    HashCompareFunc  compareFunc;
    HashCopyFunc     copyFunc;
    HashFreeFunc     freeFunc;
    HashKeyFunc      keyFunc;
    HashTableEntry  *freeList;
    HashTableEntry  *keyList;
    HashTableEntry  *contents;
};

static HashTableEntry *
findHashTableEntry
(
    gapi_hashTable  hashTable,
    HashKey         key,
    void           *object
    );

static unsigned long
reallocateTableContents (
    gapi_hashTable table
    );

static HashTableEntry *
hashTableEntryNew (
    gapi_hashTable table
    );

static void
hashTableEntryFree (
    gapi_hashTable  table,
    HashTableEntry *entry
    );

static void
insertTableEntry (
    HashTableEntry *after,
    HashTableEntry *entry
    );

static void
removeTableEntry (
    HashTableEntry *entry
    );



gapi_hashTable
gapi_hashTableNew (
    unsigned long   maxSize,        
    unsigned long   keySize,
    HashCompareFunc compareFunc,
    HashCopyFunc    copyFunc,
    HashFreeFunc    freeFunc    
    )
{
    gapi_hashTable table;

    assert(keySize > 0);

    table = (gapi_hashTable) os_malloc(C_SIZEOF(gapi_hashTable));
    if ( table != NULL ) {
        memset(table, 0, C_SIZEOF(gapi_hashTable));
        
        table->keyList  = (HashTableEntry *) os_malloc(keySize * sizeof(HashTableEntry));
        
        if ( table->keyList != NULL ) {
            unsigned long i;

            memset(table->keyList, 0, keySize * sizeof(HashTableEntry));

            for ( i = 0; i < keySize; i++ ) {
                table->keyList[i].prev = &table->keyList[i];
                table->keyList[i].next = &table->keyList[i];
                table->keyList[i].key  = i;
            }
            
            table->increment   = ALLOC_UNITS;
            table->keySize     = keySize;
            table->maxSize     = maxSize;
            table->compareFunc = compareFunc;
            if ( copyFunc != NULL ) {
                table->copyFunc = copyFunc;
                table->owns     = TRUE;
            }
            if ( freeFunc != NULL ) {
                table->freeFunc = freeFunc;
                table->owns     = TRUE;
            }

            if ( reallocateTableContents(table) == 0 ) {
                os_free(table->keyList);
                os_free(table);
                table = NULL;
            }
        } else {
            os_free(table);
            table = NULL;
        }
    }

    return table;
}

void
gapi_hashTableFree (
    gapi_hashTable table
    )
{
    if ( table != NULL ) {
        if ( table->owns ) {
            unsigned long i;
            
            for ( i = 0; i < table->size; i++ ) {
                if ( table->contents[i].object != NULL ) {
                    if ( table->freeFunc ) {
                        table->freeFunc(table->contents[i].object);
                    } else {
                        os_free(table->contents[i].object);
                    }
                }
            }
        }
        os_free(table->keyList);
        os_free(table->contents);
        os_free(table);
    }
}


HashHandle
gapi_hashTableInsert (
    gapi_hashTable  table,
    HashKey         key,
    void           *object
    )
{
    HashHandle      handle = INVALID_HASH_HANDLE;
    HashTableEntry *entry;
    
    assert(table != NULL);
    assert(object != NULL);
    assert(key < table->keySize);

    entry = findHashTableEntry(table, key, object);

    if ( entry == NULL ) {
        entry = hashTableEntryNew(table);
        if ( entry != NULL ) {
            entry->key    = key;
            if ( table->copyFunc ) {
                entry->object = table->copyFunc(object);
            } else {
                entry->object = object;
            }
            
            insertTableEntry(&table->keyList[key], entry);
            handle = (HashHandle)(entry - table->contents);
        }
    } else {
        handle = (HashHandle)(entry - table->contents);
    }
            
    return handle;
}

void
gapi_hashTableRemoveAt (
    gapi_hashTable table,
    HashHandle     handle
    )
{
    assert(table != NULL);
    
    if ( (handle != INVALID_HASH_HANDLE) && (handle < table->size) ) {
        HashTableEntry *entry = &table->contents[handle];
        if ( entry->object != NULL ) {
            removeTableEntry(entry);
            hashTableEntryFree(table, entry);
        }
    }
}

void
gapi_hashTableRemove (
    gapi_hashTable table,
    HashKey        key,
    void           *object
    )
{
    HashTableEntry *entry;

    entry = findHashTableEntry(table, key, object);

    if ( entry != NULL ) {
        removeTableEntry(entry);
        hashTableEntryFree(table, entry);
    }
}

void *
gapi_hashTableAt (
    gapi_hashTable table,
    HashHandle     handle
    )
{
    void *object = NULL;
    
    assert(table != NULL);
    
    if ( (handle != INVALID_HASH_HANDLE) && (handle < table->keySize) ) {
        object = table->contents[handle].object;
    }

    return object;
}

HashKey
gapi_hashTableKey (
    gapi_hashTable table,
    HashHandle     handle
    )
{
    HashKey key = INVALID_HASH_HANDLE;
    
    assert(table != NULL);
    
    if ( (handle != INVALID_HASH_HANDLE) && (handle < table->keySize) ) {
        key = table->contents[handle].key;
    }

    return key;
}

HashHandle
gapi_hashTableLookup (
    gapi_hashTable  table,
    HashKey         key,
    void           *object
    )
{
    HashHandle      handle = INVALID_HASH_HANDLE;
    HashTableEntry *entry;
    
    assert(table != NULL);
    assert(object != NULL);
    assert(key < table->keySize);

    entry = findHashTableEntry(table, key, object);

    if ( entry != NULL ) {
        handle = (HashHandle)(entry - table->contents);
    }
    
    return handle;
}

static HashTableEntry *
findHashTableEntry (
    gapi_hashTable  table,
    HashKey         key,
    void           *object)
{
    HashTableEntry *entry = NULL;
    HashTableEntry *ptr;
    
    ptr = table->keyList[key].next;

    while ( (entry == NULL) && (ptr->object != NULL) ) {
        if ( table->compareFunc(ptr->object, object) ) {
            entry = ptr;
        } else {
            ptr = ptr->next;
        }   
    }

    return entry;
}

static unsigned long
reallocateTableContents (
    gapi_hashTable table
    )
{
    HashTableEntry *newContents;
    unsigned long   size;

    size = table->size + table->increment;
    if ( (table->maxSize > 0) && (size > table->maxSize) ) {
        size = table->maxSize;
    }

    if ( size > table->size ) {
        newContents = (HashTableEntry *) os_malloc(size * sizeof(HashTableEntry));

        if ( newContents != NULL ) {
            unsigned long i;

            for ( i = 0; i < table->size; i++ ) {
                newContents[i] = table->contents[i];
            }

            for ( i = table->size; i < size; i++ ) {
                newContents[i].key    = INVALID_HASH_HANDLE;
                newContents[i].object = NULL;
                if ( i < (size - 1) ) {
                    newContents[i].next = &newContents[i+1];
                } else {
                    newContents[i].next = NULL;
                }
            }

            if ( table->size > 0 ) {
                os_free(table->contents);
            }

            table->contents = newContents;
            table->freeList = &newContents[table->size];
            table->size     = size;
        } else {
            size = 0;
        }
    } else {
        size = 0;
    }

    return size;
}
        
static HashTableEntry *
hashTableEntryNew (
    gapi_hashTable table
    )
{
    HashTableEntry *entry = NULL;
    
    if ( table->freeList == NULL ) {
        reallocateTableContents(table);
    }

    if ( table->freeList != NULL ) {
        entry = table->freeList;
        table->freeList = entry->next;
        entry->next = NULL;
    }

    return entry;
}

static void
hashTableEntryFree (
    gapi_hashTable  table,
    HashTableEntry *entry
    )
{
    if ( table->owns ) {
        if ( table->freeFunc ) {
            table->freeFunc(entry->object);
        } else {
            os_free(entry->object);
        }
    }
    entry->object = NULL;
    entry->prev   = NULL;
    entry->next   = table->freeList;
    

    table->freeList = entry;
}


static void
insertTableEntry (
    HashTableEntry *after,
    HashTableEntry *entry
    )
{
    entry->prev = after;
    entry->next = after->next;
    after->next->prev = entry;
    after->next = entry;
}

static void
removeTableEntry (
    HashTableEntry *entry
    )
{
    entry->next->prev = entry->prev;
    entry->prev->next = entry->next;
}
    

        

        
        


