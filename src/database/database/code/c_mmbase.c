#define MM_CLUSTER

#include "os_signature.h"
#include "os_time.h"
#include "os_heap.h"
#include "os_report.h"
#include "os_abstract.h"
#include "c_sync.h"
#include "c_mmbase.h"

#define C_MM_INITIALIZED          (0xdeadbeef)

#define ALIGNMENT                 (OS_ALIGNMENT)
#define ALIGN_COUNT(value)        (((value) + ALIGNMENT - 1) / ALIGNMENT)
#define ALIGN_SIZE(size)          (ALIGN_COUNT(size) * ALIGNMENT)
#define ALIGN_ADDRESS(address)    (ALIGN_COUNT(address) * ALIGNMENT)

#define MAX_BUCKET                (1500)
#define MAX_BUCKET_SIZE           (ALIGN_SIZE(MAX_BUCKET))
#define MAX_LISTS                 (64)
#define TRESHOLD                  (100)

#define GETMAX(a,b)               (((a)>(b))?(a):(b))

#define MmSize                    (sizeof(struct c_mm_s))
#define AdminSize                 (sizeof(struct c_mm_s))
#define ChunkHeaderSize           (ALIGN_SIZE(sizeof(struct c_mmChunk)))
#define BindingSize               (sizeof(struct c_mmBinding))
/*#define StatusSize                (sizeof(struct mmStatus_s))*/

#define ChunkAddress(chunk)       ((void *)(C_ADDRESS(chunk)+ChunkHeaderSize))
#define ChunkSize(size)           (ChunkHeaderSize + ALIGN_SIZE(size))

#define CoreSize(mm)              ((mm)->listEnd - (mm)->mapEnd)

#define SizeToMapIndex(size)      (ALIGN_COUNT(size) - 1)
#define SizeToListIndex(size)     (ALIGN_COUNT(size) % MAX_LISTS)

typedef struct c_mmChunk   *c_mmChunk;
typedef struct c_mmBinding *c_mmBinding;



struct mmStatus_s {
    c_long used;
    c_long maxUsed;
    c_long garbage;
    c_long count;
};

struct c_mm_s {
    /* only when the value of this variable is equal to C_MM_INITIALIZED
       it may be returned.
    */
    c_mmChunk    chunkMap[ALIGN_COUNT(MAX_BUCKET)];
    c_mmChunk    chunkList[MAX_LISTS];
    c_mmBinding  bindings;
    c_address    start;
    c_address    mapEnd;
    c_address    listEnd;
    c_address    end;
    c_ulong      size;
    c_ulong      fails;
    c_mutex      mapLock;
    c_mutex      listLock;
    c_mutex      bindLock;
    struct mmStatus_s   chunkMapStatus; /* protect with mapLock */
    struct mmStatus_s   chunkListStatus; /* protect with listLock */
    c_bool       shared;
#ifdef CHECK_FREEING
    c_long       chunkCount[ALIGN_COUNT(MAX_BUCKET)];
#endif /* CHECK_FREEING */
    c_ulong      initialized;
};

struct c_mmChunk {
    c_mmChunk    next;
#ifdef MM_CLUSTER
    c_mmChunk    prev;
    c_bool       freed;
#endif
    c_ulong      size;
    c_ulong      index;
};

struct c_mmBinding {
    c_mmBinding  next;
    c_char      *name;
    void        *start;
    c_long       refCount;
};

#if 0
static void       *c_mmClaimNoLock    (c_mm mm, c_ulong size);
#endif
static void        c_mmAdminInit      (c_mm mm, c_long size);
static c_mmBinding c_mmAdminLookup    (c_mm mm, const c_char *name );

/**
 * Create a new memory manager. The memory manager will manage the piece of
 * memory starting at #address# of size #size#. If size is 0, the memory manager
 * will not initialize the datastructures of the memory manager. This is
 * necessary for other threads/processes to be able to use a memory manager that
 * manages a piece of shared memory.
 * If address is NULL, the memory manager won't use the special memory manager
 * features and will just map straight to #malloc()# and #free()#. After
 * initializing the admin of the memory manager, the status are reset
 * to 0. It is the responsibility of the calling process to create a
 * \Ref{spl_stc_stat_man} to manage the status on behalf of the memory
 * manager and afterwards make a call to \Ref{c_mm_set_status_manager}
 *
 * @param address The address where the block of memory to manage starts
 * @param size The length of the block of memory, 0 if there is already a
 *    memory manager active in this piece of memory
 *
 * @return a pointer to the new created memory manager
 */
