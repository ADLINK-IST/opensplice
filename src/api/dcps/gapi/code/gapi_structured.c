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
#include "gapi_common.h"
#include "gapi_structured.h"
#include "gapi_objManag.h"

#include "os_stdlib.h"
#include "os_heap.h"
#include "c_stringSupport.h"
#include "c_iterator.h"

#ifndef NULL
#define NULL 0
#endif

/*
 * Sequence buffer management while copying
 *
 *
 *    +------------------------------+------------------------------+-----------------------------------------+
 *    | SOURCE (src)                 |  DESTINATION (dst)           |                                         |
 *    +----------+---------+---------+----------+---------+---------+-------------------------------------------------+
 *    | _maximum | _length | _buffer | _maximum | _length | _buffer |                                                 |
 *    +----------+---------+---------+----------+---------+---------+-------------------------------------------------+
 *    | 0        | 0       | 0       | 0        | 0       | 0       | Unbounded sequence with length 0 without buffer |
 *    +----------+---------+---------+----------+---------+---------+-------------------------------------------------+
 *    | 0        | 0       | Sb      | 0        | 0       | Db      | Unbounded sequence with length 0 and buffer     |
 *    +----------+---------+---------+----------+---------+---------+-------------------------------------------------+
 *    | 0        | Sl      | 0       | 0        | Dl      | 0       | Invalid combination                             |
 *    +----------+---------+---------+----------+---------+---------+-------------------------------------------------+
 *    | 0        | Sl      | Sb      | 0        | Dl      | Db      | Unbounded sequence with length > 0              |
 *    +----------+---------+---------+----------+---------+---------+-------------------------------------------------+
 *    | Sm       | 0       | 0       | Dm       | 0       | 0       | Bounded sequence with length 0 without buffer   |
 *    +----------+---------+---------+----------+---------+---------+-------------------------------------------------+
 *    | Sm       | 0       | Sb      | Dm       | 0       | Db      | Bounded sequence with length 0 with buffer      |
 *    +----------+---------+---------+----------+---------+---------+-------------------------------------------------+
 *    | Sm       | Sl      | 0       | Dm       | Dl      | 0       | Invalid combination                             |
 *    +----------+---------+---------+----------+---------+---------+-------------------------------------------------+
 *    | Sm       | Sl      | Sb      | Dm       | Dl      | Db      | Bounded sequence with length > 0                |
 *    +----------+---------+---------+----------+---------+---------+-------------------------------------------------+
 *
 * _maximum is not mutable (read only attribute)
 *
 * Destination
 *   Unbounded sequence
 *       if (dst->_length > 0)
 *           free contained entities (if required)
 *           free dst->_buffer
 *           set dst->_buffer to NULL
 *       end-if
 *       set dst->_length to src->_length
 *       if (dst->_length > 0)
 *           allocate dst->_buffer for dst->_length elements
 *           copy dst->_length elements
 *       end-if
 *
 * Bounded sequences are not used for standard type definitons
 *
 *   Bounded sequence
 *       if (dst->_buffer == NULL) {
 *           allocate dst->_buffer for dst->_maximum elements
 *       end-if
 *       if (dst->_length > 0)
 *           free contained entities (if required)
 *       end-if
 *       set dst->_length to MIN (dst->_maximum, src->_length)
 *       if (dst->_length > 0)
 *           copy dst->_length elements
 *       end-if
 */

/*
 * typedef sequence<InstanceHandle_t> InstanceHandleSeq;
 */
static void
instanceHandleSeqCopyin (
    const void *_src,
    gapi_instanceHandleSeq *dst)
{
    const gapi_instanceHandleSeq *src = _src;

    if ( dst->_maximum > 0 ) {
        if ( src->_length > dst->_maximum ) {
            if ( dst->_release ) {
                gapi_free(dst->_buffer);
            }
            dst->_maximum = 0;
            dst->_length  = 0;
            dst->_buffer  = NULL;
        }

    }

    if ( src->_length > 0 ) {
        assert(src->_buffer);

        if ( dst->_length == 0 ) {
            if ( dst->_maximum == 0 ) {
                dst->_buffer  = gapi_instanceHandleSeq_allocbuf(src->_length);
                dst->_maximum = src->_length;
                dst->_length  = 0;
                dst->_release = TRUE;
            }

        }
        if ( dst->_maximum >= src->_length ) {
            memcpy(dst->_buffer, src->_buffer, src->_length);
        }
    }

    dst->_length = src->_length;
}

static void
instanceHandleSeqCopyout (
    const gapi_instanceHandleSeq *src,
    void *_dst)
{
    gapi_instanceHandleSeq *dst = _dst;

    if ( dst->_maximum > 0 ) {
        if ( src->_length > dst->_maximum ) {
            if ( dst->_release ) {
                gapi_free(dst->_buffer);
            }
            dst->_maximum = 0;
            dst->_length  = 0;
            dst->_buffer  = NULL;
        }
    }

    if ( src->_length > 0 ) {
        assert(src->_buffer);

        if ( dst->_length == 0 ) {
            if ( dst->_maximum == 0 ) {
                dst->_buffer  = gapi_instanceHandleSeq_allocbuf(src->_length);
                dst->_maximum = src->_length;
                dst->_length  = 0;
                dst->_release = TRUE;
            }
        }

        if ( dst->_maximum >= src->_length ) {
            memcpy(dst->_buffer, src->_buffer, src->_length);
        }
    }

    dst->_length = src->_length;
}

void (*gapi_instanceHandleSeqCopyin) (const void *src, gapi_instanceHandleSeq *dst) = instanceHandleSeqCopyin;
void (*gapi_instanceHandleSeqCopyout) (const gapi_instanceHandleSeq *src, void *dst) = instanceHandleSeqCopyout;

/*
 * typedef sequence<string> StringSeq;
 */
static void
stringSeqCopyin (
    const void *_src,
    gapi_stringSeq *dst)
{
    const gapi_stringSeq *src = _src;
    gapi_unsigned_long i;

    if ( dst->_maximum > 0 ) {
        if ( src->_length > dst->_maximum ) {
            if ( dst->_release ) {
                gapi_free(dst->_buffer);
            }
            dst->_maximum = 0;
            dst->_length  = 0;
            dst->_buffer  = NULL;
        }
    }

    if ( src->_length > 0 ) {
        assert(src->_buffer);

        if ( dst->_length == 0 ) {
            if ( dst->_maximum == 0 ) {
                dst->_buffer  = gapi_stringSeq_allocbuf (src->_length);
                dst->_maximum = src->_length;
                dst->_length  = 0;
                dst->_release = TRUE;
            }

        }
        if ( dst->_maximum >= src->_length ) {
            for (i = 0; i < src->_length; i++) {
                dst->_buffer[i] = gapi_string_dup(src->_buffer[i]);
            }
        }
    }

    dst->_length = src->_length;
}

static void
stringSeqCopyout (
    const gapi_stringSeq *src,
    void *_dst)
{
    gapi_stringSeq *dst = _dst;
    gapi_unsigned_long i;

    if ( dst->_maximum > 0 ) {
        if ( src->_length > dst->_maximum ) {
            if ( dst->_release ) {
                gapi_free(dst->_buffer);
            }
            dst->_maximum = 0;
            dst->_length  = 0;
            dst->_buffer  = NULL;
        }
    }



    if ( src->_length > 0 ) {
        assert(src->_buffer);

        if ( dst->_length == 0 ) {
            if ( dst->_maximum == 0 ) {
                dst->_buffer  = gapi_stringSeq_allocbuf (src->_length);
                dst->_maximum = src->_length;
                dst->_length  = 0;
                dst->_release = TRUE;
            }

        }
        if ( dst->_maximum >= src->_length ) {
            for (i = 0; i < src->_length; i++) {
                dst->_buffer[i] = gapi_string_dup(src->_buffer[i]);
            }
        }
    }

    dst->_length = src->_length;

}

void (*gapi_stringSeqCopyin) (const void *src, gapi_stringSeq *dst) = stringSeqCopyin;
void (*gapi_stringSeqCopyout) (const gapi_stringSeq *src, void *dst) = stringSeqCopyout;

/*
 * struct Duration_t {
 *     long sec;
 *     unsigned long nanosec;
 * };
 */
static void
duration_tCopyin (
    const void *_src,
    gapi_duration_t *dst)
{
    const gapi_duration_t *src = _src;

    *dst = *src;
}

static void
duration_tCopyout (
    const gapi_duration_t *src,
    void *_dst)
{
    gapi_duration_t *dst = _dst;

    *dst = *src;
}

