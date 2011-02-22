/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#include "c_mmCache.h"
#include <assert.h>

#ifndef NDEBUG
#define C_MM_CONFIDENCE (0x214D444A)
#endif /* NDEBUG */

#define c_sample(o) ((c_sample)(o))
#define c_block(o)  ((c_block)(o))

#define sampleHeaderSize \
        C_MAXALIGNSIZE(C_SIZEOF(c_sample))

#define sampleSize(size) \
        C_MAXALIGNSIZE(sampleHeaderSize + size)

#define blockSize(count,size) \
        (C_MAXALIGNSIZE(C_SIZEOF(c_block)) + count * sampleSize(size))

#define blockFreeSize(cache) \
        (cache->size * cache->count)

#define blockSample(block,size,count) \
        c_sample((c_address)block + blockSize(count,size))

#define sampleObject(sample) \
        c_sample((c_address)sample + sampleHeaderSize)

#define blockObject(block,size,count) \
        sampleObject(blockSample(block,size,count))

typedef struct c_sample_s *c_sample;
typedef struct c_block_s  *c_block;

struct c_mmCache_s {
    c_mm mm;             /* The memory manager used to allocate memory from. */
    c_address size;      /* The memory size of the objects to be allocated by this cache. */
    c_long  count;       /* The number of object of the block allocated per block of memory. */
    c_voidp last;        /* The last block in the cache allocated block list. */
    c_voidp cache;       /* The first block in the cache allocated block list. */
    c_voidp incomplete;  /* The first block having free space in the cache allocated block list. */
    c_size  allocated;   /* The total size in bytes of all currently allocated blocks. */
    c_long  used;        /* NOT USED, SO MAY BE REMOVED. */
    c_size  free;        /* The number of free bytes available in the allocated blocks. */
#ifndef NDEBUG
    c_long  access;      /* The number of simultaneous accesses, MUST ALWAYS BE < 2. */
#endif
};

struct c_sample_s {
    c_sample nextFree;   /* The next free sample in the block, only valid if the sample itself is free. */
    c_block  block;      /* The reference to the block that contains this sample. */
#ifdef C_MM_CONFIDENCE
    c_ulong confidence;
#endif /* C_MM_CONFIDENCE */
};

struct c_block_s {
    c_block  next;       /* The next block in mmCache. Is NULL in case of being the last block. */
    c_block  prev;       /* The previous block in the mmCache block list. Is NULL in case of being the first block. */
    c_long   used;       /* The number of object in the block that are in use. */
    c_long   count;      /* The number of objects in the block that are used at least once. */
    c_sample firstFree;  /* The first free sample in this block. */
};

c_mmCache
c_mmCacheCreate (
    c_mm mm,
    c_long size,
    c_long count)
{
    c_mmCache _this;

    assert(mm != NULL);
    assert(size > 0);
    assert(count > 0);

    /* Do not use c_mmMalloc here, because the created cache has to be enlisted
     * in the memory manager for proper memory usage-statistics. */
    _this = c_mmMallocCache(mm,sizeof(struct c_mmCache_s));
    _this->mm = mm;
    _this->size = size;
    _this->count = count;
    _this->last = NULL;
    _this->cache = NULL;
    _this->incomplete = NULL;
    _this->allocated = 0;
    _this->used = 0;
    _this->free = 0;
#ifndef NDEBUG
    _this->access = 0;
#endif

    return _this;
}

void
c_mmCacheDestroy (
    c_mmCache _this)
{
    c_block b;
    c_block remove;

    assert(_this != NULL);
    /* assert(_this->cache == NULL); */
    /* assert(_this->incomplete == NULL); */
    /* assert(_this->last == NULL); */
    /* assert(_this->allocated == 0); */
    /* assert(_this->free == 0); */

    b = _this->cache;
    while (b != NULL) {
        remove = b;
        b = b->next;
        c_mmFree(_this->mm, remove);
    }

    _this->cache = NULL;
    _this->incomplete = NULL;
    _this->last = NULL;

    /* Do not use c_mmFree here, because the created cache has to be removed
     * from the cache-list in the memory manager. */
    c_mmFreeCache(_this->mm,_this);
}

