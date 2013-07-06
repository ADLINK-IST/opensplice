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

/**@file api/dcps/jni/include/jni_subscriberQos.h
 * @brief The jni_subscriberQos object is used to handle subscriber QoS
 * objects between Java and C and the other way around.
 */
#ifndef JNI_SUBSCRIBERQOS_H
#define JNI_SUBSCRIBERQOS_H

#include <jni.h>
#include "jni_typebase.h"
#include "v_kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif

/**@brief Gives access to the partition QoS of the subscriber.
 * 
 * @param env The Java VM environment.
 * @param jqos The Java subscriber QoS.
 * @return The partition policy.
 */
const c_char*   jni_subscriberQosGetPartition(JNIEnv* env, jobject jqos);

#if defined (__cplusplus)
}
#endif

#endif /* JNI_SUBSCRIBERQOS_H */
