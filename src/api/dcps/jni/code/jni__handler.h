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

#ifndef JNI__HANDLER_H
#define JNI__HANDLER_H

#include "jni_typebase.h"

#if defined (__cplusplus)
extern "C" {
#endif

C_CLASS(jni_entityKernelArg);

C_STRUCT(jni_entityKernelArg){
    v_kernel kernel;
};

#define jni_entityKernelArg(a) ((jni_entityKernelArg)(a))

c_char* jni_getFullName             (c_char* class);

c_char* jni_getFullRepresentation   (c_char* class);

void    jni_entityKernelAction      (v_entity entity, c_voidp args);

#if defined (__cplusplus)
}
#endif

#endif /* JNI__HANDLER_H */