c_mm
c_mmCreate (
    void *address,
    c_long size)
{
    c_mm mm;
    os_time delay = {0, 100000000 /* 0.1 sec */};

    if (address == NULL) {  /* Create memory management on heap. */
        mm = (void *)os_malloc(AdminSize);
        memset(mm, 0, AdminSize);
        mm->shared = FALSE;
    } else {                /* Create memory management on shared memory. */
        mm = address;
        if (size == 0) {
            /* wait until the memory manager is initialized! */
            while (mm->initialized != C_MM_INITIALIZED) {
                os_nanoSleep(delay);
            }
            return mm;
        }
        memset(mm, 0, size);
        mm->shared = TRUE;
    }
    c_mmAdminInit(mm, size);

    return mm;
}

#ifdef MM_CLUSTER
static c_mmChunk
c_mmSplitChunk(
    c_mm mm,
    c_mmChunk chunk,
    c_long size)
{
    c_mmChunk remnant;
    c_ulong restSize;

    assert(size >= 0);

    restSize = chunk->size - (c_ulong)(size);
    if (restSize < ChunkSize(1)) {
        return NULL;
    }
    if (size <= MAX_BUCKET_SIZE) {
        chunk->index = SizeToMapIndex(size);
    } else {
        chunk->index = SizeToListIndex(size);
    }
    chunk->size = size;
    remnant = C_DISPLACE(chunk,ChunkSize(size));
    remnant->next = NULL;
    remnant->prev = NULL;
    remnant->freed = FALSE;
    remnant->size = restSize - ChunkHeaderSize;
    if (remnant->size <= MAX_BUCKET_SIZE) {
        chunk->index = SizeToMapIndex(remnant->size);
    } else {
        chunk->index = SizeToListIndex(remnant->size);
    }
    return remnant;
}
#endif

/**
 * Allocate a piece of memory.
 *
 * @param mm a pointer to a memory manager
 * @param size the size of the piece of memory to claim
 *
 * @return a pointer to the piece of memory.
 */
