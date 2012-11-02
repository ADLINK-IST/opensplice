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
#include "sd_list.h"
#include "os_heap.h"

#define sd_listNode(n) ((sd_listNode)n)

C_CLASS(sd_listNode);
C_STRUCT(sd_listNode) {
    sd_listNode prev;
    sd_listNode next;
    void *object;
};

C_STRUCT(sd_list) {
    C_EXTENDS(sd_listNode);
};

static void
sd_listRemoveFirst (
    sd_list list);

static void
sd_listRemoveLast (
    sd_list list);


sd_list
sd_listNew (
    void)
{
    sd_list list;
    sd_listNode node;

    list = (sd_list) os_malloc(C_SIZEOF(sd_list));
    if ( list ) {
        node = sd_listNode(list);
        node->prev = node;
        node->next = node;
        node->object = NULL;
    }

    return list;
}

void
sd_listFree (
    sd_list list)
{
    assert(list);
    while ( !sd_listIsEmpty(list) ) {
        sd_listRemoveFirst(list);
    }
    os_free(list);
}

void
sd_listInsert (
    sd_list list,
    void *object)
{
    sd_listNode node;
    sd_listNode after = (sd_listNode)list;

    node = (sd_listNode)os_malloc(C_SIZEOF(sd_listNode));
    if ( node ) {
        node->object = object;
        node->prev = after;
        node->next = after->next;
        after->next->prev = node;
        after->next = node;
    }
}
        
void
sd_listAppend (
    sd_list list,
    void *object)
{
    sd_listNode node;
    sd_listNode before = (sd_listNode)list;

    node = (sd_listNode)os_malloc(C_SIZEOF(sd_listNode));
    if ( node ) {
        node->object = object;
        node->next = before;
        node->prev = before->prev;
        before->prev->next = node;
        before->prev = node;
    }
}

c_bool
sd_listIsEmpty (
    sd_list list)
{
    c_bool empty = FALSE;
    
    assert(list);
    
    if ( sd_listNode(list)->next == sd_listNode(list) ) {
        assert(sd_listNode(list)->prev == sd_listNode(list));
        empty = TRUE;
    } else {
        assert(sd_listNode(list)->next != sd_listNode(list));
        assert(sd_listNode(list)->prev != sd_listNode(list));
    }
    return empty;
}

static void
sd_listRemoveFirst (
    sd_list list)
{
    sd_listNode node;

    if ( !sd_listIsEmpty(list) ) {
        node = sd_listNode(list)->next;
        node->next->prev = node->prev;
        node->prev->next = node->next;
        os_free(node);
    }
}

static void
sd_listRemoveLast (
    sd_list list)
{
    sd_listNode node;

    assert(list);

    if ( !sd_listIsEmpty(list) ) {
        node = sd_listNode(list)->prev;
        node->next->prev = node->prev;
        node->prev->next = node->next;
        os_free(node);
    }
}

void *
sd_listReadFirst (
    sd_list list)
{
    assert(list);
    return sd_listNode(list)->next->object;
}
    
void *
sd_listReadLast (
    sd_list list)
{
    assert(list);
    return sd_listNode(list)->prev->object;
}

void *
sd_listTakeFirst (
    sd_list list)
{
    void *object = sd_listReadFirst(list);
    
    sd_listRemoveFirst(list);

    return object;
}

void *
sd_listTakeLast (
    sd_list list)
{
    void *object = sd_listReadLast(list);
    
    sd_listRemoveLast(list);
    
    return object;
}

void *
sd_listRemove (
    sd_list list,
    void    *object)
{
    sd_listNode node;
    c_bool  found = FALSE;
    void   *result = NULL;
    
    assert(list);

    node = sd_listNode(list)->next;

    while ( !found && (node != sd_listNode(list)) ) {
        if ( node->object == object ) {
            found = TRUE;
        } else {
            node = node->next;
        }
    }

    if ( found ) {
        node->next->prev = node->prev;
        node->prev->next = node->next;
        os_free(node);
        result = object;
    }

    return object;
}   

void
sd_listWalk (
    sd_list list,
    sd_listAction action,
    void *arg)
{
    sd_listNode node;
    c_bool proceed = TRUE;

    assert(list);

    node = sd_listNode(list)->next;

    while ( proceed && (node != sd_listNode(list)) ) {
        proceed = action(node->object, arg);
        node = node->next;
    }
}

static c_bool 
sd_listAddIterator (
    void *object,
    void *arg)
{
    c_iter iter = (c_iter) arg;

    c_iterAppend(iter, object);

    return TRUE;
}

c_iter
sd_listIterator (
    sd_list list)
{

    c_iter iter;

    assert(list);

    iter = c_iterNew(NULL);

    if ( iter ) {
        sd_listWalk(list, sd_listAddIterator, iter);
    }

    return iter;
}

static c_bool
countElements (
    void *obj,
    void *arg)
{
    c_ulong *count = (c_ulong *) arg;

    (*count)++;

    return TRUE;
}

c_ulong
sd_listSize (
    sd_list list)
{
    c_ulong size = 0;
    
    assert(list);
    
    sd_listWalk(list, countElements, &size);

    return size;
}

typedef struct {
    sd_listCompare compare;
    void          *arg;
    void          *result;
} compareObjectArg;


static c_bool
compareObject (
    void *object,
    void *arg)
{
    compareObjectArg *argument = (compareObjectArg *)arg;
    c_bool proceed = TRUE;
   
    if ( argument->compare ) {
        if ( argument->compare(object, argument->arg) ) {
            argument->result = object;
            proceed = FALSE;
        }
    } else {
        if ( object == argument->arg ) {
            argument->result = object;
            proceed = FALSE;
        }
    }

    return proceed;
}

void *
sd_listFindObject (
    sd_list list,
    void *object)
{
    compareObjectArg argument;

    argument.compare = NULL;
    argument.arg     = object;
    argument.result  = NULL;
    
    sd_listWalk(list, compareObject, &argument);

    return argument.result;
}

void *
sd_listFind (
    sd_list list,
    sd_listCompare compare,
    void *arg)
{
    compareObjectArg argument;

    argument.compare = compare;
    argument.arg     = arg;
    argument.result  = NULL;
    
    sd_listWalk(list, compareObject, &argument);

    return argument.result;
    
}

void *
sd_listAt (
    sd_list list,
    c_ulong index)
{
    c_ulong      i      = 0;
    sd_listNode  node;
    
    node = sd_listNode(list)->next;
    while ( node->object && (i < index) ) {
        node = node->next;
        i++;
    }

    return node->object;
}
