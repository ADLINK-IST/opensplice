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

/**@file api/dcps/jni/include/jni_partition.h
 * @brief The jni_partition object can be used to create a new partition.
 */
 
#ifndef JNI_PARTITION_H
#define JNI_PARTITION_H

#include "jni_typebase.h"
#include "v_kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif

/**@brief JNI partition mapping, which contains a user partition and the name.
 */
C_STRUCT(jni_partition){
    const c_char* name; /*!The name of the partition.*/
    u_partition upartition;   /*!The user partition.*/
};

#define jni_partition(d) ((jni_partition)(d))

/**@brief Creates a new partition.
 * 
 * The partition will be attached to the supplied participant.
 * 
 * @param p The participant to attach the partition to.
 * @param name The name of the partition.
 * @param qos The partition QoS.
 * @return The newly created partition.
 */
jni_partition
jni_partitionNew (
    jni_participant p, 
    const c_char* name,
    v_partitionQos qos);

/**@brief Detaches the partition and frees its resources.
 * 
 * @param _this The partition to clean up.
 * @return JNI_RESULT_OK if succeeded, JNI_RESULT_BAD_PARAMETER otherwise.
 */
jni_result
jni_partitionFree (
    jni_partition _this);

#if defined (__cplusplus)
}
#endif

#endif /* JNI_PARTITION_H */