void *
c_mmMalloc(
    c_mm mm,
    c_long size)
{
    c_mmChunk  chunk = NULL;
    c_mmChunk  prvChunk;
    c_mmChunk  remnant;
    c_long     mapIndex, listIndex;

    if (size == 0) {
        assert(size != 0);
        return NULL;
    }
    if (mm->shared == FALSE) {
        return os_malloc(size);
    }

    size = ALIGN_SIZE(size);

    if (size <= MAX_BUCKET_SIZE) {
        mapIndex = SizeToMapIndex(size);
        c_mutexLock(&mm->mapLock);
        if (mm->chunkMap[mapIndex] != NULL) {
            chunk = mm->chunkMap[mapIndex];
            mm->chunkMap[mapIndex] = chunk->next;
            mm->chunkMapStatus.garbage -= (size + ChunkHeaderSize);
#ifdef CHECK_FREEING
            mm->chunkCount[mapIndex]--;
#endif
        } else if (mm->listEnd >= (mm->mapEnd + ChunkSize(size))) {
            chunk = (c_mmChunk)mm->mapEnd;
            mm->mapEnd  = mm->mapEnd + ChunkSize(size);
            chunk->size = size;
            chunk->index = mapIndex;
	} else {
            pa_increment(&mm->fails);
            c_mutexUnlock(&mm->mapLock);
            OS_REPORT_2(OS_ERROR,"c_mmbase",0,
                        "Memory claim denied: required size (%d) exceeds available resources (%d)!",
                         ChunkSize(size),(mm->listEnd+ChunkSize(size)-mm->mapEnd));
            return NULL;
	}
        mm->chunkMapStatus.used   += (size + ChunkHeaderSize);
        mm->chunkMapStatus.maxUsed = GETMAX(mm->chunkMapStatus.used, mm->chunkMapStatus.maxUsed);
        mm->chunkMapStatus.count++;
        c_mutexUnlock(&mm->mapLock);
    } else {
        prvChunk = NULL;
        remnant = NULL;
        listIndex = SizeToListIndex(size);

        c_mutexLock(&mm->listLock);
        chunk = mm->chunkList[listIndex];
        while (chunk != NULL) {
            if (chunk->size >= (c_ulong)size) {
                if (chunk->size <= (c_ulong)(size + TRESHOLD)) {
                    /* found excelent match */
                    break;
                } else {
#ifdef MM_CLUSTER
                    if (CoreSize(mm) < ChunkSize(size)) {
                        /* No resources left in core memory.
                         * But have found a significant bigger freed block so will split the block.
			 * Unlocking and re-locking is not efficient but at this point 
			 * the operational state must be considered degraded.
			 */
                        remnant = c_mmSplitChunk(mm,chunk,size);
                        c_mutexUnlock( &mm->listLock );
                        c_mmFree(mm,ChunkAddress(remnant));
                        c_mutexLock(&mm->listLock);
                        break;
                    }
#endif
                    chunk = NULL;
                }
            } else {
                prvChunk = chunk;
                chunk = chunk->next;
            }
        }
        if (chunk != NULL) {
            if (prvChunk == NULL) {
                mm->chunkList[listIndex] = chunk->next;
            } else {
                prvChunk->next = chunk->next;
            }
#ifdef MM_CLUSTER
            if (chunk->next != NULL) {
                chunk->next->prev = chunk->prev;
            }
#endif
            mm->chunkListStatus.garbage -= (chunk->size + ChunkHeaderSize);
        } else {
            c_mutexLock(&mm->mapLock);
            mm->listEnd -= ChunkSize(size);
            if (mm->listEnd < mm->mapEnd) {
                pa_increment(&mm->fails);
                c_mutexUnlock(&mm->mapLock);
                c_mutexUnlock(&mm->listLock);
                OS_REPORT_2(OS_ERROR,"c_mmbase",0,
                            "Memory claim denied: "
                            "required size (%d) exceeds available resources (%d)!",
                             ChunkSize(size),(mm->listEnd+ChunkSize(size)-mm->mapEnd));
                return NULL;
            }
            chunk = (c_mmChunk)mm->listEnd;
            chunk->size = size;
            chunk->index = listIndex;
            c_mutexUnlock(&mm->mapLock);
	}
        mm->chunkListStatus.used += (size + ChunkHeaderSize);
        mm->chunkListStatus.maxUsed = GETMAX(mm->chunkListStatus.used,
                                             mm->chunkListStatus.maxUsed);
        mm->chunkListStatus.count++;
        c_mutexUnlock(&mm->listLock);
    }
    chunk->next = NULL;
#ifdef MM_CLUSTER
    chunk->prev = NULL;
    chunk->freed = FALSE;
#endif
    return ChunkAddress(chunk);
}

#if 0
/*
 * This is a special, lockless version of malloc, for initialization
 * purposes. It does not peek into buckets to see whether a piece of
 * memory of that size exists. This prevents us from having to implement
 * special lockless versions of find_chunk, core_claim etc.
 * Whenever you use this function, be sure that you are the only process
 * using the memory-manager!
 */
static void *
c_mmClaimNoLock(
    c_mm mm,
    c_ulong size)
{
    c_mmChunk  chunk;

    size = ALIGN_SIZE( size );

    if (size <= MAX_BUCKET_SIZE) {
        chunk = (c_mmChunk)mm->mapEnd;
        mm->mapEnd = mm->mapEnd + ChunkSize(size);
    } else {
        mm->listEnd -= ChunkSize(size);
        chunk = (c_mmChunk)mm->listEnd;
    }

    if (mm->listEnd < mm->mapEnd) {
        OS_REPORT(OS_ERROR,"c_mmbase",0,
                  "Memory claim denied: required size exceeds resources!");
        return NULL;
    }

    chunk->size = size;
    chunk->next = NULL;
#ifdef MM_CLUSTER
    chunk->prev = NULL;
    chunk->freed = FALSE;
#endif

    return ChunkAddress(chunk);
}
#endif

