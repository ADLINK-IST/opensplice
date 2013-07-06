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

/**@file api/dcps/jni/include/jni_reflection.h
 * 
 * @brief The jni_reflection object provides reflection functions for Java objects.
 * 
 * These are meant to simplify Java reflection from within the C language.
 */
 
#ifndef JNI_REFLECTION_H
#define JNI_REFLECTION_H

#include <jni.h>
#include "jni_typebase.h"

#if defined (__cplusplus)
extern "C" {
#endif

/**@brief Initializes java.util.HashSet in a given object.
 * 
 * @param env The JVM environment.
 * @param object The object the HashSet is in.
 * @param fieldName The name of the HashSet field in the object, that must
 *          be initialized.
 */
void            jni_initHashSet         (JNIEnv *env, jobject object, const char* fieldName);

#if defined (__cplusplus)
}
#endif

#endif /* JNI_REFLECTION_H */
