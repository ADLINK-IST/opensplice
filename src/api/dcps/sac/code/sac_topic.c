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

/*     // Access the status
 *     InconsistentTopicStatus
 *     get_inconsistent_topic_status();
 */
DDS_ReturnCode_t
DDS_Topic_get_inconsistent_topic_status (
        DDS_Topic this,
        DDS_InconsistentTopicStatus *status
        )
{
    DDS_ReturnCode_t result;
    gapi_inconsistentTopicStatus s;

    result = gapi_topic_get_inconsistent_topic_status ((gapi_dataReader)this,&s);
    sac_copyGapiInconsistentTopicStatus (&s, status);

    return result;
}

/*     ReturnCode_t
 *     set_listener(
 *         in TopicListener a_listener,
 *         in StatusKindMask mask);
 */
DDS_ReturnCode_t
DDS_Topic_set_listener (
    DDS_Topic this,
    const struct DDS_TopicListener *a_listener,
    const DDS_StatusMask mask
    )
{
    struct gapi_topicListener gListener;
    struct gapi_topicListener *pListener = NULL;

    if ( a_listener ) {
        sac_copySacTopicListener(a_listener, &gListener);
        pListener = &gListener;
    }
    
    return (DDS_ReturnCode_t)
	gapi_topic_set_listener (
	    (gapi_topic)this,
	    (const struct gapi_topicListener *)pListener,
	    (gapi_statusMask)mask
	);
}

/*     TopicListener
 *     get_listener();
 */
struct DDS_TopicListener
DDS_Topic_get_listener (
    DDS_Topic this
    )
{
    struct gapi_topicListener s;
    struct DDS_TopicListener d;

    s = gapi_topic_get_listener ((gapi_topic)this);
    sac_copyGapiTopicListener (&s, &d);

    return d;
}

/*     ReturnCode_t
 *     set_qos(
 *         in TopicQos qos);
 */
DDS_ReturnCode_t
DDS_Topic_set_qos (
    DDS_Topic this,
    const DDS_TopicQos *qos
    )
{
    return (DDS_ReturnCode_t)
	gapi_topic_set_qos (
	    (gapi_topic)this,
	    (const  gapi_topicQos *)qos
	);
}

/*     ReturnCode_t
 *     get_qos(
 *         inout TopicQos qos);
 */
DDS_ReturnCode_t
DDS_Topic_get_qos (
    DDS_Topic this,
    DDS_TopicQos *qos
    )
{
    return (DDS_ReturnCode_t)
    gapi_topic_get_qos (
        (gapi_topic)this,
        (gapi_topicQos *)qos
    );
}

/*     DDS_ReturnCode_t
 *     dispose_all_data();
 */
DDS_ReturnCode_t
DDS_Topic_dispose_all_data (
    DDS_Topic _this)
{
    return (DDS_ReturnCode_t)
    gapi_topic_dispose_all_data (
        (gapi_topic)_this);
}

/*     DDS_string
 *     DDS_Topic_get_metadescription();
 */
DDS_string
DDS_Topic_get_metadescription (
    DDS_Topic _this)
{
    return (DDS_string)
    gapi_topic_get_metadescription (
        (gapi_topic)_this);
}

/*     DDS_string
 *     DDS_Topic_get_keylist();
 */
DDS_string
DDS_Topic_get_keylist (
    DDS_Topic _this)
{
    return (DDS_string)
    gapi_topic_get_keylist (
        (gapi_topic)_this);
}

