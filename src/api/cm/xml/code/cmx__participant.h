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
/**@file api/cm/xml/code/cmx__participant.h
 * 
 * Offers internal routines on a participant.
 */
#ifndef CMX__PARTICIPANT_H
#define CMX__PARTICIPANT_H

#include "c_typebase.h"
#include "v_participant.h"
#include "cmx__entity.h"
#include "u_dispatcher.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "cmx_participant.h"

C_CLASS(cmx_walkParticipantArg);

C_STRUCT(cmx_walkParticipantArg){
    C_EXTENDS(cmx_walkEntityArg);
    const c_char* topicName;
};

#define cmx_walkParticipantArg(a) ((cmx_walkParticipantArg)(a))

/**
 * Initializes the participant specific part of the XML representation of the 
 * supplied kernel participant. This function should only be used by the 
 * cmx_entityNewFromWalk function.
 * 
 * @param entity The entity to create a XML representation of.
 * @return The participant specific part of the XML representation of the 
 *         entity.
 */
c_char* cmx_participantInit                 (v_participant entity);

/**
 * Entity action routine to resolve all participants in the kernel.
 * 
 * @param e An entity that 'lives' in the kernel to resolve all participants 
 *          from.
 * @param args Must be of type cmx_walkEntityArg. Its list of entities will
 *             be filled with the participants in the kernel during the 
 *             execution of this function.
 */
void    cmx_participantParticipantsAction   (v_entity e, 
                                             c_voidp args);

/**
 * Entity action routine to resolve all topics in the kernel.
 * 
 * @param e An entity that 'lives' in the kernel to resolve all topics 
 *          from.
 * @param args Must be of type cmx_walkEntityArg. Its list of entities will
 *             be filled with the topics in the kernel during the 
 *             execution of this function.
 */
void    cmx_participantTopicsAction         (v_entity e, 
                                             c_voidp args);

/**
 * Entity action routine to resolve all domains in the kernel.
 * 
 * @param e An entity that 'lives' in the kernel to resolve all domains 
 *          from.
 * @param args Must be of type cmx_walkEntityArg. Its list of entities will
 *             be filled with the domains in the kernel during the 
 *             execution of this function.
 */                                             
void    cmx_participantDomainsAction        (v_entity e, 
                                             c_voidp args);


void    cmx_participantFindTopicAction      (v_entity e,
                                             c_voidp args);

#if defined (__cplusplus)
}
#endif

#endif /* CMX__PARTICIPANT_H */
