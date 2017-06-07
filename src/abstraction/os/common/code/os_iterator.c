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
#include "os_heap.h"
#include "os_iterator.h"
#include <assert.h>

OS_CLASS(os_iterNode);

OS_STRUCT(os_iter) {
    os_uint32 length;
    os_iterNode head;
    os_iterNode tail;
};

OS_STRUCT(os_iterNode) {
    os_iterNode next;
    void *object;
};

os_iter
os_iterNew(
    void *object)
{
    os_iter l;

    l = os_malloc(sizeof *l);
    if (object == NULL) {
        l->length = 0;
        l->head = NULL;
        l->tail = NULL;
    } else {
        l->length = 1;
        l->head = os_malloc(sizeof *l->head);
        l->head->next = NULL;
        l->head->object = object;
        l->tail = l->head;
    }
    return l;
}

void
os_iterFree(
    os_iter iter)
{
    os_iterNode n,t;

    if (iter == NULL) {
        return;
    }
    n = iter->head;
    while (n != NULL) {
        t = n->next;
        os_free(n);
        n = t;
    }
    /* Do not free tail, because 'tail - 1' references tail already.*/
    os_free(iter);
}

os_iter
os_iterInsert(
    os_iter iter,
    void *object)
{
    os_iterNode n;

    if (iter == NULL) return os_iterNew(object);
    if (object == NULL) {
        return iter;
    }
    n = (os_iterNode)os_malloc(OS_SIZEOF(os_iterNode));
    n->object = object;
    n->next = iter->head;
    iter->head = n;

    if(iter->tail == NULL){
        iter->tail = n;
    }
    iter->length++;

    return iter;
}

os_iter
os_iterAppend(
    os_iter iter,
    void *object)
{
    os_iterNode n;

    if (iter == NULL) return os_iterNew(object);
    if (object == NULL) {
        return iter;
    }
    n = (os_iterNode)os_malloc(OS_SIZEOF(os_iterNode));
    n->object = object;
    n->next = NULL;

    if(iter->tail){
        iter->tail->next = n;
        iter->tail = n;
    } else {
        iter->head = n;
        iter->tail = n;
    }
    iter->length++;

    return iter;
}

void *
os_iterTakeFirst(
    os_iter iter)
{
    os_iterNode n;
    void *o;

    if (iter == NULL) return NULL;
    if (iter->head == NULL) {
        return NULL;
    }
    n = iter->head;
    o = n->object;
    iter->head = n->next;
    iter->length--;

    if(iter->length == 0){
        assert(n->next == NULL);
        iter->tail = NULL;
    }
    os_free(n);

    return o;
}

void *
os_iterTakeLast(
    os_iter iter)
{
    os_iterNode n, prev;
    void *o;

    if (iter == NULL) {
        return NULL;
    }
    if (iter->tail == NULL) {
        return NULL;
    }

    n = iter->tail;
    o = n->object;

    if (iter->head == iter->tail) {
        prev = NULL;
    } else {
        prev = iter->head;
        while (prev->next != iter->tail) {
            prev = prev->next;
        }
    }

    if (prev) {
        prev->next = NULL;
    }
    iter->tail = prev;
    iter->length--;

    if (iter->length == 0) {
        iter->head = NULL;
        assert(iter->tail == NULL);
    }

    os_free(n);

    return o;
}

void *
os_iterTake(
    os_iter iter,
    void *object)
{
    os_iterNode current, previous;

    if (iter == NULL) {
        return NULL;
    }
    if (object == NULL) {
        return NULL;
    }
    previous = NULL;
    current  = iter->head;

    while(current){
        if(current->object == object){
            if(previous){ /* current is not head */
                if(current->next == NULL){ /* current is tail */
                    iter->tail = previous;
                }
                previous->next = current->next;
            } else if(current->next){ /*current is head and not tail*/
                 iter->head = current->next;
            } else { /*current is head and tail*/
                iter->head = NULL;
                iter->tail = NULL;
                assert(iter->length == 1); /* will be 0 after this */
            }
            os_free(current);

            assert(iter->length >= 1);
            iter->length--;

            return object;
        } else {
            previous = current;
            current = current->next;
        }
    }
    return NULL;
}

