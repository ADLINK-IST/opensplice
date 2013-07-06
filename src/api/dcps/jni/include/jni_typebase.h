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

/**@file api/dcps/jni/include/jni_typebase.h
 * 
 * @brief Contains the generic objects that are used in the complete package.
 */
#ifndef JNI_TYPEBASE_H
#define JNI_TYPEBASE_H

#include <jni.h>

#include "c_iterator.h"
#include "u_user.h"
#include "c_typebase.h"

#if defined (__cplusplus)
extern "C" {
#endif


/**@brief This enumeration is used to supply information about the result of 
 * certain operations.
 */
typedef enum {
    JNI_RESULT_OK,                  /*!<Everything went fine.*/
    JNI_RESULT_ERROR,               /*!<Generic unspecified error.*/
    JNI_RESULT_BAD_PARAMETER,       /*!<A bad parameter was supplied.*/
    JNI_RESULT_UNSUPPORTED,         /*!<This operation has not been implemented.*/
    JNI_RESULT_ALREADY_DELETED,     /*!<The entity was already deleted.*/
    JNI_RESULT_OUT_OF_RESOURCES,    /*!<No more resources avaiable to allocate.*/
    JNI_RESULT_NOT_ENABLED,         /*!<The supplied entity was not enabled.*/
    JNI_RESULT_IMMUTABLE_POLICY,    /*!<The QoS policy cannot be altered.*/
    JNI_RESULT_INCONSISTENT_POLICY, /*!<The QoS policy is not consistent.*/
    JNI_RESULT_PRECONDITION_NOT_MET /*!<One or more precondition have not been met.*/
} jni_result;


C_CLASS(jni_entity);
C_CLASS(jni_nameService);
C_CLASS(jni_participant);
C_CLASS(jni_participantFactory);
C_CLASS(jni_publisher);
C_CLASS(jni_subscriber);
C_CLASS(jni_topicDescription);
C_CLASS(jni_topic);
C_CLASS(jni_writer);
C_CLASS(jni_reader);
C_CLASS(jni_partition);

/**@brief Base class for all jni entities.
 */
C_STRUCT(jni_entity){
    jobject javaObject; /*! The Java object (global reference in JVM).*/
};

#define jni_entity(ent) ((jni_entity)(ent))

/**@brief Defines the context for os reports.
 */
#define CONT_DCPSJNI "dcpsjni layer"

#if defined (__cplusplus)
}
#endif

#endif /* JNI_TYPEBASE_H */
