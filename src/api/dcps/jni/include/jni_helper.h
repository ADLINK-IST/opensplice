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

/**@file api/dcps/jni/include/jni_helper.h
 * @brief The jni_helper object can be used to look up a jni object providing
 * the Java object and the jni_participantFactory. 
 */
 
 /* When creating a jni object,
 * a global reference for the associated Java object is stored in it. Using 
 * the JNI environment, all jni objects can be found according to their Java
 * object reference.
 */
#ifndef JNI_HELPER_H
#define JNI_HELPER_H

#include "jni_typebase.h"
#include <jni.h>

#if defined (__cplusplus)
extern "C" {
#endif


/**@brief Looks up the jni_participant according to the JNI environment and the
 * associated Java object.
 * 
 * @param env The JNI environment, where the Java Virtual Machine stores
 * all object references.
 * @param pf The participant factory where the jni object must be located in.
 * @param jentity The Java object that is associated with the jni object.
 * 
 * @return The jni_participant object that is associated with the supplied
 * Java object. If the object cannot be found, NULL is returned.
 */
jni_participant jni_lookUpParticipant(  JNIEnv *env, 
                                        jni_participantFactory pf, 
                                        jobject jentity);

/**@brief Looks up the jni_publisher according to the JNI environment and the
 * associated Java object.
 * 
 * @param env The JNI environment, where the Java Virtual Machine stores
 * all object references.
 * @param pf The participant factory where the jni object must be located in.
 * @param jentity The Java object that is associated with the jni object.
 * 
 * @return The jni_publisher object that is associated with the supplied
 * Java object. If the object cannot be found, NULL is returned.
 */
jni_publisher   jni_lookUpPublisher(    JNIEnv *env, 
                                        jni_participantFactory pf, 
                                        jobject jentity);

/**@brief Looks up the jni_subscriber according to the JNI environment and the
 * associated Java object.
 * 
 * @param env The JNI environment, where the Java Virtual Machine stores
 * all object references.
 * @param pf The participant factory where the jni object must be located in.
 * @param jentity The Java object that is associated with the jni object.
 * 
 * @return The jni_subscriber object that is associated with the supplied
 * Java object. If the object cannot be found, NULL is returned.
 */
jni_subscriber  jni_lookUpSubscriber(   JNIEnv *env, 
                                        jni_participantFactory pf, 
                                        jobject jentity);

/**@brief Looks up the jni_topic according to the JNI environment and the
 * associated Java object.
 * 
 * @param env The JNI environment, where the Java Virtual Machine stores
 * all object references.
 * @param pf The participant factory where the jni object must be located in.
 * @param jentity The Java object that is associated with the jni object.
 * 
 * @return The jni_topic object that is associated with the supplied
 * Java object. If the object cannot be found, NULL is returned.
 */
jni_topic       jni_lookUpTopic(        JNIEnv *env, 
                                        jni_participantFactory pf, 
                                        jobject jentity);

/**@brief Looks up the jni_writer according to the JNI environment and the
 * associated Java object.
 * 
 * @param env The JNI environment, where the Java Virtual Machine stores
 * all object references.
 * @param pf The participant factory where the jni object must be located in.
 * @param jentity The Java object that is associated with the jni object.
 * 
 * @return The jni_writer object that is associated with the supplied
 * Java object. If the object cannot be found, NULL is returned.
 */
jni_writer      jni_lookUpWriter(       JNIEnv *env, 
                                        jni_participantFactory pf,
                                        jobject jentity);
                        
/**@brief Looks up the jni_reader according to the JNI environment and the
 * associated Java object.
 * 
 * @param env The JNI environment, where the Java Virtual Machine stores
 * all object references.
 * @param pf The participant factory where the jni object must be located in.
 * @param jentity The Java object that is associated with the jni object.
 * 
 * @return The jni_reader object that is associated with the supplied
 * Java object. If the object cannot be found, NULL is returned.
 */
jni_reader      jni_lookUpReader(       JNIEnv *env, 
                                        jni_participantFactory pf,
                                        jobject jentity);

#if defined (__cplusplus)
}
#endif

#endif /* JNI_HELPER_H */