#ifdef MM_CLUSTER
static c_mmChunk
c_mmCluster(
    c_mm mm,
    c_mmChunk chunk)
{
    c_ulong size = 0;
    c_mutex *lockRef;
    c_mmChunk next;

    assert(chunk != NULL);
    assert(chunk->freed == TRUE);

    next = C_DISPLACE(chunk,ChunkSize(chunk->size));
    if (C_ADDRESS(next) >= mm->end) {
        return chunk;
    }
    assert((C_ADDRESS(next) >= mm->start) && (C_ADDRESS(next) <= mm->end));
    assert((C_ADDRESS(next) >= mm->listEnd) || (C_ADDRESS(next) <= mm->mapEnd));
    size = next->size;

    if (next->freed == TRUE) {
        /* Dangerous situation, <next> isn't locked. */
        if (next->size <= MAX_BUCKET_SIZE) {
            lockRef = &mm->mapLock;
        } else {
            lockRef = &mm->listLock;
        }
        c_mutexLock(lockRef);
        /* recheck <next> for consistency */
        if ((next->freed == TRUE) &&
            (size == next->size) &&
            (next->prev != NULL)) {
        }
        c_mutexUnlock(lockRef);
    }
    return chunk;
}
#endif

/**
 * Free a piece of memory.
 *
 * @param mm a pointer to a memory manager
 * @param memory a pointer to the piece of memory to free
 */
void
c_mmFree(
    c_mm mm,
    void *memory)
{
    c_mmChunk  chunk;
    c_mmChunk  curChunk;
    c_mmChunk  prvChunk;
    c_long     mapIndex, listIndex;

    assert(mm != NULL);

    if (memory == NULL) {
        return;
    }
    if (mm->shared == TRUE) {
        assert(C_ADDRESS(memory) > C_ADDRESS(mm->start));
        assert(C_ADDRESS(memory) < C_ADDRESS(mm->end));
    } else {
        os_free(memory);
        return;
    }

    chunk = (c_mmChunk)(C_ADDRESS(memory) - ChunkHeaderSize);

#ifdef MM_CLUSTER
    chunk->freed = TRUE;
#if 0 /* don't call if not implemented. */
    chunk = c_mmCluster(mm,chunk);
    if (chunk == NULL) { return; }
#endif
#endif

#ifdef OSPL_STRICT_MEM
    {
       unsigned int i;
       unsigned char *cmemory = (unsigned char *)memory;
       for ( i = 0; i+7 < chunk->size; i++ ) {
          assert( OS_MAGIC_SIG_CHECK( &cmemory[i] ) );
       }
    }
#endif

    if (chunk->size <= MAX_BUCKET_SIZE) {
//        mapIndex = SizeToMapIndex(chunk->size);
        mapIndex = chunk->index;
        c_mutexLock(&mm->mapLock);
#ifdef CHECK_FREEING
        mm->chunkCount[mapIndex]++;
        curChunk = mm->chunkMap[mapIndex];
        while (curChunk != NULL) {
            if (curChunk == chunk) {
                c_mutexUnlock(&mm->mapLock);
                printf( "Trying to free the same pointer twice!" );
                return;
            }
            curChunk =  curChunk->next;
        }
#endif /* CHECK_FREEING */
#ifdef MM_CLUSTER
        chunk->prev = NULL;
        if (mm->chunkMap[mapIndex] != NULL) {
            mm->chunkMap[mapIndex]->prev = chunk;
        }
#endif
        chunk->next = mm->chunkMap[mapIndex];
        mm->chunkMap[mapIndex] = chunk;
        mm->chunkMapStatus.garbage += (chunk->size + ChunkHeaderSize);
        mm->chunkMapStatus.used -= (chunk->size + ChunkHeaderSize);
        mm->chunkMapStatus.count--;
        assert(mm->chunkMapStatus.used >= 0);
        c_mutexUnlock(&mm->mapLock);
    } else {
        prvChunk = NULL;
//        listIndex = SizeToListIndex(chunk->size);
        listIndex = chunk->index;

        c_mutexLock(&mm->listLock);

        curChunk = mm->chunkList[listIndex];

        while ((curChunk != NULL) && (curChunk->size < chunk->size)) {
            prvChunk = curChunk;
            curChunk = curChunk->next;
        }

        if (curChunk == chunk) {
            c_mutexUnlock(&mm->mapLock);
            printf( "Trying to free the same pointer twice!" );
            assert(curChunk != chunk);
            return;
        }
#ifdef MM_CLUSTER
        chunk->prev = prvChunk;
        if (curChunk) {
            curChunk->prev = chunk;
        }
#endif
        chunk->next = curChunk;
        if (prvChunk == NULL) {
            mm->chunkList[listIndex] = chunk;
        } else {
            prvChunk->next = chunk;
        }

        mm->chunkListStatus.garbage += (chunk->size + ChunkHeaderSize);
        mm->chunkListStatus.used -= (chunk->size + ChunkHeaderSize);
        mm->chunkListStatus.count--;
        /* assert(mm->chunkListStatus.used >= 0);
         * the used of the chunkListStatus can become negative, since the
         * chunkMapStatus is updated for both allocations <=MAX_BUCKET_SIZE and
         * >MAX_BUCKET_SIZE
         */
        c_mutexUnlock(&mm->listLock);
    }

}


