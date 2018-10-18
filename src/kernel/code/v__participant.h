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
#ifndef V__PARTICIPANT_H
#define V__PARTICIPANT_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "v_participant.h"
#include "v_writer.h"

#define v_participantProcessId(p) (p)->processId

void
v_participantResendManagerAddWriter(
    v_participant _this,
    v_writer w);

void
v_participantResendManagerRemoveWriter(
    v_participant _this,
    v_writer w);
void
v_participantResendManagerRemoveWriterBlocking(
    v_participant _this,
    v_writer w);

v_message
v_participantCreateCandMCommand(
    v_participant _this);

v_result
v_participantCandMCommandSetDisposeAllData(
    v_participant _this,
    v_message msg,
    char *topicExpr,
    char *partitionExpr);

v_result
v_participantWriteCandMCommand(
    v_participant _this,
    v_message msg);

c_iter
v_participantGetEntityList(
    v_participant _this);

v_typeRepresentation
v__participantAddTypeRepresentation (
    v_participant p,
    v_typeRepresentation tr);

v_typeRepresentation
v__participantRemoveTypeRepresentation (
    v_participant p,
    v_typeRepresentation tr);

void
v_participantNotifyGroupCoherentPublication(
    v_participant _this,
    v_message msg);

void
v_participantConnectGroup (
    v_participant _this,
    v_group g);

/* This operation will update the participant according to a newly discovered publication and
 * notify matching local subscriptions (synchronously).
 * The given message holds the builtin publication data describing the discovered writer.
 * This operation will lock the participant and contained subscribers and contained DataReaders.
 */
void
v_participantNotifyPublication(
    v_participant _this,
    v_message msg);

/* This operation will update the participant according to a newly discovered subscription and
 * notify matching local publications (synchronously).
 * The given message holds the builtin subscription data describing the discovered reader.
 * This operation will lock the participant and contained publishers and contained DataWriters.
 */
void
v_participantNotifySubscription(
    v_participant _this,
    v_message msg);

/* This operation will return TRUE if the writer identified given publication info is ignored by
 * this participant.
 */
os_boolean
v_participantCheckPublicationIgnored(
    v_participant _this,
    const struct v_publicationInfo *info);

#if defined (__cplusplus)
}
#endif

#endif /* V__PARTICIPANT_H */
