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

#include "gapi.h"

#include "dds_dcps.h"
#include "sac_structured.h"

/*     ReadCondition
 *     create_readcondition(
 *         in SampleStateMask sample_states,
 *         in ViewStateMask view_states,
 *         in InstanceStateMask instance_states);
 */
DDS_ReadCondition
DDS_DataReader_create_readcondition (
    DDS_DataReader this,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states
    )
{
    return (DDS_ReadCondition)
        gapi_dataReader_create_readcondition (
            (gapi_dataReader)this,
            (gapi_sampleStateMask)sample_states,
            (gapi_viewStateMask)view_states,
            (gapi_instanceStateMask)instance_states
        );
}

/*     QueryCondition
 *     create_querycondition(
 *         in SampleStateMask sample_states,
 *         in ViewStateMask view_states,
 *         in InstanceStateMask instance_states,
 *         in string query_expression,
 *         in StringSeq query_parameters);
 */
DDS_QueryCondition
DDS_DataReader_create_querycondition (
    DDS_DataReader this,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states,
    const DDS_char *query_expression,
    const DDS_StringSeq *query_parameters
    )
{
    return (DDS_QueryCondition)
        gapi_dataReader_create_querycondition (
            (gapi_dataReader)this,
            (gapi_sampleStateMask)sample_states,
            (gapi_viewStateMask)view_states,
            (gapi_instanceStateMask)instance_states,
            (gapi_char *)query_expression,
            (gapi_stringSeq *)query_parameters
        );
}

/*     ReturnCode_t
 *     delete_readcondition(
 *         in ReadCondition a_condition);
 */
DDS_ReturnCode_t
DDS_DataReader_delete_readcondition (
    DDS_DataReader this,
    const DDS_ReadCondition a_condition
    )
{
    return (DDS_ReturnCode_t)
        gapi_dataReader_delete_readcondition (
            (DDS_DataReader)this,
            (DDS_ReadCondition)a_condition
        );
}

/*     ReturnCode_t
 *     delete_contained_entities();
 */
DDS_ReturnCode_t
DDS_DataReader_delete_contained_entities (
    DDS_DataReader this
    )
{
    return (DDS_ReturnCode_t)
        gapi_dataReader_delete_contained_entities (
            (gapi_dataReader)this);
}
/*     DataReaderView
  *     create_view (
  *     in DataReaderViewQos * qos);
 */
DDS_DataReaderView
DDS_DataReader_create_view (
    DDS_DataReader this,
    const DDS_DataReaderViewQos * qos
    )
{
    return (DDS_DataReaderView)
        gapi_dataReader_create_view (
            (gapi_dataReader)this,
            (gapi_dataReaderViewQos *)qos
        );

}
/*     ReturnCode_t
 *     delete_view(
 *        in DataReaderView a_view);
 */
DDS_ReturnCode_t
DDS_DataReader_delete_view (
    DDS_DataReader this,
    DDS_DataReaderView a_view
    )
{
    return (DDS_ReturnCode_t)
        gapi_dataReader_delete_view (
            (gapi_dataReader)this,
            (gapi_dataReaderView)a_view
        );
}


/*     ReturnCode_t
 *     set_qos(
 *         in DataReaderQos qos);
 */
DDS_ReturnCode_t
DDS_DataReader_set_qos (
    DDS_DataReader this,
    const DDS_DataReaderQos *qos
    )
{
    return (DDS_ReturnCode_t)
        gapi_dataReader_set_qos (
            (gapi_dataReader)this,
            (gapi_dataReaderQos *)qos
        );
}

/*     ReturnCode_t
 *     get_qos(
 *         inout DataReaderQos qos);
 */
DDS_ReturnCode_t
DDS_DataReader_get_qos (
    DDS_DataReader this,
    DDS_DataReaderQos *qos
    )
{
    gapi_dataReader_get_qos (
        (gapi_dataReader)this,
        (gapi_dataReaderQos *)qos
    );
    return DDS_RETCODE_OK;
}

/*     ReturnCode_t
 *     set_listener(
 *         in DataReaderListener a_listener,
 *         in StatusKindMask mask);
 */
