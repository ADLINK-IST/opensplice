/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
/**@file api/cm/xml/include/cmx_topic.h
 * Represents a topic in Splice in XML format.
 */
#ifndef CMX_TOPIC_H
#define CMX_TOPIC_H

#include "c_typebase.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_CMXML
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/**
 * Creates a new topic. It is creates by creating its user layer 
 * counterpart and serializing it into XML format. The topic must be registered
 * in Splice prior to calling this function.
 * 
 * An XML topic looks like:
   @verbatim
   <entity>
       <pointer>...</pointer>
       <handle_index>...</handle_index>
       <handle_serial>..</handle_serial>
       <name>...</name>
       <enabled>...</enabled>
       <keyList>...</keyList>       
       <typename>...</typename>
       <kind>TOPIC</kind> 
   </entity>
   @endverbatim
 * @param participant The XML representation of the participant to attach the 
 *                    topic to.
 * @param name The name for the topic.
 * @param typeName The type name of the topic.
 * @param keyList The key list of the topic.
 * @param qos The qos for the topic. If NULL is supplied, the function checks
 *            whether the topic already exists in the kernel and resolves its
 *            qos to create the topic.
 * @return The XML representation of the created topic or NULL if it could
 *         not be created.
 */
OS_API c_char* cmx_topicNew        (const c_char* participant,
                                    const c_char* name,
                                    const c_char* typeName,
                                    const c_char* keyList,
                                    const c_char* qos);

/**
 * @brief Provides access to the data type of the supplied topic.
 * 
 * The result of this function is in XML format. The data type is serialized
 * using the serialization service.
 * 
 * @param topic The topic, which data type must be resolved.
 * @return The data type of the supplied topic or NULL if the topic could not
 * be resolved.
 */
OS_API c_char* cmx_topicDataType   (const c_char* topic);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* CMX_TOPIC_H */