void (*gapi_duration_tCopyin) (const void *src, gapi_duration_t *dst) = duration_tCopyin;
void (*gapi_duration_tCopyout) (const gapi_duration_t *src, void *dst) = duration_tCopyout;

/*
 * struct Time_t {
 *     long sec;
 *     unsigned long nanosec;
 * };
 */
static void
time_tCopyin (
    const void *_src,
    gapi_time_t *dst)
{
    const gapi_time_t *src = _src;

    *dst = *src;
}

static void
time_tCopyout (
    const gapi_time_t *src,
    void *_dst)
{
    gapi_time_t *dst = _dst;

    *dst = *src;
}

void (*gapi_time_tCopyin) (const void *src, gapi_time_t *dst) = time_tCopyin;
void (*gapi_time_tCopyout) (const gapi_time_t *src, void *dst) = time_tCopyout;

/*
 * struct InconsistentTopicStatus {
 *     long total_count;
 *     long total_count_change;
 * };
 */
static void
inconsistentTopicStatusCopyin (
    const void *_src,
    gapi_inconsistentTopicStatus *dst)
{
    const gapi_inconsistentTopicStatus *src = _src;

    *dst = *src;
}

static void
inconsistentTopicStatusCopyout (
    const gapi_inconsistentTopicStatus *src,
    void *_dst)
{
    gapi_inconsistentTopicStatus *dst = _dst;

    *dst = *src;
}

void (*gapi_inconsistentTopicStatusCopyin) (const void *src, gapi_inconsistentTopicStatus *dst) =
        inconsistentTopicStatusCopyin;
void (*gapi_inconsistentTopicStatusCopyout) (const gapi_inconsistentTopicStatus *src, void *dst) =
        inconsistentTopicStatusCopyout;

/*
 * struct SampleLostStatus {
 *     long total_count;
 *     long total_count_change;
 * };
 */
static void
sampleLostStatusCopyin (
    const void *_src,
    gapi_sampleLostStatus *dst)
{
    const gapi_sampleLostStatus *src = _src;

    *dst = *src;
}

static void
sampleLostStatusCopyout (
    const gapi_sampleLostStatus *src,
    void *_dst)
{
    gapi_sampleLostStatus *dst = _dst;

    *dst = *src;
}

void (*gapi_sampleLostStatusCopyin) (const void *src, gapi_sampleLostStatus *dst) = sampleLostStatusCopyin;
void (*gapi_sampleLostStatusCopyout) (const gapi_sampleLostStatus *src, void *dst) = sampleLostStatusCopyout;

/*
 * struct SampleRejectedStatus {
 *     long total_count;
 *     long total_count_change;
 *     SampleRejectedStatusKind last_reason;
 *     InstanceHandle_t last_instance_handle;
 * };
 */
static void
sampleRejectedStatusCopyin (
    const void *_src,
    gapi_sampleRejectedStatus *dst)
{
    const gapi_sampleRejectedStatus *src = _src;

    *dst = *src;
}

static void
sampleRejectedStatusCopyout (
    const gapi_sampleRejectedStatus *src,
    void *_dst)
{
    gapi_sampleRejectedStatus *dst = _dst;

    *dst = *src;
}

void (*gapi_sampleRejectedStatusCopyin) (const void *src, gapi_sampleRejectedStatus *dst) =
        sampleRejectedStatusCopyin;
void (*gapi_sampleRejectedStatusCopyout) (const gapi_sampleRejectedStatus *src, void *dst) =
        sampleRejectedStatusCopyout;

/*
 * struct LivelinessLostStatus {
 *     long total_count;
 *     long total_count_change;
 * };
 */
static void
livelinessLostStatusCopyin (
    const void *_src,
    gapi_livelinessLostStatus *dst)
{
    const gapi_livelinessLostStatus *src = _src;

    *dst = *src;
}

static void
livelinessLostStatusCopyout (
    const gapi_livelinessLostStatus *src,
    void *_dst)
{
    gapi_livelinessLostStatus *dst = _dst;

    *dst = *src;
}

void (*gapi_livelinessLostStatusCopyin) (const void *src, gapi_livelinessLostStatus *dst) =
        livelinessLostStatusCopyin;
void (*gapi_livelinessLostStatusCopyout) (const gapi_livelinessLostStatus *src, void *dst) =
        livelinessLostStatusCopyout;

/*
 * struct LivelinessChangedStatus {
 *     long active_count;
 *     long inactive_count;
 *     long active_count_change;
 *     long inactive_count_change;
 * };
 */
static void
livelinessChangedStatusCopyin (
    const void *_src,
    gapi_livelinessChangedStatus *dst)
{
    const gapi_livelinessChangedStatus *src = _src;

    *dst = *src;
}

static void
livelinessChangedStatusCopyout (
    const gapi_livelinessChangedStatus *src,
    void *_dst)
{
    gapi_livelinessChangedStatus *dst = _dst;

    *dst = *src;
}

void (*gapi_livelinessChangedStatusCopyin) (const void *src, gapi_livelinessChangedStatus *dst) =
        livelinessChangedStatusCopyin;
void (*gapi_livelinessChangedStatusCopyout) (const gapi_livelinessChangedStatus *src, void *dst) =
        livelinessChangedStatusCopyout;

/*
 * struct OfferedDeadlineMissedStatus {
 *     long total_count;
 *     long total_count_change;
 *     InstanceHandle_t last_instance_handle;
 * };
 */
static void
offeredDeadlineMissedStatusCopyin (
    const void *_src,
    gapi_offeredDeadlineMissedStatus *dst)
{
    const gapi_offeredDeadlineMissedStatus *src = _src;

    *dst = *src;
}

static void
offeredDeadlineMissedStatusCopyout (
    const gapi_offeredDeadlineMissedStatus *src,
    void *_dst)
{
    gapi_offeredDeadlineMissedStatus *dst = _dst;

    *dst = *src;
}

void (*gapi_offeredDeadlineMissedStatusCopyin) (const void *src, gapi_offeredDeadlineMissedStatus *dst) =
        offeredDeadlineMissedStatusCopyin;
void (*gapi_offeredDeadlineMissedStatusCopyout) (const gapi_offeredDeadlineMissedStatus *src, void *dst) =
        offeredDeadlineMissedStatusCopyout;

/*
 * struct RequestedDeadlineMissedStatus {
 *     long total_count;
 *     long total_count_change;
 *     InstanceHandle_t last_instance_handle;
 * };
 */
static void
requestedDeadlineMissedStatusCopyin (
    const void *_src,
    gapi_requestedDeadlineMissedStatus *dst)
{
    const gapi_requestedDeadlineMissedStatus *src = _src;

    *dst = *src;
}

static void
requestedDeadlineMissedStatusCopyout (
    const gapi_requestedDeadlineMissedStatus *src,
    void *_dst)
{
    gapi_requestedDeadlineMissedStatus *dst = _dst;

    *dst = *src;
}

void (*gapi_requestedDeadlineMissedStatusCopyin) (const void *src, gapi_requestedDeadlineMissedStatus *dst) =
        requestedDeadlineMissedStatusCopyin;
void (*gapi_requestedDeadlineMissedStatusCopyout) (const gapi_requestedDeadlineMissedStatus *src, void *dst) =
        requestedDeadlineMissedStatusCopyout;

/*
 * struct QosPolicyCount {
 *     QosPolicyId_t policy_id;
 *     long count;
 * };
 */
static void
qosPolicyCountCopyin (
    const void *_src,
    gapi_qosPolicyCount *dst)
{
    const gapi_qosPolicyCount *src = _src;

    *dst = *src;
}

static void
qosPolicyCountCopyout (
    const gapi_qosPolicyCount *src,
    void *_dst)
{
    gapi_qosPolicyCount *dst = _dst;

    *dst = *src;
}

void (*gapi_qosPolicyCountCopyin) (const void *src, gapi_qosPolicyCount *dst) = qosPolicyCountCopyin;
void (*gapi_qosPolicyCountCopyout) (const gapi_qosPolicyCount *src, void *dst) = qosPolicyCountCopyout;

/*
 * typedef sequence<QosPolicyCount> QosPolicyCountSeq;
 */
