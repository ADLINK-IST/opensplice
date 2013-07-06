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
#include "ut__stack.h"

#include "os.h"

#include <assert.h>
#include <string.h>

/**************************************************************
 * Private functions
 **************************************************************/

/**************************************************************
 * constructor/destructor
 **************************************************************/
ut_stack
ut_stackNew(
    os_uint32 increment)
{
    ut_stack s;

    assert(increment > 0);

    s = os_malloc(OS_SIZEOF(ut_stack));
    if (s != NULL) {
        s->depth = increment;
        s->increment = increment;
        s->ptr = 0;
        s->stack = os_malloc(sizeof(void *) * s->depth);
        if (s->stack == NULL) {
            os_free(s);
            s = NULL;
        }
    }

    return s;
}

ut_result
ut_stackFree(
    ut_stack stack)
{
    os_free(stack->stack);
    os_free(stack);

    return UT_RESULT_OK;
}


/**************************************************************
 * Protected functions
 **************************************************************/

/**************************************************************
 * Public functions
 **************************************************************/
ut_result
ut_stackPush(
    ut_stack stack,
    void *o)
{
    void **newStack;
    ut_result result = UT_RESULT_OK;

    assert(stack->ptr <= stack->depth);

    if (stack->ptr == stack->depth) {
        newStack = os_malloc(sizeof(void *) * 
                       (stack->depth + stack->increment));
        if (newStack != NULL) {
            memcpy(newStack, stack->stack, (sizeof(void *) * stack->depth));
            os_free(stack->stack);
            stack->stack = newStack;
            stack->depth += stack->increment;
            stack->stack[stack->ptr++] = o;
        } else {
            result = UT_RESULT_OUT_OF_MEMORY;
        }
    } else {
        stack->stack[stack->ptr++] = o;
    }

    return result;
}

void *
ut_stackPop(
    ut_stack stack)
{
    void *o;
    if (stack->ptr != 0) {
        o = stack->stack[--stack->ptr];
    } else {
        o = NULL;
    }
    return o;
}

os_int32
ut_stackIsEmpty(
    ut_stack stack)
{
    os_int32 empty;

    if (stack->ptr == 0) {
        empty = 1;
    } else {
        empty = 0;
    }
    return empty;
}

ut_result
ut_stackWalk(
    ut_stack stack,
    ut_stackWalkAction action,
    void *arg)
{
    ut_result utr;
    os_uint32 i;
    
    utr = UT_RESULT_OK;
    if (stack->ptr > 0) {
        i = stack->ptr-1;
        while ((i > 0) && (utr == UT_RESULT_OK)) {
            utr = action(stack->stack[i], arg);
            i--;
        }
        utr = action(stack->stack[i], arg);
    }
    return utr;
}
