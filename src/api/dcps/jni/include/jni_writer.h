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

/**@file api/dcps/jni/include/jni_writer.h
 * 
 * @brief The jni_writer object writes instances of topics.
 * It maps to the DCPS DataWriter object.
 */
 
#ifndef JNI_WRITER_H
#define JNI_WRITER_H

#include "jni_typebase.h"
#include "sd_serializer.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_DCPSJNI
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**@brief The writer object.
 * 
 * Maps to DCPS DataWriter.
 */
C_STRUCT(jni_writer){
    C_EXTENDS(jni_entity);
    u_writer uwriter;           /*!<The user writer*/
    jni_publisher publisher;    /*!<The publisher it lives in.*/
    jni_topic topic;            /*!<The topic it writes.*/
    sd_serializer deserializer; /*!<The sample deserializer.*/
};

#define jni_writer(w) ((jni_writer)(w))

/**@brief Creates a new writer. 
 * 
 * This function should only be called by the
 * jni_publisher object (jni_createWriter). 
 * 
 * @param pub The publisher to create the writer in.
 * @param top The topic that the writer must write instances of.
 * @param qos The desired QoS policies.
 * 
 * @return The newly created writer.
 *         If the operation failed, NULL is returned.
 */
OS_API jni_writer
jni_writerNew (
    jni_publisher pub,
    jni_topic top,
    v_writerQos qos);

/**@brief Deletes the supplied writer. 
 * 
 * This function should only be called by the
 * jni_publisher object (jni_deleteWriter).
 * 
 * @param wri The writer to delete.
 * 
 * @return JNI_RESULT_OK if succeeded, any other jni_result otherwise.
 */
OS_API jni_result
jni_writerFree (
    jni_writer wri);

/**Writes userData to the writer.
 * 
 * The data must be supplied in XML format.
 * 
 * @param wri The writer to write to.
 * @param xmlUserData The data to write (in XML format).
 */
OS_API jni_result
jni_writerWrite (
    jni_writer wri, 
    const c_char* xmlUserData);

#undef OS_API
   
#if defined (__cplusplus)
}
#endif

#endif /* JNI_WRITER_H */