static void
qosPolicyCountSeqCopyin (
    const void *_src,
    gapi_qosPolicyCountSeq *dst)
{
    const gapi_qosPolicyCountSeq *src = _src;

    if ( dst->_maximum > 0 ) {
        if ( src->_length > dst->_maximum ) {
            if ( dst->_release ) {
                gapi_free(dst->_buffer);
            }
            dst->_maximum = 0;
            dst->_length  = 0;
            dst->_buffer  = NULL;
        }
    }

    if ( src->_length > 0 ) {
        assert(src->_buffer);

        if ( dst->_length == 0 ) {
            if ( dst->_maximum == 0 ) {
                dst->_buffer  = gapi_qosPolicyCountSeq_allocbuf(src->_length);
                dst->_maximum = src->_length;
                dst->_length  = 0;
                dst->_release = TRUE;
            }

            if ( dst->_maximum >= src->_length ) {
                memcpy(dst->_buffer, src->_buffer, src->_length);
            }
        }
    }

    dst->_length = src->_length;
}

static void
qosPolicyCountSeqCopyout (
    const gapi_qosPolicyCountSeq *src,
    void *_dst)
{
    gapi_qosPolicyCountSeq *dst = _dst;

    if ( dst->_maximum > 0 ) {
        if ( src->_length > dst->_maximum ) {
            if ( dst->_release ) {
                gapi_free(dst->_buffer);
            }
            dst->_maximum = 0;
            dst->_length  = 0;
            dst->_buffer  = NULL;
        }
    }

    if ( src->_length > 0 ) {
        assert(src->_buffer);

        if ( dst->_length == 0 ) {
            if ( dst->_maximum == 0 ) {
                dst->_buffer  = gapi_qosPolicyCountSeq_allocbuf(src->_length);
                dst->_maximum = src->_length;
                dst->_length  = 0;
                dst->_release = TRUE;
            }

            if ( dst->_maximum >= src->_length ) {
                memcpy(dst->_buffer, src->_buffer, src->_length);
            }
        }
    }

    dst->_length = src->_length;
}

void (*gapi_qosPolicyCountSeqCopyin) (const void *src, gapi_qosPolicyCountSeq *dst) =
        qosPolicyCountSeqCopyin;
void (*gapi_qosPolicyCountSeqCopyout) (const gapi_qosPolicyCountSeq *src, void *dst) =
        qosPolicyCountSeqCopyout;

/*
 * struct OfferedIncompatibleQosStatus {
 *     long total_count;
 *     long total_count_change;
 *     QosPolicyId_t last_policy_id;
 *     QosPolicyCountSeq policies;
 * };
 */
static void
offeredIncompatibleQosStatusCopyin (
    const void *_src,
    gapi_offeredIncompatibleQosStatus *dst)
{
    const gapi_offeredIncompatibleQosStatus *src = _src;

    dst->total_count = src->total_count;
    dst->total_count_change = src->total_count_change;
    dst->last_policy_id = src->last_policy_id;
    qosPolicyCountSeqCopyin (&src->policies, &dst->policies);
}

static void
offeredIncompatibleQosStatusCopyout (
    const gapi_offeredIncompatibleQosStatus *src,
    void *_dst)
{
    gapi_offeredIncompatibleQosStatus *dst = _dst;

    dst->total_count = src->total_count;
    dst->total_count_change = src->total_count_change;
    dst->last_policy_id = src->last_policy_id;
    qosPolicyCountSeqCopyout (&src->policies, &dst->policies);
}

void (*gapi_offeredIncompatibleQosStatusCopyin) (const void *src, gapi_offeredIncompatibleQosStatus *dst) =
        offeredIncompatibleQosStatusCopyin;
void (*gapi_offeredIncompatibleQosStatusCopyout) (const gapi_offeredIncompatibleQosStatus *src, void *dst) =
        offeredIncompatibleQosStatusCopyout;

/*
 * struct RequestedIncompatibleQosStatus {
 *     long total_count;
 *     long total_count_change;
 *     QosPolicyId_t last_policy_id;
 *     QosPolicyCountSeq policies;
 * };
 */
static void
requestedIncompatibleQosStatusCopyin (
    const void *_src,
    gapi_requestedIncompatibleQosStatus *dst)
{
    const gapi_requestedIncompatibleQosStatus *src = _src;

    dst->total_count = src->total_count;
    dst->total_count_change = src->total_count_change;
    dst->last_policy_id = src->last_policy_id;
    qosPolicyCountSeqCopyin (&src->policies, &dst->policies);
}

static void
requestedIncompatibleQosStatusCopyout (
    const gapi_requestedIncompatibleQosStatus *src,
    void *_dst)
{
    gapi_requestedIncompatibleQosStatus *dst = _dst;

    dst->total_count = src->total_count;
    dst->total_count_change = src->total_count_change;
    dst->last_policy_id = src->last_policy_id;
    qosPolicyCountSeqCopyout (&src->policies, &dst->policies);
}

void (*gapi_requestedIncompatibleQosStatusCopyin) (const void *src, gapi_requestedIncompatibleQosStatus *dst) =
        requestedIncompatibleQosStatusCopyin;
void (*gapi_requestedIncompatibleQosStatusCopyout) (const gapi_requestedIncompatibleQosStatus *src, void *dst) =
        requestedIncompatibleQosStatusCopyout;

/*
 * struct PublicationMatchStatus {
 *     long total_count;
 *     long total_count_change;
 *     InstanceHandle_t last_subscription_handle;
 * };
 */
static void
publicationMatchStatusCopyin (
    const void *_src,
    gapi_publicationMatchedStatus *dst)
{
    const gapi_publicationMatchedStatus *src = _src;

    *dst = *src;
}

static void
publicationMatchStatusCopyout (
    const gapi_publicationMatchedStatus *src,
    void *_dst)
{
    gapi_publicationMatchedStatus *dst = _dst;

    *dst = *src;
}

void (*gapi_publicationMatchedStatusCopyin) (const void *src, gapi_publicationMatchedStatus *dst) =
        publicationMatchStatusCopyin;
void (*gapi_publicationMatchedStatusCopyout) (const gapi_publicationMatchedStatus *src, void *dst) =
        publicationMatchStatusCopyout;

/*
 * struct SubscriptionMatchStatus {
 *     long total_count;
 *     long total_count_change;
 *     InstanceHandle_t last_publication_handle;
 * };
 */
static void
subscriptionMatchStatusCopyin (
    const void *_src,
    gapi_subscriptionMatchedStatus *dst)
{
    const gapi_subscriptionMatchedStatus *src = _src;

    *dst = *src;
}

static void
subscriptionMatchStatusCopyout (
    const gapi_subscriptionMatchedStatus *src,
    void *_dst)
{
    gapi_subscriptionMatchedStatus *dst = _dst;

    *dst = *src;
}

void (*gapi_subscriptionMatchedStatusCopyin) (const void *src, gapi_subscriptionMatchedStatus *dst) =
        subscriptionMatchStatusCopyin;
void (*gapi_subscriptionMatchedStatusCopyout) (const gapi_subscriptionMatchedStatus *src, void *dst) =
        subscriptionMatchStatusCopyout;

/*
 * typedef sequence<Topic> TopicSeq;
 */

static void
topicSeqCopyin (
    const void *_src,
    gapi_topicSeq *dst)
{
    const gapi_topicSeq *src = _src;

    if ( dst->_maximum > 0 ) {
        if ( src->_length > dst->_maximum ) {
            if ( dst->_release ) {
                gapi_free(dst->_buffer);
            }
            dst->_maximum = 0;
            dst->_length  = 0;
            dst->_buffer  = NULL;
        }
    }

    if ( src->_length > 0 ) {
        if ( dst->_length == 0 ) {
            if ( dst->_maximum == 0 ) {
                dst->_buffer  = gapi_topicSeq_allocbuf(src->_length);
                dst->_maximum = src->_length;
                dst->_length  = 0;
                dst->_release = TRUE;
            }

            if ( dst->_maximum >= src->_length ) {
                memcpy(dst->_buffer, src->_buffer, src->_length);
            }
        }
    }

    dst->_length = src->_length;
}

static void
topicSeqCopyout (
    const gapi_topicSeq *src,
    void *_dst)
{
    gapi_topicSeq *dst = _dst;

    if ( dst->_maximum > 0 ) {
        if ( src->_length > dst->_maximum ) {
            if ( dst->_release ) {
                gapi_free(dst->_buffer);
            }
            dst->_maximum = 0;
            dst->_length  = 0;
            dst->_buffer  = NULL;
        }
    }

    if ( src->_length > 0 ) {
        if ( dst->_length == 0 ) {
            if ( dst->_maximum == 0 ) {
                dst->_buffer  = gapi_topicSeq_allocbuf(src->_length);
                dst->_maximum = src->_length;
                dst->_length  = 0;
                dst->_release = TRUE;
            }

            if ( dst->_maximum >= src->_length ) {
                memcpy(dst->_buffer, src->_buffer, src->_length);
            }
        }
    }

    dst->_length = src->_length;
}

