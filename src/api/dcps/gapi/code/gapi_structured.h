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
#ifndef GAPI_STRUCTURED_H
#define GAPI_STRUCTURED_H

#include "gapi.h"
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define GAPI_DATASAMPLESEQ_INCREMENT               32
#define V_DATAREADERSAMPLESEQ_INITIAL             128
#define V_DATAREADERSAMPLESEQ_INCREMENT           128


extern void (*gapi_instanceHandleSeqCopyin) (const void *src, gapi_instanceHandleSeq *dst);
extern void (*gapi_instanceHandleSeqCopyout) (const gapi_instanceHandleSeq *src, void *dst);

extern void (*gapi_stringSeqCopyin) (const void *src, gapi_stringSeq *dst);
extern void (*gapi_stringSeqCopyout) (const gapi_stringSeq *src, void *dst);

extern void (*gapi_duration_tCopyin) (const void *src, gapi_duration_t *dst);
extern void (*gapi_duration_tCopyout) (const gapi_duration_t *src, void *dst);

extern void (*gapi_time_tCopyin) (const void *src, gapi_time_t *dst);
extern void (*gapi_time_tCopyout) (const gapi_time_t *src, void *dst);

extern void (*gapi_inconsistentTopicStatusCopyin) (const void *src, gapi_inconsistentTopicStatus *dst);
extern void (*gapi_inconsistentTopicStatusCopyout) (const gapi_inconsistentTopicStatus *src, void *dst);

OS_API extern void (*gapi_sampleLostStatusCopyin) (const void *src, gapi_sampleLostStatus *dst);
OS_API extern void (*gapi_sampleLostStatusCopyout) (const gapi_sampleLostStatus *src, void *dst);

OS_API extern void (*gapi_sampleRejectedStatusCopyin) (const void *src, gapi_sampleRejectedStatus *dst);
OS_API extern void (*gapi_sampleRejectedStatusCopyout) (const gapi_sampleRejectedStatus *src, void *dst);

OS_API extern void (*gapi_livelinessLostStatusCopyin) (const void *src, gapi_livelinessLostStatus *dst);
OS_API extern void (*gapi_livelinessLostStatusCopyout) (const gapi_livelinessLostStatus *src, void *dst);

OS_API extern void (*gapi_livelinessChangedStatusCopyin) (const void *src, gapi_livelinessChangedStatus *dst);
OS_API extern void (*gapi_livelinessChangedStatusCopyout) (const gapi_livelinessChangedStatus *src, void *dst);

OS_API extern void (*gapi_offeredDeadlineMissedStatusCopyin) (const void *src, gapi_offeredDeadlineMissedStatus *dst);
OS_API extern void (*gapi_offeredDeadlineMissedStatusCopyout) (const gapi_offeredDeadlineMissedStatus *src, void *dst);

OS_API extern void (*gapi_requestedDeadlineMissedStatusCopyin) (const void *src, gapi_requestedDeadlineMissedStatus *dst);
OS_API extern void (*gapi_requestedDeadlineMissedStatusCopyout) (const gapi_requestedDeadlineMissedStatus *src, void *dst);

OS_API extern void (*gapi_qosPolicyCountCopyin) (const void *src, gapi_qosPolicyCount *dst);
OS_API extern void (*gapi_qosPolicyCountCopyout) (const gapi_qosPolicyCount *src, void *dst);

OS_API extern void (*gapi_qosPolicyCountSeqCopyin) (const void *src, gapi_qosPolicyCountSeq *dst);
OS_API extern void (*gapi_qosPolicyCountSeqCopyout) (const gapi_qosPolicyCountSeq *src, void *dst);

OS_API extern void (*gapi_offeredIncompatibleQosStatusCopyin) (const void *src, gapi_offeredIncompatibleQosStatus *dst);
OS_API extern void (*gapi_offeredIncompatibleQosStatusCopyout) (const gapi_offeredIncompatibleQosStatus *src, void *dst);

OS_API extern void (*gapi_requestedIncompatibleQosStatusCopyin) (const void *src, gapi_requestedIncompatibleQosStatus *dst);
OS_API extern void (*gapi_requestedIncompatibleQosStatusCopyout) (const gapi_requestedIncompatibleQosStatus *src, void *dst);

OS_API extern void (*gapi_publicationMatchedStatusCopyin) (const void *src, gapi_publicationMatchedStatus *dst);
OS_API extern void (*gapi_publicationMatchedStatusCopyout) (const gapi_publicationMatchedStatus *src, void *dst);

OS_API extern void (*gapi_subscriptionMatchedStatusCopyin) (const void *src, gapi_subscriptionMatchedStatus *dst);
OS_API extern void (*gapi_subscriptionMatchedStatusCopyout) (const gapi_subscriptionMatchedStatus *src, void *dst);

extern void (*gapi_topicSeqCopyin) (const void *src, gapi_topicSeq *dst);
extern void (*gapi_topicSeqCopyout) (const gapi_topicSeq *src, void *dst);

extern void (*gapi_dataReaderSeqCopyin) (const void *src, gapi_dataReaderSeq *dst);
extern void (*gapi_dataReaderSeqCopyout) (const gapi_dataReaderSeq *src, void *dst);

extern void (*gapi_dataReaderViewSeqCopyin) (const void *src, gapi_dataReaderViewSeq *dst);
extern void (*gapi_dataReaderViewSeqCopyout) (const gapi_dataReaderViewSeq *src, void *dst);

