
#include "jni_misc.h"

jni_result
jni_convertResult(
    u_result ur)
{
    jni_result r;

    switch(ur){
        case U_RESULT_OK:               r = JNI_RESULT_OK;              break;
        case U_RESULT_NOT_INITIALISED:  r = JNI_RESULT_BAD_PARAMETER;   break;
        case U_RESULT_OUT_OF_MEMORY:    r = JNI_RESULT_OUT_OF_RESOURCES;break;
        case U_RESULT_INTERNAL_ERROR:   r = JNI_RESULT_ERROR;           break;
        case U_RESULT_ILL_PARAM:        r = JNI_RESULT_BAD_PARAMETER;   break;
        case U_RESULT_CLASS_MISMATCH:   r = JNI_RESULT_BAD_PARAMETER;   break;
        case U_RESULT_DETACHING:        r = JNI_RESULT_ALREADY_DELETED; break;
        default:                        r = JNI_RESULT_ERROR;           break;
    }
    return r;    
}
