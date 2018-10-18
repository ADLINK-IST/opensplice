/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
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
c_char* cmx_participantInit (v_participant entity);

/**
 * Entity action routine to resolve all participants in the kernel.
 *
 * @param e An entity that 'lives' in the kernel to resolve all participants
 *          from.
 * @param args Must be of type cmx_walkEntityArg. Its list of entities will
 *             be filled with the participants in the kernel during the
 *             execution of this function.
 */
void cmx_participantParticipantsAction (v_public p, c_voidp args);

/**
 * Entity action routine to resolve all topics in the kernel.
 *
 * @param e An entity that 'lives' in the kernel to resolve all topics
 *          from.
 * @param args Must be of type cmx_walkEntityArg. Its list of entities will
 *             be filled with the topics in the kernel during the
 *             execution of this function.
 */
void cmx_participantTopicsAction (v_public p, c_voidp args);

/**
 * Entity action routine to resolve all domains in the kernel.
 *
 * @param e An entity that 'lives' in the kernel to resolve all domains
 *          from.
 * @param args Must be of type cmx_walkEntityArg. Its list of entities will
 *             be filled with the domains in the kernel during the
 *             execution of this function.
 */                                             
void cmx_participantDomainsAction (v_public p, c_voidp args);


void cmx_participantFindTopicAction (v_public e, c_voidp args);

#if defined (__cplusplus)
}
#endif

#endif /* CMX__PARTICIPANT_H */
