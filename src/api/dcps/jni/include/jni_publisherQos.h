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

/**@file api/dcps/jni/include/jni_publisherQos.h
 * @brief The jni_publisherQos object is used to handle publisher QoS
 * objects between Java and C and the other way around.
 */
#ifndef JNI_PUBLISHERQOS_H
#define JNI_PUBLISHERQOS_H

#include <jni.h>
#include "jni_typebase.h"
#include "v_kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif


/**@brief Gives access to the partition QoS of the provided publisher QoS.
 * 
 * @param env The Java VM environment.
 * @param jqos the Java publisher QoS.
 * 
 * @return The string representation of the Partition.
 */
const c_char*   jni_publisherQosGetPartition(JNIEnv* env, jobject jqos);

#if defined (__cplusplus)
}
#endif

#endif /* JNI_PUBLISHERQOS_H */
