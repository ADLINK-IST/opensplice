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

/*     DataWriter
 *     create_datawriter(
 *         in Topic a_topic,
 *         in DataWriterQos qos,
 *         in DataWriterListener a_listener);
 */
DDS_DataWriter
DDS_Publisher_create_datawriter (
    DDS_Publisher this,
    const DDS_Topic a_topic,
    const DDS_DataWriterQos *qos,
    const struct DDS_DataWriterListener *a_listener,
    const DDS_StatusMask mask)
{
    DDS_DataWriter writer;
    struct gapi_dataWriterListener gListener;
    struct gapi_dataWriterListener *pListener = NULL;

    if ( a_listener ) {
        sac_copySacDataWriterListener(a_listener, &gListener);
        pListener = &gListener;
    }
    
    writer = gapi_publisher_create_datawriter (
                    (gapi_publisher)this,
                    (gapi_topic)a_topic,
                    (const gapi_dataWriterQos *)qos,
                    (const struct gapi_dataWriterListener *)pListener,
                    (gapi_statusMask) mask
                );

    if(writer){
        gapi_publisherQos *pqos = gapi_publisherQos__alloc();
        if(pqos){
            if(gapi_publisher_get_qos(this, pqos) == GAPI_RETCODE_OK){
                if(pqos->entity_factory.autoenable_created_entities) {
                    gapi_entity_enable(writer);
                }
            }
            gapi_free(pqos);
        }
    }


    return writer;
}

/*     ReturnCode_t
 *     delete_datawriter(
 *         in DataWriter a_datawriter);
 */
DDS_ReturnCode_t
DDS_Publisher_delete_datawriter (
    DDS_Publisher this,
    const DDS_DataWriter a_datawriter)
{
    return (DDS_ReturnCode_t)
	gapi_publisher_delete_datawriter (
	    (gapi_publisher)this,
	    (gapi_dataWriter)a_datawriter);
}

/*     DataWriter
 *     lookup_datawriter(
 *         in string topic_name);
 */
DDS_DataWriter
DDS_Publisher_lookup_datawriter (
    DDS_Publisher this,
    const DDS_char *topic_name)
{
    return (DDS_DataWriter)
	gapi_publisher_lookup_datawriter (
	    (gapi_publisher)this,
	    (const gapi_char *)topic_name);
}

/*     ReturnCode_t
 *     delete_contained_entities();
 */
DDS_ReturnCode_t
DDS_Publisher_delete_contained_entities (
    DDS_Publisher this)
{
    return (DDS_ReturnCode_t)
	gapi_publisher_delete_contained_entities (
	    (gapi_publisher)this);
}

/*     ReturnCode_t
 *     set_qos(
 *         in PublisherQos qos);
 */
DDS_ReturnCode_t
DDS_Publisher_set_qos (
    DDS_Publisher this,
    const DDS_PublisherQos *qos)
{
    return (DDS_ReturnCode_t)
	gapi_publisher_set_qos (
	    (gapi_publisher)this,
	    (const gapi_publisherQos *)qos);
}

/*     ReturnCode_t
 *     get_qos(
 *         inout PublisherQos qos);
 */
DDS_ReturnCode_t
DDS_Publisher_get_qos (
    DDS_Publisher this,
    DDS_PublisherQos *qos)
{
    return (DDS_ReturnCode_t)
    gapi_publisher_get_qos (
	(gapi_publisher)this,
	(gapi_publisherQos *)qos);
}

/*     ReturnCode_t
 *     set_listener(
 *         in PublisherListener a_listener,
 *         in StatusKindMask mask);
 */
DDS_ReturnCode_t
DDS_Publisher_set_listener (
    DDS_Publisher this,
    const struct DDS_PublisherListener *a_listener,
    const DDS_StatusMask mask)
{
    struct gapi_publisherListener gListener;
    struct gapi_publisherListener *pListener = NULL;
    
    if ( a_listener ) {
        sac_copySacPublisherListener(a_listener, &gListener);
        pListener = &gListener;
    }
    
    return (DDS_ReturnCode_t)
	gapi_publisher_set_listener (
	    (gapi_publisher)this,
	    (const struct gapi_publisherListener *)pListener,
	    (gapi_statusMask)mask);
}

