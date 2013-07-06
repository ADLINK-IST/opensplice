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

/*     ReturnCode_t
 *     set_qos(
 *         in DataWriterQos qos);
 */
DDS_ReturnCode_t
DDS_DataWriter_set_qos (
    DDS_DataWriter this,
    const DDS_DataWriterQos *qos
    )
{
    return (DDS_ReturnCode_t)
	gapi_dataWriter_set_qos (
	    (gapi_dataWriter)this,
	    (const gapi_dataWriterQos *)qos
	);
}

/*     ReturnCode_t
 *     get_qos(
 *         inout DataWriterQos qos);
 */
DDS_ReturnCode_t
DDS_DataWriter_get_qos (
    DDS_DataWriter this,
    DDS_DataWriterQos *qos
    )
{
    return (DDS_ReturnCode_t)
    gapi_dataWriter_get_qos (
	(gapi_dataWriter)this,
	(gapi_dataWriterQos *)qos
    );

}

/*     ReturnCode_t
 *     set_listener(
 *         in DataWriterListener a_listener,
 *         in StatusKindMask mask);
 */
DDS_ReturnCode_t
DDS_DataWriter_set_listener (
    DDS_DataWriter this,
    const struct DDS_DataWriterListener *a_listener,
    const DDS_StatusMask mask
    )
{
    struct gapi_dataWriterListener gListener;
    struct gapi_dataWriterListener *pListener = NULL;

    if ( a_listener ) {
        sac_copySacDataWriterListener(a_listener, &gListener);
        pListener = &gListener;
    }

    return (DDS_ReturnCode_t)
	gapi_dataWriter_set_listener (
	    (gapi_dataWriter)this,
	    (const struct gapi_dataWriterListener *)pListener,
	    (DDS_StatusMask)mask
	);
}

/*     DataWriterListener
 *     get_listener();
 */
struct DDS_DataWriterListener
DDS_DataWriter_get_listener (
    DDS_DataWriter this
    )
{
    struct gapi_dataWriterListener gapiListener;
    struct DDS_DataWriterListener sacListener;

    gapiListener = gapi_dataWriter_get_listener ((gapi_dataWriter)this);
    sac_copyGapiDataWriterListener (&gapiListener, &sacListener);

    return sacListener;
}

/*     Topic
 *     get_topic();
 */
DDS_Topic
DDS_DataWriter_get_topic (
    DDS_DataWriter this
    )
{
    return (DDS_Topic)
	gapi_dataWriter_get_topic (
	    (gapi_dataWriter)this
	);
}

/*     Publisher
 *     get_publisher();
 */
DDS_Publisher
DDS_DataWriter_get_publisher (
    DDS_DataWriter this
    )
{
    return (DDS_Publisher)
	gapi_dataWriter_get_publisher (
	    (gapi_dataWriter)this
	);
}

/* ReturnCode_t
 *   wait_for_acknowledgments(
 *      in Duration_t max_wait);
 */
DDS_ReturnCode_t
DDS_DataWriter_wait_for_acknowledgments (
    DDS_DataWriter _this,
    const DDS_Duration_t *max_wait
    )
{
    return (DDS_ReturnCode_t)
    gapi_dataWriter_wait_for_acknowledgments(
         (gapi_publisher) _this,
         (const gapi_duration_t *) max_wait);
}

/*     // Access the status
 * ReturnCode_t
 * get_liveliness_lost_status(
 *       inout LivelinessLostStatus a_status);
 */
DDS_ReturnCode_t
DDS_DataWriter_get_liveliness_lost_status (
    DDS_DataWriter this,
    DDS_LivelinessLostStatus *status
    )
{
    DDS_ReturnCode_t result;
    gapi_livelinessLostStatus s;

    result = gapi_dataWriter_get_liveliness_lost_status ((gapi_dataWriter)this,&s);
    DDS_LivelinessLostStatusCopyin (&s, status);

    return result;
}

/*     // Access the status
 * ReturnCode_t
 * get_offered_deadline_missed_status(
 *       inout OfferedDeadlineMissedStatus a_status);
 */
DDS_ReturnCode_t
DDS_DataWriter_get_offered_deadline_missed_status (
    DDS_DataWriter this,
    DDS_OfferedDeadlineMissedStatus *status
    )
{
    DDS_ReturnCode_t result;
    gapi_offeredDeadlineMissedStatus s;

    result = gapi_dataWriter_get_offered_deadline_missed_status ((gapi_dataWriter)this,&s);
    DDS_OfferedDeadlineMissedStatusCopyin (&s, status);

    return result;
}

/*     // Access the status
 * ReturnCode_t
 * get_offered_incompatible_qos_status(
 *       inout OfferedIncompatibleQosStatus a_status);
 */
DDS_ReturnCode_t
DDS_DataWriter_get_offered_incompatible_qos_status (
    DDS_DataWriter this,
    DDS_OfferedIncompatibleQosStatus *status
    )
{
    DDS_ReturnCode_t result;
    gapi_offeredIncompatibleQosStatus* s;

    s = gapi_offeredIncompatibleQosStatus_alloc();
    result = gapi_dataWriter_get_offered_incompatible_qos_status ((gapi_dataWriter)this,s);
    DDS_OfferedIncompatibleQosStatusCopyin (s, status);
    gapi_free(s);

    return result;
}

/*     // Access the status
 * ReturnCode_t
 * get_publication_matched_status(
 *       inout PublicationMatchedStatus a_status);
 */
DDS_ReturnCode_t
DDS_DataWriter_get_publication_matched_status (
    DDS_DataWriter this,
    DDS_PublicationMatchedStatus *status
    )
{
    DDS_ReturnCode_t result;
    gapi_publicationMatchedStatus s;

    result = gapi_dataWriter_get_publication_matched_status ((gapi_dataWriter)this,&s);
    DDS_PublicationMatchedStatusCopyin(&s, status);

    return result;
}

/*     ReturnCode_t
 *     assert_liveliness();
 */
DDS_ReturnCode_t
DDS_DataWriter_assert_liveliness (
    DDS_DataWriter this
    )
{
    return (DDS_ReturnCode_t)
	gapi_dataWriter_assert_liveliness (
	(gapi_dataWriter)this
    );
}

/*     ReturnCode_t
 *     get_matched_subscriptions(
 *         inout InstanceHandleSeq subscription_handles);
 */
DDS_ReturnCode_t
DDS_DataWriter_get_matched_subscriptions (
    DDS_DataWriter this,
    DDS_InstanceHandleSeq *subscription_handles
    )
{
    return (DDS_ReturnCode_t)
        gapi_dataWriter_get_matched_subscriptions (
	    (gapi_dataWriter)this,
	    (gapi_instanceHandleSeq *)subscription_handles
	);
}

/*     ReturnCode_t
 *     get_matched_subscription_data(
 *         inout SubscriptionBuiltinTopicData subscription_data,
 *         in InstanceHandle_t subscription_handle);
 */
DDS_ReturnCode_t
DDS_DataWriter_get_matched_subscription_data (
    DDS_DataWriter this,
    DDS_SubscriptionBuiltinTopicData *subscription_data,
    const DDS_InstanceHandle_t subscription_handle
    )
{
    return (DDS_ReturnCode_t)
	gapi_dataWriter_get_matched_subscription_data (
	    (gapi_dataWriter)this,
	    (gapi_subscriptionBuiltinTopicData *)subscription_data,
	    (gapi_instanceHandle_t)subscription_handle
	);
}
