/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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

#include "StrObjMap.h"
#include <string.h>
#include "ut_collection.h"

struct StrObjMapWalkArg {
    DDS::OpenSplice::StrObjMap::StrObjMapActionFunc actionFunc;
    void *actionArg;
};

os_equality
DDS::OpenSplice::StrObjMap::fnCompareElements(
    const char *element1,
    const char *element2,
    void *arg)
{
    os_equality result;
    int cmp = strcmp(element1, element2);
    OS_UNUSED_ARG(arg);

    if (cmp < 0) {
        result = OS_LT;
    } else if(cmp > 0) {
        result = OS_GT;
    } else {
        result = OS_EQ;
    }
    return result;
}

void
DDS::OpenSplice::StrObjMap::fnFreeKey(
    char *element,
    void *arg)
{
    OS_UNUSED_ARG(arg);
    DDS::string_free(element);
}

void
DDS::OpenSplice::StrObjMap::fnFreeValue(
    DDS::Object_ptr element,
    void *arg)
{
    DDS::Boolean *keepRef = reinterpret_cast<DDS::Boolean *>(arg);
    if (*keepRef) {
        DDS::release(element);
    }
}

os_int32
DDS::OpenSplice::StrObjMap::strObjMapActionWrapper(const char *k, DDS::Object_ptr o, void *arg)
{
    os_int32 result;

    StrObjMapWalkArg *somArg = reinterpret_cast<StrObjMapWalkArg *>(arg);
    result = static_cast<os_int32>(somArg->actionFunc(k, o, somArg->actionArg));
    return result;
}

DDS::OpenSplice::StrObjMap::StrObjMap(DDS::Boolean keepRef) : myStrMap(NULL)
{
    this->keepRef = keepRef;
}

DDS::ReturnCode_t
DDS::OpenSplice::StrObjMap::init()
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    myStrMap = ut_tableNew(
            (ut_compareElementsFunc) DDS::OpenSplice::StrObjMap::fnCompareElements,
            NULL,
            (ut_freeElementFunc) DDS::OpenSplice::StrObjMap::fnFreeKey,
            NULL,
            (ut_freeElementFunc) DDS::OpenSplice::StrObjMap::fnFreeValue,
            &keepRef);
    if (myStrMap == NULL) {
        result = DDS::RETCODE_OUT_OF_RESOURCES;
        // TODO: OS_REPORT.
    }
    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::StrObjMap::deinit()
{
    if (myStrMap) {
        ut_tableFree(myStrMap);
        myStrMap = NULL;
    }
    return DDS::RETCODE_OK;
}

DDS::OpenSplice::StrObjMap::~StrObjMap() {
    (void) deinit();
}

DDS::Boolean
DDS::OpenSplice::StrObjMap::walk(DDS::OpenSplice::StrObjMap::StrObjMapActionFunc action, void *arg)
{
    StrObjMapWalkArg somArg;
    os_int32 result;

    somArg.actionFunc = action;
    somArg.actionArg = arg;
    result = ut_tableKeyValueWalk(myStrMap, (ut_actionKeyValueFunc) strObjMapActionWrapper, &somArg);
    return static_cast<DDS::Boolean>(result);
}

const DDS::Object *
DDS::OpenSplice::StrObjMap::insertElement(const char *key, DDS::Object_ptr element)
{
    DDS::Object_ptr result;
    char *keyCopy = DDS::string_dup(key);
    os_int32 inserted;

    inserted = ut_tableInsert(myStrMap, keyCopy, element);
    if (inserted) {
        result = DDS::Object::_duplicate(element);
    } else {
        DDS::string_free(keyCopy);
        result = NULL;
    }
    return result;
}

DDS::Object_ptr
DDS::OpenSplice::StrObjMap::removeElement(const char *key)
{
    return reinterpret_cast<DDS::Object_ptr>(ut_remove((ut_collection) myStrMap, const_cast<char *>(key)));
}

DDS::Object_ptr
DDS::OpenSplice::StrObjMap::findElement(const char *key)
{
    DDS::Object_ptr element;

    element = reinterpret_cast<DDS::Object_ptr>(ut_get((ut_collection) myStrMap, const_cast<char *>(key)));
    return DDS::Object::_duplicate(element);
}

DDS::Long
DDS::OpenSplice::StrObjMap::getNrElements()
{
    return static_cast<DDS::Long>(ut_count((ut_collection) myStrMap));
}

typedef struct {
    DDS::Long index;
    DDS::ObjSeq *objects;
} ToSeqArg;

DDS::Boolean
DDS::OpenSplice::StrObjMap::toObjSeq(const char *key, DDS::Object_ptr element, void *arg)
{
    ToSeqArg *seqArg = reinterpret_cast<ToSeqArg *>(arg);
    (*seqArg->objects)[seqArg->index++] = DDS::Object::_duplicate(element);
    OS_UNUSED_ARG(key);
    return TRUE;
}

DDS::ObjSeq *
DDS::OpenSplice::StrObjMap::getObjSeq()
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
DDS::OpenSplice::StrObjMap::getObjSeq(DDS::ObjSeq &destination)
{
    DDS::Long nrElements = getNrElements();
    ToSeqArg arg;

    arg.index = 0;
    arg.objects = &destination;
    destination.length(nrElements);
    walk(toObjSeq, &arg);
}

void
DDS::OpenSplice::StrObjMap::clear()
{
    ut_tableClear(myStrMap);
}
