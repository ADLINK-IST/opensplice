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

#include "os_heap.h"
#include "os_abstract.h"
#include <string.h>

#include "gapi.h"

#include "dds_dcps.h"
#include "dds_dcps_private.h"

void *
DDS__malloc (
    DDS_boolean (*ff)(void *),
    DDS_unsigned_long hl,
    DDS_unsigned_long len)
{
    return
        gapi__malloc (
            ff,
            (gapi_unsigned_long) hl,
            (gapi_unsigned_long) len
    );
}

void
DDS__free (
    void *object)
{
    gapi__free (object);
}

void *
DDS__header (
    void *object)
{
    return gapi__header (object);
}


void *
DDS_alloc (
    DDS_unsigned_long l
    )
{
    return gapi_alloc(
        (gapi_unsigned_long)l
    );
}

void
DDS_free (
    void *a
    )
{
    gapi_free(a);
}

DDS_char *
DDS_string_alloc (
    DDS_unsigned_long len
    )
{
    return (DDS_char *)
        gapi_string_alloc (
            (gapi_unsigned_long) len
    );

}

DDS_char *
DDS_string_dup (
    const DDS_char *src
    )
{
    return (DDS_char *)
        gapi_string_dup (
            (gapi_char *)src
    );
}

void
DDS_string_clean (
    DDS_char **string
    )
{
    gapi_string_clean (
        (gapi_char **)string
    );
}

void
DDS_string_replace (
    DDS_char *src,
    DDS_char **dst
    )
{
    gapi_string_replace (
        (gapi_char *)src,
        (gapi_char **)dst
    );
}

DDS_boolean
DDS_sequence_free (
    void *sequence
    )
{
    return gapi_sequence_free ( sequence );
}

void
DDS_sequence_clean (
    void *sequence
    )
{
    gapi_sequence_clean ( sequence );
}

void *
DDS_sequence_malloc (
    void
    )
{
    return gapi_sequence_malloc ();
}

void *
DDS_sequence_allocbuf (
    DDS_boolean (*ff)(void *),
    DDS_unsigned_long len,
    DDS_unsigned_long count
    )
{
    return gapi_sequence_allocbuf (
        ff,
        (gapi_unsigned_long) len,
        (gapi_unsigned_long) count
    );
}

void
DDS_sequence_replacebuf (
    void *sequence,
    void *(*allocbuf)(DDS_unsigned_long len),
    DDS_unsigned_long count
    )
{
    gapi_sequence_replacebuf (
        sequence,
        allocbuf,
        (gapi_unsigned_long) count
    );
}

void *
DDS_sequence_create (
    DDS_boolean (*ff)(void *),
    DDS_unsigned_long len,
    DDS_unsigned_long count
    )
{
    return gapi_sequence_create (
        ff,
        (gapi_unsigned_long) len,
        (gapi_unsigned_long) count
    );

}

DDS_sequence_octet *
DDS_sequence_octet__alloc (
    void
    )
{
   return (DDS_sequence_octet *)DDS_sequence_malloc();
}

DDS_octet *
DDS_sequence_octet_allocbuf (
    DDS_unsigned_long len
    )
{
    return (DDS_octet *)DDS_sequence_allocbuf (NULL, sizeof (DDS_octet), len);
}

DDS_octSeq *
DDS_octSeq__alloc (void)
{
    return (DDS_octSeq *)DDS_sequence_octet__alloc ();
}

DDS_octet *
DDS_octSeq_allocbuf (DDS_unsigned_long len)
{
    return (DDS_octet *)DDS_sequence_octet_allocbuf(len);
}

DDS_sequence_DDS_InstanceHandle_t *
DDS_sequence_DDS_InstanceHandle_t__alloc (
    void
    )
{
    return (DDS_sequence_DDS_InstanceHandle_t *)
        gapi_instanceHandleSeq__alloc ();

}

DDS_InstanceHandle_t *
DDS_sequence_DDS_InstanceHandle_t_allocbuf (
    DDS_unsigned_long len
    )
{
    return (DDS_InstanceHandle_t *)
        gapi_instanceHandleSeq_allocbuf (
            (gapi_unsigned_long) len
    );

}

