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
#ifndef UT__STACK_H
#define UT__STACK_H

#include "ut_stack.h"

#if defined (__cplusplus)
extern "C" {
#endif

OS_STRUCT(ut_stack) {
    os_uint32 depth;
    os_uint32 increment;
    os_uint32 ptr;
    void **stack;
};

#if defined (__cplusplus)
}
#endif

#endif /* UT__STACK_H */
