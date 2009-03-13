#ifndef V__WRITER_H
#define V__WRITER_H

#include "v_writer.h"
#include "v_writerInstance.h"
#include "v_entity.h"

#define v_writerKeyList(_this) \
        c_tableKeyList(v_writer(_this)->instances)

typedef struct v_writerNotifyChangedQosArg_s {
    /* the following fields are set when the partitionpolicy has changed. */
    c_iter addedDomains;
    c_iter removedDomains;
} v_writerNotifyChangedQosArg;

c_bool
v_writerPublishGroup (
    v_writer _this,
    v_group g);

c_bool
v_writerUnPublishGroup (
    v_writer _this,
    v_group g);

void
v_writerNotifyIncompatibleQos (
    v_writer _this,
    v_policyId id);

void
v_writerNotifyChangedQos (
    v_writer _this,
    v_writerNotifyChangedQosArg *arg);

void
v_writerNotifyLivelinessLost (
    v_writer _this);

/* To be used by sendQueue */
void
v_writerGroupsWrite (
    v_writer _this,
    v_message message);

v_result
v_writerSetQos (
    v_writer _this,
    v_writerQos qos);

void
v_writerDeadLineListUpdateInstance (
    v_writer _this,
    v_writerInstance instance);

void
v_writerAssertByPublisher (
    v_writer _this);

void
v_writerNotifyTake (
    v_writer _this,
    v_writerInstance instance);

c_bool
v_writerCompareKeyValues (
    v_writer _this,
    v_message message,
    v_writerInstance instance);

v_message
v_writerKeepMessage (
    v_writer _this,
    c_ulong seqNr);

void
v_writerResendMessage (
    v_writer _this,
    c_ulong seqNr);

void
v_writerCheckDeadlineMissed (
    v_writer _this,
    c_time now);

void
v_writerResumePublication (
    v_writer _this,
    c_time *suspendTime);

#endif
