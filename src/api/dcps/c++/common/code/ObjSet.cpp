/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "ObjSet.h"
#include <string.h>
#include "ut_collection.h"


struct ObjSetWalkArg {
    DDS::OpenSplice::ObjSet::ObjSetActionFunc actionFunc;
    void *actionArg;
};

os_equality
DDS::OpenSplice::ObjSet::fnCompareElements(
    const DDS::Object_ptr element1,
    const DDS::Object_ptr element2,
    void *arg)
{
    os_address el1 = reinterpret_cast<os_address>(element1);
    os_address el2 = reinterpret_cast<os_address>(element2);
    os_equality result;
    OS_UNUSED_ARG(arg);

    if (el1 < el2) {
        result = OS_LT;
    } else if(el1 > el2) {
        result = OS_GT;
    } else {
        result = OS_EQ;
    }
    return result;
}

void
DDS::OpenSplice::ObjSet::fnFreeValue(
    DDS::Object_ptr element,
    void *arg)
{
    DDS::Boolean *keepRef = reinterpret_cast<DDS::Boolean *>(arg);
    if (*keepRef) {
        DDS::release(element);
    }
}

os_int32
DDS::OpenSplice::ObjSet::objSetActionWrapper(DDS::Object_ptr o, void *arg)
{
    os_int32 result;

    ObjSetWalkArg *somArg = reinterpret_cast<ObjSetWalkArg *>(arg);
    result = static_cast<os_int32>(somArg->actionFunc(o, somArg->actionArg));
    return result;
}

typedef struct {
    DDS::Long index;
    DDS::ObjSeq *objects;
} ToSeqArg;

DDS::Boolean
DDS::OpenSplice::ObjSet::toObjSeq(DDS::Object_ptr element, void *arg)
{
    ToSeqArg *seqArg = reinterpret_cast<ToSeqArg *>(arg);
    (*seqArg->objects)[seqArg->index++] = DDS::Object::_duplicate(element);
    return TRUE;
}


DDS::OpenSplice::ObjSet::ObjSet(DDS::Boolean keepRef) : myObjSet(NULL)
{
    this->keepRef = keepRef;
}

DDS::ReturnCode_t
DDS::OpenSplice::ObjSet::init()
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    myObjSet = ut_setNew(
            (ut_compareElementsFunc) DDS::OpenSplice::ObjSet::fnCompareElements,
            NULL,
            (ut_freeElementFunc) DDS::OpenSplice::ObjSet::fnFreeValue,
            &keepRef);
    if (myObjSet == NULL) {
        result = DDS::RETCODE_OUT_OF_RESOURCES;
        // TODO: OS_REPORT.
    }
    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::ObjSet::deinit()
{
    if (myObjSet) {
        ut_setFree(myObjSet);
        myObjSet = NULL;
    }
    return DDS::RETCODE_OK;
}

DDS::OpenSplice::ObjSet::~ObjSet() {
    (void) deinit();
}

DDS::Boolean
DDS::OpenSplice::ObjSet::walk(DDS::OpenSplice::ObjSet::ObjSetActionFunc action, void *arg)
{
    ObjSetWalkArg somArg;
    os_int32 result;

    somArg.actionFunc = action;
    somArg.actionArg = arg;
    result = ut_walk((ut_collection) myObjSet, (ut_actionFunc) objSetActionWrapper, &somArg);
    return static_cast<DDS::Boolean>(result);
}

DDS::Boolean
DDS::OpenSplice::ObjSet::insertElement(DDS::Object_ptr element)
{
    os_int32 result;

    result = ut_setInsert(myObjSet, element);
    if (result && keepRef) {
        (void) DDS::Object::_duplicate(element);
    }
    return static_cast<DDS::Boolean>(result);
}

DDS::Boolean
DDS::OpenSplice::ObjSet::containsElement(DDS::Object_ptr element)
{
    return (DDS::Boolean)ut_contains((ut_collection) myObjSet, element);
}

DDS::Boolean
DDS::OpenSplice::ObjSet::removeElement(DDS::Object_ptr element)
{
    DDS::Boolean result = FALSE;

    DDS::Object_ptr removed = reinterpret_cast<DDS::Object_ptr>(
            ut_remove((ut_collection) myObjSet, element));
    if (removed == element) {
        if (keepRef)
        {
            DDS::release(removed);
        }
        result = TRUE;
    }
    return result;
}

DDS::Long
DDS::OpenSplice::ObjSet::getNrElements()
{
    return static_cast<DDS::Long>(ut_count((ut_collection) myObjSet));
}

DDS::ObjSeq *
DDS::OpenSplice::ObjSet::getObjSeq()
{
    DDS::Long nrElements = getNrElements();
    ObjSeq *objects = new ObjSeq(nrElements);
    ToSeqArg arg;

    arg.index = 0;
    arg.objects = objects;
    objects->length(nrElements);
    walk(toObjSeq, &arg);
    return objects;
}

void
DDS::OpenSplice::ObjSet::getObjSeq(DDS::ObjSeq &destination)
{
    DDS::Long nrElements = getNrElements();
    ToSeqArg arg;

    arg.index = 0;
    arg.objects = &destination;
    destination.length(nrElements);
    walk(toObjSeq, &arg);
}

void
DDS::OpenSplice::ObjSet::clear()
{
    ut_setClear(myObjSet);
}