DDS_ReturnCode_t
DDS_DataReader_set_listener (
    DDS_DataReader this,
    const struct DDS_DataReaderListener *a_listener,
    const DDS_StatusMask mask
    )
{
    struct gapi_dataReaderListener gListener;
    struct gapi_dataReaderListener *pListener = NULL;

    if ( a_listener ) {
        sac_copySacDataReaderListener(a_listener, &gListener);
        pListener = &gListener;
    }

    return (DDS_ReturnCode_t)
        gapi_dataReader_set_listener (
            (gapi_dataReader)this,
            (const struct gapi_dataReaderListener *)pListener,
            (gapi_statusMask)mask
        );
}

/*     DataReaderListener
 *     get_listener();
 */
struct DDS_DataReaderListener
DDS_DataReader_get_listener (
    DDS_DataReader this
    )
{
    struct DDS_DataReaderListener d;
    struct gapi_dataReaderListener s;

    s = gapi_dataReader_get_listener ((gapi_dataReader)this);
    sac_copyGapiDataReaderListener (&s, &d);

    return d;
}

/*     TopicDescription
 *     get_topicdescription();
 */
DDS_TopicDescription
DDS_DataReader_get_topicdescription (
    DDS_DataReader this
    )
{
    return (DDS_TopicDescription)
        gapi_dataReader_get_topicdescription (
            (gapi_dataReader)this
        );
}

/*     Subscriber
 *     get_subscriber();
 */
DDS_Subscriber
DDS_DataReader_get_subscriber (
    DDS_DataReader this
    )
{
    return (DDS_Subscriber)
        gapi_dataReader_get_subscriber (
            (gapi_dataReader)this
        );
}


/* ReturnCode_t
 *get_sample_rejected_status(
 *inout SampleRejectedStatus status);
 */
DDS_ReturnCode_t
DDS_DataReader_get_sample_rejected_status (
    DDS_DataReader this,
    DDS_SampleRejectedStatus *status
    )
{
    DDS_ReturnCode_t result;
    gapi_sampleRejectedStatus s;

    result = gapi_dataReader_get_sample_rejected_status ((gapi_dataReader)this,&s);
    DDS_SampleRejectedStatusCopyin (&s, status);

    return result;
}

/* ReturnCode_t
 *get_liveliness_changed_status(
 *inout LivelinessChangedStatus status);
*/
DDS_ReturnCode_t
DDS_DataReader_get_liveliness_changed_status (
    DDS_DataReader this,
    DDS_LivelinessChangedStatus *status
    )
{
    DDS_ReturnCode_t result;
    gapi_livelinessChangedStatus s;

    result = gapi_dataReader_get_liveliness_changed_status ((gapi_dataReader)this,&s);
    DDS_LivelinessChangedStatusCopyin (&s, status);

    return result;
}


/* ReturnCode_t
 *get_requested_deadline_missed_status(
 *inout RequestedDeadlineMissedStatus status);
*/
DDS_ReturnCode_t
DDS_DataReader_get_requested_deadline_missed_status (
    DDS_DataReader this,
    DDS_RequestedDeadlineMissedStatus *status
    )
{
    DDS_ReturnCode_t result;
    gapi_requestedDeadlineMissedStatus s;

    result = gapi_dataReader_get_requested_deadline_missed_status ((gapi_dataReader)this,&s);
    DDS_RequestedDeadlineMissedStatusCopyin (&s, status);

    return result;
}

/* ReturnCode_t
 *get_requested_incompatible_qos_status(
 *inout RequestedIncompatibleQosStatus status);
*/
DDS_ReturnCode_t
DDS_DataReader_get_requested_incompatible_qos_status (
    DDS_DataReader this,
    DDS_RequestedIncompatibleQosStatus *status
    )
{
    DDS_ReturnCode_t result;
    gapi_requestedIncompatibleQosStatus* s;

    s = gapi_requestedIncompatibleQosStatus_alloc();

    result = gapi_dataReader_get_requested_incompatible_qos_status ((gapi_dataReader)this, s);
    DDS_RequestedIncompatibleQosStatusCopyin (s, status);
    gapi_free(s);

    return result;
}

/* ReturnCode_t
 *get_sample_lost_status(
 *inout SampleLostStatus status);
*/
DDS_ReturnCode_t
DDS_DataReader_get_sample_lost_status (
    DDS_DataReader this,
    DDS_SampleLostStatus *status
    )
{
    DDS_ReturnCode_t result;
    gapi_sampleLostStatus s;

    result = gapi_dataReader_get_sample_lost_status ((gapi_dataReader)this,&s);
    DDS_SampleLostStatusCopyin (&s, status);

    return result;
}

/* ReturnCode_t
 * get_subscription_matched_status(
 *      inout SubscriptionMatchedStatus status);
 */
