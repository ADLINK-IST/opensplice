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

#include "jni_misc.h"

jni_result
jni_convertResult(
    u_result ur)
{
    jni_result r;

    switch(ur){
        case U_RESULT_OK:               r = JNI_RESULT_OK;              break;
        case U_RESULT_NOT_INITIALISED:  r = JNI_RESULT_BAD_PARAMETER;   break;
        case U_RESULT_OUT_OF_MEMORY: /* intended fall-through */
        case U_RESULT_OUT_OF_RESOURCES: r = JNI_RESULT_OUT_OF_RESOURCES;break;
        case U_RESULT_INTERNAL_ERROR:   r = JNI_RESULT_ERROR;           break;
        case U_RESULT_ILL_PARAM:        r = JNI_RESULT_BAD_PARAMETER;   break;
        case U_RESULT_CLASS_MISMATCH:   r = JNI_RESULT_BAD_PARAMETER;   break;
        case U_RESULT_DETACHING:        r = JNI_RESULT_ALREADY_DELETED; break;
        default:                        r = JNI_RESULT_ERROR;           break;
    }
    return r;    
}
