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
