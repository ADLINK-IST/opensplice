#ifndef GAPI_ENTITYVALIDITY_H
#define GAPI_ENTITYVALIDITY_H

#include "gapi_common.h"

#define _ContentFilteredTopicNowValid(obj) \
        (_EntityValidity(obj)->magic = _ContentFilteredTopic_v)

#define _DataReaderNowValid(obj) \
        (_EntityValidity(obj)->magic = _DataReader_v)

#define _DataWriterNowValid(obj) \
        (_EntityValidity(obj)->magic = _DataWriter_v)

#define _DomainEntityNowValid(obj) \
        (_EntityValidity(obj)->magic = _DomainEntity_v)

#define _DomainParticipantNowValid(obj) \
        (_EntityValidity(obj)->magic = _DomainParticipant_v)

#define _EntityNowValid(obj) \
        (_EntityValidity(obj)->magic = _Entity_v)

#define _FooDataReaderNowValid(obj) \
        (_EntityValidity(obj)->magic = _FooDataReader_v)

#define _FooDataWriterNowValid(obj) \
        (_EntityValidity(obj)->magic = _FooDataWriter_v)

#define _FooTypeSupportNowValid(obj) \
        (_EntityValidity(obj)->magic = _FooTypeSupport_v)

#define _MultiTopicNowValid(obj) \
        (_EntityValidity(obj)->magic = _MultiTopic_v)

#define _PublisherNowValid(obj) \
        (_EntityValidity(obj)->magic = _Publisher_v)

#define _SubscriberNowValid(obj) \
        (_EntityValidity(obj)->magic = _Subscriber_v)

#define _TopicNowValid(obj) \
        (_EntityValidity(obj)->magic = _Topic_v)

#define _TopicDescriptionNowValid(obj) \
        (_EntityValidity(obj)->magic = _TopicDescription_v)

#define _TypeSupportNowValid(obj) \
        (_EntityValidity(obj)->magic = _TypeSupport_v)

#define _ReadConditionNowValid(obj) \
        (_EntityValidity(obj)->magic = _ReadCondition_v)

#define _StatusConditionNowValid(obj) \
        (_EntityValidity(obj)->magic = _StatusCondition_v)

#define _QueryConditionNowValid(obj) \
        (_EntityValidity(obj)->magic = _QueryCondition_v)

#define _GuardConditionNowValid(obj) \
        (_EntityValidity(obj)->magic = _GuardCondition_v)

#define _WaitSetNowValid(obj) \
        (_EntityValidity(obj)->magic = _WaitSet_v)


_EntityValidity
_EntityValidityInit (
    void
    );

void
_EntityValidityDispose (
    _EntityValidity entityvalidity
    );

#endif /* GAPI_ENTITYVALIDITY_H */
