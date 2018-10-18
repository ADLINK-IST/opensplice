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
#include "ut__stack.h"

#include "vortex_os.h"

#include <assert.h>
#include <string.h>

ut_stack
ut_stackNew(
    os_uint32 increment)
{
    ut_stack s;

    assert(increment > 0);

    s = os_malloc(OS_SIZEOF(ut_stack));
    s->depth = increment;
    s->increment = increment;
    s->ptr = 0;
    s->stack = os_malloc(sizeof(void *) * s->depth);
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
    os_uint32 i = stack->ptr;

    utr = UT_RESULT_OK;
    if (i) do {
        utr = action(stack->stack[--i], arg);
    } while (i > 0 && utr == UT_RESULT_OK);
    return utr;
}