extern void (*gapi_conditionSeqCopyin) (const void *src, gapi_conditionSeq *dst);
extern void (*gapi_conditionSeqCopyout) (const gapi_conditionSeq *src, void *dst);

extern void (*gapi_sampleStateSeqCopyin) (const void *src, gapi_sampleStateSeq *dst);
extern void (*gapi_sampleStateSeqCopyout) (const gapi_sampleStateSeq *src, void *dst);

extern void (*gapi_instanceStateSeqCopyin) (const void *src, gapi_instanceStateSeq *dst);
extern void (*gapi_instanceStateSeqCopyout) (const gapi_instanceStateSeq *src, void *dst);

extern gapi_boolean (*gapi_participantBuiltinTopicDataCopyin) (const void *src, gapi_participantBuiltinTopicData *dst);
extern void (*gapi_participantBuiltinTopicDataCopyout) (const gapi_participantBuiltinTopicData *src, void *dst);

extern gapi_boolean (*gapi_topicBuiltinTopicDataCopyin) (const void *src, gapi_topicBuiltinTopicData *dst);
extern void (*gapi_topicBuiltinTopicDataCopyout) (const gapi_topicBuiltinTopicData *src, void *dst);

extern gapi_boolean (*gapi_publicationBuiltinTopicDataCopyin) (const void *src, gapi_publicationBuiltinTopicData *dst);
extern void (*gapi_publicationBuiltinTopicDataCopyout) (const gapi_publicationBuiltinTopicData *src, void *dst);

extern gapi_boolean (*gapi_subscriptionBuiltinTopicDataCopyin) (const void *src, gapi_subscriptionBuiltinTopicData *dst);
extern void (*gapi_subscriptionBuiltinTopicDataCopyout) (const gapi_subscriptionBuiltinTopicData *src, void *dst);

extern void (*gapi_sampleInfoCopyin) (const void *src, gapi_sampleInfo *dst);
extern void (*gapi_sampleInfoCopyout) (const gapi_sampleInfo *src, void *dst);

extern void (*gapi_sampleInfoSeqCopyin) (const void *src, gapi_sampleInfoSeq *dst);
extern void (*gapi_sampleInfoSeqCopyout) (const gapi_sampleInfoSeq *src, void *dst);

gapi_boolean
subscriptionKeyQosPolicyEqual (
    const gapi_subscriptionKeyQosPolicy *orig,
    const gapi_subscriptionKeyQosPolicy *req);

gapi_boolean
viewKeyQosPolicyEqual (
    const gapi_viewKeyQosPolicy *orig,
    const gapi_viewKeyQosPolicy *req);

/*
 * Qos Copy functions
 */
OS_API gapi_domainParticipantQos *
gapi_domainParticipantQosCopy (
    const gapi_domainParticipantQos *src,
    gapi_domainParticipantQos *dst);

OS_API gapi_topicQos *
gapi_topicQosCopy (
    const gapi_topicQos *src,
    gapi_topicQos *dst);

OS_API gapi_dataWriterQos *
gapi_dataWriterQosCopy (
    const gapi_dataWriterQos *src,
    gapi_dataWriterQos *dst);

OS_API gapi_publisherQos *
gapi_publisherQosCopy (
    const gapi_publisherQos *src,
    gapi_publisherQos *dst);

OS_API gapi_dataReaderQos *
gapi_dataReaderQosCopy (
    const gapi_dataReaderQos *src,
    gapi_dataReaderQos *dst);

OS_API gapi_dataReaderViewQos *
gapi_dataReaderViewQosCopy (
    const gapi_dataReaderViewQos *src,
    gapi_dataReaderViewQos *dst);

OS_API gapi_subscriberQos *
gapi_subscriberQosCopy (
    const gapi_subscriberQos *src,
    gapi_subscriberQos *dst);

/*
 * Qos Merge functions
 */
OS_API gapi_dataWriterQos *
gapi_mergeTopicQosWithDataWriterQos (
    const gapi_topicQos      *srcTopicQos,
    gapi_dataWriterQos       *dstWriterQos);

OS_API gapi_dataReaderQos  *
gapi_mergeTopicQosWithDataReaderQos (
    const gapi_topicQos      *srcTopicQos,
    gapi_dataReaderQos       *dstReaderQos);

gapi_string
gapi_stringSeq_to_String (
    const gapi_stringSeq *sequence,
    const gapi_string     delimiter
    );

gapi_boolean
gapi_string_to_StringSeq (
    const gapi_string  string,
    const gapi_string  delimiter,
    gapi_stringSeq    *sequence);

gapi_boolean
gapi_dataSampleSeq_setLength (
    gapi_dataSampleSeq *seq,
    gapi_unsigned_long  length
    );

gapi_stringSeq *
gapi_stringSeq_dup(
    const gapi_stringSeq * in
    );

typedef struct {
    gapi_unsigned_long  _maximum;
    gapi_unsigned_long  _length;
    v_readerSample     *_buffer;
    gapi_boolean        _release; 
} v_readerSampleSeq;   

v_readerSampleSeq *
v_readerSampleSeq__alloc (
    void);

void
v_readerSampleSeq_free (
    v_readerSampleSeq *seq);

void
v_readerSampleSeq_freebuf (
    v_readerSampleSeq *seq);

v_readerSample *
v_readerSampleSeq_allocbuf (
    gapi_unsigned_long len);

gapi_boolean
v_readerSampleSeq_setLength (
    v_readerSampleSeq  *seq,
    gapi_unsigned_long  length);

#undef OS_API

#endif /* GAPI_STRUCTURED_H */