void (*gapi_topicSeqCopyin) (const void *src, gapi_topicSeq *dst) =
        topicSeqCopyin;
void (*gapi_topicSeqCopyout) (const gapi_topicSeq *src, void *dst) =
        topicSeqCopyout;

/*
 * typedef sequence<DataReader> DataReaderSeq;
 */
static void
dataReaderSeqCopyin (
    const void *_src,
    gapi_dataReaderSeq *dst)
{
    const gapi_dataReaderSeq *src = _src;

    if ( dst->_maximum > 0 ) {
        if ( src->_length != dst->_maximum ) {
            if ( dst->_release ) {
                gapi_free(dst->_buffer);
            }
            dst->_maximum = 0;
            dst->_length  = 0;
            dst->_buffer  = NULL;
        }
    }

    if ( src->_length > 0 ) {
        assert(src->_buffer);

        if ( dst->_length == 0 ) {
            if ( dst->_maximum == 0 ) {
                dst->_buffer  = gapi_dataReaderSeq_allocbuf(src->_length);
                dst->_maximum = src->_length;
                dst->_length  = 0;
                dst->_release = TRUE;
            }

            if ( dst->_maximum >= src->_length ) {
                memcpy(dst->_buffer, src->_buffer, src->_length);
            }
        }
    }

    dst->_length = src->_length;
}

static void
dataReaderSeqCopyout (
    const gapi_dataReaderSeq *src,
    void *_dst)
{
    gapi_dataReaderSeq *dst = _dst;

    if ( dst->_maximum > 0 ) {
        if ( src->_length != dst->_maximum ) {
            if ( dst->_release ) {
                gapi_free(dst->_buffer);
            }
            dst->_maximum = 0;
            dst->_length  = 0;
            dst->_buffer  = NULL;
        }
    }

    if ( src->_length > 0 ) {
        assert(src->_buffer);

        if ( dst->_length == 0 ) {
            if ( dst->_maximum == 0 ) {
                dst->_buffer  = gapi_dataReaderSeq_allocbuf(src->_length);
                dst->_maximum = src->_length;
                dst->_length  = 0;
                dst->_release = TRUE;
            }

            if ( dst->_maximum >= src->_length ) {
                memcpy(dst->_buffer, src->_buffer, src->_length);
            }
        }
    }

    dst->_length = src->_length;
}

void (*gapi_dataReaderSeqCopyin) (const void *src, gapi_dataReaderSeq *dst) =
        dataReaderSeqCopyin;
void (*gapi_dataReaderSeqCopyout) (const gapi_dataReaderSeq *src, void *dst) =
        dataReaderSeqCopyout;

/*
 * typedef sequence<DataReaderView> DataReaderViewSeq;
 */
static void
dataReaderViewSeqCopyin (
    const void *_src,
    gapi_dataReaderViewSeq *dst)
{
    const gapi_dataReaderViewSeq *src = _src;

    if ( dst->_maximum > 0 ) {
        if ( src->_length > dst->_maximum ) {
             gapi_free(dst->_buffer);
             dst->_maximum = 0;
             dst->_length  = 0;
             dst->_buffer  = NULL;
        }
    }

    if ( src->_length > 0 ) {
        assert(src->_buffer);

        if ( dst->_length == 0 ) {
            if ( dst->_maximum == 0 ) {
                dst->_buffer  = gapi_dataReaderViewSeq_allocbuf(src->_length);
                dst->_maximum = src->_length;
                dst->_length  = 0;
                dst->_release = TRUE;
            }

            if ( dst->_maximum >= src->_length ) {
                memcpy(dst->_buffer, src->_buffer, src->_length);
            }
        }
    }

    dst->_length = src->_length;
}

static void
dataReaderViewSeqCopyout (
    const gapi_dataReaderViewSeq *src,
    void *_dst)
{
    gapi_dataReaderViewSeq *dst = _dst;

    if ( dst->_maximum > 0 ) {
        if ( src->_length > dst->_maximum ) {
             gapi_free(dst->_buffer);
             dst->_maximum = 0;
             dst->_length  = 0;
             dst->_buffer  = NULL;
        }
    }

    if ( src->_length > 0 ) {
        assert(src->_buffer);

        if ( dst->_length == 0 ) {
            if ( dst->_maximum == 0 ) {
                dst->_buffer  = gapi_dataReaderViewSeq_allocbuf(src->_length);
                dst->_maximum = src->_length;
                dst->_length  = 0;
                dst->_release = TRUE;
            }

            if ( dst->_maximum >= src->_length ) {
                memcpy(dst->_buffer, src->_buffer, src->_length);
            }
        }
    }

    dst->_length = src->_length;
}

void (*gapi_dataReaderViewSeqCopyin) (const void *src, gapi_dataReaderViewSeq *dst) =
        dataReaderViewSeqCopyin;
void (*gapi_dataReaderViewSeqCopyout) (const gapi_dataReaderViewSeq *src, void *dst) =
        dataReaderViewSeqCopyout;

/*
 * typedef sequence<Condition> ConditionSeq;
 */

static void
conditionSeqCopyin (
    const void *_src,
    gapi_conditionSeq *dst)
{
    const gapi_conditionSeq *src = _src;

    if ( dst->_maximum > 0 ) {
        if ( src->_length > dst->_maximum ) {
            if ( dst->_release ) {
                gapi_free(dst->_buffer);
            }
            dst->_maximum = 0;
            dst->_length  = 0;
            dst->_buffer  = NULL;
        }
    }

    if ( src->_length > 0 ) {
        if ( dst->_length == 0 ) {
            if ( dst->_maximum == 0 ) {
                dst->_buffer  = gapi_conditionSeq_allocbuf(src->_length);
                dst->_maximum = src->_length;
                dst->_length  = 0;
                dst->_release = TRUE;
            }

            if ( dst->_maximum >= src->_length ) {
                memcpy(dst->_buffer, src->_buffer, src->_length);
            }
        }
    }

    dst->_length = src->_length;
}

static void
conditionSeqCopyout (
    const gapi_conditionSeq *src,
    void *_dst)
{
    gapi_conditionSeq *dst = _dst;

    if ( dst->_maximum > 0 ) {
        if ( src->_length > dst->_maximum ) {
            if ( dst->_release ) {
                gapi_free(dst->_buffer);
            }
            dst->_maximum = 0;
            dst->_length  = 0;
            dst->_buffer  = NULL;
        }
    }

    if ( src->_length > 0 ) {
        assert(src->_buffer);

        if ( dst->_length == 0 ) {
            if ( dst->_maximum == 0 ) {
                dst->_buffer  = gapi_conditionSeq_allocbuf(src->_length);
                dst->_maximum = src->_length;
                dst->_length  = 0;
                dst->_release = TRUE;
            }

            if ( dst->_maximum >= src->_length ) {
                memcpy(dst->_buffer, src->_buffer, src->_length);
            }
        }
    }

    dst->_length = src->_length;
}

void (*gapi_conditionSeqCopyin) (const void *src, gapi_conditionSeq *dst) =
        conditionSeqCopyin;
void (*gapi_conditionSeqCopyout) (const gapi_conditionSeq *src, void *dst) =
        conditionSeqCopyout;

/*
 * // Sample states to support reads
 * typedef unsigned long SampleStateKind;
 * typedef sequence <SampleStateKind> SampleStateSeq;
 */

static void
sampleStateSeqCopyin (
    const void *_src,
    gapi_sampleStateSeq *dst)
{
    const gapi_sampleStateSeq *src = _src;

    if ( dst->_maximum > 0 ) {
        if ( src->_length > dst->_maximum ) {
            if ( dst->_release ) {
                gapi_free(dst->_buffer);
            }
            dst->_maximum = 0;
            dst->_length  = 0;
            dst->_buffer  = NULL;
        }
    }

    if ( src->_length > 0 ) {
        assert(src->_buffer);

        if ( dst->_length == 0 ) {
            if ( dst->_maximum == 0 ) {
                dst->_buffer  = gapi_sampleStateSeq_allocbuf(src->_length);
                dst->_maximum = src->_length;
                dst->_length  = 0;
                dst->_release = TRUE;
            }

            if ( dst->_maximum >= src->_length ) {
                memcpy(dst->_buffer, src->_buffer, src->_length);
            }
        }
    }

    dst->_length = src->_length;
}

