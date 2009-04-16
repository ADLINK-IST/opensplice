/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#include "u__types.h"

c_char *
u_result_image(
    u_result result)
{
    c_char *image = NULL;

#define _CASE_(o) case o: image = #o; break;
    switch (result) {
    _CASE_(U_RESULT_UNDEFINED);
    _CASE_(U_RESULT_OK);
    _CASE_(U_RESULT_INTERRUPTED);
    _CASE_(U_RESULT_NOT_INITIALISED);
    _CASE_(U_RESULT_OUT_OF_MEMORY);
    _CASE_(U_RESULT_INTERNAL_ERROR);
    _CASE_(U_RESULT_ILL_PARAM);
    _CASE_(U_RESULT_CLASS_MISMATCH);
    _CASE_(U_RESULT_DETACHING);
    _CASE_(U_RESULT_TIMEOUT);
    _CASE_(U_RESULT_INCONSISTENT_QOS);
    _CASE_(U_RESULT_IMMUTABLE_POLICY);
    _CASE_(U_RESULT_PRECONDITION_NOT_MET);
    _CASE_(U_RESULT_UNSUPPORTED);
    default:
        image = "Internal error: no image for illegal result value";
    break;
    }
    return image;
#undef _CASE_
}
