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
#ifndef NN_ENTITY_H
#define NN_ENTITY_H

#include "os_classbase.h"
#include "os_iterator.h"
#include "os_heap.h"
#include "kernelModule.h"
#include "u_user.h"

OS_CLASS(in_entity);
OS_CLASS(in_participant);
OS_CLASS(in_reader);
OS_CLASS(in_writer);
OS_CLASS(in_group);

#define in_entity(e) ((in_entity)(e))
#define in_participant(p) ((in_participant)(p))
#define in_reader(r) ((in_reader)(r))
#define in_writer(w) ((in_writer)(w))

struct nn_xpack;

in_participant
in_participantNew(
    struct v_participantInfo* ddsParticipant);

os_equality
in_participantCompare(
    in_participant p1,
    in_participant p2,
    void* args);

in_participant
in_participantLookup(
    v_builtinTopicKey* key);

os_int32
in_participantReport(
    in_participant participant,
    void* arg);

os_int32
in_writerReport(
    in_writer writer,
    void* arg);

in_writer
in_writerLookup(
    v_builtinTopicKey* key);

os_int32
in_readerReport(
    in_reader reader,
    void* arg);

in_reader
in_readerLookup(
    v_builtinTopicKey* key);

void
in_participantFree(
    in_participant participant,
    void* dummy);

in_reader
in_fictitiousTransientReaderNew(
        in_participant participant,
        v_group group);

in_reader
in_readerNew(
    in_participant participant,
    struct v_subscriptionInfo* ddsReader);

void
in_readerFree(
    in_reader reader,
    void* dummy);

in_writer
in_writerNew(
    in_participant participant,
    struct v_publicationInfo* ddsWriter);

void
in_writerFree(
    in_writer writer,
    void* dummy);

os_boolean
in_writerWrite(
    struct nn_xpack *xp,
    in_writer writer,
    v_message message);

void
reportEntities();

os_boolean
in_entityAdminInit(
    u_participant participant);

void
in_entityAdminDestroy();

#endif /* NN_ENTITY_H */