static void
sampleStateSeqCopyout (
    const gapi_sampleStateSeq *src,
    void *_dst)
{
    gapi_sampleStateSeq *dst = _dst;

    if ( dst->_maximum > 0 ) {
        if ( src->_length > dst->_maximum ) {
            if ( dst->_release ) {
                gapi_free(dst->_buffer);
            }
            dst->_maximum = 0;
            dst->_length  = 0;
            dst->_buffer  = NULL;
        }
    }

    if ( src->_length > 0 ) {
        if ( dst->_length == 0 ) {
            if ( dst->_maximum == 0 ) {
                dst->_buffer  = gapi_sampleStateSeq_allocbuf(src->_length);
                dst->_maximum = src->_length;
                dst->_length  = 0;
                dst->_release = TRUE;
            }

            if ( dst->_maximum >= src->_length ) {
                memcpy(dst->_buffer, src->_buffer, src->_length);
            }
        }
    }

    dst->_length = src->_length;
}

void (*gapi_sampleStateSeqCopyin) (const void *src, gapi_sampleStateSeq *dst) = sampleStateSeqCopyin;
void (*gapi_sampleStateSeqCopyout) (const gapi_sampleStateSeq *src, void *dst) = sampleStateSeqCopyout;

/* // Instance states to support reads
 * typedef unsigned long InstanceStateKind;
 * typedef sequence<InstanceStateKind> InstanceStateSeq;
 */

static void
instanceStateSeqCopyin (
    const void *_src,
    gapi_instanceStateSeq *dst)
{
    const gapi_instanceStateSeq *src = _src;

    if ( dst->_maximum > 0 ) {
        if ( src->_length > dst->_maximum ) {
            if ( dst->_release ) {
                gapi_free(dst->_buffer);
            }
            dst->_maximum = 0;
            dst->_length  = 0;
            dst->_buffer  = NULL;
        }
    }

    if ( src->_length > 0 ) {
        if ( dst->_length == 0 ) {
            if ( dst->_maximum == 0 ) {
                dst->_buffer  = gapi_instanceStateSeq_allocbuf(src->_length);
                dst->_maximum = src->_length;
                dst->_length  = 0;
                dst->_release = TRUE;
            }

            if ( dst->_maximum >= src->_length ) {
                memcpy(dst->_buffer, src->_buffer, src->_length);
            }
        }
    }

    dst->_length = src->_length;
}

static void
instanceStateSeqCopyout (
    const gapi_instanceStateSeq *src,
    void *_dst)
{
    gapi_instanceStateSeq *dst = _dst;

    if ( dst->_maximum > 0 ) {
        if ( src->_length > dst->_maximum ) {
            if ( dst->_release ) {
                 gapi_free(dst->_buffer);
            }
            dst->_maximum = 0;
            dst->_length  = 0;
            dst->_buffer  = NULL;
        }
    }

    if ( src->_length > 0 ) {
        if ( dst->_length == 0 ) {
            if ( dst->_maximum == 0 ) {
                dst->_buffer  = gapi_instanceStateSeq_allocbuf(src->_length);
                dst->_maximum = src->_length;
                dst->_length  = 0;
                dst->_release = TRUE;
            }

            if ( dst->_maximum >= src->_length ) {
                memcpy(dst->_buffer, src->_buffer, src->_length);
            }
        }
    }

    dst->_length = src->_length;
}

void (*gapi_instanceStateSeqCopyin) (const void *src, gapi_instanceStateSeq *dst) = instanceStateSeqCopyin;
void (*gapi_instanceStateSeqCopyout) (const gapi_instanceStateSeq *src, void *dst) = instanceStateSeqCopyout;

/*
 * struct UserDataQosPolicy {
 *     sequence<octet> value;
 * };
 */

static void
userDataQosPolicyCopy (
    const gapi_userDataQosPolicy *src,
    gapi_userDataQosPolicy *dst)
{
    if ( dst->value._maximum > 0 ) {
        if ( src->value._length > dst->value._maximum ) {
            if ( dst->value._release ) {
                gapi_free(dst->value._buffer);
            }
            dst->value._maximum = 0;
            dst->value._length  = 0;
            dst->value._buffer  = NULL;
        }
    }

    if ( src->value._length > 0 ) {
        if ( dst->value._length == 0 ) {
            if ( dst->value._maximum == 0 ) {
                dst->value._buffer  = gapi_octetSeq_allocbuf(src->value._length);
                dst->value._maximum = src->value._length;
                dst->value._length  = 0;
                dst->value._release = TRUE;
            }

            if ( dst->value._maximum >= src->value._length ) {
                memcpy(dst->value._buffer, src->value._buffer, src->value._length);
                dst->value._length = src->value._length;
            }
        }
    }

    dst->value._length = src->value._length;
}


/*
 * struct TopicDataQosPolicy {
 *     sequence<octet> value;
 * };
 */
static void
topicDataQosPolicyCopy (
    const gapi_topicDataQosPolicy *src,
    gapi_topicDataQosPolicy *dst)
{
    if ( dst->value._maximum > 0 ) {
        if ( src->value._length > dst->value._maximum ) {
            if ( dst->value._release ) {
                gapi_free(dst->value._buffer);
            }
            dst->value._maximum = 0;
            dst->value._length  = 0;
            dst->value._buffer  = NULL;
        }
    }

    if ( src->value._length > 0 ) {
        if ( dst->value._length == 0 ) {
            if ( dst->value._maximum == 0 ) {
                dst->value._buffer  = gapi_octetSeq_allocbuf(src->value._length);
                dst->value._maximum = src->value._length;
                dst->value._length  = 0;
                dst->value._release = TRUE;
            }

            if ( dst->value._maximum >= src->value._length ) {
                memcpy(dst->value._buffer, src->value._buffer, src->value._length);
            }
        }
    }

    dst->value._length = src->value._length;
}

/* struct PartitionQosPolicy {
 *     StringSeq name;
 * };
 */
static void
partitionQosPolicyCopy (
    const gapi_partitionQosPolicy *src,
    gapi_partitionQosPolicy *dst)
{
    stringSeqCopyin (&src->name, &dst->name);
}


/* struct GroupDataQosPolicy {
 *     sequence<octet> value;
 * };
 */
static void
groupDataQosPolicyCopy (
    const gapi_groupDataQosPolicy *src,
    gapi_groupDataQosPolicy *dst)
{
    if ( dst->value._maximum > 0 ) {
        if ( src->value._length > dst->value._maximum ) {
            if ( dst->value._release ) {
                gapi_free(dst->value._buffer);
            }
            dst->value._maximum = 0;
            dst->value._length  = 0;
            dst->value._buffer  = NULL;
        }
    }

    if ( src->value._length > 0 ) {
        if ( dst->value._length == 0 ) {
            if ( dst->value._maximum == 0 ) {
                dst->value._buffer  = gapi_octetSeq_allocbuf(src->value._length);
                dst->value._maximum = src->value._length;
                dst->value._length  = 0;
                dst->value._release = TRUE;
            }

            if ( dst->value._maximum >= src->value._length ) {
                memcpy(dst->value._buffer, src->value._buffer, src->value._length);
            }
        }
    }

    dst->value._length = src->value._length;
}


/* struct SubscriptionKeyQosPolicy {
 *     boolean   use_key_list;
 *     StringSeq key_list;
 * };
 */
static void
subscriptionKeyQosPolicyCopy (
    const gapi_subscriptionKeyQosPolicy *src,
    gapi_subscriptionKeyQosPolicy *dst)
{
    dst->use_key_list = src->use_key_list;
    stringSeqCopyin (&src->key_list, &dst->key_list);
}

gapi_boolean
subscriptionKeyQosPolicyEqual (
    const gapi_subscriptionKeyQosPolicy *orig,
    const gapi_subscriptionKeyQosPolicy *req)
{
    gapi_boolean equal = TRUE;

    if ( orig->use_key_list != req->use_key_list ) {
        equal = FALSE;
    }

    if ( equal && orig->use_key_list ) {
        if ( orig->key_list._length != req->key_list._length ) {
            equal = FALSE;
        } else {
            gapi_unsigned_long i;
            for ( i = 0; equal && (i < orig->key_list._length); i++ ) {
                if ( (orig->key_list._buffer[i] == NULL) && (req->key_list._buffer[i] != NULL) ) {
                    equal = FALSE;
                } else if ( (orig->key_list._buffer[i] != NULL) && (req->key_list._buffer[i] == NULL) ) {
                    equal = FALSE;
                } else {
                    if ( strcmp(orig->key_list._buffer[i], req->key_list._buffer[i]) != 0 ) {
                        equal = FALSE;
                    }
                }
            }
        }
    }

    return equal;
}



