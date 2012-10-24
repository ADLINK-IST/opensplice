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
#include "d_object.h"
#include "os_abstract.h"
#include "os_heap.h"

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