void *
c_mmCacheMalloc (
    c_mmCache _this)
{
    c_voidp  object;
    c_long   size;
    c_block  block, ref;
    c_sample sample;

#ifndef NDEBUG
    _this->access++;
    assert(_this->access == 1);
#endif

    if (_this->incomplete == NULL) {
        /** There are no blocks that have free space so a new
            block must be allocated. This block will be the only
            incomplete block and therefore _this->incomplete must
            be set to this block as well as _this->last.
        */
        size = blockSize(_this->count,_this->size);
        block = c_mmMalloc(_this->mm,size);
        block->prev = _this->last;
        block->next = NULL;
        block->used = 1;
        block->count = 1;
        block->firstFree = NULL;
        if (_this->last != NULL) {
            c_block(_this->last)->next = block;
        }
        _this->last = block;
        if (block->count < _this->count) {
            /* In the rare situation that blocks are specified to
               provide space for only one object _this->incomplete
               must NOT be set to the new block.
            */
            _this->incomplete = block;
        }
        if (_this->cache == NULL) {
            _this->cache = block;
        }
        /** Get a reference to the first sample and initialise the sample.
        */
        sample = blockSample(block,_this->size,0);
#ifdef C_MM_CONFIDENCE
        sample->confidence = C_MM_CONFIDENCE;
#endif /* C_MM_CONFIDENCE */
        sample->block = block;
        /** Get the address of the first object to be returned.
        */
        object = sampleObject(sample);
        /** Update the cache statistics.
        */
        _this->allocated += size;
        /* While blockSize(_this->count,_this->size) memory has been allocated,
         * only _this->size * _this->count is available (rest is headers), so
         * add that amount to the free-memory. */
        _this->free += blockFreeSize(_this);
    } else {
        block = _this->incomplete;
        if (block->firstFree == NULL) {
            assert(block->count <= _this->count);
            sample = blockSample(block,_this->size,block->count);
#ifdef C_MM_CONFIDENCE
            sample->confidence = C_MM_CONFIDENCE;
#endif /* C_MM_CONFIDENCE */
            sample->block = block;
            object = sampleObject(sample);
            block->count++;
        } else {
            sample = block->firstFree;
            block->firstFree = sample->nextFree;
#ifdef C_MM_CONFIDENCE
            sample->confidence = C_MM_CONFIDENCE;
#endif /* C_MM_CONFIDENCE */
            sample->nextFree = NULL;
            object = sampleObject(sample);
        }
        block->used++;
        if (block->used == _this->count) {
            _this->incomplete = block->next;
        } else {
            ref = block;
            while ((ref->prev != NULL) && (ref->prev->used < block->used)) {
                ref = ref->prev;
            }
#ifndef NDEBUG
            assert(_this->access == 1);
#endif
            if (ref != block) {
                /* need to reorder blocks because blocks are ordered */
                /* by available space. Less space is put in front of */
                /* blocks having more space.                         */
                /* block must be placed in front of ref.             */
                if (block->next != NULL) {
                    block->next->prev = block->prev;
                } else {
                    /* Block is last in the row so _this->last needs */
                    /* to be updated to the forelast block.          */
                    assert(_this->last == block);
                    _this->last = block->prev;
                }
                assert(block->prev != NULL);
                block->prev->next = block->next;
                /* block is now removed from the list. */
                /* Now insert block before ref.        */
                if (ref->prev != NULL) {
                    ref->prev->next = block;
                }
                block->prev = ref->prev;
                ref->prev = block;
                block->next = ref;
                if (block->prev == NULL) {
                    /* Block is first in the row so _this->cache needs */
                    /* to be updated to this block.                    */
                    _this->cache = block;
                }
                if (_this->incomplete == block->next) {
                    _this->incomplete = block;
                }
            }
        }
    }
    /* One extra slot has been malloc'd , so substract '1 * size' from free.*/
    _this->free -= _this->size;

#ifndef NDEBUG
    assert(_this->access == 1);
#endif

#ifndef NDEBUG
    {
        /** This piece of debugging code verifies the consistentcy of the
            c_mmCache. The code will walk over all allocated memory blocks
            and verify if all prev and next pointers have correct values.
            This code also verifies if the first incomplete block encountered
            during the walk is the same as refered by _this->incomplete.
            If an assertion fails in this code then it is likely that the
            consistency is corrupted during this method because all actions
            on the cache verify consistentcy at the end of the action. But also
            multy threaded access may have corrupted the consistency since the
            cache itself does not protect against MT access.
        */
        c_bool incomplete = FALSE;

        block = _this->cache;
        if (block != NULL) {
            /** The current block is the first block in the list.
                So now verify if the block->prev field is NULL.
            */
            assert(block->prev == NULL);
        }
        while (block != NULL) {
            if (!incomplete && block->used < _this->count) {
                incomplete = TRUE;
                /** The current block is the first incomplete block in the list.
                    So now verify if the _this->incomplete field refers to this block.
                */
                assert(_this->incomplete == block);
            }
            if (block->next != NULL) {
                /* The current block is not the last so verify if the next block refers
                   to this block and verify that the next block does not have more free
                   space then this block. All blocks are ordered by available free space.
                   Less space blocks must be in front of more space blocks in the list.
                */
                assert(block->next->prev == block);
                assert(block->next->used <= block->used);
            } else {
                /** The current block is the last block in the list.
                   So now verify if the _this->last field refers to this block.
                */
                assert(_this->last == block);
            }
            block = block->next;
        }
    }
#endif
#ifndef NDEBUG
    _this->access--;
    assert(_this->access == 0);
#endif /* NDEBUG */
#ifdef C_MM_CONFIDENCE
    sample = c_sample((c_address)object - sampleHeaderSize);
    assert(sample->confidence == C_MM_CONFIDENCE);
#endif /* C_MM_CONFIDENCE */

    return object;
}