/* struct ViewKeyQosPolicy {
 *     boolean   use_key_list;
 *     StringSeq key_list;
 * };
 */
static void
viewKeyQosPolicyCopy (
    const gapi_viewKeyQosPolicy *src,
    gapi_viewKeyQosPolicy *dst)
{
    dst->use_key_list = src->use_key_list;
    stringSeqCopyin (&src->key_list, &dst->key_list);
}

gapi_boolean
viewKeyQosPolicyEqual (
    const gapi_viewKeyQosPolicy *orig,
    const gapi_viewKeyQosPolicy *req)
{
    gapi_boolean equal = TRUE;

    if ( orig->use_key_list != req->use_key_list ) {
        equal = FALSE;
    }

    if ( equal && orig->use_key_list ) {
        if ( orig->key_list._length != req->key_list._length ) {
            equal = FALSE;
        } else {
            gapi_unsigned_long i;
            for ( i = 0; equal && (i < orig->key_list._length); i++ ) {
                if ( (orig->key_list._buffer[i] == NULL) && (req->key_list._buffer[i] != NULL) ) {
                    equal = FALSE;
                } else if ( (orig->key_list._buffer[i] != NULL) && (req->key_list._buffer[i] == NULL) ) {
                    equal = FALSE;
                } else {
                    if ( strcmp(orig->key_list._buffer[i], req->key_list._buffer[i]) != 0 ) {
                        equal = FALSE;
                    }
                }
            }
        }
    }

    return equal;
}

/* struct ShareQosPolicy {
 *     string   name;
 *     boolean  enable;
 * };
 */
static void
shareQosPolicyCopy (
    const gapi_shareQosPolicy *src,
    gapi_shareQosPolicy *dst)
{
    dst->enable = src->enable;
    gapi_string_replace(src->name, &dst->name);
}


/* struct DomainParticipantQos {
 *     UserDataQosPolicy user_data;
 *     EntityFactoryQosPolicy entity_factory;
 * };
 */
gapi_domainParticipantQos *
gapi_domainParticipantQosCopy (
    const gapi_domainParticipantQos *src,
    gapi_domainParticipantQos *dst)
{
    userDataQosPolicyCopy (&src->user_data, &dst->user_data);

    dst->entity_factory      = src->entity_factory;
    dst->watchdog_scheduling = src->watchdog_scheduling;
    dst->listener_scheduling = src->listener_scheduling;

    return dst;
}

/*
 * struct TopicQos {
 *     TopicDataQosPolicy topic_data;
 *     DurabilityQosPolicy durability;
 *     DeadlineQosPolicy deadline;
 *     LatencyBudgetQosPolicy latency_budget;
 *     LivelinessQosPolicy liveliness;
 *     ReliabilityQosPolicy reliability;
 *     DestinationOrderQosPolicy destination_order;
 *     HistoryQosPolicy history;
 *     ResourceLimitsQosPolicy resource_limits;
 *     TransportPriorityQosPolicy transport_priority;
 *     LifespanQosPolicy lifespan;
 *     OwnershipQosPolicy ownership;
 * };
 */
gapi_topicQos *
gapi_topicQosCopy (
    const gapi_topicQos *src,
    gapi_topicQos *dst)
{
    topicDataQosPolicyCopy (&src->topic_data, &dst->topic_data);

    dst->durability         = src->durability;
    dst->durability_service = src->durability_service;
    dst->deadline           = src->deadline;
    dst->latency_budget     = src->latency_budget;
    dst->liveliness         = src->liveliness;
    dst->reliability        = src->reliability;
    dst->destination_order  = src->destination_order;
    dst->history            = src->history;
    dst->resource_limits    = src->resource_limits;
    dst->transport_priority = src->transport_priority;
    dst->lifespan           = src->lifespan;
    dst->ownership          = src->ownership;

    return dst;
}

/*
 * struct DataWriterQos {
 *     DurabilityQosPolicy durability;
 *     DeadlineQosPolicy deadline;
 *     LatencyBudgetQosPolicy latency_budget;
 *     LivelinessQosPolicy liveliness;
 *     ReliabilityQosPolicy reliability;
 *     DestinationOrderQosPolicy destination_order;
 *     HistoryQosPolicy history;
 *     ResourceLimitsQosPolicy resource_limits;
 *     TransportPriorityQosPolicy transport_priority;
 *     LifespanQosPolicy lifespan;
 *     UserDataQosPolicy user_data;
 *     OwnershipQosPolicy ownership;
 *     OwnershipStrengthQosPolicy ownership_strength;
 *     WriterDataLifecycleQosPolicy writer_data_lifecycle;
 * };
 */
gapi_dataWriterQos *
gapi_dataWriterQosCopy (
    const gapi_dataWriterQos *src,
    gapi_dataWriterQos *dst)
{
   userDataQosPolicyCopy (&src->user_data, &dst->user_data);

   dst->durability            = src->durability;
   dst->deadline              = src->deadline;
   dst->latency_budget        = src->latency_budget;
   dst->liveliness            = src->liveliness;
   dst->reliability           = src->reliability;
   dst->destination_order     = src->destination_order;
   dst->history               = src->history;
   dst->resource_limits       = src->resource_limits;
   dst->transport_priority    = src->transport_priority;
   dst->lifespan              = src->lifespan;
   dst->ownership             = src->ownership;
   dst->ownership_strength    = src->ownership_strength;
   dst->writer_data_lifecycle = src->writer_data_lifecycle;

    return dst;
}

/*
 * struct PublisherQos {
 *     PresentationQosPolicy presentation;
 *     PartitionQosPolicy partition;
 *     GroupDataQosPolicy group_data;
 *     EntityFactoryQosPolicy entity_factory;
 * };
 */
gapi_publisherQos *
gapi_publisherQosCopy (
    const gapi_publisherQos *src,
    gapi_publisherQos *dst)
{
    groupDataQosPolicyCopy (&src->group_data, &dst->group_data);
    partitionQosPolicyCopy (&src->partition, &dst->partition);

    dst->presentation   = src->presentation;
    dst->entity_factory = src->entity_factory;

    return dst;
}


/*
 * struct DataReaderQos {
 *     DurabilityQosPolicy durability;
 *     DeadlineQosPolicy deadline;
 *     LatencyBudgetQosPolicy latency_budget;
 *     LivelinessQosPolicy liveliness;
 *     ReliabilityQosPolicy reliability;
 *     DestinationOrderQosPolicy destination_order;
 *     HistoryQosPolicy history;
 *     ResourceLimitsQosPolicy resource_limits;
 *     UserDataQosPolicy user_data;
 *     OwnershipQosPolicy ownership;
 *     TimeBasedFilterQosPolicy time_based_filter;
 *     ReaderDataLifecycleQosPolicy reader_data_lifecycle;
 * };
 */
gapi_dataReaderQos *
gapi_dataReaderQosCopy (
    const gapi_dataReaderQos *src,
    gapi_dataReaderQos *dst)
{
    userDataQosPolicyCopy (&src->user_data, &dst->user_data);
    subscriptionKeyQosPolicyCopy (&src->subscription_keys, &dst->subscription_keys);
    shareQosPolicyCopy (&src->share, &dst->share);

    dst->durability            = src->durability ;
    dst->deadline              = src->deadline;
    dst->latency_budget        = src->latency_budget;
    dst->liveliness            = src->liveliness;
    dst->reliability           = src->reliability;
    dst->destination_order     = src->destination_order;
    dst->history               = src->history;
    dst->resource_limits       = src->resource_limits;
    dst->ownership             = src->ownership;
    dst->time_based_filter     = src->time_based_filter;
    dst->reader_data_lifecycle = src->reader_data_lifecycle;
    dst->reader_lifespan       = src->reader_lifespan;

    return dst;
}

/*
 * struct DataReaderViewQos {
 *     SubscriptionKeyQosPolicy subscription_keys;
 * };
 */
gapi_dataReaderViewQos *
gapi_dataReaderViewQosCopy (
    const gapi_dataReaderViewQos *src,
    gapi_dataReaderViewQos *dst)
{
    viewKeyQosPolicyCopy (&src->view_keys, &dst->view_keys);

    return dst;
}

/*
 * struct SubscriberQos {
 *     PresentationQosPolicy presentation;
 *     PartitionQosPolicy partition;
 *     GroupDataQosPolicy group_data;
 *     EntityFactoryQosPolicy entity_factory;
 * };
 */
