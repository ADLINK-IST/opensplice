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

#include "vortex_os.h"
#include "c_iterator.h"
#include "c_base.h"

#include "c_list_tmpl.h"
#define C__ITERIMPL_EQUALS(a,b) (a) == (b)
C__LIST_DECLS_TMPL(static, c__iterImpl, void *, __attribute_unused__)
C__LIST_CODE_TMPL(static, c__iterImpl, void *, NULL, C__ITERIMPL_EQUALS, os_malloc, os_free)

C_STRUCT(c_iter) {
    struct c__iterImpl_s x;
};

c_iter c_iterNew(void *object)
{
    c_iter iter = os_malloc(sizeof(*iter));
    c__iterImplInit(&iter->x);
    if (object != NULL) {
        c__iterImplAppend(&iter->x, object);
    }
    return iter;
}

void c_iterFree(c_iter iter)
{
    if (iter != NULL) {
        c__iterImplFree(&iter->x);
        os_free(iter);
    }
}

c_iter c_iterInsert(c_iter iter, void *object)
{
    if (iter == NULL) {
        return c_iterNew(object);
    } else {
        c__iterImplInsert(&iter->x, object);
    }
    return iter;
}

c_iter c_iterAppend(c_iter iter, void *object)
{
    if (iter == NULL) {
        return c_iterNew(object);
    } else {
        if (object != NULL) {
            c__iterImplAppend(&iter->x, object);
        }
        return iter;
    }
}

void *c_iterTakeFirst(c_iter iter)
{
    if (iter == NULL) {
        return NULL;
    } else {
        return c__iterImplTakeFirst(&iter->x);
    }
}

void *c_iterTakeLast(c_iter iter)
{
    if (iter == NULL) {
        return NULL;
    } else {
        return c__iterImplTakeLast(&iter->x);
    }
}

void *c_iterTake(c_iter iter, void *object)
{
    if (iter == NULL || object == NULL) {
        return NULL;
    } else {
        return c__iterImplRemove(&iter->x, object);
    }
}

void *c_iterTakeAction(c_iter iter, c_iterAction condition, c_iterActionArg arg)
{
    if (iter == NULL) {
        return NULL;
    } else if (condition == NULL) {
        return c_iterTakeFirst(iter);
    } else {
        struct c__iterImplIterD_s it;
        void *o;
        for (o = c__iterImplIterDFirst(&iter->x, &it); o != NULL; o = c__iterImplIterDNext(&it)) {
            if (condition(o, arg)) {
                c__iterImplIterDRemove(&it);
                return o;
            }
        }
        return NULL;
    }
}

void *c_iterReadAction (c_iter iter, c_iterAction condition, c_iterActionArg arg)
{
    if (iter == NULL) {
        return NULL;
    } else if (condition == NULL) {
        struct c__iterImplIter_s it;
        return c__iterImplIterFirst(&iter->x, &it);
    } else {
        struct c__iterImplIter_s it;
        void *o;
        for (o = c__iterImplIterFirst(&iter->x, &it); o != NULL; o = c__iterImplIterNext(&it)) {
            if (condition(o, arg)) {
                return o;
            }
        }
        return NULL;
    }
}

c_iter c_iterConcat(c_iter head, c_iter tail)
{
    if (head == NULL) {
        return tail;
    } else if (tail == NULL) {
        return head;
    } else {
        c__iterImplAppendList(&head->x, &tail->x);
        os_free(tail);
        return head;
    }
}

c_iter c_iterCopy(c_iter iter)
{
    if (iter == NULL) {
        return NULL;
    } else {
        struct c__iterImplIter_s it;
        c_iter copy;
        void *o;
        copy = os_malloc(sizeof(*copy));
        c__iterImplInit(&copy->x);
        for (o = c__iterImplIterFirst(&iter->x, &it); o != NULL; o = c__iterImplIterNext(&it)) {
            c__iterImplAppend(&copy->x, o);
        }
        return copy;
    }
}
c_ulong c_iterLength(c_iter iter)
{
    if (iter == NULL) {
        return 0;
    } else {
        return c__iterImplCount(&iter->x);
    }
}

void *c_iterResolve(c_iter iter, c_iterResolveCompare compare, c_iterResolveCompareArg compareArg)
{
    if (iter == NULL) {
        return NULL;
    } else {
        struct c__iterImplIter_s it;
        void *o;
        for (o = c__iterImplIterFirst(&iter->x, &it); o != NULL; o = c__iterImplIterNext(&it)) {
            if (compare(o, compareArg) == C_EQ) {
                return o;
            }
        }
        return NULL;
    }
}

void *c_iterObject(c_iter iter, c_ulong index)
{
    if (iter == NULL) {
        return NULL;
    } else {
        return c__iterImplIndex(&iter->x, index);
    }
}

void c_iterWalk(c_iter iter, c_iterWalkAction action, c_iterActionArg actionArg)
{
    if (iter != NULL) {
        struct c__iterImplIter_s it;
        void *o;
        for (o = c__iterImplIterFirst(&iter->x, &it); o != NULL; o = c__iterImplIterNext(&it)) {
            action(o, actionArg);
        }
    }
}

c_bool c_iterWalkUntil(c_iter iter, c_iterAction action, c_iterActionArg actionArg)
{
    if (iter != NULL) {
        struct c__iterImplIter_s it;
        void *o;
        for (o = c__iterImplIterFirst(&iter->x, &it); o != NULL; o = c__iterImplIterNext(&it)) {
            if (!action(o, actionArg)) {
                return 0;
            }
        }
    }
    return 1;
}

void c_iterArray(c_iter iter, void *ar[])
{
    if (iter != NULL) {
        struct c__iterImplIter_s it;
        void *o;
        os_uint32 i = 0;
        for (o = c__iterImplIterFirst(&iter->x, &it); o != NULL; o = c__iterImplIterNext(&it)) {
            ar[i++] = o;
        }
    }
}

c_bool c_iterContains(c_iter iter, void *object)
{
    if (iter != NULL && object != NULL) {
        struct c__iterImplIter_s it;
        void *o;
        for (o = c__iterImplIterFirst(&iter->x, &it); o != NULL; o = c__iterImplIterNext(&it)) {
            if (o == object) {
                return 1;
            }
        }
    }
    return 0;
}

c_iterIter c_iterIterGet(c_iter i)
{
    c_iterIter result;
    memset(&result, 0, sizeof(result));
    if (i != NULL) {
        result.current = c__iterImplIterFirst(&i->x, &result.it);
    }
    return result;
}

void *c_iterNext(c_iterIter* iterator)
{
    void *result = 0;
    if (iterator->current) {
        result = iterator->current;
        iterator->current = c__iterImplIterNext(&iterator->it);
    }
    return result;
}


c_iterIterD c_iterIterGetD(c_iter i)
{
    c_iterIterD result;
    memset(&result, 0, sizeof(result));
    result.initSource = i;
    return result;
}

void *c_iterNextD(c_iterIterD* iterator)
{
    void *result;
    if (iterator->initSource) {
        result = c__iterImplIterDFirst(&iterator->initSource->x, &iterator->it);
        iterator->initSource = NULL;
    } else {
        result = c__iterImplIterDNext(&iterator->it);
    }
    return result;
}

void c_iterRemoveD(c_iterIterD *iterator)
{
    c__iterImplIterDRemove(&iterator->it);
}
