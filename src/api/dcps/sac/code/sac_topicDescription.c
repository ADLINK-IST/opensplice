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
#include "sac_objManag.h"
#include "sac_object.h"
#include "sac_entity.h"
#include "sac_typeSupport.h"
#include "sac_topicDescription.h"
#include "sac_domainParticipant.h"
#include "sac_domainParticipantFactory.h"
#include "u_topic.h"
#include "sac_report.h"

/* DDS_parseExpression() is part of the generated sac_parser.c (see sac_parser.y).
 * Including sac_parser.h (generated from sac_parser.l) triggers a whole lot of compiler errros.
 * Simplest solution is to indicate the function as extern.
 */
extern q_expr DDS_parseExpression (const char *queryString);

#define DDS_TopicDescriptionClaim(_this, topic) \
        DDS_Object_claim(DDS_Object(_this), DDS_TOPICDESCRIPTION, (_Object *)topic)

#define DDS_TopicDescriptionClaimRead(_this, topic) \
        DDS_Object_claim(DDS_Object(_this), DDS_TOPICDESCRIPTION, (_Object *)topic)

#define DDS_TopicDescriptionRelease(_this) \
        DDS_Object_release(DDS_Object(_this))

#define DDS_TopicDescriptionCheck(_this, topic) \
        DDS_Object_check_and_assign(DDS_Object(_this), DDS_TOPICDESCRIPTION, (_Object *)topic)

#define _TopicDescription_get_user_entity(_this) \
        u_topic(_Entity_get_user_entity(_Entity(_TopicDescription(_this))))

DDS_ReturnCode_t
DDS_TopicDescription_deinit (
    DDS_TopicDescription _this)
{
    DDS_ReturnCode_t result;
    _TopicDescription td;

    td = _TopicDescription(_this);
    if (td != NULL) {
        if (td->refCount == 0) {
            td->participant = NULL;
            DDS_free(td->type_name);
            DDS_free(td->typeSupport);
            DDS_free(td->topic_name);
            DDS_free(td->expression);
            result = _Entity_deinit(_this);
        } else {
            result = DDS_RETCODE_PRECONDITION_NOT_MET;
            SAC_REPORT(result, "%d Entities are still using this Topic",
                        td->refCount);
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "TopicDescription = NULL");
    }
    return result;
}

DDS_ReturnCode_t
DDS_TopicDescription_init (
    DDS_TopicDescription _this,
    const DDS_DomainParticipant participant,
    const DDS_char *topic_name,
    const DDS_char *type_name,
    const DDS_char *expression,
    const DDS_TypeSupport typeSupport,
    const u_topic uTopic)
{
    DDS_ReturnCode_t result;

    result = DDS_Entity_init(_this, u_entity(uTopic));
    if (result == DDS_RETCODE_OK) {
        _TopicDescription(_this)->participant = participant;
        _TopicDescription(_this)->type_name = DDS_string_dup(type_name);
        _TopicDescription(_this)->typeSupport = DDS_TypeSupportKeep(typeSupport);
        _TopicDescription(_this)->topic_name = DDS_string_dup(topic_name);
        _TopicDescription(_this)->expression = DDS_string_dup(expression);
        _TopicDescription(_this)->refCount = 0;
    }
    return result;
}

DDS_TopicDescription
DDS_TopicDescription_keep(
    DDS_TopicDescription _this)
{
    DDS_ReturnCode_t result;
    DDS_TopicDescription t = NULL;
    _TopicDescription td;

    result = DDS_TopicDescriptionClaim(_this, &td);
    if (result == DDS_RETCODE_OK) {
        if (td->refCount >= 0) {
            td->refCount++;
            t = _this;
        } else {
            SAC_PANIC(
                "Object state corrupted, reference count '%ld'.", td->refCount);
        }
        DDS_TopicDescriptionRelease(_this);
    }
    return t;
}

DDS_ReturnCode_t
DDS_TopicDescription_free (
    DDS_TopicDescription _this)
{
    DDS_ReturnCode_t result;
    _TopicDescription td;

    result = DDS_TopicDescriptionClaim(_this, &td);
    if (result == DDS_RETCODE_OK) {
        if (td->refCount > 0) {
            td->refCount--;
        } else {
            SAC_PANIC(
                "Object state corrupted, reference count '%ld'.", td->refCount);
        }
        DDS_TopicDescriptionRelease(_this);
    }
    return result;
}

/*     string
 *     get_type_name();
 */
