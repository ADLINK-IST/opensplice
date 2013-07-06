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

#include "os.h"
#include "c_iterator.h"
#include "c_base.h"

C_STRUCT(c_iter) {
    c_long length;
    c_iterNode head;
    c_iterNode tail;
};

C_STRUCT(c_iterNode) {
    c_iterNode next;
    void *object;
};

c_iter
c_iterNew(
    void *object)
{
    c_iter l;

    l = (c_iter)os_malloc(C_SIZEOF(c_iter));
    if (object == NULL) {
        l->length = 0;
        l->head = NULL;
        l->tail = NULL;
    } else {
        l->length = 1;
        l->head = (c_iterNode)os_malloc(C_SIZEOF(c_iterNode));
        l->head->next = NULL;
        l->head->object = object;
        l->tail = l->head;
    }
    return l;
}

void
c_iterFree(
    c_iter iter)
{
    c_iterNode n,t;

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

c_iter
c_iterInsert(
    c_iter iter,
    void *object)
{
    c_iterNode n;

    if (iter == NULL) return c_iterNew(object);
    if (object == NULL) {
        return iter;
    }
    n = (c_iterNode)os_malloc(C_SIZEOF(c_iterNode));
    n->object = object;
    n->next = iter->head;
    iter->head = n;

    if(iter->tail == NULL){
        iter->tail = n;
    }
    iter->length++;

    return iter;
}

c_iter
c_iterAppend(
    c_iter iter,
    void *object)
{
    c_iterNode n;

    if (iter == NULL) return c_iterNew(object);
    if (object == NULL) {
        return iter;
    }
    n = (c_iterNode)os_malloc(C_SIZEOF(c_iterNode));
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
c_iterTakeFirst(
    c_iter iter)
{
    c_iterNode n;
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
c_iterTakeLast(
    c_iter iter)
{
    c_iterNode n, prev;
    void *o;

    if (iter == NULL) return NULL;
    if (iter->tail == NULL) {
        return NULL;
    }
    n = iter->tail;
    o = n->object;
    if (iter->head == iter->tail) {
        prev = NULL;
    } else {
        prev = iter->head;
        while (prev->next != iter->tail) prev = prev->next;
    }
    iter->tail = prev;
    prev->next = NULL;
    iter->length--;

    if(iter->length == 0){
        iter->head = NULL;
    }
    os_free(n);

    return o;
}

void *
c_iterTake(
    c_iter iter,
    void *object)
{
    c_iterNode current, previous;

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
c_iterTakeAction (
    c_iter iter,
    c_iterAction condition,
    c_iterActionArg arg)
{
    c_iterNode *p, p2;
    c_object object;

    if (iter == NULL) {
        return NULL;
    }
    if (condition == NULL) {
        return c_iterTakeFirst(iter);
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
c_iterReadAction (
    c_iter iter,
    c_iterAction condition,
    c_iterActionArg arg)
{
    c_iterNode *p;

    if (iter == NULL) {
        return NULL;
    }
    if (condition == NULL) {
        return c_iterTakeFirst(iter);
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

c_iter
c_iterConcat(
    c_iter head,
    c_iter tail)
{
    c_iterNode *l;

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

c_iter
c_iterCopy(
    c_iter iter)
{
    c_iterNode n;
    c_iter l = NULL;

    if (iter == NULL) {
        return NULL;
    }
    n = iter->head;
    while (n != NULL) {
        l = c_iterAppend(l,n->object);
        n = n->next;
    }
    if(l){
        l->tail = iter->tail;
    }
    return l;
}
c_long
c_iterLength(
    c_iter iter)
{
    if (iter == NULL) {
        return 0;
    }
    return iter->length;
}

void *
c_iterResolve(
    c_iter iter,
    c_iterResolveCompare compare,
    c_iterResolveCompareArg compareArg)
{
    c_iterNode l;
    if (iter == NULL) {
        return NULL;
    }
    l = iter->head;
    while (l != NULL) {
        if (compare(l->object,compareArg) == C_EQ) {
            return l->object;
        }
        l = l->next;
    }
    return NULL;
}

void *
c_iterObject(
    c_iter iter,
    c_long index)
{
    c_iterNode l;
    c_long i;

    if (iter == NULL) {
        return NULL;
    }
    if ((index < 0) || (index >= iter->length)) {
        return NULL;
    }
    l = iter->head;
    for (i=0;i<index;i++) l = l->next;
    return l->object;
}

void
c_iterWalk(
    c_iter iter,
    c_iterWalkAction action,
    c_iterActionArg actionArg)
{
    c_iterNode l;
    if (iter == NULL) {
        return;
    }
    l = iter->head;
    while (l != NULL) {
        action(l->object,actionArg);
        l = l->next;
    }
}

c_bool
c_iterWalkUntil(
    c_iter iter,
    c_iterAction action,
    c_iterActionArg actionArg)
{
    c_iterNode l;
    c_bool proceed = TRUE;

    if (iter == NULL) {
        return 1;
    }
    l = iter->head;
    while ((proceed) && (l != NULL)) {
        proceed = action(l->object,actionArg);
        l = l->next;
    }
    return proceed;
}

void
c_iterArray(
    c_iter iter,
    void *ar[])
{
    c_iterNode l;
    c_long i;

    if (iter == NULL) return;
    l = iter->head; i = 0;
    while (l != NULL) {
        ar[i++] = l->object;
        l = l->next;
    }
}

c_bool
c_iterContains(
    c_iter iter,
     void *object)
{
    c_iterNode p;

    if (iter == NULL) return 0;
    if (object == NULL) return 0;

    p = iter->head;

    while (p != NULL) {
        if (p->object == object) {
            return 1;
        }
        p = p->next;
    }
    return 0;
}

c_iterIter
c_iterIterGet(
    c_iter i)
{
    c_iterIter result;

    result.current = i->head;

    return result;
}

void*
c_iterNext(
    c_iterIter* iterator)
{
    void* result;

    result = 0;
    if(iterator->current) {
        result = iterator->current->object;
        iterator->current = iterator->current->next;
    }

    return result;
}