DDS_InstanceHandleSeq *
DDS_InstanceHandleSeq__alloc (
    void
    )
{
    return (DDS_InstanceHandleSeq *)
        gapi_instanceHandleSeq__alloc ();

}

DDS_InstanceHandle_t *
DDS_InstanceHandleSeq_allocbuf (
    DDS_unsigned_long len
    )
{
    return (DDS_InstanceHandle_t *)
        gapi_instanceHandleSeq_allocbuf (
            (gapi_unsigned_long) len
    );

}

gapi_boolean DDS_sequence_string_freebuf (void *buffer)
{
    DDS_unsigned_long *count = (DDS_unsigned_long *)DDS__header (buffer);
    DDS_string *b = (DDS_string *)buffer;
    DDS_unsigned_long i;
    for (i = 0; i < *count; i++) {
        DDS_string_clean (&b[i]);
    }
    return TRUE;
}

DDS_sequence_string *DDS_sequence_string__alloc (void)
{
    return (DDS_sequence_string *)DDS_sequence_malloc();
}

DDS_string *DDS_sequence_string_allocbuf (DDS_unsigned_long len)
{
    return (DDS_string *)DDS_sequence_allocbuf (DDS_sequence_string_freebuf, sizeof (DDS_string), len);
}

DDS_StringSeq *
DDS_StringSeq__alloc (
    void
    )
{
    return (DDS_StringSeq *)
        gapi_stringSeq__alloc ();

}

DDS_string *
DDS_StringSeq_allocbuf (
    DDS_unsigned_long len
    )
{
    return (DDS_string *)
        gapi_stringSeq_allocbuf (
            (gapi_unsigned_long) len
    );

}

DDS_Duration_t *
DDS_Duration_t__alloc (
    void
    )
{
    return (DDS_Duration_t *)
        gapi_duration_t__alloc ();

}

DDS_Time_t *
DDS_Time_t__alloc (
    void
    )
{
    return (DDS_Time_t *)
        gapi_time_t__alloc ();

}

DDS_sequence_DDS_QosPolicyCount *
DDS_sequence_DDS_QosPolicyCount__alloc (
    void
    )
{
    return (DDS_sequence_DDS_QosPolicyCount *)
        gapi_qosPolicyCountSeq__alloc ();

}

DDS_QosPolicyCount *
DDS_sequence_DDS_QosPolicyCount_allocbuf (
    DDS_unsigned_long len
    )
{
    return (DDS_QosPolicyCount *)
        gapi_qosPolicyCountSeq_allocbuf (
            (gapi_unsigned_long) len
    );

}

DDS_QosPolicyCountSeq *
DDS_QosPolicyCountSeq__alloc (
    void
    )
{
    return (DDS_QosPolicyCountSeq *)
        gapi_qosPolicyCountSeq__alloc ();

}

DDS_QosPolicyCount *
DDS_QosPolicyCountSeq_allocbuf (
    DDS_unsigned_long len
    )
{
    return (DDS_QosPolicyCount *)
        gapi_qosPolicyCountSeq_allocbuf (
            (gapi_unsigned_long) len
    );
}

DDS_sequence_DDS_Topic *
DDS_sequence_DDS_Topic__alloc (
    void
    )
{
    return (DDS_sequence_DDS_Topic *)
        gapi_topicSeq__alloc ();

}

DDS_Topic *
DDS_sequence_DDS_Topic_allocbuf (
    DDS_unsigned_long len
    )
{
    return (DDS_Topic *)
        gapi_topicSeq_allocbuf (
            (gapi_unsigned_long) len
    );

}

DDS_TopicSeq *
DDS_TopicSeq__alloc (
    void
    )
{
    return (DDS_TopicSeq *)
        gapi_topicSeq__alloc ();

}

DDS_Topic *
DDS_TopicSeq_allocbuf (
    DDS_unsigned_long len
    )
{
    return (DDS_Topic *)
        gapi_topicSeq_allocbuf (
            (gapi_unsigned_long) len
    );

}

DDS_sequence_DDS_DataReader *
DDS_sequence_DDS_DataReader__alloc (
    void
    )
{
    return (DDS_sequence_DDS_DataReader *)
        gapi_dataReaderSeq__alloc ();

}

