
/**@file api/dcps/jni/include/jni_subscriberQos.h
 * @brief The jni_subscriberQos object is used to handle subscriber QoS
 * objects between Java and C and the other way around.
 */
#ifndef JNI_SUBSCRIBERQOS_H
#define JNI_SUBSCRIBERQOS_H

#include "jni.h"
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
