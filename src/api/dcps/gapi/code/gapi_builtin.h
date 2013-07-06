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

#ifndef GAPI_BUILTINDATAREADER_H
#define GAPI_BUILTINDATAREADER_H

#include "gapi.h"
#include "gapi_common.h"
#include "gapi_dataReader.h"

#define MAX_BUILTIN_TOPIC 4

typedef struct BuiltinTopicTypeInfo {
    char                 *topicName;
    char                 *typeName;
    gapi_unsigned_long    defaultAllocSize;
    gapi_topicAllocBuffer defaultAllocBuffer;
    gapi_readerCopy       defaultReaderCopy;
    gapi_copyOut          defaultCopyOut;
} BuiltinTopicTypeInfo;


_Subscriber
_BuiltinSubscriberNew (
    u_participant uParticipant,
    _DomainParticipantFactory factory,
    _DomainParticipant participant);

void
_BuiltinSubscriberFree (
    _Subscriber subscriber);

const char *
_BuiltinTopicName  (
    long index);

const char *
_BuiltinTopicTypeName  (
    long index);

const BuiltinTopicTypeInfo *
_BuiltinTopicFindTypeInfoByName (
    const char *topicName);

const BuiltinTopicTypeInfo *
_BuiltinTopicFindTypeInfoByType (
    const char *typeName);

const char *
_BuiltinFindTopicName (
    _Entity entity);

#endif
