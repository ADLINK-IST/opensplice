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

#include "cm_misc.h"

/* Unused for now
const char*
cm_getUserResult(
    u_result r)
{
    if(r == U_RESULT_OK){
        return "U_RESULT_OK";
    }
    else if(r == U_RESULT_NOT_INITIALISED){
        return "U_RESULT_NOT_INITIALISED";
    }
    else if(r == U_RESULT_OUT_OF_MEMORY){
        return "U_RESULT_OUT_OF_MEMORY";
    }
    else if(r == U_RESULT_INTERNAL_ERROR){
        return "U_RESULT_INTERNAL_ERROR";
    }
    else if(r == U_RESULT_ILL_PARAM){
        return "U_RESULT_ILL_PARAM";
    }
    else if(r == U_RESULT_CLASS_MISMATCH){
        return "U_RESULT_MISMATCH";
    }
    else if(r == U_RESULT_DETACHING){
        return "U_RESULT_DETACHING";
    }
    else{
        return "U_RESULT_UNDEFINED";
    }
}
*/

cm_result
cm_convertResult(
    u_result ur)
{
    cm_result r;
    
    switch(ur){
        case U_RESULT_OK:               r = CM_RESULT_OK;               break;
        case U_RESULT_NOT_INITIALISED:  r = CM_RESULT_BAD_PARAMETER;    break;
        case U_RESULT_OUT_OF_MEMORY:    r = CM_RESULT_ERROR;            break;
        case U_RESULT_INTERNAL_ERROR:   r = CM_RESULT_ERROR;            break;
        case U_RESULT_ILL_PARAM:        r = CM_RESULT_BAD_PARAMETER;    break;
        case U_RESULT_CLASS_MISMATCH:   r = CM_RESULT_BAD_PARAMETER;    break;
        case U_RESULT_DETACHING:        r = CM_RESULT_ERROR;            break;
        default:                        r = CM_RESULT_UNDEFINED;        break;
    }
    return r;    
}

int
cm_resultIntValue(
    cm_result cmr)
{
    int value;
    
    switch(cmr){
       case CM_RESULT_OK:               value = 0;  break;
       case CM_RESULT_ERROR:            value = 1;  break;
       case CM_RESULT_BAD_PARAMETER:    value = 2;  break;
       case CM_RESULT_UNSUPPORTED:      value = 3;  break;
       default:                         value = 4;  break;
    }
    return value;
}