/**
 * Detach this memory-manager from the system and free the locally held
 * resources. This means that all resources held by the memory-manager
 * that are not allocated in shared memory are freed.
 *
 * @param mm the memory-manager to free
 *
 */
void
c_mmDestroy (
    c_mm mm )
{
    c_mmBinding temp;

    if ( mm->size == 0 ) {
        c_mutexUnlock( &mm->bindLock );
        c_mutexUnlock( &mm->mapLock );
        c_mutexUnlock( &mm->listLock );

        while ( mm->bindings ) {
            temp = mm->bindings;
            mm->bindings = mm->bindings->next;

            c_mmFree(mm,temp->start);
            c_mmFree(mm,temp->name);
            c_mmFree(mm,temp);
        }
    }
    if (mm->shared == FALSE) {
        os_free(mm);
    }
}

void
c_mmAdminInit (
    c_mm mm,
    c_long size )
{
    c_long mapIndex, listIndex;

    /* Determine the start of our memory managed block, without the admin
     * data, rounding it up to the next alignment boundary */
    mm->start = ALIGN_ADDRESS(C_ADDRESS(mm) + AdminSize);

    /* Determine the end of our memory managed block, rounding it down to the
     * nearest alignment boundary */
    mm->end = ALIGN_ADDRESS(C_ADDRESS(mm) + size);
    mm->end = ALIGN_ADDRESS((C_ADDRESS(mm->end) / ALIGNMENT ) * ALIGNMENT );

    mm->size = C_ADDRESS(mm->end) - C_ADDRESS(mm->start);

    for ( mapIndex = ALIGN_COUNT(MAX_BUCKET) - 1; mapIndex >= 0; mapIndex-- ) {
        mm->chunkMap[mapIndex] = NULL;
#ifdef CHECK_FREEING
        mm->chunkCount[mapIndex] = 0;
#endif /* CHECK_FREEING */
    }

    for ( listIndex = ALIGN_COUNT(MAX_LISTS) - 1; listIndex >= 0; listIndex-- ) {
        mm->chunkList[listIndex] = NULL;
    }

    mm->mapEnd   = mm->start;
    mm->listEnd  = mm->end;
    mm->bindings = NULL;

    mm->size = size - AdminSize;
    mm->fails = 0;

    mm->chunkMapStatus.used    = 0;
    mm->chunkMapStatus.maxUsed = 0;
    mm->chunkMapStatus.garbage = 0;
    mm->chunkMapStatus.count   = 0;

    mm->chunkListStatus.used    = 0;
    mm->chunkListStatus.maxUsed = 0;
    mm->chunkListStatus.garbage = 0;
    mm->chunkListStatus.count   = 0;

    c_mutexInit(&mm->listLock,SHARED_MUTEX);
    c_mutexInit(&mm->mapLock,SHARED_MUTEX);
    c_mutexInit(&mm->bindLock,SHARED_MUTEX);
    mm->initialized = C_MM_INITIALIZED;
}


void *
c_mmBind (
    c_mm mm,
    const c_char *name,
    void *memory )
{
    c_mmBinding curBind;

    c_mutexLock( &mm->bindLock );

    if (mm->bindings) {
        curBind = c_mmAdminLookup( mm, name );

        if ( curBind == NULL ) {
            curBind = (c_mmBinding)c_mmMalloc(mm, ALIGN_SIZE(BindingSize ));
            curBind->name = (char *)c_mmMalloc(mm, ALIGN_SIZE(strlen(name)+1));
            strcpy( curBind->name, name );
            curBind->start = memory;
            curBind->refCount = 0;

            curBind->next = mm->bindings;
            mm->bindings = curBind;
        }
    } else {
        curBind = (c_mmBinding)c_mmMalloc(mm, ALIGN_SIZE(BindingSize));
        curBind->name = (char *)c_mmMalloc(mm, ALIGN_SIZE(strlen(name)+1));
        strcpy( curBind->name, name );
        curBind->start = memory;
        curBind->refCount = 0;

        curBind->next = NULL;
        mm->bindings = curBind;
    }

    curBind->refCount++;

    c_mutexUnlock( &mm->bindLock );

    return curBind->start;
}