/*     PublisherListener
 *     get_listener();
 */
struct DDS_PublisherListener
DDS_Publisher_get_listener (
    DDS_Publisher this)
{
    struct DDS_PublisherListener d;
    struct gapi_publisherListener s;

    s = gapi_publisher_get_listener ((gapi_publisher)this);
    sac_copyGapiPublisherListener (&s, &d);

    return d;
}

/*     ReturnCode_t
 *     suspend_publications();
 */
DDS_ReturnCode_t
DDS_Publisher_suspend_publications (
    DDS_Publisher this)
{
    return (DDS_ReturnCode_t)
	gapi_publisher_suspend_publications (
	    (gapi_publisher)this);
}

/*     ReturnCode_t
 *     resume_publications();
 */
DDS_ReturnCode_t
DDS_Publisher_resume_publications (
    DDS_Publisher this)
{
    return (DDS_ReturnCode_t)
	gapi_publisher_resume_publications (
	    (gapi_publisher)this);
}

/*     ReturnCode_t
 *     begin_coherent_changes();
 */
DDS_ReturnCode_t
DDS_Publisher_begin_coherent_changes (
    DDS_Publisher this)
{
    return (DDS_ReturnCode_t)
	gapi_publisher_begin_coherent_changes (
	    (gapi_publisher)this);
}

/*     ReturnCode_t
 *     end_coherent_changes();
 */
DDS_ReturnCode_t
DDS_Publisher_end_coherent_changes (
    DDS_Publisher this)
{
    return (DDS_ReturnCode_t)
	gapi_publisher_end_coherent_changes (
	    (gapi_publisher)this);
}

/* ReturnCode_t 
 *   wait_for_acknowledgments(
 *      in Duration_t max_wait);
 */
DDS_ReturnCode_t
DDS_Publisher_wait_for_acknowledgments (
    DDS_Publisher _this,
    const DDS_Duration_t *max_wait)
{
    return (DDS_ReturnCode_t)
    gapi_publisher_wait_for_acknowledgments(
             (gapi_publisher) _this, 
             (const gapi_duration_t *) max_wait);
}


/*     DomainParticipant
 *     get_participant();
 */
DDS_DomainParticipant
DDS_Publisher_get_participant (
    DDS_Publisher this)
{
    return (DDS_DomainParticipant)
	gapi_publisher_get_participant (
	    (gapi_publisher)this);
}

/*     ReturnCode_t
 *     set_default_datawriter_qos(
 *         in DataWriterQos qos);
 */
DDS_ReturnCode_t
DDS_Publisher_set_default_datawriter_qos (
    DDS_Publisher this,
    const DDS_DataWriterQos *qos)
{
    return (DDS_ReturnCode_t)
	gapi_publisher_set_default_datawriter_qos (
	    (gapi_publisher)this,
	    (const gapi_dataWriterQos *)qos);
}

/*     ReturnCode_t
 *     get_default_datawriter_qos(
 *         inout DataWriterQos qos);
 */
DDS_ReturnCode_t
DDS_Publisher_get_default_datawriter_qos (
    DDS_Publisher this,
    DDS_DataWriterQos *qos)
{
    return (DDS_ReturnCode_t)
    gapi_publisher_get_default_datawriter_qos (
	(gapi_publisher)this,
	(gapi_dataWriterQos *)qos);
}

/*     ReturnCode_t
 *     copy_from_topic_qos(
 *         inout DataWriterQos a_datawriter_qos,
 *         in TopicQos a_topic_qos);
 */
DDS_ReturnCode_t
DDS_Publisher_copy_from_topic_qos (
    DDS_Publisher this,
    DDS_DataWriterQos *a_datawriter_qos,
    const DDS_TopicQos *a_topic_qos)
{
    return (DDS_ReturnCode_t)
	gapi_publisher_copy_from_topic_qos (
	    (gapi_publisher)this,
	    (gapi_dataWriterQos *)a_datawriter_qos,
	    (const gapi_topicQos *)a_topic_qos);
}