DDS_DataReader *
DDS_sequence_DDS_DataReader_allocbuf (
    DDS_unsigned_long len
    )
{
    return (DDS_DataReader *)
        gapi_dataReaderSeq_allocbuf (
            (gapi_unsigned_long) len
    );

}

DDS_DataReaderSeq *
DDS_DataReaderSeq__alloc (
    void
    )
{
    return (DDS_DataReaderSeq *)
        gapi_dataReaderSeq__alloc ();

}

DDS_DataReader *
DDS_DataReaderSeq_allocbuf (
    DDS_unsigned_long len
    )
{
    return (DDS_DataReader *)
        gapi_dataReaderSeq_allocbuf (
            (gapi_unsigned_long) len
    );

}

struct DDS_TopicListener *
DDS_TopicListener__alloc (
    void
    )
{
    return (struct DDS_TopicListener *)
        gapi_alloc (sizeof(struct DDS_TopicListener));

}

struct DDS_ExtTopicListener *
DDS_ExtTopicListener__alloc (
    void
    )
{
    return (struct DDS_ExtTopicListener *)
        gapi_alloc (sizeof(struct DDS_ExtTopicListener));

}

struct DDS_DataWriterListener *
DDS_DataWriterListener__alloc (
    void
    )
{
    return (struct DDS_DataWriterListener *)
        gapi_alloc (sizeof(struct DDS_DataWriterListener));

}

struct DDS_PublisherListener *
DDS_PublisherListener__alloc (
    void
    )
{
    return(struct DDS_PublisherListener *)
        gapi_alloc (sizeof(struct DDS_PublisherListener));
}

struct DDS_DataReaderListener *
DDS_DataReaderListener__alloc (
    void
    )
{
    return (struct DDS_DataReaderListener *)
        gapi_alloc (sizeof(struct DDS_DataReaderListener));

}

struct DDS_SubscriberListener *
DDS_SubscriberListener__alloc (
    void
    )
{
    return (struct DDS_SubscriberListener *)
        gapi_alloc (sizeof(struct DDS_SubscriberListener));

}

struct DDS_DomainParticipantListener *
DDS_DomainParticipantListener__alloc (
    void
    )
{
    return (struct DDS_DomainParticipantListener *)
        gapi_alloc (sizeof(struct DDS_DomainParticipantListener));

}

DDS_sequence_DDS_Condition *
DDS_sequence_DDS_Condition__alloc (
    void
    )
{
    return (DDS_sequence_DDS_Condition *)
        gapi_conditionSeq__alloc ();

}

DDS_Condition *
DDS_sequence_DDS_Condition_allocbuf (
    DDS_unsigned_long len
    )
{
    return (DDS_Condition *)
        gapi_conditionSeq_allocbuf (
            (gapi_unsigned_long) len
    );

}

DDS_ConditionSeq *
DDS_ConditionSeq__alloc (
    void
    )
{
    return (DDS_ConditionSeq *)
        gapi_conditionSeq__alloc ();

}

DDS_Condition *
DDS_ConditionSeq_allocbuf (
    DDS_unsigned_long len
    )
{
    return (DDS_Condition *)
        gapi_conditionSeq_allocbuf (
            (gapi_unsigned_long) len
    );

}

DDS_sequence_DDS_SampleStateKind *
DDS_sequence_DDS_SampleStateKind__alloc (
    void
    )
{
    return (DDS_sequence_DDS_SampleStateKind *)
        gapi_sampleStateSeq__alloc ();

}

DDS_SampleStateKind *
DDS_sequence_DDS_SampleStateKind_allocbuf (
    DDS_unsigned_long len
    )
{
    return (DDS_SampleStateKind *)
        gapi_sampleStateSeq_allocbuf (
            (gapi_unsigned_long) len
    );

}

DDS_SampleStateSeq *
DDS_SampleStateSeq__alloc (
    void
    )
{
    return (DDS_SampleStateSeq *)
        gapi_sampleStateSeq__alloc ();

}

DDS_SampleStateKind *
DDS_SampleStateSeq_allocbuf (
    DDS_unsigned_long len
    )
{
    return (DDS_SampleStateKind *)
        gapi_sampleStateSeq_allocbuf (
            (gapi_unsigned_long) len
    );

}

