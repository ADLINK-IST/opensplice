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

/**@file api/dcps/jni/include/jni_reader.h
 * 
 * @brief The jni_reader object represents a DCPS DataReader.
 */

#ifndef JNI_READER_H
#define JNI_READER_H

#include "jni_typebase.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_DCPSJNI
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**@brief The reader object.
 * 
 * Maps to DCPS DataReader.
 */
C_STRUCT(jni_reader){
    C_EXTENDS(jni_entity);
    jni_subscriber subscriber;          /*!The subscriber it lives in.*/
    jni_topicDescription description;   /*!The topic it reads.*/
    u_dataReader ureader;               /*!The user dataReader.*/
    u_query uquery;                     /*!The user query.*/
};

#define jni_reader(w) ((jni_reader)(w))

/**@brief Creates a new jni_reader object. This function should only be called by
 * the jni_subscriber object (jni_createReader).
 * 
 * @param sub The subscriber to create the reader in.
 * @param top The topic description where the reader reads instances of.
 * @param qos The desired QoS policies.
 * 
 * @return the newly created reader.
 */
OS_API jni_reader  jni_readerNew(  jni_subscriber sub,
                            jni_topicDescription top,
                            v_readerQos qos);

/**@brief Deletes the supplied reader. This funtion should only be called by the
 * jni_subscriber object (jni_deleteReader).
 * 
 * @param rea The reader to delete.
 * 
 * @return JNI_RESULT_OK if succeeded, any other jni_result otherwise.
 */
OS_API jni_result  jni_readerFree( jni_reader rea);

/**@brief Reads a sample from the supplied reader.
 * 
 * @param rea The reader to read from.
 * @return The XML representation of the read sample.
 */
OS_API c_char*     jni_readerRead( jni_reader rea);

/**@brief Reads a sample from the supplied reader and removes it.
 * 
 * @param rea The reader to read from.
 * @return The XML representation of the read sample.
 */
OS_API c_char*     jni_readerTake( jni_reader rea);

/**@brief Attaches a query to the supplied reader. 
 * 
 * At this time only one query can be attached and it cannot be removed
 * after it has been attached. The only way to change it, is to delete the
 * reader and create a new one.
 * 
 * @param rea The reader to attach the query to.
 * @param query_expression The query expression that must have the syntax of the
 * 'WHERE' clause of an OQL expression.
 * @param params The query parameters of the query expression.
 * @return JNI_RESULT_OK if succeeded, any other jni_result otherwise.
 */
OS_API jni_result  jni_readerSetQuery( jni_reader rea,
                                const c_char* query_expression, 
                                c_value params[]);

#undef OS_API 
   
#if defined (__cplusplus)
}
#endif

#endif /* JNI_READER_H */
