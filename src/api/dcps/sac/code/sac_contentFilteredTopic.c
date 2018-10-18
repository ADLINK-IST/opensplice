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

#include "dds_dcps.h"
#include "sac_common.h"
#include "sac_object.h"

#include "sac_topicDescription.h"
#include "sac_contentFilteredTopic.h"
#include "sac_topic.h"

#include "sac_report.h"

#define DDS_ContentFilteredTopicClaim(_this, topic) \
        DDS_Object_claim(DDS_Object(_this), DDS_CONTENTFILTEREDTOPIC, (_Object *)topic)

#define DDS_ContentFilteredTopicClaimRead(_this, topic) \
        DDS_Object_claim(DDS_Object(_this), DDS_CONTENTFILTEREDTOPIC, (_Object *)topic)

#define DDS_ContentFilteredTopicRelease(_this) \
        DDS_Object_release(DDS_Object(_this))

#define DDS_ContentFilteredTopicCheck(_this, topic) \
        DDS_Object_check_and_assign(DDS_Object(_this), DDS_CONTENTFILTEREDTOPIC, (_Object *)topic)

#define _ContentFilteredTopic_get_user_entity(_this) \
        u_topic(_Entity_get_user_entity(_Entity(_ContentFilteredTopic(_this))))

static DDS_ReturnCode_t
_ContentFilteredTopic_deinit (
    _Object _this)
{
    DDS_ReturnCode_t result;
    _ContentFilteredTopic topic;

    topic = _ContentFilteredTopic(_this);

    /* the DDS_TopicDescription_deinit is called before destroying the sub-class.
     * This is not the opposite of the construction as expected but this is done because
     * the DDS_TopicDescription_deinit also checks if any associated readers exist and
     * doing it the wrong way around doesn't matter in this case.
     */
    result = DDS_TopicDescription_deinit(_this);
    if (result == DDS_RETCODE_OK) {
        result = DDS_TopicDescription_free(topic->relatedTopic);
        DDS_free(topic->expression);
        DDS_free(topic->parameters);
    }
    return result;
}


DDS_ContentFilteredTopic
DDS_ContentFilteredTopicNew (
    DDS_DomainParticipant participant,
    const DDS_char *topic_name,
    const DDS_Topic related_topic,
    const DDS_char *filter_expression,
    const DDS_StringSeq *filter_parameters)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    DDS_TypeSupport typeSupport;
    DDS_char *expression = NULL;
    DDS_char *related_topic_name = NULL;
    DDS_char *related_type_name = NULL;
    size_t length;
    DDS_TopicQos qos;
    _ContentFilteredTopic topic = NULL;
    const DDS_char *format = "select * from %s where %s";

    if (topic_name == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "Topic_name = NULL");
    }
    if (related_topic == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "Related_topic = NULL");
    }
    if (filter_expression == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "Filter_expression = NULL");
    }
    if (filter_parameters == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "Filter_parameters = NULL");
    }
    if (participant == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "Participant = NULL");
    }
    if (result == DDS_RETCODE_OK) {
        related_topic_name = DDS_TopicDescription_get_name(DDS_TopicDescription(related_topic));
        related_type_name = DDS_TopicDescription_get_type_name(DDS_TopicDescription(related_topic));
        length = strlen(format) + strlen(related_topic_name) + strlen(filter_expression) + 1;
        expression = (char *) os_malloc(length);

        if ( expression ) {
            snprintf(expression, length, format, related_topic_name, filter_expression);
            result = DDS_TopicDescription_get_typeSupport_locked_dp(related_topic, &typeSupport);
            if (result == DDS_RETCODE_OK) {
                result = DDS_Topic_get_qos(related_topic, &qos);
            }
        } else {
            result = DDS_RETCODE_OUT_OF_RESOURCES;
            SAC_REPORT(result, "Could not allocate expression of size %d", length);
        }
    }
    if (result == DDS_RETCODE_OK) {
        result = DDS_Topic_validate_filter(related_topic, filter_expression, filter_parameters);
    }
    if (result == DDS_RETCODE_OK) {
        result = DDS_Object_new(DDS_CONTENTFILTEREDTOPIC,
                                _ContentFilteredTopic_deinit,
                                (_Object *)&topic);

        if (result == DDS_RETCODE_OK) {
            DDS_Object_set_domain_id(_Object(topic), DDS_Object_get_domain_id(related_topic));
            result = DDS_TopicDescription_init(topic, participant,
                                               topic_name, related_type_name, expression,
                                               typeSupport, NULL);
        }

        if (result == DDS_RETCODE_OK) {
            topic->relatedTopic = DDS_TopicDescription_keep(DDS_TopicDescription(related_topic));
            topic->expression = DDS_string_dup(filter_expression);
            topic->parameters = DDS_StringSeq_dup(filter_parameters);
        }
    }
    os_free(expression);
    DDS_free(related_topic_name);
    DDS_free(related_type_name);

    return (DDS_Topic)topic;
}