gapi_subscriberQos *
gapi_subscriberQosCopy (
    const gapi_subscriberQos *src,
    gapi_subscriberQos *dst)
{
    partitionQosPolicyCopy (&src->partition, &dst->partition);
    groupDataQosPolicyCopy (&src->group_data, &dst->group_data);
    shareQosPolicyCopy (&src->share, &dst->share);

    dst->presentation   = src->presentation;
    dst->entity_factory = src->entity_factory;

    return dst;
}

gapi_dataWriterQos *
gapi_mergeTopicQosWithDataWriterQos (
    const gapi_topicQos      *srcTopicQos,
    gapi_dataWriterQos       *dstWriterQos)
{
    dstWriterQos->durability         = srcTopicQos->durability;
    dstWriterQos->deadline           = srcTopicQos->deadline;
    dstWriterQos->latency_budget     = srcTopicQos->latency_budget;
    dstWriterQos->liveliness         = srcTopicQos->liveliness;
    dstWriterQos->reliability        = srcTopicQos->reliability;
    dstWriterQos->destination_order  = srcTopicQos->destination_order;
    dstWriterQos->history            = srcTopicQos->history;
    dstWriterQos->resource_limits    = srcTopicQos->resource_limits;
    dstWriterQos->ownership          = srcTopicQos->ownership;
    dstWriterQos->transport_priority = srcTopicQos->transport_priority;
    dstWriterQos->lifespan           = srcTopicQos->lifespan;

    return dstWriterQos;
}

gapi_dataReaderQos *
gapi_mergeTopicQosWithDataReaderQos (
    const gapi_topicQos      *srcTopicQos,
    gapi_dataReaderQos       *dstReaderQos)
{
    dstReaderQos->durability         = srcTopicQos->durability;
    dstReaderQos->deadline           = srcTopicQos->deadline;
    dstReaderQos->latency_budget     = srcTopicQos->latency_budget;
    dstReaderQos->liveliness         = srcTopicQos->liveliness;
    dstReaderQos->reliability        = srcTopicQos->reliability;
    dstReaderQos->destination_order  = srcTopicQos->destination_order;
    dstReaderQos->history            = srcTopicQos->history;
    dstReaderQos->ownership          = srcTopicQos->ownership;
    dstReaderQos->resource_limits    = srcTopicQos->resource_limits;

    return dstReaderQos;
}



/*
 * struct ParticipantBuiltinTopicData {
 *     BuiltinTopicKey_t key;
 *     UserDataQosPolicy user_data;
 * };
 */
static gapi_boolean
participantBuiltinTopicDataCopy (
    const gapi_participantBuiltinTopicData *src,
    gapi_participantBuiltinTopicData *dst)
{
    dst->key[0] = src->key[0];
    dst->key[1] = src->key[1];
    dst->key[2] = src->key[2];
    userDataQosPolicyCopy (&src->user_data, &dst->user_data);

    return TRUE;
}


/*
 * struct TopicBuiltinTopicData {
 *     BuiltinTopicKey_t key;
 *     string name;
 *     string type_name;
 *     DurabilityQosPolicy durability;
 *     DeadlineQosPolicy deadline;
 *     LatencyBudgetQosPolicy latency_budget;
 *     LivelinessQosPolicy liveliness;
 *     ReliabilityQosPolicy reliability;
 *     TransportPriorityQosPolicy transport_priority;
 *     LifespanQosPolicy lifespan;
 *     DestinationOrderQosPolicy destination_order;
 *     HistoryQosPolicy history;
 *     ResourceLimitsQosPolicy resource_limits;
 *     OwnershipQosPolicy ownership;
 *     TopicDataQosPolicy topic_data;
 * };
 */
static gapi_boolean
topicBuiltinTopicDataCopy (
    const gapi_topicBuiltinTopicData *src,
    gapi_topicBuiltinTopicData *dst)
{
    dst->key[0] = src->key[0];
    dst->key[1] = src->key[1];
    dst->key[2] = src->key[2];
    dst->name = gapi_string_dup (src->name);
    dst->type_name = gapi_string_dup (src->type_name);

    topicDataQosPolicyCopy (&src->topic_data, &dst->topic_data);

    dst->durability         = src->durability;
    dst->deadline           = src->deadline;
    dst->latency_budget     = src->latency_budget;
    dst->liveliness         = src->liveliness;
    dst->reliability        = src->reliability;
    dst->destination_order  = src->destination_order;
    dst->history            = src->history;
    dst->resource_limits    = src->resource_limits;
    dst->transport_priority = src->transport_priority;
    dst->lifespan           = src->lifespan;
    dst->ownership          = src->ownership;

    return TRUE;
}

/*
 * struct PublicationBuiltinTopicData {
 *     BuiltinTopicKey_t key;
 *     BuiltinTopicKey_t participant_key;
 *     string topic_name;
 *     string type_name;
 *     DurabilityQosPolicy durability;
 *     DeadlineQosPolicy deadline;
 *     LatencyBudgetQosPolicy latency_budget;
 *     LivelinessQosPolicy liveliness;
 *     ReliabilityQosPolicy reliability;
 *     LifespanQosPolicy lifespan;
 *     UserDataQosPolicy user_data;
 *     OwnershipStrengthQosPolicy ownership_strength;
 *     PresentationQosPolicy presentation;
 *     PartitionQosPolicy partition;
 *     TopicDataQosPolicy topic_data;
 *     GroupDataQosPolicy group_data;
 * };
 */
static gapi_boolean
publicationBuiltinTopicDataCopy (
    const gapi_publicationBuiltinTopicData *src,
    gapi_publicationBuiltinTopicData *dst)
{
    dst->key[0] = src->key[0];
    dst->key[1] = src->key[1];
    dst->key[2] = src->key[2];
    dst->participant_key[0] = src->participant_key[0];
    dst->participant_key[1] = src->participant_key[1];
    dst->participant_key[2] = src->participant_key[2];
    dst->topic_name = gapi_string_dup (src->topic_name);
    dst->type_name = gapi_string_dup (src->type_name);

    partitionQosPolicyCopy (&src->partition, &dst->partition);
    topicDataQosPolicyCopy (&src->topic_data, &dst->topic_data);
    groupDataQosPolicyCopy (&src->group_data, &dst->group_data);
    userDataQosPolicyCopy (&src->user_data, &dst->user_data);

    dst->durability         = src->durability;
    dst->deadline           = src->deadline;
    dst->latency_budget     = src->latency_budget;
    dst->liveliness         = src->liveliness;
    dst->reliability        = src->reliability;
    dst->presentation       = src->presentation;
    dst->lifespan           = src->lifespan;
    dst->ownership          = src->ownership;
    dst->ownership_strength = src->ownership_strength;

    return TRUE;
}


/*
 * struct SubscriptionBuiltinTopicData {
 *     BuiltinTopicKey_t key;
 *     BuiltinTopicKey_t participant_key;
 *     string topic_name;
 *     string type_name;
 *     DurabilityQosPolicy durability;
 *     DeadlineQosPolicy deadline;
 *     LatencyBudgetQosPolicy latency_budget;
 *     LivelinessQosPolicy liveliness;
 *     ReliabilityQosPolicy reliability;
 *     DestinationOrderQosPolicy destination_order;
 *     UserDataQosPolicy user_data;
 *     TimeBasedFilterQosPolicy time_based_filter;
 *     PresentationQosPolicy presentation;
 *     PartitionQosPolicy partition;
 *     TopicDataQosPolicy topic_data;
 *     GroupDataQosPolicy group_data;
 * };
 */
static gapi_boolean
subscriptionBuiltinTopicDataCopy (
    const gapi_subscriptionBuiltinTopicData *src,
    gapi_subscriptionBuiltinTopicData *dst)
{
    dst->key[0] = src->key[0];
    dst->key[1] = src->key[1];
    dst->key[2] = src->key[2];
    dst->participant_key[0] = src->participant_key[0];
    dst->participant_key[1] = src->participant_key[1];
    dst->participant_key[2] = src->participant_key[2];
    dst->topic_name = gapi_string_dup (src->topic_name);
    dst->type_name = gapi_string_dup (src->type_name);

    partitionQosPolicyCopy (&src->partition, &dst->partition);
    topicDataQosPolicyCopy (&src->topic_data, &dst->topic_data);
    groupDataQosPolicyCopy (&src->group_data, &dst->group_data);
    userDataQosPolicyCopy (&src->user_data, &dst->user_data);

    dst->durability         = src->durability;
    dst->deadline           = src->deadline;
    dst->latency_budget     = src->latency_budget;
    dst->liveliness         = src->liveliness;
    dst->reliability        = src->reliability;
    dst->ownership          = src->ownership;
    dst->presentation       = src->presentation;
    dst->destination_order  = src->destination_order;
    dst->time_based_filter  = src->time_based_filter;

    return TRUE;
}