DDS_string
DDS_TopicDescription_get_type_name (
    DDS_TopicDescription _this)
{
    DDS_ReturnCode_t result;
    _TopicDescription topic;
    DDS_string name = NULL;

    SAC_REPORT_STACK();

    result = DDS_TopicDescriptionCheck(_this, &topic);
    if (result == DDS_RETCODE_OK) {
#if 1
        if (topic->type_name != NULL ) {
            name = DDS_string_dup(topic->type_name);
        } else {
            name = NULL;
        }
#else
        name = u_topicTypeName(_TopicDescription_get_user_entity(topic));
#endif
    }

    SAC_REPORT_FLUSH(_this, name == NULL);

    return name;
}

/*     string
 *     get_name();
 */
DDS_string
DDS_TopicDescription_get_name (
    DDS_TopicDescription _this)
{
    DDS_ReturnCode_t result;
    _TopicDescription topic;
    DDS_string name = NULL;

    SAC_REPORT_STACK();

    result = DDS_TopicDescriptionCheck(_this, &topic);
    if (result == DDS_RETCODE_OK) {
        if (topic->topic_name != NULL) {
            name = DDS_string_dup(topic->topic_name);
        }
    }

    SAC_REPORT_FLUSH(_this, name == NULL);

    return name;
}

/*     DomainParticipant
 *     get_participant();
 */
DDS_DomainParticipant
DDS_TopicDescription_get_participant (
    DDS_TopicDescription _this)
{
    DDS_ReturnCode_t result;
    _TopicDescription topic;
    DDS_DomainParticipant participant = NULL;

    SAC_REPORT_STACK();

    result = DDS_TopicDescriptionCheck(_this, &topic);
    if (result == DDS_RETCODE_OK) {
        participant = topic->participant;
    }

    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);

    return participant;
}

DDS_ReturnCode_t
DDS_TopicDescription_get_typeSupport_locked_dp (
    DDS_TopicDescription _this,
    DDS_TypeSupport *typeSupport)
{
    DDS_ReturnCode_t result;
    _TopicDescription topic;

    if (typeSupport != NULL) {
        result = DDS_TopicDescriptionCheck(_this, &topic);
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
    }
    if (result == DDS_RETCODE_OK) {
        if (topic->typeSupport == NULL) {
            result = DDS_DomainParticipant_find_type_locked(topic->participant,
                                                            topic->type_name,
                                                            &topic->typeSupport);
        }
        *typeSupport = topic->typeSupport;
    }
    return result;
}

DDS_ReturnCode_t
DDS_TopicDescription_get_typeSupport (
    DDS_TopicDescription _this,
    DDS_TypeSupport *typeSupport)
{
    DDS_ReturnCode_t result;
    _TopicDescription topic;

    if (typeSupport != NULL) {
        result = DDS_TopicDescriptionCheck(_this, &topic);
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
    }
    if (result == DDS_RETCODE_OK) {
        if (topic->typeSupport == NULL) {
            result = DDS_DomainParticipant_find_type(topic->participant,
                                                     topic->type_name,
                                                     typeSupport);
            topic->typeSupport = DDS_TypeSupportKeep(*typeSupport);
        } else {
            *typeSupport = topic->typeSupport;
        }
    }
    return result;
}

os_char *
DDS_TopicDescription_get_expr (
    DDS_TopicDescription _this)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    _TopicDescription td;
    os_char *expression = NULL;

    result = DDS_TopicDescriptionClaimRead(_this, &td);
    if (result == DDS_RETCODE_OK) {
        expression = os_strdup(td->expression);
        DDS_TopicDescriptionRelease(_this);
    }
    return expression;
}

DDS_ReturnCode_t
DDS_TopicDescription_get_qos (
    DDS_TopicDescription _this,
    DDS_TopicQos *qos)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    _TopicDescription td;
    u_topic uTopic;
    u_topicQos uTopicQos;
    u_result uResult;

    result = DDS_TopicDescriptionClaimRead (_this, &td);
    if (result == DDS_RETCODE_OK) {
        if (qos == NULL) {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "TopicQos = NULL");
        } else if (qos == DDS_TOPIC_QOS_DEFAULT) {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "QoS 'TOPIC_QOS_DEFAULT' is read-only.");
        }
    }
    if (result == DDS_RETCODE_OK) {
        uTopic = _TopicDescription_get_user_entity(td);
        uResult = u_topicGetQos(uTopic, &uTopicQos);
        result = DDS_ReturnCode_get(uResult);

        if (result == DDS_RETCODE_OK) {
            result = DDS_TopicQos_copyOut(uTopicQos, qos);
            u_topicQosFree(uTopicQos);
        }

        DDS_TopicDescriptionRelease(_this);
    }

    return result;
}
