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

/**@file api/dcps/jni/include/jni_participantFactory.h
 * 
 * @brief jni_participantFactory maps to the DCPS DomainParticipantFactory. 
 * 
 * Its purpose is to create and delete participants. It also contains a reference to the 
 * name service to be able to map domainId's to Splice2v3 kernels.
 * 
 * The participant factory is a singleton object.
 */
 
#ifndef JNI_PARTICIPANTFACTORY_H
#define JNI_PARTICIPANTFACTORY_H

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

/**@brief The singleton participant factory. Maps to DCPS DomainParticipantFactory.
 */
C_STRUCT(jni_participantFactory){
   C_EXTENDS(jni_entity);
   c_iter domains;              /*!Set of known Domain objects.*/
   c_iter participants;         /*!Set of participants created.*/
};

#define jni_participantFactory(pf) ((jni_participantFactory)(pf))

/**@brief Creates a new participant factory or returns the already existing instance.
 * 
 * @return The participant factory instance.
 */
OS_API jni_participantFactory
jni_getParticipantFactoryInstance ();

/**@brief Explicitly deletes the participant factory. 
 * 
 * This is necessary to neatly close the kernel and detach the user layer.
 * 
 * @return JNI_RESULT_OK if succeeded, any other jni_result otherwise.
 */
OS_API jni_result
jni_deleteParticipantFactory ();

/**@brief Creates a new participant in a specified Domain.
 * 
 * @param pf The participant factory instance.
 * @param domainId The id of the Domain to create the participant in.
 * @param qos The desired QoS policies.
 * 
 * @return The newly created participant. If the operation failed, NULL is 
 * returned.
 */
OS_API jni_participant
jni_createParticipant (
    jni_participantFactory pf,
    long domainId, 
    v_qos qos);

/**@brief Deletes a participant. 
 * 
 * All contained entities of the participant should be
 * deleted before calling this function. If this is not the case, this operation
 * will fail and return JNI_PRECONDITION_NOT_MET.
 * 
 * @param p The participant to delete.
 * 
 * @return JNI_RESULT_OK of succeeded, any other jni_result otherwise.
 */
OS_API jni_result
jni_deleteParticipant (
    jni_participant p);


#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* JNI_PARTICIPANTFACTORY_H */
