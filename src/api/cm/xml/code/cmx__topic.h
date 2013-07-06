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
/**
 * @file api/cm/xml/code/cmx__topic.h
 * 
 * Offers internal routines on a topic.
 */
#ifndef CMX__TOPIC_H
#define CMX__TOPIC_H

#include "c_typebase.h"
#include "v_topic.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "cmx_topic.h"

/**
 * Initializes the topic specific part of the XML representation of the 
 * supplied kernel topic. This function should only be used by the 
 * cmx_entityNewFromWalk function.
 * 
 * @param entity The entity to create a XML representation of.
 * @return The topic specific part of the XML representation of the entity.
 */
c_char* cmx_topicInit           (v_topic entity);

/**
 * Entity action routine to resolve the data type of a topic.
 * 
 * @param entity The kernel topic to resolve the type of.
 * @param args Must be of type struct cmx_topicArg which will be filled with the
 *             XML representation of the data type of the supplied topic during
 *             the execution of the function.
 */
void  cmx_topicDataTypeAction   (v_entity entity,
                                 c_voidp args);

/**
 * Entity action routine to resolve the qos of a topic.
 * 
 * @param entity The kernel topic to resolve the qos of.
 * @param args Must be of type struct cmx_topicQos which will be filled with the
 *             qos of the supplied topic during the execution of the function.
 */
void  cmx_topicQosAction        (v_entity entity,
                                 c_voidp args);

#if defined (__cplusplus)
}
#endif

#endif /* CMX__TOPIC_H */
