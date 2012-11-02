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
#ifndef IN_RESULT_H_
#define IN_RESULT_H_

/* collection includes */
#include "Coll_Defs.h"

#if defined (__cplusplus)
extern "C" {
#endif

typedef enum in_result_e
{
    IN_RESULT_OK,
    IN_RESULT_PRECONDITION_NOT_MET,
    IN_RESULT_ERROR,
    IN_RESULT_OUT_OF_MEMORY,
    IN_RESULT_ALREADY_EXISTS,
    IN_RESULT_NOT_FOUND,
    IN_RESULT_TIMEDOUT,
    in_result_elements
} in_result;

#define IN_COLLRESULT_TO_RESULT(collResult, result)                             \
    switch(collResult)                                                          \
    {                                                                           \
    case COLL_ERROR_ALLOC:                                                      \
        result = IN_RESULT_OUT_OF_MEMORY;                                       \
        break;                                                                  \
    case COLL_ERROR_ALREADY_EXISTS:                                             \
        result = IN_RESULT_ALREADY_EXISTS;                                      \
        break;                                                                  \
    case COLL_OK:                                                               \
        result = IN_RESULT_OK;                                                  \
        break;                                                                  \
    default:                                                                    \
		result = IN_RESULT_ERROR;   											\
        assert(0);                                                              \
    }

#if defined (__cplusplus)
}
#endif

#endif /* IN_RESULT_H_ */
