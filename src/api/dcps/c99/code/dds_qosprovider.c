/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#include <dds_dcps.h>
#include <dds.h>
#include <dds__qos.h>


/* The QoSProvider is created given the file uri and the profile within the file
 *  Arguments :
 *   -# qp  The QosProvider
 *   -# uri _the file uri (absolute path) to th QoS XML file
 *   -# profile the profile within the file
 *   -# Returns _Success_(return == 0)
 */
int dds_qosprovider_create(dds_entity_t * qp, const char *uri, const char *profile)
{
	int result = DDS_RETCODE_OK;

	if(uri == NULL) {
		result = DDS_RETCODE_ERROR;
	} else if((*qp = DDS_QosProvider__alloc(uri, profile)) == NULL) {
		result = DDS_RETCODE_ERROR;
	}

	return result;
}

/* Given the QoSProvider, populate the created QoS
 *  Arguments :
 *   -# qp  the QoSProvider
 *   -# qos the QoS to be populated. Qos must be created at this point.
 *   -# id
 *   -# Returns _Success_(return == 0)
 */
int dds_qosprovider_get_participant_qos(dds_entity_t qp, dds_qos_t *qos, const char *id)
{
	int result = DDS_RETCODE_OK;

	DDS_DomainParticipantQos* participantQos = NULL;
	if((participantQos = DDS_DomainParticipantQos__alloc()) == NULL) {
		result = DDS_RETCODE_ERROR;
	} else if((result = DDS_QosProvider_get_participant_qos(qp, participantQos, id)) == DDS_RETCODE_OK) {
		/* copy the participantQos values to qos */
        dds_qos_from_participant_qos( qos, participantQos);
        DDS_free (participantQos);
	}

	return result;
}

/* Given the QoSProvider, populate the created QoS
 *  Arguments :
 *   -# qp  the QoSProvider
 *   -# qos the QoS to be populated. Qos must be created at this point.
 *   -# id
 *   -# Returns _Success_(return == 0)
 */
int dds_qosprovider_get_topic_qos(dds_entity_t qp, dds_qos_t *qos, const char *id)
{
	int result = DDS_RETCODE_OK;

	DDS_TopicQos* topicQos = NULL;
	if((topicQos = DDS_TopicQos__alloc()) == NULL) {
		result = DDS_RETCODE_ERROR;
	} else if((result = DDS_QosProvider_get_topic_qos(qp, topicQos, id)) == DDS_RETCODE_OK) {
		/* copy the topicQos values to qos */
        dds_qos_from_topic_qos( qos, topicQos);
        DDS_free (topicQos);
	}

	return result;
}

/* Given the QoSProvider, populate the created QoS
 *  Arguments :
 *   -# qp  the QoSProvider
 *   -# qos the QoS to be populated. Qos must be created at this point.
 *   -# id
 *   -# Returns _Success_(return == 0)
 */
int dds_qosprovider_get_subscriber_qos(dds_entity_t qp, dds_qos_t *qos, const char *id)
{
	int result = DDS_RETCODE_OK;

	DDS_SubscriberQos* subscriberQos = NULL;
	if((subscriberQos = DDS_SubscriberQos__alloc()) == NULL) {
		result = DDS_RETCODE_ERROR;
	} else if((result = DDS_QosProvider_get_subscriber_qos(qp, subscriberQos, id)) == DDS_RETCODE_OK) {
		/* copy the subscriberQos values to qos */
        dds_qos_from_subscriber_qos( qos, subscriberQos);
        DDS_free (subscriberQos);
	}

	return result;
}

/* Given the QoSProvider, populate the created QoS
 *  Arguments :
 *   -# qp  the QoSProvider
 *   -# qos the QoS to be populated. Qos must be created at this point.
 *   -# id
 *   -# Returns _Success_(return == 0)
 */
int dds_qosprovider_get_reader_qos(dds_entity_t qp, dds_qos_t *qos, const char *id)
{
	int result = DDS_RETCODE_OK;

	DDS_DataReaderQos* readerQos = NULL;
	if((readerQos = DDS_DataReaderQos__alloc()) == NULL) {
		result = DDS_RETCODE_ERROR;
	} else if((result = DDS_QosProvider_get_datareader_qos(qp, readerQos, id)) == DDS_RETCODE_OK) {
		/* copy the readerQos values to qos */
        dds_qos_from_reader_qos( qos, readerQos);
        DDS_free (readerQos);
	}

	return result;
}

/* Given the QoSProvider, populate the created QoS
 *  Arguments :
 *   -# qp  the QoSProvider
 *   -# qos the QoS to be populated. Qos must be created at this point.
 *   -# id
 *   -# Returns _Success_(return == 0)
 */
int dds_qosprovider_get_writer_qos(dds_entity_t qp, dds_qos_t *qos, const char *id)
{
	int result = DDS_RETCODE_OK;

	DDS_DataWriterQos* writerQos = NULL;
	if((writerQos = DDS_DataWriterQos__alloc()) == NULL) {
		result = DDS_RETCODE_ERROR;
	} else if((result = DDS_QosProvider_get_datawriter_qos(qp, writerQos, id)) == DDS_RETCODE_OK) {
		/* copy the writerQos values to qos */
        dds_qos_from_writer_qos( qos, writerQos);
        DDS_free (writerQos);
	}

	return result;
}

/* Given the QoSProvider, populate the created QoS
 *  Arguments :
 *   -# qp  the QoSProvider
 *   -# qos the QoS to be populated. Qos must be created at this point.
 *   -# id
 *   -# Returns _Success_(return == 0)
 */
int dds_qosprovider_get_publisher_qos(dds_entity_t qp, dds_qos_t *qos, const char *id)
{
	int result = DDS_RETCODE_OK;

	DDS_PublisherQos* publisherQos = NULL;
	if((publisherQos = DDS_PublisherQos__alloc()) == NULL) {
		return 1;
	} else if((result = DDS_QosProvider_get_publisher_qos(qp, publisherQos, id)) == DDS_RETCODE_OK) {
		/* copy the publisherQos values to qos */
        dds_qos_from_publisher_qos( qos, publisherQos);
        DDS_free (publisherQos);
	}

	return result;
}