/* string
 * get_filter_expression();
 */
DDS_string
DDS_ContentFilteredTopic_get_filter_expression (
    DDS_ContentFilteredTopic _this)
{
    DDS_ReturnCode_t result;
    _ContentFilteredTopic topic;
    DDS_string expr = NULL;

    SAC_REPORT_STACK();

    result = DDS_ContentFilteredTopicCheck(_this, &topic);
    if (result == DDS_RETCODE_OK) {
        expr = DDS_string_dup(topic->expression);
    }

    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);

    return expr;
}

/* StringSeq
 * get_expression_parameters();
 */
DDS_ReturnCode_t
DDS_ContentFilteredTopic_get_expression_parameters (
    DDS_ContentFilteredTopic _this,
    DDS_StringSeq *expression_parameters)
{
    DDS_ReturnCode_t result;
    _ContentFilteredTopic topic;

    SAC_REPORT_STACK();

    result = DDS_ContentFilteredTopicClaimRead(_this, &topic);
    if (result == DDS_RETCODE_OK) {
        DDS_sequence_clean((_DDS_sequence) expression_parameters);
        result = DDS_StringSeq_init(expression_parameters, topic->parameters);
        DDS_ContentFilteredTopicRelease(_this);
    }

    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);

    return result;
}

/* ReturnCode_t
 * set_expression_parameters(
 *      in StringSeq expression_parameters);
 */
DDS_ReturnCode_t
DDS_ContentFilteredTopic_set_expression_parameters (
    DDS_ContentFilteredTopic _this,
    const DDS_StringSeq *expression_parameters)
{
    OS_UNUSED_ARG(_this);
    OS_UNUSED_ARG(expression_parameters);

    return DDS_RETCODE_UNSUPPORTED;
}

DDS_ReturnCode_t
DDS_ContentFilteredTopic_get_parameters (
    DDS_ContentFilteredTopic _this,
    c_value **params,
    DDS_unsigned_long *nrOfParams)
{
    DDS_ReturnCode_t result;
    _ContentFilteredTopic topic;
    DDS_StringSeq *parms;
    c_ulong n;

    SAC_REPORT_STACK();

    if (params != NULL) {
        result = DDS_ContentFilteredTopicClaimRead(_this, &topic);
        if (result == DDS_RETCODE_OK) {
            parms = topic->parameters;
            if(parms->_length > 0) {
                *params = (c_value *)os_malloc(parms->_length * sizeof(struct c_value));
                for (n=0;n<parms->_length;n++) {
                    (*params)[n] = c_stringValue(parms->_buffer[n]);
                }
                *nrOfParams = parms->_length;
            } else {
                *params = NULL;
                *nrOfParams = 0;
            }
            DDS_ContentFilteredTopicRelease(_this);
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
    }

    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);

    return result;
}

/* Topic
 * get_related_topic();
 */
DDS_Topic
DDS_ContentFilteredTopic_get_related_topic (
    DDS_ContentFilteredTopic _this)
{
    DDS_ReturnCode_t result;
    _ContentFilteredTopic topic;
    DDS_Topic rt = NULL;

    SAC_REPORT_STACK();

    result = DDS_ContentFilteredTopicCheck(_this, &topic);
    if (result == DDS_RETCODE_OK) {
        rt = topic->relatedTopic;
    }

    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);

    return rt;
}