void *
os_iterTakeAction (
    os_iter iter,
    os_iterAction condition,
    os_iterActionArg arg)
{
    os_iterNode *p, p2;
    void *object;

    if (iter == NULL) {
        return NULL;
    }
    if (condition == NULL) {
        return os_iterTakeFirst(iter);
    }
    p = &iter->head;
    while (*p != NULL) {
        if (condition((*p)->object,arg)) {
            if((*p)->next == NULL){
                if(*p == iter->head){
                    iter->tail = NULL;
                } else {
                    iter->tail = *p;
                }
            }
            object = (*p)->object;
            p2 = *p;
            *p = (*p)->next;
            os_free(p2);
            iter->length--;
            return object;
        }
        p = &((*p)->next);
    }
    return NULL;
}

void *
os_iterReadAction (
    os_iter iter,
    os_iterAction condition,
    os_iterActionArg arg)
{
    os_iterNode *p;

    if (iter == NULL) {
        return NULL;
    }
    if (condition == NULL) {
        return os_iterTakeFirst(iter);
    }
    p = &iter->head;
    while (*p != NULL) {
        if (condition((*p)->object,arg)) {
            return (*p)->object;
        }
        p = &((*p)->next);
    }
    return NULL;
}

os_iter
os_iterConcat(
    os_iter head,
    os_iter tail)
{
    os_iterNode *l;

    if (head == NULL) {
        return tail;
    }
    if (tail == NULL) {
        return head;
    }
    if (head->head == NULL) {
        os_free(head);
        return tail;
    }
    if (tail->head == NULL) {
        os_free(tail);
        return head;
    }
    l = &head->head;
    while ((*l)->next != NULL) l = &(*l)->next;
    (*l)->next = tail->head;
    head->length += tail->length;
    head->tail = tail->tail;

    os_free(tail);
    return head;
}

os_iter
os_iterCopy(
    os_iter iter)
{
    os_iterNode n;
    os_iter l = NULL;

    if (iter == NULL) {
        return NULL;
    }
    n = iter->head;
    while (n != NULL) {
        l = os_iterAppend(l,n->object);
        n = n->next;
    }
    if(l){
        l->tail = iter->tail;
    }
    return l;
}

os_uint32
os_iterLength(
    os_iter iter)
{
    if (iter == NULL) {
        return 0;
    }
    return iter->length;
}

void *
os_iterResolve(
    os_iter iter,
    os_iterResolveCompare compare,
    os_iterResolveCompareArg compareArg)
{
    os_iterNode l;
    if (iter == NULL) {
        return NULL;
    }
    l = iter->head;
    while (l != NULL) {
        if (compare(l->object,compareArg) == OS_EQ) {
            return l->object;
        }
        l = l->next;
    }
    return NULL;
}

void *
os_iterObject(
    os_iter iter,
    os_uint32 index)
{
    os_iterNode l;
    os_uint32 i;

    if (iter == NULL) {
        return NULL;
    }
    if (index >= iter->length) {
        return NULL;
    }
    l = iter->head;
    for (i = 0; i < index; i++) l = l->next;
    return l->object;
}

void
os_iterWalk(
    os_iter iter,
    os_iterWalkAction action,
    os_iterActionArg actionArg)
{
    os_iterNode l;
    if (iter == NULL) {
        return;
    }
    l = iter->head;
    while (l != NULL) {
        action(l->object,actionArg);
        l = l->next;
    }
}

void
os_iterArray(
    os_iter iter,
    void *ar[])
{
    os_iterNode l;
    os_uint32 i;

    if (iter == NULL) return;
    l = iter->head;
    i = 0;
    while (l != NULL) {
        ar[i++] = l->object;
        l = l->next;
    }
}

os_int32
os_iterContains(
    os_iter iter,
    void *object)
{
    os_iterNode *p;

    if (iter == NULL) return 0;
    if (object == NULL) return 0;

    p = &iter->head;

    while (*p != NULL) {
        if ((*p)->object == object) {
            return 1;
        }
        p = &((*p)->next);
    }
    return 0;
}

void
os_iterSort (
    os_iter iter,
    os_iterSortAction action,
    os_boolean ascending)
{
    os_equality eq;
    os_iterNode k, l;
    void *o;

    assert (action != NULL);

    if (iter != NULL) {
        for (k = iter->head; k->next != NULL; k = k->next) {
            for (l = k->next; l != NULL; l = l->next) {
                eq = action (k->object, l->object);
                if ((eq > OS_EQ && ascending == OS_TRUE) ||
                    (eq < OS_EQ && ascending == OS_FALSE))
                {
                    o = k->object;
                    k->object = l->object;
                    l->object = o;
                }
            }
        }
    }
}
