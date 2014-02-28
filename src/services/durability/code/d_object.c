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
#include "d_object.h"
#include "os_abstract.h"
#include "os_heap.h"

#define CHECK_REF (0)

#if CHECK_REF
#include "os_stdlib.h"
#include "os_report.h"
#include "os_abstract.h"
#include <execinfo.h>

#define CHECK_REF_DEPTH (64)
static d_kind CHECK_REF_TYPE = D_NAMESPACE;
static char* CHECK_REF_FILE = NULL;

#define UT_TRACE(msgFormat, ...) do { \
    void *tr[CHECK_REF_DEPTH];\
    char **strs;\
    size_t s,i; \
    FILE* stream; \
    \
    if(!CHECK_REF_FILE){ \
        CHECK_REF_FILE = os_malloc(24); \
        os_sprintf(CHECK_REF_FILE, "mem.log"); \
    } \
    s = backtrace(tr, CHECK_REF_DEPTH);\
    strs = backtrace_symbols(tr, s);\
    stream = fopen(CHECK_REF_FILE, "a");\
    fprintf(stream, msgFormat, __VA_ARGS__);              \
    for (i=0;i<s;i++) fprintf(stream, "%s\n", strs[i]);\
    free(strs);\
    fflush(stream);\
    fclose(stream);\
  } while (0)
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
    "D_DISPOSE_HELPER      ",
    "D_GROUP_CREATION_QUEUE",
    "D_READER_REQUEST      ",
    "D_MERGE_ACTION        ",
    "D_KINDCOUNT           " };

static c_ulong allocationCount = 0;
static c_ulong maxObjectCount = 0;
static c_ulong typedObjectCount[D_KINDCOUNT];
static c_ulong maxTypedObjectCount[D_KINDCOUNT];

#ifndef NDEBUG
static c_bool
doAdd(
    d_kind kind)
{
    c_ulong i;
    c_long add;

    add = pa_increment(&maxObjectCount);

    if(add == 1){
        for(i=0; i<D_KINDCOUNT; i++){
            typedObjectCount[i] = 0;
            maxTypedObjectCount[i] = 0;
        }
    }
    pa_increment(&allocationCount);
    pa_increment(&(typedObjectCount[kind]));
    pa_increment(&(maxTypedObjectCount[kind]));

    return TRUE;
}

static c_bool
doSub(
    d_kind kind)
{
    pa_decrement(&allocationCount);
    pa_decrement(&(typedObjectCount[kind]));

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

    if(object){
        object->confidence = D_CONFIDENCE;
        object->kind       = kind;
        object->refCount   = 1;
        object->deinit     = deinit;
#if CHECK_REF
        if (kind == CHECK_REF_TYPE) {
            UT_TRACE("\n\n============ New(%p) =============\n", (void*)object);
        }
#endif
        assert(doAdd(kind));
    }
}

c_bool
d_objectValidate(
    c_ulong expected)
{
    c_bool result;
    c_ulong i;

    printf("\nHeap allocation report:\n");
    printf("-------------------------------------\n");
    printf("Type\t\t\tCurrent\tTotal\n");
    printf("-------------------------------------\n");

    for(i=1; i<D_KINDCOUNT; i++){ /*Not counting D_BAD_TYPE and D_KINDCOUNT*/
        printf("%s\t%d\t%d\n", d_kindString[i], typedObjectCount[i], maxTypedObjectCount[i]);
    }
    printf("-------------------------------------\n");
    printf("\n#allocated: %d, #remaining: %d, #expected: %d\n",
            maxObjectCount, allocationCount, expected);

    if(expected != allocationCount){
        printf("Allocation validation [ FAILED ]\n");
        result = FALSE;
    } else {
        printf("Allocation validation [   OK   ]\n");
        result = TRUE;
    }

    return TRUE;
}

void
d_objectFree(
    d_object object,
    d_kind kind)
{
    os_uint32 refCount;

    OS_UNUSED_ARG(kind);
    assert(d_objectIsValid(object, kind) == TRUE);

    if(object){
       assert(object->confidence == D_CONFIDENCE);
       assert(object->kind == kind);
       assert(object->refCount >= 1);

       refCount = pa_decrement(&(object->refCount));

       if(refCount == 0){
           if(object->deinit){
               object->deinit(object);
           }
           object->confidence = D_CONFIDENCE_NULL;
           object->kind = D_BAD_TYPE;
           os_free(object);
           assert(doSub(kind));
       }
#if CHECK_REF
       if (kind == CHECK_REF_TYPE) {
           UT_TRACE("\n\n============ Free(%p): %d -> %d =============\n",
                   (void*)object, refCount+1, refCount);
       }
#endif
    }
}

d_object
d_objectKeep(
    d_object object)
{
    d_object result = NULL;

    assert(object);
    assert(object->confidence == D_CONFIDENCE);

    if(object){
        pa_increment(&(object->refCount));
        result = object;

#if CHECK_REF
       if (object->kind == CHECK_REF_TYPE) {
           UT_TRACE("\n\n============ Keep(%p): %d -> %d =============\n",
                   (void*)object, refCount-1, refCount);
       }
#endif
    }
    return result;
}

c_ulong
d_objectGetRefCount(
    d_object object)
{
    c_ulong refCount = 0;
    assert(object);
    assert(object->confidence == D_CONFIDENCE);

    if(object){
        refCount = object->refCount;
    }
    return refCount;
}

c_bool
d_objectIsValid(
    d_object object,
    d_kind kind)
{
    c_bool result = FALSE;

    if(object){
        if((object->kind == kind) && (object->confidence == D_CONFIDENCE)){
            result = TRUE;
        }
    }
    return result;
}