void
c_mmUnbind (
    c_mm mm,
    const c_char *name )
{
    c_mmBinding curBind;
    c_mmBinding prevBind;

    c_mutexLock( &mm->bindLock );

    curBind = mm->bindings;
    prevBind = NULL;

    while (curBind) {
        if (strcmp(curBind->name, name) == 0) {
            if ( curBind->refCount > 1 ) {
                curBind->refCount--;
            } else {
                if (prevBind) {
                    prevBind->next = curBind->next;
                } else {
                    mm->bindings = curBind->next;
                }
                c_mmFree(mm, curBind->start );
                c_mmFree(mm, curBind->name );
                c_mmFree(mm, (void *)curBind );
            }
            c_mutexUnlock( &mm->bindLock );
        }
        prevBind = curBind;
        curBind  = curBind->next;
    }
    c_mutexUnlock( &mm->bindLock );
}


/**
 * An internal function that looks up a chunk of memory based on the name that
 * is passed to it. Names cannot be longer than 7 characters, excluding the
 * trailing \0-character. Names longer than that will have only the 7 first
 * characters evaluated for a match.
 *
 * @param admin A pointer to the admin structures of the mm
 * @param name The name of the region to look up
 *
 * @return A pointer to a c_mmBinding structure when the name is found, NULL
 * when no match could be found.
 *
 * @memo Will look up a named region based on a name
 */
static c_mmBinding
c_mmAdminLookup(
    c_mm mm,
    const c_char *name )
{
    c_mmBinding curBind;

    curBind = mm->bindings;

    while (curBind) {
        if (strncmp(curBind->name, name, 7) == 0) {
            return curBind;
        } else {
            curBind = curBind->next;
        }
    }
    return NULL;
}


void *
c_mmLookup(
    c_mm mm,
    const c_char *name )
{
    c_mmBinding curBind;

    c_mutexLock( &mm->bindLock );

    if (  mm->bindings ) {
        curBind = c_mmAdminLookup( mm, name );

        if ( curBind == NULL ) {
            c_mutexUnlock( &mm->bindLock );
            return NULL;
        } else {
            curBind->refCount++;
            c_mutexUnlock( &mm->bindLock );
            return curBind->start;
        }
    } else {
        c_mutexUnlock( &mm->bindLock );
        return NULL;
    }
}

void *
c_mmAddress (
    c_mm mm)
{
    return (void *)mm;
}


c_mmStatus
c_mmMapState (
    c_mm mm)
{
    c_mmStatus s;

    s.fails = mm->fails;
    s.size = CoreSize(mm);

    c_mutexLock(&mm->mapLock);
    s.used    = mm->chunkMapStatus.used;
    s.maxUsed = mm->chunkMapStatus.maxUsed;
    s.garbage = mm->chunkMapStatus.garbage;
    s.count   = mm->chunkMapStatus.count;
    c_mutexUnlock(&mm->mapLock);

    return s;
}

c_mmStatus
c_mmListState (
    c_mm mm)
{
    c_mmStatus s;

    s.fails = mm->fails;
    s.size = CoreSize(mm);

    c_mutexLock(&mm->listLock);
    s.used    = mm->chunkListStatus.used;
    s.maxUsed = mm->chunkListStatus.maxUsed;
    s.garbage = mm->chunkListStatus.garbage;
    s.count   = mm->chunkListStatus.count;
    c_mutexUnlock(&mm->listLock);

    return s;
}

c_mmStatus
c_mmState (
    c_mm mm)
{
    c_mmStatus s;

    s.fails = mm->fails;
    s.size = mm->size;

    c_mutexLock(&mm->mapLock);
    s.used    = mm->chunkMapStatus.used;
    s.maxUsed = mm->chunkMapStatus.maxUsed;
    s.garbage = mm->chunkMapStatus.garbage;
    s.count   = mm->chunkMapStatus.count;
    c_mutexUnlock(&mm->mapLock);

    c_mutexLock(&mm->listLock);
    s.used    += mm->chunkListStatus.used;
    s.maxUsed  = s.maxUsed + mm->chunkListStatus.maxUsed;
    s.garbage += mm->chunkListStatus.garbage;
    s.count   += mm->chunkListStatus.count;
    c_mutexUnlock(&mm->listLock);

    return s;
}