DDS_sequence_DDS_ViewStateKind *
DDS_sequence_DDS_ViewStateKind__alloc (
    void
    )
{
    return (DDS_sequence_DDS_ViewStateKind *)
        gapi_viewStateSeq__alloc ();

}

DDS_ViewStateKind *
DDS_sequence_DDS_ViewStateKind_allocbuf (
    DDS_unsigned_long len
    )
{
    return (DDS_ViewStateKind *)
        gapi_viewStateSeq_allocbuf (
            (gapi_unsigned_long) len
    );

}

DDS_ViewStateSeq *
DDS_ViewStateSeq__alloc (
    void
    )
{
    return (DDS_ViewStateSeq *)
        gapi_viewStateSeq__alloc ();

}

DDS_ViewStateKind *
DDS_ViewStateSeq_allocbuf (
    DDS_unsigned_long len
    )
{
    return (DDS_ViewStateKind *)
        gapi_viewStateSeq_allocbuf (
            (gapi_unsigned_long) len
    );

}

DDS_sequence_DDS_InstanceStateKind *
DDS_sequence_DDS_InstanceStateKind__alloc (
    void
    )
{
    return (DDS_sequence_DDS_InstanceStateKind *)
        gapi_instanceStateSeq__alloc ();

}

DDS_InstanceStateKind *
DDS_sequence_DDS_InstanceStateKind_allocbuf (
    DDS_unsigned_long len
    )
{
    return (DDS_InstanceStateKind *)
        gapi_instanceStateSeq_allocbuf (
            (gapi_unsigned_long) len
    );

}

DDS_InstanceStateSeq *
DDS_InstanceStateSeq__alloc (
    void
    )
{
    return (DDS_InstanceStateSeq *)
        gapi_instanceStateSeq__alloc ();

}

DDS_InstanceStateKind *
DDS_InstanceStateSeq_allocbuf (
    DDS_unsigned_long len
    )
{
    return (DDS_InstanceStateKind *)
        gapi_instanceStateSeq_allocbuf (
            (gapi_unsigned_long) len
    );

}

DDS_DomainParticipantFactoryQos *
DDS_DomainParticipantFactoryQos__alloc (
    void
    )
{
    return (DDS_DomainParticipantFactoryQos *)
        gapi_domainParticipantFactoryQos__alloc ();
}

DDS_DomainParticipantQos *
DDS_DomainParticipantQos__alloc (
    void
    )
{
    return (DDS_DomainParticipantQos *)
        gapi_domainParticipantQos__alloc ();

}

DDS_TopicQos *
DDS_TopicQos__alloc (
    void
    )
{
    return (DDS_TopicQos *)
        gapi_topicQos__alloc ();

}

DDS_DataWriterQos *
DDS_DataWriterQos__alloc (
    void
    )
{
    return (DDS_DataWriterQos *)
        gapi_dataWriterQos__alloc ();

}

DDS_PublisherQos *
DDS_PublisherQos__alloc (
    void
    )
{
    return (DDS_PublisherQos *)
        gapi_publisherQos__alloc ();
}

DDS_DataReaderQos *
DDS_DataReaderQos__alloc (
    void
    )
{
    return (DDS_DataReaderQos *)
        gapi_dataReaderQos__alloc ();
}

DDS_DataReaderViewQos *
DDS_DataReaderViewQos__alloc (
    void
    )
{
    return (DDS_DataReaderViewQos *)
        gapi_dataReaderViewQos__alloc ();
}

DDS_SubscriberQos *
DDS_SubscriberQos__alloc (
    void
    )
{
    return (DDS_SubscriberQos *)
        gapi_subscriberQos__alloc ();

}

DDS_ParticipantBuiltinTopicData *
DDS_ParticipantBuiltinTopicData__alloc (
    void
    )
{
    return (DDS_ParticipantBuiltinTopicData *)
        gapi_participantBuiltinTopicData__alloc ();

}

DDS_sequence_DDS_ParticipantBuiltinTopicData *
DDS_sequence_DDS_ParticipantBuiltinTopicData__alloc (
    void
    )
{
    return(DDS_sequence_DDS_ParticipantBuiltinTopicData *)
        gapi_participantBuiltinTopicDataSeq__alloc ();

}

