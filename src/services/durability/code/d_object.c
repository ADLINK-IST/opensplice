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
#include "d_object.h"
#include "os_abstract.h"
#include "os_atomics.h"
#include "os_heap.h"

#define CHECK_REF (0)

#if CHECK_REF
#include "os_stdlib.h"
#include "os_report.h"
#include "os_abstract.h"
#include "os_process.h"
#include "d__thread.h"
#include <execinfo.h>

#define CHECK_REF_DEPTH (64)
static d_kind CHECK_REF_TYPE = D_NAMESPACE;
static char* CHECK_REF_FILE = NULL;
static c_bool CHECK_REF_TIMESTAMP = FALSE;

#define UT_TRACE(msgFormat, ...) do { \
    void *tr[CHECK_REF_DEPTH];\
    char **strs;\
    int s,i;\
    FILE* stream; \
    \
    if(!CHECK_REF_FILE){ \
        CHECK_REF_FILE = os_malloc(24); \
        os_sprintf(CHECK_REF_FILE, "mem_%u.log", os_procIdSelf()); \
    } \
    s = backtrace(tr, CHECK_REF_DEPTH);\
    strs = backtrace_symbols(tr, s);\
    stream = fopen(CHECK_REF_FILE, "a");\
    if (CHECK_REF_TIMESTAMP) { \
        os_timeM now = os_timeMGet(); \
        fprintf(stream, "%" PA_PRItime " ", OS_TIMEM_PRINT(now)); \
    } \
    fprintf(stream, msgFormat, __VA_ARGS__);\
    for (i=0;i<s;i++) fprintf(stream, "%s\n", strs[i]);\
    fprintf(stream, "\n\n"); \
    os_free(strs);\
    fflush(stream);\
    fclose(stream);\
  } while (0)

static const char *
getThreadNameSelf(void)
{
    return d_threadSelfName();
}
#endif

static char* d_kindString[] = {
    "D_BAD_TYPE            ",
    "D_DURABILITY          ",
    "D_CONFIGURATION       ",
    "D_FELLOW              ",
    "D_ADMIN               ",
    "D_GROUP               ",
    "D_LISTENER            ",
    "D_POLICY              ",
    "D_NAMESPACE           ",
    "D_PUBLISHER           ",
    "D_SUBSCRIBER          ",
    "D_TABLE               ",
    "D_CHAIN               ",
    "D_EVENT_LISTENER      ",
    "D_ADMIN_EVENT         ",
    "D_STORE               ",
    "D_WAITSET             ",
    "D_WAITSET_ENTITY      ",
    "D_ACTION              ",
    "D_ACTION_QUEUE        ",
    "D_GROUP_CREATION_QUEUE",
    "D_READER_REQUEST      ",
    "D_MERGE_ACTION        ",
    "D_ALIGNER_STATS       ",
    "D_ALIGNEE_STATS       ",
    "D_ADMIN_STATS_INFO    ",
    "D_CONFLICT            ",
    "D_CONFLICT_MONITOR    ",
    "D_CONFLICT_RESOLVER   ",
    "D_HISTORICAL_DATA_REQ ",
    "D_HISTORICAL_DATA     ",
    "D_DURABILITY_STATE_REQ",
    "D_DURABILITY_STATE    ",
    "D_PART_TOPIC_STATE    ",
    "D_FILTER              ",
    "D_CLIENT              ",
    "D_KINDCOUNT           " };

static pa_uint32_t allocationCount = PA_UINT32_INIT(0);
static pa_uint32_t maxObjectCount = PA_UINT32_INIT(0);
static pa_uint32_t typedObjectCount[D_KINDCOUNT];
static pa_uint32_t maxTypedObjectCount[D_KINDCOUNT];


#ifndef NDEBUG
static c_bool
doAdd(
    d_kind kind)
{
    c_ulong i;
    os_uint32 add;

    add = pa_inc32_nv(&maxObjectCount);

    if(add == 1){
        for(i=0; i<D_KINDCOUNT; i++){
            pa_st32(&typedObjectCount[i], 0);
            pa_st32(&maxTypedObjectCount[i], 0);
        }
    }
    (void)pa_inc32_nv(&allocationCount);
    (void)pa_inc32_nv(&(typedObjectCount[kind]));
    (void)pa_inc32_nv(&(maxTypedObjectCount[kind]));

    return TRUE;
}


static c_bool
doSub(
    d_kind kind)
{
    pa_dec32_nv(&allocationCount);
    pa_dec32_nv(&(typedObjectCount[kind]));

    return TRUE;
}
#endif