DDS_ReturnCode_t
DDS_DataReader_get_subscription_matched_status (
    DDS_DataReader this,
    DDS_SubscriptionMatchedStatus *status
    )
{
    DDS_ReturnCode_t result;
    gapi_subscriptionMatchedStatus s;

    result = gapi_dataReader_get_subscription_matched_status ((gapi_dataReader)this,&s);
    DDS_SubscriptionMatchedStatusCopyin (&s, status);

    return result;
}

/*     ReturnCode_t
 *     wait_for_historical_data(
 *         in Duration_t max_wait);
 */
DDS_ReturnCode_t
DDS_DataReader_wait_for_historical_data (
    DDS_DataReader this,
    const DDS_Duration_t *max_wait
    )
{
    return (DDS_ReturnCode_t)
        gapi_dataReader_wait_for_historical_data (
            (gapi_dataReader)this,
            (const gapi_duration_t *)max_wait
        );
}

DDS_ReturnCode_t
DDS_DataReader_wait_for_historical_data_w_condition (
    DDS_DataReader this,
    const DDS_char *filter_expression,
    const DDS_StringSeq *filter_parameters,
    const DDS_Time_t *min_source_timestamp,
    const DDS_Time_t *max_source_timestamp,
    const DDS_ResourceLimitsQosPolicy *resource_limits,
    const DDS_Duration_t *max_wait)
{
    return (DDS_ReturnCode_t)
    gapi_dataReader_wait_for_historical_data_w_condition(
            (gapi_dataReader)this,
            (const gapi_char *)filter_expression,
            (const gapi_stringSeq *)filter_parameters,
            (const gapi_time_t*)min_source_timestamp,
            (const gapi_time_t*)max_source_timestamp,
            (const gapi_resourceLimitsQosPolicy*)resource_limits,
            (const gapi_duration_t *)max_wait);
}

/*     ReturnCode_t get_matched_publications(
 *     inout InstanceHandleSeq publication_handles);
 */
DDS_ReturnCode_t
DDS_DataReader_get_matched_publications (
    DDS_DataReader this,
    DDS_InstanceHandleSeq *publication_handles
    )
{
    return (DDS_ReturnCode_t)
        gapi_dataReader_get_matched_publications (
            (gapi_dataReader)this,
            (gapi_instanceHandleSeq *)publication_handles
        );
}

/*     ReturnCode_t
 *     get_matched_publication_data(
 *         inout PublicationBuiltinTopicData publication_data,
 *         in InstanceHandle_t publication_handle);
 */
DDS_ReturnCode_t
DDS_DataReader_get_matched_publication_data (
    DDS_DataReader this,
    DDS_PublicationBuiltinTopicData *publication_data,
    const DDS_InstanceHandle_t publication_handle
    )
{
    return (DDS_ReturnCode_t)
        gapi_dataReader_get_matched_publication_data (
            (gapi_dataReader)this,
            (gapi_publicationBuiltinTopicData *)publication_data,
            (gapi_instanceHandle_t)publication_handle
        );
}

/*     ReturnCode_t
 *     set_default_datareaderview_qos(
 *         in DataReaderViewQos qos);
 */
DDS_ReturnCode_t
DDS_DataReader_set_default_datareaderview_qos (
    DDS_DataReader _this,
    const DDS_DataReaderViewQos *qos
    )
{
    return (DDS_ReturnCode_t)
        gapi_dataReader_set_default_datareaderview_qos(
            (gapi_dataReader)_this,
            (const gapi_dataReaderViewQos*)qos
        );
}

/*     ReturnCode_t
 *     get_default_datareaderview_qos(
 *         inout DataReaderViewQos qos);
 */
DDS_ReturnCode_t
DDS_DataReader_get_default_datareaderview_qos (
    DDS_DataReader _this,
    DDS_DataReaderViewQos *qos
    )
{
    return (DDS_ReturnCode_t)
    gapi_dataReader_get_default_datareaderview_qos(
        (gapi_dataReader)_this,
        (gapi_dataReaderViewQos*)qos
        );
}

/*     ReturnCode_t
 *     set_notread_threshold(
 *         in long threshold);
 */
DDS_ReturnCode_t
DDS_DataReader_set_notread_threshold (
    DDS_DataReader _this,
    DDS_long threshold)
{
    return (DDS_ReturnCode_t)
    gapi_dataReader_set_notread_threshold(
        (gapi_dataReader)_this,
        threshold
        );
}