DDS_ParticipantBuiltinTopicData *
DDS_sequence_DDS_ParticipantBuiltinTopicData_allocbuf (
    DDS_unsigned_long len
    )
{
    return (DDS_ParticipantBuiltinTopicData *)
        gapi_participantBuiltinTopicDataSeq_allocbuf (
            (gapi_unsigned_long) len
    );

}

DDS_TopicBuiltinTopicData *
DDS_TopicBuiltinTopicData__alloc (
    void
    )
{
    return (DDS_TopicBuiltinTopicData *)
        gapi_topicBuiltinTopicData__alloc ();

}

DDS_sequence_DDS_TopicBuiltinTopicData *
DDS_sequence_DDS_TopicBuiltinTopicData__alloc (
    void
    )
{
    return (DDS_sequence_DDS_TopicBuiltinTopicData *)
        gapi_topicBuiltinTopicDataSeq__alloc ();

}

DDS_TopicBuiltinTopicData *
DDS_sequence_DDS_TopicBuiltinTopicData_allocbuf (
    DDS_unsigned_long len
    )
{
    return (DDS_TopicBuiltinTopicData *)
        gapi_topicBuiltinTopicDataSeq_allocbuf (
            (gapi_unsigned_long) len
    );

}

DDS_PublicationBuiltinTopicData *
DDS_PublicationBuiltinTopicData__alloc (
    void
    )
{
    return (DDS_PublicationBuiltinTopicData *)
        gapi_publicationBuiltinTopicData__alloc ();

}

DDS_sequence_DDS_PublicationBuiltinTopicData *
DDS_sequence_DDS_PublicationBuiltinTopicData__alloc (
    void
    )
{
    return (DDS_sequence_DDS_PublicationBuiltinTopicData *)
        gapi_publicationBuiltinTopicData__alloc ();

}

DDS_PublicationBuiltinTopicData *
DDS_sequence_DDS_PublicationBuiltinTopicData_allocbuf (
    DDS_unsigned_long len
    )
{
    return (DDS_PublicationBuiltinTopicData *)
        gapi_publicationBuiltinTopicDataSeq_allocbuf (
            (gapi_unsigned_long) len
    );

}

DDS_SubscriptionBuiltinTopicData *
DDS_SubscriptionBuiltinTopicData__alloc (
    void
    )
{
    return (DDS_SubscriptionBuiltinTopicData *)
        gapi_subscriptionBuiltinTopicData__alloc ();

}

DDS_sequence_DDS_SubscriptionBuiltinTopicData *
DDS_sequence_DDS_SubscriptionBuiltinTopicData__alloc (
    void
    )
{
    return (DDS_sequence_DDS_SubscriptionBuiltinTopicData *)
        gapi_subscriptionBuiltinTopicDataSeq__alloc ();

}

DDS_SubscriptionBuiltinTopicData *
DDS_sequence_DDS_SubscriptionBuiltinTopicData_allocbuf (
    DDS_unsigned_long len
    )
{
    return (DDS_SubscriptionBuiltinTopicData *)
        gapi_subscriptionBuiltinTopicDataSeq_allocbuf (
            (gapi_unsigned_long) len
    );

}

DDS_sequence_DDS_SampleInfo *
DDS_sequence_DDS_SampleInfo__alloc (
    void
    )
{
    return (DDS_sequence_DDS_SampleInfo *)
        gapi_sampleInfoSeq__alloc ();

}

DDS_SampleInfo *
DDS_sequence_DDS_SampleInfo_allocbuf (
    DDS_unsigned_long len
    )
{
    return( DDS_SampleInfo *)
        gapi_sampleInfoSeq_allocbuf (
            (gapi_unsigned_long) len
    );

}

DDS_SampleInfoSeq *
DDS_SampleInfoSeq__alloc (
    void
    )
{
    return (DDS_SampleInfoSeq *)
        gapi_sampleInfoSeq__alloc ();

}

DDS_SampleInfo *
DDS_SampleInfoSeq_allocbuf (
    DDS_unsigned_long len
    )
{
    return (DDS_SampleInfo *)
        gapi_sampleInfoSeq_allocbuf (
            (gapi_unsigned_long) len
    );

}