void
d_objectInit(
    d_object object,
    d_kind kind,
    d_objectDeinitFunc deinit)
{
    assert(object);

    if (object) {
        object->confidence = D_CONFIDENCE;
        object->kind       = kind;
        pa_st32(&object->refCount, 1);
        object->deinit     = deinit;
#if CHECK_REF
        if (kind == CHECK_REF_TYPE) {
            UT_TRACE("============ New(%p) [%s] =============\n", (void*)object, getThreadNameSelf());
        }
#endif
        /* Increase the number of objects for this kind.
         * The assert will only be activated when NDEBUG flag
         * is not set.
         */
        assert(doAdd(kind));
    }
}


void
d_objectDeinit(
    d_object object)
{
    if (object) {
        assert(object->confidence == D_CONFIDENCE);
        assert(pa_ld32(&object->refCount) == 0);

        object->confidence = D_CONFIDENCE_NULL;
        object->kind = D_BAD_TYPE;
        /* After the deinit, the deinit cannot be called again. */
        object->deinit = NULL;
    }
}


/**
 * \brief Print an object allocation report when durability terminates
 *
 * The full report is printed under one of the following conditions
 *
 *   1. If the number of expected remaining objects is non-zero (specified by 'expected')
 *   2. If the number of remaining objects differs from what was expected
 *   3. If enable_allocation_report is non-zero.
 *
 * In all other cases a one-line allocation overview is printed.
 *
 * @return TRUE, if the remaining allocations coincide with what was expected,
 *         FALSe, otherwise
 *
 * NOTE:
 * To force printing of a full report in it is possible to set the
 * environment variable OSPL_DURABILITY_ALLOCATION_REPORT=1
 */
c_bool
d_objectValidate(
    c_ulong expected,
    int enable_allocation_report)
{
    c_ulong i;
    int header_printed = 0;
    os_uint32 cnt = 0;

    for(i=1; i<D_KINDCOUNT; i++){ /* Not counting D_BAD_TYPE and D_KINDCOUNT */
        cnt = pa_ld32(&typedObjectCount[i]);
        if ((expected != 0) || (cnt != 0) || (enable_allocation_report != 0)) {
            if (!header_printed) {
                printf("\nHeap allocation report:\n");
                printf("-------------------------------------\n");
                printf("Type\t\t\tCurrent\tTotal\n");
                printf("-------------------------------------\n");
                header_printed = 1;
            }
            printf("%s\t%d\t%d\n", d_kindString[i], cnt, pa_ld32(&maxTypedObjectCount[i]));
        }
    }

    cnt = pa_ld32(&allocationCount);
    if (header_printed) {
        printf("-------------------------------------\n");
    }
    printf("\nAllocation validation [ %s ] #allocated: %d, #remaining: %d, #expected: %d\n",
            expected != cnt ? "FAILED" : "  OK  ",
            pa_ld32(&maxObjectCount), cnt, expected);

    return TRUE;
}

void
d_objectFree(
    d_object object)
{
    os_uint32 refCount;
#if CHECK_REF || !defined NDEBUG
    d_kind kind;
#endif

    if (object) {
        assert(object->confidence == D_CONFIDENCE);
        assert(pa_ld32(&object->refCount) >= 1);
#if CHECK_REF || !defined NDEBUG
        kind = object->kind;
#endif
        refCount = pa_dec32_nv(&(object->refCount));
        /* Check that refCount did not cross zero boundary. */
        assert(refCount + 1 > refCount);
        /* Only call deinit when refcount is 0.
         * In this case no other thread waits for the object
         * so deinitializing the object is safe without
         * having to lock the object.
         */
        if (refCount == 0) {
            /* Call deinit of implementing class */
            if (object->deinit) {
                object->deinit(object);
            }
            /* reset the object fields */
            object->confidence = D_CONFIDENCE_NULL;
            object->kind = D_BAD_TYPE;
            /* decrease the number of objects for this kind */
            assert(doSub(kind));
            /* free the object */
            os_free(object);
        }
#if CHECK_REF
        if (kind == CHECK_REF_TYPE) {
            UT_TRACE("============ Free(%p): %d -> %d [%s] =============\n",
                (void*)object, refCount+1, refCount, getThreadNameSelf());
        }
#endif
    }
}


d_object
d_objectKeep(
    d_object object)
{
    d_object result = NULL;

    if (object) {
        assert(object->confidence == D_CONFIDENCE);
#if CHECK_REF
        os_uint32 refCount = pa_inc32_nv(&(object->refCount));
        result = object;
        if (object->kind == CHECK_REF_TYPE) {
            UT_TRACE("============ Keep(%p): %d -> %d [%s] =============\n",
                     (void*)object, refCount-1, refCount, getThreadNameSelf());
        }
#else
        pa_inc32(&(object->refCount));
        result = object;
#endif
    }
    return result;
}


c_bool
d_objectIsValid(
    d_object object,
    d_kind kind)
{
    c_bool result = FALSE;

    if (object) {
        if ((object->kind == kind) && (object->confidence == D_CONFIDENCE)) {
            result = TRUE;
        }
    }
    return result;
}