/*
 * struct SampleInfo {
 *     SampleStateKind sample_state;
 *     ViewStateKind view_state;
 *     InstanceStateKind instance_state;
 *     boolean valid_data;
 *     Time_t source_timestamp;
 *     InstanceHandle_t instance_handle;
 *     long disposed_generation_count;
 *     long no_writers_generation_count;
 *     long sample_rank;
 *     long generation_rank;
 *     long absolute_generation_rank;
 * };
 */
static void
sampleInfoCopyin (
    const void *_src,
    gapi_sampleInfo *dst)
{
    const gapi_sampleInfo *src = _src;

    *dst = *src;
}

static void
sampleInfoCopyout (
    const gapi_sampleInfo *src,
    void *_dst)
{
    gapi_sampleInfo *dst = _dst;

    *dst = *src;
}

void
(*gapi_sampleInfoCopyin) (
    const void *src,
    gapi_sampleInfo *dst) =
        sampleInfoCopyin;

void
(*gapi_sampleInfoCopyout) (
    const gapi_sampleInfo *src,
    void *dst) =
        sampleInfoCopyout;

/*
 * typedef sequence<SampleInfo> SampleInfoSeq;
 */

static void
sampleInfoSeqCopyin (
    const void *_src,
    gapi_sampleInfoSeq *dst)
{
    const gapi_sampleInfoSeq *src = _src;

    if ( dst->_maximum > 0 ) {
        if ( src->_length != dst->_maximum ) {
             gapi_free(dst->_buffer);
             dst->_maximum = 0;
             dst->_length  = 0;
             dst->_buffer  = NULL;
        }
    }

    if ( src->_length > 0 ) {
        if ( dst->_length == 0 ) {
            if ( dst->_maximum == 0 ) {
                dst->_buffer  = gapi_sampleInfoSeq_allocbuf(src->_length);
                dst->_maximum = src->_length;
                dst->_length  = 0;
                dst->_release = TRUE;
            }

            if ( dst->_maximum >= src->_length ) {
                memcpy(dst->_buffer, src->_buffer, src->_length);
            }
        }
    }

    dst->_length = src->_length;
}

static void
sampleInfoSeqCopyout (
    const gapi_sampleInfoSeq *src,
    void *_dst)
{
    gapi_sampleInfoSeq *dst = _dst;

    if ( dst->_maximum > 0 ) {
        if ( src->_length != dst->_maximum ) {
             gapi_free(dst->_buffer);
             dst->_maximum = 0;
             dst->_length  = 0;
             dst->_buffer  = NULL;
        }
    }

    if ( src->_length > 0 ) {
        if ( dst->_length == 0 ) {
            if ( dst->_maximum == 0 ) {
                dst->_buffer  = gapi_sampleInfoSeq_allocbuf(src->_length);
                dst->_maximum = src->_length;
                dst->_length  = 0;
                dst->_release = TRUE;
            }

            if ( dst->_maximum >= src->_length ) {
                memcpy(dst->_buffer, src->_buffer, src->_length);
            }
        }
    }

    dst->_length = src->_length;
}

void
(*gapi_sampleInfoSeqCopyin) (
    const void *src,
    gapi_sampleInfoSeq *dst) =
        sampleInfoSeqCopyin;

void
(*gapi_sampleInfoSeqCopyout) (
    const gapi_sampleInfoSeq *src,
    void *dst) =
        sampleInfoSeqCopyout;

gapi_string
gapi_stringSeq_to_String (
    const gapi_stringSeq *sequence,
    const gapi_string     delimiter)
{
    unsigned long i;
    unsigned long size = 0;
    gapi_string   rstring = NULL;

    assert(sequence != NULL);
    assert(delimiter != NULL);

    for ( i = 0; i < sequence->_length; i++ ) {
        size += strlen(sequence->_buffer[i]);
    }

    if ( size > 0 ) {
        size += (sequence->_length * strlen(delimiter)) + 1;
        rstring = (gapi_string) os_malloc(size);

        if ( rstring != NULL ) {
            rstring[0] = '\0';
            for ( i = 0; i < sequence->_length; i++ ) {
                if ( sequence->_buffer[i] ) {
                    if ( i != 0 ) {
                        os_strcat(rstring, delimiter);
                    }
                    os_strcat(rstring, sequence->_buffer[i]);
                }
            }
        }
    } else {
        rstring = (gapi_string)os_malloc(1);
        rstring[0] = '\0';
    }

    return rstring;
}

gapi_boolean
gapi_string_to_StringSeq (
    const gapi_string  string,
    const gapi_string  delimiter,
    gapi_stringSeq    *sequence)
{
    gapi_boolean result = TRUE;
    unsigned long size  = 0UL;

    assert(delimiter);
    assert(sequence);

    if ( string ) {
        c_iter iter = c_splitString(string, delimiter);
        if ( iter ) {
            unsigned long size = c_iterLength(iter);
            if ( gapi_stringSeq_set_length(sequence, size) ) {
                unsigned long i;

                for ( i = 0UL; i < size; i++ ) {
                    gapi_string s = (gapi_string)c_iterTakeFirst(iter);
                    gapi_string_replace(s, &sequence->_buffer[i]);
                    os_free(s);
                    if ( !sequence->_buffer[i] ) {
                        result = FALSE;
                    }
                }
            }
            c_iterFree(iter);
        } else {
            result = FALSE;
        }
    } else {
        result = gapi_stringSeq_set_length(sequence, size);
    }

    return result;
}

gapi_boolean
gapi_dataSampleSeq_setLength (
    gapi_dataSampleSeq  *seq,
    gapi_unsigned_long   length
    )
{
    gapi_boolean result = TRUE;

    assert(seq);

    if ( length > seq->_maximum ) {
        gapi_dataSample  *newBuffer;
        gapi_unsigned_long size;

        size = seq->_maximum + GAPI_DATASAMPLESEQ_INCREMENT;
        newBuffer = gapi_dataSampleSeq_allocbuf(size);
        if ( newBuffer ) {
            memcpy(newBuffer, seq->_buffer, seq->_length * sizeof(gapi_dataSample));
            if ( seq->_release ) {
                gapi_free(seq->_buffer);
            }
            seq->_buffer  = newBuffer;
            seq->_maximum = size;
            seq->_length  = length;
        } else {
            result = FALSE;
        }
    } else {
        seq->_length = length;
    }

    return result;
}

gapi_stringSeq *
gapi_stringSeq_dup (
    const gapi_stringSeq * in)
{
    gapi_stringSeq *dup;

    dup = gapi_stringSeq__alloc();

    if ( dup != NULL ) {
        gapi_stringSeqCopyout(in, dup);
    }

    return dup;
}

v_readerSampleSeq *
v_readerSampleSeq__alloc (
    void)
{
    return (v_readerSampleSeq *)gapi_sequence_malloc ();
}

void
v_readerSampleSeq_free (
    v_readerSampleSeq *seq)
{
    if ( seq->_release && seq->_buffer ) {
        gapi_free(seq->_buffer);
        seq->_buffer  = NULL;
        seq->_maximum = 0UL;
    }
    gapi_free(seq);
}


void
v_readerSampleSeq_freebuf (
    v_readerSampleSeq *seq)
{
    if ( seq->_release && seq->_buffer ) {
        gapi_free(seq->_buffer);
    }
}

v_readerSample *
v_readerSampleSeq_allocbuf (
    gapi_unsigned_long len)
{
    return (v_readerSample *)gapi_sequence_allocbuf(NULL,
                                                    sizeof(v_readerSample),
                                                    len);
}

gapi_boolean
v_readerSampleSeq_setLength (
    v_readerSampleSeq  *seq,
    gapi_unsigned_long  length)
{
    gapi_boolean result = TRUE;

    assert(seq);

    if ( length > seq->_maximum ) {
        v_readerSample *newBuffer;
        gapi_unsigned_long size;

        size = seq->_maximum + V_DATAREADERSAMPLESEQ_INCREMENT;
        newBuffer = v_readerSampleSeq_allocbuf(size);
        if ( newBuffer ) {
            memcpy(newBuffer, seq->_buffer, seq->_length * sizeof(v_readerSample));
            if ( seq->_release ) {
                gapi_free(seq->_buffer);
            }
            seq->_buffer  = newBuffer;
            seq->_maximum = size;
            seq->_length  = length;
            seq->_release = TRUE;
        } else {
            result = FALSE;
        }
    } else {
        seq->_length = length;
    }

    return result;
}

