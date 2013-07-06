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

    l = (os_iter)os_malloc(OS_SIZEOF(os_iter));
    if (object == NULL) {
        l->length = 0;
        l->head = NULL;
        l->tail = NULL;
    } else {
        l->length = 1;
        l->head = (os_iterNode)os_malloc(OS_SIZEOF(os_iterNode));
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

os_int32
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
