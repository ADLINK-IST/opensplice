/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#ifndef V__PARTICIPANT_H
#define V__PARTICIPANT_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "v_participant.h"
#include "v_writer.h"

v_result
v_participantSetQos(
    v_participant _this,
    v_participantQos qos);

void
v_participantResendManagerAddWriter(
    v_participant _this,
    v_writer w);

void
v_participantResendManagerRemoveWriter(
    v_participant _this,
    v_writer w);

#if defined (__cplusplus)
}
#endif

#endif /* V__PARTICIPANT_H */