void
c_mmCacheFree (
    c_mmCache _this,
    void *memory)
{
    c_block block, ref;
    c_sample sample;
    c_long size;

    assert(_this != NULL);
    assert(memory != NULL);

#ifndef NDEBUG
    _this->access++;
    assert(_this->access == 1);
#endif

    sample = c_sample((c_address)memory - sampleHeaderSize);
#ifdef C_MM_CONFIDENCE
    assert(sample->confidence == C_MM_CONFIDENCE);
    sample->confidence = 0;
#endif /* C_MM_CONFIDENCE */
    block = sample->block;
    assert(block->used > 0);
    block->used--;
#if 0
    if ((block->used == 0) && (block->next)) {
#else
    if (block->used == 0) {
#endif
        if (block->prev == NULL) {
            _this->cache = block->next;
        } else {
            block->prev->next = block->next;
        }
        if (block->next != NULL) {
            block->next->prev = block->prev;
        }
        if (_this->last == block) {
            _this->last = block->prev;
        }
        if (_this->incomplete == block) {
            _this->incomplete = block->next;
        }
        c_mmFree(_this->mm, block);
        size = blockSize(_this->count,_this->size);
        _this->allocated -= size;
        _this->free -= blockFreeSize(_this);;
    } else {
        sample->nextFree = block->firstFree;
        block->firstFree = sample;
        ref = block;
        while ((ref->next != NULL) && (ref->next->used >= block->used)) {
            ref = ref->next;
        }
#ifndef NDEBUG
        assert(_this->access == 1);
#endif
        if (ref != block) {
            /* need to reorder blocks because blocks are ordered */
            /* by available space. Less space is put in front of */
            /* blocks having more space.                         */
            /* block must be placed after ref.                   */
            if (block->prev != NULL) {
                block->prev->next = block->next;
            } else {
                /* Block is first in the row so _this->cache needs */
                /* to be updated to the second block in the row.   */
                assert(_this->cache == block);
                _this->cache = block->next;
            }
            assert(block->next != NULL);
            block->next->prev = block->prev;
            /* block is now removed from the list. */
            /* Now insert block after ref.         */
            if (_this->incomplete == block) {
                _this->incomplete = block->next;
            }
            if (ref->next != NULL) {
                ref->next->prev = block;
            }
            block->next = ref->next;
            ref->next = block;
            block->prev = ref;
            if (block->next == NULL) {
                /* Block is last in the row so _this->last needs */
                /* to be updated to this block.                  */
                _this->last = block;
            }
        }
        if ((_this->incomplete == NULL) || (_this->incomplete == block->next)) {
            _this->incomplete = block;
        }
    }
    /* One slot has been freed , so add '1 * size' to free.*/
    _this->free += _this->size;

#ifndef NDEBUG
     assert(_this->access == 1);
#endif

#ifndef NDEBUG
    {
        /** This piece of debugging code verifies the consistentcy of the
            c_mmCache. The code will walk over all allocated memory blocks
            and verify if all prev and next pointers have correct values.
            This code also verifies if the first incomplete block encountered
            during the walk is the same as refered by _this->incomplete.
            If an assertion fails in this code then it is likely that the
            consistency is corrupted during this method because all actions
            on the cache verify consistentcy at the end of the action. But also
            multy threaded access may have corrupted the consistency since the
            cache itself does not protect against MT access.
        */
        c_bool incomplete = FALSE;

        block = _this->cache;
        if (block != NULL) {
            /** The current block is the first block in the list.
                So now verify if the block->prev field is NULL.
            */
            assert(block->prev == NULL);
        }
        while (block != NULL) {
            if (!incomplete && block->used < _this->count) {
                incomplete = TRUE;
                /** The current block is the first incomplete block in the list.
                    So now verify if the _this->incomplete field refers to this block.
                */
               assert(_this->incomplete == block);
            }
            if (block->next != NULL) {
                /* The current block is not the last so verify if the next block refers
                   to this block and verify that the next block does not have more free
                   space then this block. All blocks are ordered by available free space.
                   Less space blocks must be in front of more space blocks in the list.
                */
                assert(block->next->prev == block);
                assert(block->next->used <= block->used);
            } else {
                /** The current block is the last block in the list.
                   So now verify if the _this->last field refers to this block.
                */
                assert(_this->last == block);
            }
            block = block->next;
        }
    }
#endif
#ifndef NDEBUG
    _this->access--;
    assert(_this->access == 0);
#endif
}

c_size
c_mmCacheGetAllocated(
    c_mmCache _this)
{
    assert(_this);

    return _this->allocated;
}


c_size
c_mmCacheGetFree(
    c_mmCache _this)
{
    assert(_this);

    return _this->free;
}

