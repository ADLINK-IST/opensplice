/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
#include "sac_topic.h"
#include "sac_typeSupport.h"
#include "sac_contentFilteredTopic.h"
#include "sac_multiTopic.h"
#include "sac_publisher.h"
#include "sac_subscriber.h"
#include "sac_dataReader.h"
#include "sac_listenerDispatcher.h"
#include "sac_domainParticipant.h"
#include "sac_domainParticipantFactory.h"
#include "v_kernel.h"
#include "v_public.h"
#include "v_dataReaderInstance.h"
#include "u_observable.h"
#include "u_instanceHandle.h"
#include "u_reader.h"
#include "u_participant.h"
#include "u_topic.h"
#include "sac_report.h"
#include "c_stringSupport.h"

#define DDS_DomainParticipantClaim(_this, participant) \
        DDS_Object_claim(DDS_Object(_this), DDS_DOMAINPARTICIPANT, (_Object *)participant)

#define DDS_DomainParticipantClaimRead(_this, participant) \
        DDS_Object_claim(DDS_Object(_this), DDS_DOMAINPARTICIPANT, (_Object *)participant)

#define DDS_DomainParticipantRelease(_this) \
        DDS_Object_release(DDS_Object(_this))

#define DDS_DomainParticipantCheck(_this, participant) \
        DDS_Object_check_and_assign(DDS_Object(_this), DDS_DOMAINPARTICIPANT, (_Object *)participant)

#define DDS_DomainParticipant_get_user_entity(_this) \
        u_participant(_Entity_get_user_entity(_Entity(_DomainParticipant(_this))))

#define PARTICIPANT_STATUS_MASK (DDS_INCONSISTENT_TOPIC_STATUS         |\
                                 DDS_LIVELINESS_LOST_STATUS            |\
                                 DDS_OFFERED_DEADLINE_MISSED_STATUS    |\
                                 DDS_OFFERED_INCOMPATIBLE_QOS_STATUS   |\
                                 DDS_DATA_ON_READERS_STATUS            |\
                                 DDS_LIVELINESS_LOST_STATUS            |\
                                 DDS_DATA_AVAILABLE_STATUS             |\
                                 DDS_SAMPLE_REJECTED_STATUS            |\
                                 DDS_LIVELINESS_CHANGED_STATUS         |\
                                 DDS_REQUESTED_DEADLINE_MISSED_STATUS  |\
                                 DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS |\
                                 DDS_PUBLICATION_MATCHED_STATUS        |\
                                 DDS_SUBSCRIPTION_MATCHED_STATUS)

C_CLASS(_TypeSupportBinding);

C_STRUCT(_TypeSupportBinding) {
    DDS_char *type_name;
    DDS_TypeSupport *typeSupport;
};

static c_equality
compareBinding(
    c_voidp o,
    c_iterResolveCompareArg arg)
{
    _TypeSupportBinding binding = (_TypeSupportBinding)o;
    DDS_char *type_name = (DDS_char *)arg;
    c_equality result;

    assert(o != NULL);
    assert(type_name != NULL);

    if (strcmp(binding->type_name, type_name) == 0) {
        result = C_EQ;
    } else if (strcmp(_TypeSupport(binding->typeSupport)->type_name, type_name) == 0){
        result = C_EQ;
    } else {
        result = C_NE;
    }
    return result;
}

static DDS_ReturnCode_t
_DomainParticipant_deinit (
    _Object _this)
{
    DDS_ReturnCode_t result;
    DDS_Topic topic;
    _TypeSupportBinding binding;
    _DomainParticipant dp;

    dp = _DomainParticipant(_this);
    if (dp != NULL) {
        result = DDS_RETCODE_OK;
        if (c_iterLength(dp->publisherList) != 0) {
            result = DDS_RETCODE_PRECONDITION_NOT_MET;
            SAC_REPORT(result, "DomainParticipant has %d Publishers",
                        c_iterLength(dp->publisherList));
        }
        if (c_iterLength(dp->subscriberList) != 0) {
            result = DDS_RETCODE_PRECONDITION_NOT_MET;
            SAC_REPORT(result, "DomainParticipant has %d Subscribers",
                        c_iterLength(dp->subscriberList));
        }
        if (c_iterLength(dp->cfTopicList) != 0) {
            result = DDS_RETCODE_PRECONDITION_NOT_MET;
            SAC_REPORT(result, "DomainParticipant has %d ContentFilteredTopics",
                        c_iterLength(dp->cfTopicList));
        }
        if (c_iterLength(dp->multiTopicList) != 0) {
             result = DDS_RETCODE_PRECONDITION_NOT_MET;
             SAC_REPORT(result, "DomainParticipant has %d multi Topics",
                         c_iterLength(dp->multiTopicList));
        }
        if (c_iterLength(dp->topicList) != 0) {
            result = DDS_RETCODE_PRECONDITION_NOT_MET;
            SAC_REPORT(result, "DomainParticipant has %d Topics",
                        c_iterLength(dp->topicList));
        }
        if (result == DDS_RETCODE_OK) {
            result = DDS_SubscriberFree(dp->builtinSubscriber);
        }
        if (result == DDS_RETCODE_OK) {
            topic = c_iterTakeFirst(dp->builtinTopicList);
            while (topic != NULL) {
                result = DDS__free(topic);
                if (result != DDS_RETCODE_OK) {
                    SAC_PANIC("Could not free Topic");
                }
                topic = c_iterTakeFirst(dp->builtinTopicList);
            }
        }
        if (result == DDS_RETCODE_OK) {
            DDS_Entity_set_listener_interest (DDS_Entity(dp), 0);

            /* TODO : hack, breaks encapsulation. */
            if (_Entity (_this)->listenerDispatcher != NULL) {
                result = cmn_listenerDispatcher_free (
                    _Entity (_this)->listenerDispatcher);
                _Entity (_this)->listenerDispatcher = NULL;
            }
        }
        if (result == DDS_RETCODE_OK) {
            c_iterFree(dp->builtinTopicList);
            binding = c_iterTakeFirst(dp->typeSupportBindings);
            while (binding != NULL) {
                DDS_free(binding->typeSupport);
                os_free(binding->type_name);
                os_free(binding);
                binding = c_iterTakeFirst(dp->typeSupportBindings);
            }
            c_iterFree(dp->typeSupportBindings);
            DDS_free(dp->defaultPublisherQos);
            DDS_free(dp->defaultSubscriberQos);
            DDS_free(dp->defaultTopicQos);
            c_iterFree(dp->publisherList);
            c_iterFree(dp->subscriberList);
            c_iterFree(dp->cfTopicList);
            c_iterFree(dp->multiTopicList);
            c_iterFree(dp->topicList);
            DDS_free(dp->domainName);
            result = _Entity_deinit(_this);
            if (result != DDS_RETCODE_OK) {
                SAC_PANIC("Could not cleanup Entity");
            }
        }
    } else {
        assert(FALSE);
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "DomainParticipant = NULL");

    }
    return result;
}

DDS_DomainParticipant
DDS_DomainParticipantNew(
    const DDS_DomainParticipantFactory factory,
    const DDS_char *name,
    const DDS_DomainId_t domain_id,
    const DDS_DomainParticipantQos *qos)
{
    DDS_ReturnCode_t result;
    _DomainParticipant _this = NULL;
    u_participantQos pQos;
    u_participant uParticipant;
    c_ulong timeout = 1;

    result = DDS_DomainParticipantQos_is_consistent(qos);
    if (result == DDS_RETCODE_OK) {
        if (qos == DDS_PARTICIPANT_QOS_DEFAULT) {
            DDS_DomainParticipantQos _qos;
            result = DDS_DomainParticipantFactory_get_default_participant_qos(factory, &_qos);
            if (result == DDS_RETCODE_OK) {
                pQos = DDS_DomainParticipantQos_copyIn(&_qos);
                DDS_DomainParticipantQos_deinit(&_qos);
            }
        } else {
            pQos = DDS_DomainParticipantQos_copyIn(qos);
        }
    }
    if (result == DDS_RETCODE_OK) {
        uParticipant = u_participantNew(NULL, domain_id, timeout, name, pQos, FALSE);
        if (uParticipant != NULL) {
            result = DDS_Object_new(DDS_DOMAINPARTICIPANT,
                                    _DomainParticipant_deinit,
                                    (_Object *)&_this);
            if (result == DDS_RETCODE_OK) {
                result = DDS_Entity_init(_this, u_entity(uParticipant));
                DDS_Object_set_domain_id(_Object(_this), u_participantGetDomainId(uParticipant));
            }
        } else {
            result = DDS_RETCODE_PRECONDITION_NOT_MET;
        }
        u_participantQosFree(pQos);
    }
    if (result == DDS_RETCODE_OK) {
        _this->factory = factory;
        _this->domainName = NULL;

        _this->defaultPublisherQos = DDS_PublisherQos__alloc();
        if (_this->defaultPublisherQos != NULL) {
            result = DDS_PublisherQos_init(
                _this->defaultPublisherQos, DDS_PUBLISHER_QOS_DEFAULT);
        } else {
            result = DDS_RETCODE_OUT_OF_RESOURCES;
        }
    }

    if (result == DDS_RETCODE_OK) {
        _this->defaultSubscriberQos = DDS_SubscriberQos__alloc();
        if (_this->defaultSubscriberQos != NULL) {
            result = DDS_SubscriberQos_init(
                _this->defaultSubscriberQos, DDS_SUBSCRIBER_QOS_DEFAULT);
        } else {
            result = DDS_RETCODE_OUT_OF_RESOURCES;
        }

        _this->defaultTopicQos = DDS_TopicQos__alloc();
        if (_this->defaultTopicQos != NULL) {
            result = DDS_TopicQos_init(
                _this->defaultTopicQos, DDS_TOPIC_QOS_DEFAULT);
        } else {
            result = DDS_RETCODE_OUT_OF_RESOURCES;
        }

        _this->builtinSubscriber = NULL;
        _this->builtinTopicList = NULL;
        _this->publisherList = NULL;
        _this->subscriberList = NULL;
        _this->cfTopicList = NULL;
        _this->multiTopicList = NULL;
        _this->topicList = NULL;

        _this->factoryAutoEnable = qos->entity_factory.autoenable_created_entities;
    }

    if (result == DDS_RETCODE_OK) {
        cmn_listenerDispatcher dispatcher;
        os_schedClass scheduling_class;
        os_int32 scheduling_priority;

        scheduling_class = DDS_ListenerDispatcher_scheduling_class (
            &qos->listener_scheduling);
        scheduling_priority = DDS_ListenerDispatcher_scheduling_priority (
            &qos->listener_scheduling);

        dispatcher = cmn_listenerDispatcher_new (
            uParticipant,
            scheduling_class,
            scheduling_priority,
            &DDS_ListenerDispatcher_event_handler,
            _this, OS_TRUE);
        if (dispatcher != NULL) {
            result = DDS_Entity_set_listenerDispatcher (
                _this, dispatcher);
            if (result != DDS_RETCODE_OK) {
                (void)cmn_listenerDispatcher_free (dispatcher);
                dispatcher = NULL;
            }
        }
    }

    if (result != DDS_RETCODE_OK) {
        (void) DDS_DomainParticipantFree(_this);
        _this = NULL;
    }

    return (DDS_DomainParticipant)_this;
}

static DDS_ReturnCode_t
_DomainParticipant_delete_contained_entities (
    _DomainParticipant _this)
{
    DDS_ReturnCode_t result, endResult = DDS_RETCODE_OK;
    DDS_Publisher pub;
    DDS_Subscriber sub;
    DDS_Topic topic;
    DDS_ContentFilteredTopic cfTopic;
    DDS_MultiTopic multiTopic;
    c_iter reinsertList = NULL;

    pub = DDS_Publisher(c_iterTakeFirst(_this->publisherList));
    while (pub) {
        result = DDS_PublisherFree(pub);
        if (result != DDS_RETCODE_OK) {
            reinsertList = c_iterInsert(reinsertList, pub);
            endResult = result;
        }
        pub = DDS_Publisher(c_iterTakeFirst(_this->publisherList));
    }
    pub = DDS_Publisher(c_iterTakeFirst(reinsertList));
    while (pub) {
        c_iterInsert(_this->publisherList, pub);
        pub = DDS_Publisher(c_iterTakeFirst(reinsertList));
    }

    sub = DDS_Subscriber(c_iterTakeFirst(_this->subscriberList));
    while (sub) {
        result = DDS_SubscriberFree(sub);
        if (result != DDS_RETCODE_OK) {
            reinsertList = c_iterInsert(reinsertList, sub);
            endResult = result;
        }
        sub = DDS_Subscriber(c_iterTakeFirst(_this->subscriberList));
    }
    sub = DDS_Subscriber(c_iterTakeFirst(reinsertList));
    while (sub) {
        c_iterInsert(_this->subscriberList, sub);
        sub = DDS_Subscriber(c_iterTakeFirst(reinsertList));
    }

    multiTopic = DDS_MultiTopic(c_iterTakeFirst(_this->multiTopicList));
    while (multiTopic) {
        result = DDS__free(multiTopic);
        if (result != DDS_RETCODE_OK) {
            reinsertList = c_iterInsert(reinsertList, multiTopic);
            endResult = result;
        }
        multiTopic = DDS_MultiTopic(c_iterTakeFirst(_this->multiTopicList));
    }
    multiTopic = DDS_MultiTopic(c_iterTakeFirst(reinsertList));
    while (multiTopic) {
        c_iterInsert(_this->multiTopicList, multiTopic);
        multiTopic = DDS_MultiTopic(c_iterTakeFirst(reinsertList));
    }

    cfTopic = DDS_ContentFilteredTopic(c_iterTakeFirst(_this->cfTopicList));
    while (cfTopic) {
        result = DDS__free(cfTopic);
        if (result != DDS_RETCODE_OK) {
            reinsertList = c_iterInsert(reinsertList, cfTopic);
            endResult = result;
        }
        cfTopic = DDS_ContentFilteredTopic(c_iterTakeFirst(_this->cfTopicList));
    }
    cfTopic = DDS_ContentFilteredTopic(c_iterTakeFirst(reinsertList));
    while (cfTopic) {
        c_iterInsert(_this->cfTopicList, cfTopic);
        cfTopic = DDS_ContentFilteredTopic(c_iterTakeFirst(reinsertList));
    }

    topic = DDS_Topic(c_iterTakeFirst(_this->topicList));
    while (topic) {
        result = DDS__free(topic);
        if (result != DDS_RETCODE_OK) {
            reinsertList = c_iterInsert(reinsertList, topic);
            endResult = result;
        }
        topic = DDS_Topic(c_iterTakeFirst(_this->topicList));
    }
    topic = DDS_Topic(c_iterTakeFirst(reinsertList));
    while (topic) {
        c_iterInsert(_this->topicList, topic);
        topic = DDS_Topic(c_iterTakeFirst(reinsertList));
    }

    c_iterFree(reinsertList);

    return endResult;
}

DDS_ReturnCode_t
DDS_DomainParticipantFree (
    DDS_DomainParticipant _this)
{
    DDS_ReturnCode_t result;
    _DomainParticipant dp;

    result = DDS_DomainParticipantClaim(_this, &dp);
    if (result == DDS_RETCODE_OK) {
        result = _DomainParticipant_delete_contained_entities(dp);
        if (result == DDS_RETCODE_OK) {
            result = _DomainParticipant_deinit(_Object(_this));
        }
        if (result != DDS_RETCODE_OK) {
            DDS_DomainParticipantRelease(_this);
        }
    }
    return result;
}

static os_char*
_DomainParticipant_child_name (
    _DomainParticipant _this,
    const char* prefix)
{
    os_char *child = NULL;
    os_char  pid[25];
    os_char *name;
    os_char *end;
    int length;

    assert(_this);
    assert(prefix);

    /* Get the participants' name. */
    name = u_entityName(u_entity(DDS_DomainParticipant_get_user_entity(_this)));
    if (name) {
        /* Remove possibly " <pid>" name extension. */
        length = (int)strlen(name);
        if (length > 3) {
            if (name[length - 1] == '>') {
                snprintf(pid, 25, " <%d>", os_procIdSelf());
                end = strstr(name, pid);
                if (end) {
                    *end = '\0';
                }
            }
        }

        /* Merge prefix and name into the childs' name. */
        length = (int)strlen(prefix) + (int)strlen(name) + 4 /* " <>\0"*/;
        child = (os_char*)os_malloc(length);
        if (child) {
            snprintf(child, length, "%s <%s>", prefix, name);
        }

        os_free(name);
    }

    return child;
}


/*     Publisher
 *     create_publisher(
 *         in PublisherQos qos,
 *         in PublisherListener a_listener);
 */
DDS_Publisher
DDS_DomainParticipant_create_publisher (
    DDS_DomainParticipant _this,
    const DDS_PublisherQos *qos,
    const struct DDS_PublisherListener *a_listener,
    const DDS_StatusMask mask)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    DDS_Publisher pub = NULL;
    _DomainParticipant dp;
    DDS_boolean autoEnable;
    os_char *name;

    SAC_REPORT_STACK();

    result = DDS_PublisherQos_is_consistent (qos);
    if (result == DDS_RETCODE_OK) {
        result = DDS_DomainParticipantClaim(_this, &dp);
    }
    if (result == DDS_RETCODE_OK) {
        if (qos == DDS_PUBLISHER_QOS_DEFAULT) {
            qos = dp->defaultPublisherQos;
        }
        autoEnable = dp->factoryAutoEnable;
        name = _DomainParticipant_child_name(dp, "publisher");
        pub = DDS_PublisherNew (_this, name, qos);
        os_free(name);
        dp->publisherList = c_iterInsert (dp->publisherList, pub);
        DDS_DomainParticipantRelease(_this);
        if (pub) {
            result = DDS_Publisher_set_listener(pub, a_listener, mask);
            if (result == DDS_RETCODE_OK) {
                if (autoEnable) {
                    result = DDS_Entity_enable(pub);
                }
            }
            if (result != DDS_RETCODE_OK) {
                if (DDS_DomainParticipantClaim(_this, &dp) == DDS_RETCODE_OK) {
                    (void)c_iterTake(dp->publisherList, pub);
                    DDS_DomainParticipantRelease(_this);
                    (void)DDS__free(pub);
                }
                pub = NULL;
            }
        } else {
            result = DDS_RETCODE_ERROR;
        }
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return pub;
}

/*     ReturnCode_t
 *     delete_publisher(
 *         in Publisher p);
 */
DDS_ReturnCode_t
DDS_DomainParticipant_delete_publisher (
    DDS_DomainParticipant _this,
    const DDS_Publisher pub)
{
    DDS_ReturnCode_t result;
    DDS_Publisher found;
    _DomainParticipant dp;

    SAC_REPORT_STACK();

    if (pub != NULL) {
        result = DDS_DomainParticipantClaim(_this, &dp);
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "Publisher = NULL");
    }
    if (result == DDS_RETCODE_OK) {
        found = c_iterTake(dp->publisherList, pub);
        if (found != pub) {
            if (DDS_Object_get_kind(DDS_Object(pub)) == DDS_PUBLISHER) {
                result = DDS_RETCODE_PRECONDITION_NOT_MET;
                SAC_REPORT(result, "Publisher does not belong to this DomainParticipant");
            } else {
                result = DDS_RETCODE_BAD_PARAMETER;
                SAC_REPORT(result, "Publisher parameter 'pub' is of type %s",
                            DDS_ObjectKind_image(DDS_Object_get_kind(DDS_Object(pub))));
            }
        } else {
            result = DDS__free(pub);
        }
        if ((found) && (result != DDS_RETCODE_OK)) {
            (void)c_iterInsert(dp->publisherList, found);
        }
        (void)DDS_DomainParticipantRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     Subscriber
 *     create_subscriber(
 *         in SubscriberQos qos,
 *         in SubscriberListener a_listener);
 */
DDS_Subscriber
DDS_DomainParticipant_create_subscriber (
    DDS_DomainParticipant _this,
    const DDS_SubscriberQos *qos,
    const struct DDS_SubscriberListener *a_listener,
    const DDS_StatusMask mask)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    DDS_Subscriber sub = NULL;
    _DomainParticipant dp;
    DDS_boolean autoEnable;
    os_char *name;

    SAC_REPORT_STACK();

    if (qos != DDS_SUBSCRIBER_QOS_DEFAULT) {
        result = DDS_SubscriberQos_is_consistent(qos);
    }

    if (result == DDS_RETCODE_OK) {
        result = DDS_DomainParticipantClaim(_this, &dp);
    }
    if (result == DDS_RETCODE_OK) {
        if (qos == DDS_SUBSCRIBER_QOS_DEFAULT) {
            qos = dp->defaultSubscriberQos;
        }
        autoEnable = dp->factoryAutoEnable;
        name = _DomainParticipant_child_name(dp, "subscriber");
        sub = DDS_SubscriberNew (_this, name, qos);
        os_free(name);
        dp->subscriberList = c_iterInsert (dp->subscriberList, sub);
        DDS_DomainParticipantRelease(_this);
        if (sub) {
            result = DDS_Subscriber_set_listener(sub, a_listener, mask);
            if (result == DDS_RETCODE_OK) {
                if (autoEnable) {
                    result = DDS_Entity_enable(sub);
                }
            }
            if (result != DDS_RETCODE_OK) {
                if (DDS_DomainParticipantClaim(_this, &dp) == DDS_RETCODE_OK) {
                    (void)c_iterTake(dp->subscriberList, sub);
                    DDS_DomainParticipantRelease(_this);
                    (void)DDS__free(sub);
                }
                sub = NULL;
            }

        } else {
            result = DDS_RETCODE_ERROR;
        }
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return sub;
}

/*     ReturnCode_t
 *     delete_subscriber(
 *         in Subscriber s);
 */
DDS_ReturnCode_t
DDS_DomainParticipant_delete_subscriber (
    DDS_DomainParticipant _this,
    const DDS_Subscriber sub)
{
    DDS_ReturnCode_t result;
    DDS_Subscriber found;
    _DomainParticipant dp;

    SAC_REPORT_STACK();

    if (sub != NULL) {
        result = DDS_DomainParticipantClaim(_this, &dp);
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "Subscriber = NULL");
    }
    if (result == DDS_RETCODE_OK) {
        if(sub == dp->builtinSubscriber){
            result = DDS_Subscriber_delete_contained_entities(sub);

            if(result == DDS_RETCODE_OK){
                result = DDS__free(sub);

                if(result == DDS_RETCODE_OK){
                    dp->builtinSubscriber = NULL;
                }
            }
        } else {
            found = c_iterTake(dp->subscriberList, sub);
            if (found != sub) {
                /* The following call is expensive so only use it in case of exceptions. */
                if (DDS_Object_get_kind(DDS_Object(sub)) == DDS_SUBSCRIBER) {
                    result = DDS_RETCODE_PRECONDITION_NOT_MET;
                    SAC_REPORT(result, "Subscriber does not belong to this DomainParticipant");
                } else {
                    result = DDS_RETCODE_BAD_PARAMETER;
                    SAC_REPORT(result, "Subscriber parameter 'sub' is of type %s",
                                DDS_ObjectKind_image(DDS_Object_get_kind(DDS_Object(sub))));
                }
            } else {
                result = DDS__free(sub);
            }
            if ((found) && (result != DDS_RETCODE_OK)) {
                c_iterInsert(dp->subscriberList, found);
            }
        }
        DDS_DomainParticipantRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

static void
initBuiltinSubscriberQos(
    DDS_SubscriberQos *sQos)
{
    *sQos = DDS_SubscriberQos_default;
    sQos->share.name = DDS_string_dup(DDS_SubscriberQos_default.share.name);
    sQos->presentation.access_scope = DDS_TOPIC_PRESENTATION_QOS;
    DDS_string_to_StringSeq("__BUILT-IN PARTITION__",
                            ",",
                            &sQos->partition.name);
}

static c_equality
compareTopicName (
    c_voidp o,
    c_iterResolveCompareArg arg)
{
    c_equality equality = C_NE;
    _TopicDescription descr = (_TopicDescription) o;
    const DDS_char *topic_name = (const DDS_char *) arg;
    DDS_string name;

    name = DDS_TopicDescription_get_name(descr);
    if (name && topic_name) {
        if (strcmp(name, topic_name) == 0) {
            equality = C_EQ;
        }
    }
    DDS_free(name);

    return equality;
}

/*     Subscriber
 *     get_builtin_subscriber();
 */
DDS_Subscriber
DDS_DomainParticipant_get_builtin_subscriber (
    DDS_DomainParticipant _this)
{
    DDS_ReturnCode_t result;
    DDS_Subscriber sub = NULL;
    _DomainParticipant dp;
    DDS_SubscriberQos *sQos;

    SAC_REPORT_STACK();

    result = DDS_DomainParticipantClaim(_this, &dp);
    if (result == DDS_RETCODE_OK) {
        if ( _Entity_is_enabled(_Entity(dp))) {
            if ( dp->builtinSubscriber == NULL ) {
                sQos = DDS_SubscriberQos__alloc();
                if (sQos) {
                    initBuiltinSubscriberQos(sQos);
                    dp->builtinSubscriber = DDS_SubscriberNew(_this,
                                                              "BuiltinSubscriber",
                                                              sQos);
                    /* Now check factory enable policy and enable the subscriber */
                    if (dp->factoryAutoEnable) {
                        DDS_Entity_enable(dp->builtinSubscriber);
                    }
                    DDS_free(sQos);
                } else {
                    result = DDS_RETCODE_OUT_OF_RESOURCES;
                    SAC_REPORT(result, "Failed to allocate heap memory of size %d",
                                sizeof(DDS_SubscriberQos));
                }
            }
            sub = dp->builtinSubscriber;
        } else {
            result = DDS_RETCODE_PRECONDITION_NOT_MET;
            SAC_REPORT(result, "DomainParticipant is not enabled");
        }
        DDS_DomainParticipantRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return sub;
}

/*     Topic
 *     create_topic(
 *         in string topic_name,
 *         in string type_name,
 *         in TopicQos qos,
 */
DDS_Topic
DDS_DomainParticipant_create_topic (
    DDS_DomainParticipant _this,
    const DDS_char *topic_name,
    const DDS_char *type_name,
    const DDS_TopicQos *qos,
    const struct DDS_TopicListener *a_listener,
    const DDS_StatusMask mask)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    _DomainParticipant dp;
    u_topicQos tQos;
    DDS_char *typeName;
    DDS_char *key_list;
    u_topic uTopic;
    u_participant uParticipant;
    DDS_TypeSupport typeSupport;
    DDS_Topic topic = NULL;
    _TypeSupportBinding found;

    SAC_REPORT_STACK();

    if (topic_name == NULL || type_name == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "Topic_name = %s and type_name = %s",
                    topic_name?topic_name:"NULL", type_name?type_name:"NULL");
    } else {
        result = DDS_TopicQos_is_consistent (qos);
    }

    if (result == DDS_RETCODE_OK) {
        result = DDS_DomainParticipantClaim(_this, &dp);
    }
    if (result == DDS_RETCODE_OK) {
        if (qos == DDS_TOPIC_QOS_DEFAULT) {
            qos = dp->defaultTopicQos;
        }
        if (result == DDS_RETCODE_OK) {
            found = c_iterResolve(dp->typeSupportBindings,
                                  compareBinding,
                                  (c_voidp)type_name);
            if (found != NULL) {
                typeSupport = found->typeSupport;
            } else {
                result = DDS_RETCODE_PRECONDITION_NOT_MET;
                SAC_REPORT(result, "Failed to resolve type_name = %s",
                            type_name?type_name:"NULL");
            }
        }
        if (result == DDS_RETCODE_OK) {
            tQos = DDS_TopicQos_copyIn(qos);
            if (tQos != NULL) {
                typeName = DDS_TypeSupport_get_internal_type_name(typeSupport);
                key_list = DDS_TypeSupport_get_key_list(typeSupport);
                uParticipant = DDS_DomainParticipant_get_user_entity(dp);
                uTopic = u_topicNew(uParticipant,
                                    topic_name,
                                    typeName,
                                    key_list,
                                    tQos);
                if (uTopic != NULL) {
                    topic = DDS_TopicNew(_this, topic_name, type_name, typeSupport, uTopic);
                    if ( topic ) {
                        dp->topicList = c_iterInsert (dp->topicList, topic);
                    } else {
                         result = DDS_RETCODE_ERROR;
                         SAC_REPORT(result, "Kernel Topic creation failed");
                    }
                } else {
                    result = DDS_RETCODE_ERROR;
                    SAC_REPORT(result, "Userlayer Topic creation failed");
                }
                DDS_free(typeName);
                DDS_free(key_list);
                u_topicQosFree(tQos);
            }
        }
        DDS_DomainParticipantRelease(_this);
    }
    if (result == DDS_RETCODE_OK) {
        DDS_Topic_set_listener(topic, a_listener, mask);
        if (dp->factoryAutoEnable) {
            DDS_Entity_enable(topic);
        }
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return topic;
}

/*     ReturnCode_t
 *     delete_topic(
 *         in Topic a_topic);
 */
DDS_ReturnCode_t
DDS_DomainParticipant_delete_topic (
    DDS_DomainParticipant _this,
    const DDS_Topic a_topic)
{
    DDS_ReturnCode_t result;
    DDS_Topic found;
    _DomainParticipant dp;

    SAC_REPORT_STACK();

    if (a_topic != NULL) {
        result = DDS_DomainParticipantClaim(_this, &dp);
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "Topic = NULL");
    }
    if (result == DDS_RETCODE_OK) {
        found = c_iterTake(dp->topicList, a_topic);
        if (found != a_topic) {
            /* The following call is expensive so only use it in case of exceptions. */
            if (DDS_Object_get_kind(DDS_Object(a_topic)) == DDS_TOPIC) {
                DDS_string name = DDS_TopicDescription_get_name(a_topic);
                result = DDS_RETCODE_PRECONDITION_NOT_MET;
                SAC_REPORT(result, "Topic %s does not belong to this DomainParticipant", name);
                DDS_free(name);
            } else {
                result = DDS_RETCODE_BAD_PARAMETER;
                SAC_REPORT(result, "Topic parameter 'a_topic' is of type %s",
                            DDS_ObjectKind_image(DDS_Object_get_kind(DDS_Object(a_topic))));
            }
        } else {
            result = DDS__free(a_topic);
            if (result != DDS_RETCODE_OK) {
                c_iterInsert(dp->topicList, a_topic);
            }
        }
        DDS_DomainParticipantRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

static os_boolean
keyCompare(
    const char *key1,
    const char *key2)
{
    c_iter key1Names, key2Names;
    c_char *name1, *name2;
    os_boolean consistent;

    key1Names = c_splitString(key1,", \t");
    key2Names = c_splitString(key2,", \t");

    consistent = (c_iterLength(key1Names) == c_iterLength(key2Names));
    if (consistent) {
        name1 = (c_char *)c_iterTakeFirst(key1Names);
        name2 = (c_char *)c_iterTakeFirst(key2Names);
        while ((name1 != NULL) && (name2 != NULL) && consistent) {
            consistent = (strcmp(name1,name2) == 0);
            os_free(name1);
            os_free(name2);
            if (consistent) {
                name1 = (c_char *)c_iterTakeFirst(key1Names);
                name2 = (c_char *)c_iterTakeFirst(key2Names);
            }
        }
    }
    name1 = (c_char *)c_iterTakeFirst(key1Names);
    while (name1 != NULL) {
        os_free(name1);
        name1 = (c_char *)c_iterTakeFirst(key1Names);
    }
    c_iterFree(key1Names);
    name2 = (c_char *)c_iterTakeFirst(key2Names);
    while (name2 != NULL) {
        os_free(name2);
        name2 = (c_char *)c_iterTakeFirst(key2Names);
    }
    c_iterFree(key2Names);

    return consistent;
}

/*     Topic
 *     find_topic(
 *         in string topic_name,
 *         in Duration_t timeout);
 */
DDS_Topic
DDS_DomainParticipant_find_topic (
    DDS_DomainParticipant _this,
    const DDS_char *topic_name,
    const DDS_Duration_t *timeout)
{
    DDS_ReturnCode_t result;
    DDS_TypeSupport typeSupport = NULL;
    DDS_char *type_name;
    DDS_Topic topic = NULL;
    _DomainParticipant dp;
    _TypeSupportBinding found;
    os_duration duration;
    c_iter list = NULL;
    u_participant uParticipant;
    u_topic uTopic;

    SAC_REPORT_STACK();

    result = DDS_Duration_copyIn(timeout, &duration);
    if (result == DDS_RETCODE_OK) {
        result = DDS_DomainParticipantClaim(_this, &dp);
        if (result == DDS_RETCODE_OK) {
            uParticipant = u_participant(_Entity_get_user_entity(_Entity(dp)));
            DDS_DomainParticipantRelease(_this);
            list = u_participantFindTopic(uParticipant, topic_name, duration);
        }
    }
    if (result == DDS_RETCODE_OK) {
        uTopic = c_iterTakeFirst(list);
        if (uTopic != NULL) {
            type_name = u_topicTypeName(uTopic);
            result = DDS_DomainParticipantClaim(_this, &dp);
            if (result == DDS_RETCODE_OK) {
                found = c_iterResolve(dp->typeSupportBindings, compareBinding, (c_voidp)type_name);
                if (found != NULL) {
                    typeSupport = found->typeSupport;
                }
                /*
                 * Verify if TypeSupport keylist matches the found Topic keylist
                 */
                if ((result == DDS_RETCODE_OK) && (typeSupport != NULL)) {
                    DDS_string type_key_list = DDS_TypeSupport_get_key_list(typeSupport);
                    char *topic_key_list = u_topicKeyExpr(uTopic);
                    if (type_key_list == NULL || topic_key_list == NULL) {
                        if (type_key_list != topic_key_list) {
                            SAC_REPORT_WARNING(
                                    "TypeSupport (%s) key \"%s\" doesn't match Topic (%s) key \"%s\".",
                                    type_name, type_key_list?topic_key_list:"NULL",
                                    topic_name, topic_key_list?topic_key_list:"NULL");
                        }
                    } else if (keyCompare(type_key_list, topic_key_list) == OS_FALSE) {
                        SAC_REPORT_WARNING(
                                "TypeSupport (%s) key \"%s\" doesn't match Topic (%s) key \"%s\".",
                                type_name, type_key_list, topic_name, topic_key_list);
                    }
                    DDS_free(type_key_list);
                    os_free(topic_key_list);
                }

                if (result == DDS_RETCODE_OK) {
                    topic = DDS_TopicNew(_this, topic_name, type_name, typeSupport, uTopic);
                    if ( topic ) {
                        dp->topicList = c_iterInsert (dp->topicList, topic);
                    } else {
                         result = DDS_RETCODE_ERROR;
                    }
                }
                if (result != DDS_RETCODE_OK) {
                    u_objectFree(u_object(uTopic));
                }
                os_free(type_name);
                DDS_DomainParticipantRelease(_this);
            }
        } else {
            result = DDS_RETCODE_PRECONDITION_NOT_MET;
            SAC_REPORT(result, "Failed to resolve Topic %s",
                        topic_name?topic_name:"NULL");
        }
    }
    if (list != NULL) {
        c_iterFree(list);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return topic;
}

/*     TopicDescription
 *     lookup_topicdescription(
 *         in string name);
 */
DDS_TopicDescription
DDS_DomainParticipant_lookup_topicdescription (
    DDS_DomainParticipant _this,
    const DDS_char *name)
{
    DDS_ReturnCode_t result;
    DDS_TopicDescription descr = NULL;
    _DomainParticipant dp;

    SAC_REPORT_STACK();

    result = DDS_DomainParticipantClaimRead(_this, &dp);
    if (result == DDS_RETCODE_OK) {
        descr = c_iterResolve(dp->topicList,
                    (c_iterResolveCompare)compareTopicName,
                    (void *)name);
        if (descr == NULL) {
            descr = c_iterResolve(dp->cfTopicList,
                        (c_iterResolveCompare)compareTopicName,
                        (void *)name);
            if (descr == NULL) {
                descr = c_iterResolve(dp->multiTopicList,
                            (c_iterResolveCompare)compareTopicName,
                            (void *)name);
                if (descr == NULL) {
                    descr = c_iterResolve(dp->builtinTopicList,
                                (c_iterResolveCompare)compareTopicName,
                                (void *)name);
                }
            }
        }
        DDS_DomainParticipantRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return descr;
}

/*     Topic
 *     lookup_builtin_topic(
 *         in string name);
 */
DDS_Topic
DDS_DomainParticipant_lookup_builtin_topic (
    DDS_DomainParticipant _this,
    const DDS_char *topic_name)
{
    DDS_ReturnCode_t result;
    DDS_TypeSupport typeSupport = NULL;
    DDS_char *type_name;
    DDS_Topic topic = NULL;
    _DomainParticipant dp;
    _TypeSupportBinding found;
    c_iter list = NULL;
    u_participant uParticipant;
    u_topic uTopic;

    SAC_REPORT_STACK();

    result = DDS_DomainParticipantClaim(_this, &dp);
    if (result == DDS_RETCODE_OK) {
        topic = c_iterResolve(dp->builtinTopicList,
                              (c_iterResolveCompare)compareTopicName,
                              (void *)topic_name);
        uParticipant = u_participant(_Entity_get_user_entity(_Entity(dp)));
        list = u_participantFindTopic(uParticipant, topic_name, 0);
        uTopic = c_iterTakeFirst(list);
        if (uTopic != NULL) {
            type_name = u_topicTypeName(uTopic);
            found = c_iterResolve(dp->typeSupportBindings,
                                  compareBinding,
                                  (c_voidp)type_name);
            if (found != NULL) {
                typeSupport = found->typeSupport;
            } else {
                /* Now check if type_name is an internal builtin Topic name */
                if (strcmp(topic_name, "DCPSParticipant") == 0) {
                    typeSupport = DDS_ParticipantBuiltinTopicDataTypeSupport__alloc();
                } else if (strcmp(topic_name, "DCPSTopic") == 0) {
                    typeSupport = DDS_TopicBuiltinTopicDataTypeSupport__alloc();
                } else if (strcmp(topic_name, "DCPSPublication") == 0) {
                    typeSupport = DDS_PublicationBuiltinTopicDataTypeSupport__alloc();
                } else if (strcmp(topic_name, "DCPSSubscription") == 0) {
                    typeSupport = DDS_SubscriptionBuiltinTopicDataTypeSupport__alloc();
                } else if (strcmp(topic_name, "CMParticipant") == 0) {
                    typeSupport = DDS_CMParticipantBuiltinTopicDataTypeSupport__alloc();
                } else if (strcmp(topic_name, "CMPublisher") == 0) {
                    typeSupport = DDS_CMPublisherBuiltinTopicDataTypeSupport__alloc();
                } else if (strcmp(topic_name, "CMSubscriber") == 0) {
                    typeSupport = DDS_CMSubscriberBuiltinTopicDataTypeSupport__alloc();
                } else if (strcmp(topic_name, "CMDataWriter") == 0) {
                    typeSupport = DDS_CMDataWriterBuiltinTopicDataTypeSupport__alloc();
                } else if (strcmp(topic_name, "CMDataReader") == 0) {
                    typeSupport = DDS_CMDataReaderBuiltinTopicDataTypeSupport__alloc();
                } else if (strcmp(topic_name, "DCPSType") == 0) {
                    typeSupport = DDS_TypeBuiltinTopicDataTypeSupport__alloc();
                } else {
                    typeSupport = NULL;
                }
                dp = NULL;
                DDS_DomainParticipantRelease(_this);
                if ( typeSupport ) {
                    result = DDS_TypeSupport_register_type(typeSupport, _this, type_name);
                }
                if (result == DDS_RETCODE_OK) {
                    result = DDS_DomainParticipantClaim(_this, &dp);
                } else {
                    (void)DDS_DomainParticipantClaim(_this, &dp);
                }
            }
            if (result == DDS_RETCODE_OK) {
                topic = DDS_TopicNew(_this, topic_name, type_name, typeSupport, uTopic);
                if ( topic ) {
                    dp->builtinTopicList = c_iterInsert (dp->builtinTopicList, topic);
                } else {
                     result = DDS_RETCODE_ERROR;
                }
            }
            if (result != DDS_RETCODE_OK) {
                u_objectFree(u_object(uTopic));
            }
            DDS_free(typeSupport);
            os_free(type_name);
        } else {
            result = DDS_RETCODE_PRECONDITION_NOT_MET;
            SAC_REPORT(result, "Failed to resolve Topic %s",
                        topic_name?topic_name:"NULL");
        }
        DDS_DomainParticipantRelease(_this);
    }
    if (list != NULL) {
        c_iterFree(list);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return topic;
}

/*     ContentFilteredTopic
 *     create_contentfilteredtopic(
 *         in string name,
 *         in Topic related_topic,
 *         in string filter_expression,
 *         in StringSeq filter_parameters);
 */
DDS_ContentFilteredTopic
DDS_DomainParticipant_create_contentfilteredtopic (
    DDS_DomainParticipant _this,
    const DDS_char *name,
    const DDS_Topic related_topic,
    const DDS_char *filter_expression,
    const DDS_StringSeq *filter_parameters)
{
    DDS_ReturnCode_t result;
    DDS_ContentFilteredTopic topic = NULL;
    _DomainParticipant dp;

    SAC_REPORT_STACK();

    result = DDS_DomainParticipantClaim(_this, &dp);
    if (result == DDS_RETCODE_OK) {
        topic = DDS_ContentFilteredTopicNew(_this,
                                            name,
                                            related_topic,
                                            filter_expression,
                                            filter_parameters);
        if ( topic ) {
            cmn_listenerDispatcher listenerDispatcher;
            listenerDispatcher = DDS_Entity_get_listenerDispatcher(_this);
            result = DDS_Entity_set_listenerDispatcher(topic, listenerDispatcher);
            if (result == DDS_RETCODE_OK) {
                dp->cfTopicList = c_iterInsert (dp->cfTopicList, topic);
            } else {
                (void)DDS__free(topic);
            }
        } else {
            result = DDS_RETCODE_ERROR;
        }
        result = DDS_DomainParticipantRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return topic;
}

/*     ReturnCode_t
 *     delete_contentfilteredtopic(
 *         in ContentFilteredTopic a_contentfilteredtopic);
 */
DDS_ReturnCode_t
DDS_DomainParticipant_delete_contentfilteredtopic (
    DDS_DomainParticipant _this,
    const DDS_ContentFilteredTopic topic)
{
    DDS_ReturnCode_t result;
    DDS_ContentFilteredTopic found;
    _DomainParticipant dp;

    SAC_REPORT_STACK();

    result = DDS_DomainParticipantClaim(_this, &dp);
    if (result == DDS_RETCODE_OK) {
        found = c_iterTake(dp->cfTopicList, topic);
        if (found != topic) {
            /* The following call is expensive so only use it in case of exceptions. */
            if (DDS_Object_get_kind(DDS_Object(topic)) == DDS_CONTENTFILTEREDTOPIC) {
                result = DDS_RETCODE_PRECONDITION_NOT_MET;
                SAC_REPORT(result, "ContentFilteredTopic does not belong to this DomainParticipant");
            } else {
                result = DDS_RETCODE_BAD_PARAMETER;
                SAC_REPORT(result, "ContentFilteredTopic parameter 'topic' is of type %s",
                            DDS_ObjectKind_image(DDS_Object_get_kind(DDS_Object(topic))));
            }
        } else {
            result = DDS__free(topic);
            if (result != DDS_RETCODE_OK) {
                c_iterInsert(dp->cfTopicList, topic);
            }
        }
        DDS_DomainParticipantRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     MultiTopic
 *     create_multitopic(
 *         in string name,
 *         in string type_name,
 *         in string subscription_expression,
 *         in StringSeq expression_parameters);
 */
DDS_MultiTopic
DDS_DomainParticipant_create_multitopic (
    DDS_DomainParticipant _this,
    const DDS_char *name,
    const DDS_char *type_name,
    const DDS_char *subscription_expression,
    const DDS_StringSeq *expression_parameters)
{
    DDS_ReturnCode_t result;
    DDS_MultiTopic topic = NULL;
    _DomainParticipant dp;

    SAC_REPORT_STACK();

    result = DDS_DomainParticipantClaim(_this, &dp);
    if (result == DDS_RETCODE_OK) {
        topic = DDS_MultiTopicNew(_this,
                                  name,
                                  type_name,
                                  subscription_expression,
                                  expression_parameters);
        if ( topic ) {
            dp->multiTopicList = c_iterInsert (dp->multiTopicList, topic);
        }
        result = DDS_DomainParticipantRelease(_this);
    }
    if (result == DDS_RETCODE_OK) {
        cmn_listenerDispatcher listenerDispatcher;

        listenerDispatcher = DDS_Entity_get_listenerDispatcher(_this);
        result = DDS_Entity_set_listenerDispatcher(topic, listenerDispatcher);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return topic;
}

/*     ReturnCode_t
 *     delete_multitopic(
 *         in MultiTopic a_multitopic);
 */
DDS_ReturnCode_t
DDS_DomainParticipant_delete_multitopic (
    DDS_DomainParticipant _this,
    const DDS_MultiTopic a_multitopic)
{
    DDS_ReturnCode_t result;
    DDS_MultiTopic found;
    _DomainParticipant dp;

    SAC_REPORT_STACK();

    result = DDS_DomainParticipantClaim(_this, &dp);
    if (result == DDS_RETCODE_OK) {
        found = c_iterTake(dp->multiTopicList, a_multitopic);
        if (found != a_multitopic) {
            /* The following call is expensive so only use it in case of exceptions. */
            if (DDS_Object_get_kind(DDS_Object(a_multitopic)) == DDS_MULTITOPIC) {
                result = DDS_RETCODE_PRECONDITION_NOT_MET;
                SAC_REPORT(result, "MultiTopic does not belong to this DomainParticipant");
            } else {
                result = DDS_RETCODE_BAD_PARAMETER;
                SAC_REPORT(result, "MultiTopic parameter 'a_multitopic' is of type %s",
                            DDS_ObjectKind_image(DDS_Object_get_kind(DDS_Object(a_multitopic))));
            }
        } else {
            result = DDS__free(a_multitopic);
            if (result != DDS_RETCODE_OK) {
                c_iterInsert(dp->multiTopicList, a_multitopic);
            }
        }
        DDS_DomainParticipantRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     ReturnCode_t
 *     delete_contained_entities();
 */
DDS_ReturnCode_t
DDS_DomainParticipant_delete_contained_entities (
    DDS_DomainParticipant _this)
{
    DDS_ReturnCode_t result;
    _DomainParticipant dp;

    SAC_REPORT_STACK();

    result = DDS_DomainParticipantClaim(_this, &dp);
    if (result == DDS_RETCODE_OK) {
        result = _DomainParticipant_delete_contained_entities(dp);
        DDS_DomainParticipantRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

DDS_boolean
DDS_DomainParticipant_has_contained_entities (
    DDS_DomainParticipant _this)
{
    DDS_ReturnCode_t result;
    _DomainParticipant dp;
    DDS_unsigned_long count;

    SAC_REPORT_STACK();

    result = DDS_DomainParticipantClaimRead(_this, &dp);
    if (result == DDS_RETCODE_OK) {
        count = c_iterLength(dp->publisherList);
        if (count == 0) {
            count = c_iterLength(dp->subscriberList);
        }
        if (count == 0) {
            count = c_iterLength(dp->topicList);
        }
        if (count == 0) {
            count = c_iterLength(dp->cfTopicList);
        }
        if (count == 0) {
            count = c_iterLength(dp->multiTopicList);
        }
        DDS_DomainParticipantRelease(_this);
    } else {
        assert(FALSE);
        count = 0;
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return (count != 0);
}


/*     ReturnCode_t
 *     set_qos(
 *         in DomainParticipantQos qos);
 */
DDS_ReturnCode_t
DDS_DomainParticipant_set_qos (
    DDS_DomainParticipant _this,
    const DDS_DomainParticipantQos *qos)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    DDS_DomainParticipantQos participantQos;
    _DomainParticipant participant;
    u_participantQos uParticipantQos = NULL;
    u_participant uParticipant;
    u_result uResult;

    SAC_REPORT_STACK();

    memset(&participantQos, 0, sizeof(DDS_DomainParticipantQos));
    (void)DDS_DomainParticipantQos_init(&participantQos, DDS_PARTICIPANT_QOS_DEFAULT);

    result = DDS_DomainParticipantQos_is_consistent(qos);
    if (result == DDS_RETCODE_OK) {
        result = DDS_DomainParticipantClaim(_this, &participant);
    }
    if (result == DDS_RETCODE_OK) {
        cmn_listenerDispatcher dispatcher = NULL;
        os_schedClass scheduling_class, old_scheduling_class;
        os_int32 scheduling_priority, old_scheduling_priority;

        if (qos == DDS_PARTICIPANT_QOS_DEFAULT) {
            result = DDS_DomainParticipantFactory_get_default_participant_qos(
                participant->factory, &participantQos);
            qos = &participantQos;
        }
        if (result == DDS_RETCODE_OK) {
            uParticipantQos = DDS_DomainParticipantQos_copyIn(qos);
            if (uParticipantQos == NULL) {
                result = DDS_RETCODE_OUT_OF_RESOURCES;
                SAC_REPORT(result, "Could not copy DomainParticipantQos");
            }
        }
        /* Try resetting scheduling on listener dispatcher thread before
           updating user layer participant QoS to simplify rollback. */
        if (result == DDS_RETCODE_OK) {
            dispatcher = DDS_Entity_get_listenerDispatcher (participant);
            scheduling_class = DDS_ListenerDispatcher_scheduling_class (
                &qos->listener_scheduling);
            scheduling_priority = DDS_ListenerDispatcher_scheduling_priority (
                &qos->listener_scheduling);

            cmn_listenerDispatcher_get_scheduling (
                dispatcher, &old_scheduling_class, &old_scheduling_priority);
            result = cmn_listenerDispatcher_set_scheduling (
                dispatcher, scheduling_class, scheduling_priority);
        }
        if (result == DDS_RETCODE_OK) {
            uParticipant = DDS_DomainParticipant_get_user_entity(participant);
            uResult = u_participantSetQos(uParticipant, uParticipantQos);
            result = DDS_ReturnCode_get(uResult);
            if (result == DDS_RETCODE_OK) {
                participant->factoryAutoEnable =
                    uParticipantQos->entityFactory.v.autoenable_created_entities;
            }
        }
        /* Try reverting scheduling changes if something went wrong. */
        if (result != DDS_RETCODE_OK && dispatcher != NULL) {
            if (OS_RETCODE_OK != cmn_listenerDispatcher_set_scheduling (
                    dispatcher, old_scheduling_class, old_scheduling_priority))
            {
                SAC_REPORT (
                    DDS_RETCODE_ERROR, "Could not revert scheduling changes");
            }
        }

        if (uParticipantQos != NULL) {
            u_participantQosFree (uParticipantQos);
        }

        DDS_DomainParticipantRelease (_this);
    }

    (void)DDS_DomainParticipantQos_deinit (&participantQos);

    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     ReturnCode_t
 *     get_qos(
 *         inout DomainParticipantQos qos);
 */
DDS_ReturnCode_t
DDS_DomainParticipant_get_qos (
    DDS_DomainParticipant _this,
    DDS_DomainParticipantQos *qos)
{
    DDS_ReturnCode_t result;
    _DomainParticipant dp;
    u_participantQos uQos;
    u_result uResult;

    SAC_REPORT_STACK();

    result = DDS_DomainParticipantCheck(_this, &dp);
    if (result == DDS_RETCODE_OK) {
        if (qos == NULL) {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "DomainParticipantQos = NULL");
        } else if (qos == DDS_PARTICIPANT_QOS_DEFAULT) {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "QoS 'PARTICIPANT_QOS_DEFAULT' is read-only.");
        }
    }
    if (result == DDS_RETCODE_OK) {
        uResult = u_participantGetQos(DDS_DomainParticipant_get_user_entity(dp), &uQos);
        if (uResult == U_RESULT_OK) {
            result = DDS_DomainParticipantQos_copyOut(uQos, qos);
            u_participantQosFree(uQos);
        } else {
            result = DDS_ReturnCode_get(uResult);
        }
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

static c_bool
set_topic_interest(
    void *object,
    c_iterActionArg arg)
{
    assert(object);
    assert(arg);

    DDS_Topic_set_participantListenerInterest(DDS_Topic(object), *(DDS_StatusMask *)arg);
    return TRUE;
}

/*     ReturnCode_t
 *     set_listener(
 *         in DomainParticipantListener a_listener,
 *         in StatusKindMask mask);
 */
DDS_ReturnCode_t
DDS_DomainParticipant_set_listener (
    DDS_DomainParticipant _this,
    const struct DDS_DomainParticipantListener *a_listener,
    const DDS_StatusMask mask)
{
    DDS_ReturnCode_t result;
    _DomainParticipant p;
    DDS_StatusMask _mask;

    SAC_REPORT_STACK();

    result = DDS_DomainParticipantClaim(_this, &p);
    if (result == DDS_RETCODE_OK) {
        memset(&p->listener, 0, sizeof(struct DDS_ExtDomainParticipantListener));
        if (a_listener != NULL) {
            if (mask & DDS_ALL_DATA_DISPOSED_TOPIC_STATUS) {
                memcpy(&p->listener, a_listener, sizeof(struct DDS_ExtDomainParticipantListener));
            } else {
                memcpy(&p->listener, a_listener, sizeof(struct DDS_DomainParticipantListener));
            }
        }
        _mask = mask;
        result = DDS_Entity_set_listener_interest(DDS_Entity(p), _mask);
        if (result == DDS_RETCODE_OK) {
            _mask = _mask & (DDS_INCONSISTENT_TOPIC_STATUS | DDS_ALL_DATA_DISPOSED_TOPIC_STATUS);
            c_iterWalk(p->topicList, (c_iterWalkAction)set_topic_interest, &_mask);
        }
        DDS_DomainParticipantRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}



/*     DomainParticipantListener
 *     get_listener();
 */
struct DDS_DomainParticipantListener
DDS_DomainParticipant_get_listener (
    DDS_DomainParticipant _this)
{
    DDS_ReturnCode_t result;
    struct DDS_DomainParticipantListener listener;
    _DomainParticipant p;

    SAC_REPORT_STACK();

    result = DDS_DomainParticipantCheck(_this, &p);
    if (result == DDS_RETCODE_OK) {
        memcpy(&listener, &p->listener, sizeof(struct DDS_DomainParticipantListener));
    } else {
        memset(&listener, 0, sizeof(struct DDS_DomainParticipantListener));
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return listener;
}

/*     ReturnCode_t
 *     set_listener_mask();
 */
DDS_ReturnCode_t
DDS_DomainParticipant_set_listener_mask (
    _DomainParticipant _this,
    const DDS_StatusMask mask)
{
    DDS_ReturnCode_t result;
    DDS_StatusMask _mask;

    assert(_this);

    _mask = mask;
    result = DDS_Entity_set_listener_interest(DDS_Entity(_this), _mask);
    if (result == DDS_RETCODE_OK) {
        _mask = _mask & (DDS_INCONSISTENT_TOPIC_STATUS | DDS_ALL_DATA_DISPOSED_TOPIC_STATUS);
        c_iterWalk(_this->topicList, (c_iterWalkAction)set_topic_interest, &_mask);
    }

    return result;
}

/*     ReturnCode_t
 *     ignore_participant(
 *         in InstanceHandle_t handle);
 */
DDS_ReturnCode_t
DDS_DomainParticipant_ignore_participant (
    DDS_DomainParticipant _this,
    const DDS_InstanceHandle_t handle)
{
    DDS_ReturnCode_t result;
    _DomainParticipant p;

    OS_UNUSED_ARG(handle);

    SAC_REPORT_STACK();

    result = DDS_DomainParticipantClaim(_this, &p);
    if (result == DDS_RETCODE_OK) {
        result = DDS_RETCODE_UNSUPPORTED;
        SAC_REPORT(result, "This operation is currently unsupported");
        DDS_DomainParticipantRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     ReturnCode_t
 *     ignore_topic(
 *         in InstanceHandle_t handle);
 */
DDS_ReturnCode_t
DDS_DomainParticipant_ignore_topic (
    DDS_DomainParticipant _this,
    const DDS_InstanceHandle_t handle)
{
    DDS_ReturnCode_t result;
    _DomainParticipant p;

    OS_UNUSED_ARG(handle);

    SAC_REPORT_STACK();

    result = DDS_DomainParticipantClaim(_this, &p);
    if (result == DDS_RETCODE_OK) {
        result = DDS_RETCODE_UNSUPPORTED;
        SAC_REPORT(result, "This operation is currently unsupported");
        DDS_DomainParticipantRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     ReturnCode_t
 *     ignore_publication(
 *         in InstanceHandle_t handle);
 */
DDS_ReturnCode_t
DDS_DomainParticipant_ignore_publication (
    DDS_DomainParticipant _this,
    const DDS_InstanceHandle_t handle)
{
    DDS_ReturnCode_t result;
    _DomainParticipant p;

    OS_UNUSED_ARG(handle);

    SAC_REPORT_STACK();

    result = DDS_DomainParticipantClaim(_this, &p);
    if (result == DDS_RETCODE_OK) {
        result = DDS_RETCODE_UNSUPPORTED;
        SAC_REPORT(result, "This operation is currently unsupported");
        DDS_DomainParticipantRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     ReturnCode_t
 *     ignore_subscription(
 *         in InstanceHandle_t handle);
 */
DDS_ReturnCode_t
DDS_DomainParticipant_ignore_subscription (
    DDS_DomainParticipant _this,
    const DDS_InstanceHandle_t handle)
{
    DDS_ReturnCode_t result;
    _DomainParticipant p;

    OS_UNUSED_ARG(handle);

    SAC_REPORT_STACK();

    result = DDS_DomainParticipantClaim(_this, &p);
    if (result == DDS_RETCODE_OK) {
        result = DDS_RETCODE_UNSUPPORTED;
        SAC_REPORT(result, "This operation is currently unsupported");
        DDS_DomainParticipantRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     DomainId_t
 *     get_domain_id();
 */
DDS_DomainId_t
DDS_DomainParticipant_get_domain_id (
    DDS_DomainParticipant _this)
{
    DDS_ReturnCode_t result;
    DDS_DomainId_t domainId = DDS_DOMAIN_ID_INVALID;
    u_domainId_t uDomainId;
    _DomainParticipant dp;
    u_participant uParticipant;

    SAC_REPORT_STACK();

    result = DDS_DomainParticipantCheck(_this, &dp);
    if (result == DDS_RETCODE_OK) {
        uParticipant = DDS_DomainParticipant_get_user_entity(dp);
        uDomainId = u_participantGetDomainId(uParticipant);
        if (uDomainId != U_DOMAIN_ID_INVALID) {
            domainId = uDomainId;
        }
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return domainId;
}

/*     void
 *     assert_liveliness();
 */
DDS_ReturnCode_t
DDS_DomainParticipant_assert_liveliness (
    DDS_DomainParticipant _this)
{
    DDS_ReturnCode_t result;
    _DomainParticipant p;
    u_result uresult;

    SAC_REPORT_STACK();

    result = DDS_DomainParticipantCheck(_this, &p);
    if (result == DDS_RETCODE_OK) {
        uresult = u_participantAssertLiveliness(u_participant(_Entity_get_user_entity(_Entity(p))));
        result = DDS_ReturnCode_get(uresult);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     ReturnCode_t
 *     set_default_publisher_qos(
 *         in PublisherQos qos);
 */
DDS_ReturnCode_t
DDS_DomainParticipant_set_default_publisher_qos (
    DDS_DomainParticipant _this,
    const DDS_PublisherQos *qos)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    _DomainParticipant p;
    DDS_PublisherQos *publisherQos = NULL;

    SAC_REPORT_STACK();

    result = DDS_PublisherQos_is_consistent(qos);
    if (result == DDS_RETCODE_OK) {
        publisherQos = DDS_PublisherQos__alloc();
        if (publisherQos != NULL) {
            result = DDS_PublisherQos_init(publisherQos, qos);
        } else {
            result = DDS_RETCODE_OUT_OF_RESOURCES;
            SAC_REPORT(result, "Failed to copy DDS_PublisherQos");
        }
    }
    if (result == DDS_RETCODE_OK) {
        result = DDS_DomainParticipantClaim(_this, &p);
    }
    if (result == DDS_RETCODE_OK) {
        DDS_free(p->defaultPublisherQos);
        p->defaultPublisherQos = publisherQos;
        DDS_DomainParticipantRelease(_this);
    } else {
        DDS_free(publisherQos);
    }

    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     ReturnCode_t
 *     get_default_publisher_qos(
 *         inout PublisherQos qos);
 */
DDS_ReturnCode_t
DDS_DomainParticipant_get_default_publisher_qos (
    DDS_DomainParticipant _this,
    DDS_PublisherQos *qos)
{
    DDS_ReturnCode_t result;
    _DomainParticipant p;

    SAC_REPORT_STACK();

    result = DDS_DomainParticipantClaimRead(_this, &p);
    if (result == DDS_RETCODE_OK) {
        result = DDS_PublisherQos_init(qos, p->defaultPublisherQos);
        DDS_DomainParticipantRelease(_this);
    }

    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     ReturnCode_t
 *     set_default_subscriber_qos(
 *         in SubscriberQos qos);
 */
DDS_ReturnCode_t
DDS_DomainParticipant_set_default_subscriber_qos (
    DDS_DomainParticipant _this,
    const DDS_SubscriberQos *qos)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    _DomainParticipant p;
    DDS_SubscriberQos *subscriberQos = NULL;

    SAC_REPORT_STACK();

    result = DDS_SubscriberQos_is_consistent(qos);
    if (result == DDS_RETCODE_OK) {
        subscriberQos = DDS_SubscriberQos__alloc();
        if (subscriberQos != NULL) {
            result = DDS_SubscriberQos_init(subscriberQos, qos);
        } else {
            result = DDS_RETCODE_OUT_OF_RESOURCES;
            SAC_REPORT(result, "Failed to copy DDS_SubscriberQos");
        }
    }
    if (result == DDS_RETCODE_OK) {
        result = DDS_DomainParticipantClaim(_this, &p);
    }
    if (result == DDS_RETCODE_OK) {
        DDS_free (p->defaultSubscriberQos);
        p->defaultSubscriberQos = subscriberQos;
        DDS_DomainParticipantRelease(_this);
    } else  {
        DDS_free (subscriberQos);
    }

    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     ReturnCode_t
 *     get_default_subscriber_qos(
 *         inout SubscriberQos qos);
 */
DDS_ReturnCode_t
DDS_DomainParticipant_get_default_subscriber_qos (
    DDS_DomainParticipant _this,
    DDS_SubscriberQos *qos)
{
    DDS_ReturnCode_t result;
    _DomainParticipant p;

    SAC_REPORT_STACK();

    result = DDS_DomainParticipantClaimRead(_this, &p);
    if (result == DDS_RETCODE_OK) {
        result = DDS_SubscriberQos_init(qos, p->defaultSubscriberQos);
        DDS_DomainParticipantRelease(_this);
    }

    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     ReturnCode_t
 *     set_default_topic_qos(
 *         in TopicQos qos);
 */
DDS_ReturnCode_t
DDS_DomainParticipant_set_default_topic_qos (
    DDS_DomainParticipant _this,
    const DDS_TopicQos *qos)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    _DomainParticipant p;
    DDS_TopicQos *topicQos = NULL;

    SAC_REPORT_STACK();

    result = DDS_TopicQos_is_consistent(qos);
    if (result == DDS_RETCODE_OK) {
        topicQos = DDS_TopicQos__alloc ();
        if (topicQos != NULL) {
            result = DDS_TopicQos_init (topicQos, qos);
        } else {
            result = DDS_RETCODE_OUT_OF_RESOURCES;
            SAC_REPORT(result, "Failed to copy DDS_TopicQos");
        }
    }
    if (result == DDS_RETCODE_OK) {
        result = DDS_DomainParticipantClaim(_this, &p);
    }
    if (result == DDS_RETCODE_OK) {
        DDS_free(p->defaultTopicQos);
        p->defaultTopicQos = topicQos;
        DDS_DomainParticipantRelease(_this);
    } else {
        DDS_free(topicQos);
    }

    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     ReturnCode_t
 *     get_default_topic_qos(
 *         inout TopicQos qos);
 */
DDS_ReturnCode_t
DDS_DomainParticipant_get_default_topic_qos (
    DDS_DomainParticipant _this,
    DDS_TopicQos *qos)
{
    DDS_ReturnCode_t result;
    _DomainParticipant p;

    SAC_REPORT_STACK();

    result = DDS_DomainParticipantClaimRead(_this, &p);
    if (result == DDS_RETCODE_OK) {
        result = DDS_TopicQos_init(qos, p->defaultTopicQos);
        DDS_DomainParticipantRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

static u_result
copyInstanceHandles(
    u_instanceHandle *list,
    os_uint32 length,
    c_voidp arg)
{
    u_result result = U_RESULT_OK;
    DDS_InstanceHandleSeq *seq = (DDS_InstanceHandleSeq *)arg;

    /*buffer alloc*/
    if (length > seq->_maximum) {

        /* if release is true free the current buffer*/
        if (seq->_release) {
            DDS_free(seq->_buffer);
        }
        /* reallocate a new buffer */
        seq->_buffer = DDS_InstanceHandleSeq_allocbuf(length);
        seq->_release = TRUE;
        seq->_length = length;
        seq->_maximum = length;
    }
    memcpy(seq->_buffer, list, length * sizeof(DDS_InstanceHandle_t));
    return result;
}

/*     ReturnCode_t
 *     get_discovered_participants (
 *         inout InstanceHandleSeq participant_handles);
 */
DDS_ReturnCode_t
DDS_DomainParticipant_get_discovered_participants (
    DDS_DomainParticipant _this,
    DDS_InstanceHandleSeq  *participant_handles)
{
    DDS_ReturnCode_t result;
    DDS_Subscriber sub;
    DDS_DataReader r;
    u_dataReader uReader;
    u_result uResult;

    SAC_REPORT_STACK();

    participant_handles->_length =0;

    sub = DDS_DomainParticipant_get_builtin_subscriber(_this);
    if (sub != NULL) {
        r = DDS_Subscriber_lookup_datareader(sub, "DCPSParticipant");
        if (r) {
            uReader = u_dataReader(_Entity_get_user_entity(_Entity(r)));
            uResult = u_dataReaderGetInstanceHandles(uReader,copyInstanceHandles,participant_handles);
            result = DDS_ReturnCode_get(uResult);
        } else {
            result = DDS_RETCODE_ERROR;
            SAC_REPORT(result, "Unable to resolve builtin \"DCPSParticipant\" DataReader");
        }
    } else {
        result = DDS_RETCODE_ERROR;
        SAC_REPORT(result, "No builtin Subscriber available");
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

struct copyDiscoveredDataArg
{
    c_voidp to;
    _DataReader reader;
};

static v_actionResult
copyDiscoveredData(
    c_voidp from,
    c_voidp to)
{
    v_message msg;
    c_voidp data;
    v_actionResult result = 0;
    struct copyDiscoveredDataArg *arg = (struct copyDiscoveredDataArg *) to;

    if (from) {
        msg = v_dataReaderSampleMessage(from);
        data = C_DISPLACE(msg, arg->reader->userdataOffset);
        arg->reader->copy_out(data, arg->to);
    }
    return result;
}

/*     ReturnCode_t
 *     get_discovered_participant_data (
 *         in InstanceHandle_t handle,
 *         inout ParticipantBuiltinTopicData *participant_data);
 */
DDS_ReturnCode_t
DDS_DomainParticipant_get_discovered_participant_data (
    DDS_DomainParticipant _this,
    DDS_ParticipantBuiltinTopicData *participant_data,
    DDS_InstanceHandle_t  handle)
{
    DDS_ReturnCode_t result;
    DDS_Subscriber sub;
    DDS_DataReader r;
    struct copyDiscoveredDataArg arg;

    SAC_REPORT_STACK();

    sub = DDS_DomainParticipant_get_builtin_subscriber(_this);
    if (sub != NULL) {
        r = DDS_Subscriber_lookup_datareader(sub, "DCPSParticipant");
        if (r) {
            arg.to = participant_data;
            arg.reader = r;
            result = DDS_DataReader_read_instance_action(r, handle,
                                                         copyDiscoveredData,
                                                         (void *)&arg);
        } else {
            result = DDS_RETCODE_ERROR;
            SAC_REPORT(result, "Unable to resolve builtin \"DCPSParticipant\" DataReader");
        }
    } else {
        result = DDS_RETCODE_ERROR;
        SAC_REPORT(result, "No builtin Subscriber available");
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     ReturnCode_t
 *     get_discovered_topics (
 *         inout InstanceHandleSeq topic_handles);
 */
DDS_ReturnCode_t
DDS_DomainParticipant_get_discovered_topics (
    DDS_DomainParticipant _this,
    DDS_InstanceHandleSeq  *topic_handles)
{
    DDS_ReturnCode_t result;
    DDS_Subscriber sub;
    DDS_DataReader r;
    u_dataReader uReader;
    u_result uResult;

    topic_handles->_length =0;

    SAC_REPORT_STACK();

    sub = DDS_DomainParticipant_get_builtin_subscriber(_this);
    if (sub != NULL) {
        r = DDS_Subscriber_lookup_datareader(sub, "DCPSTopic");
        if (r) {
            uReader = u_dataReader(_Entity_get_user_entity(_Entity(r)));
            uResult = u_dataReaderGetInstanceHandles(uReader,copyInstanceHandles,topic_handles);
            result = DDS_ReturnCode_get(uResult);
        } else {
            result = DDS_RETCODE_ERROR;
            SAC_REPORT(result, "Unable to resolve builtin \"DCPSTopic\" dataReader");
        }
    } else {
        result = DDS_RETCODE_ERROR;
        SAC_REPORT(result, "No builtin Subscriber available");
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

static v_actionResult
copyDiscoveredTopicData(
    c_voidp from,
    c_voidp to)
{
    v_message msg;
    c_voidp data;
    v_actionResult result = 0;
    struct copyDiscoveredDataArg *arg = (struct copyDiscoveredDataArg *) to;

    if (from) {
        msg = v_dataReaderSampleMessage(from);
        data = C_DISPLACE(msg, arg->reader->userdataOffset);
        arg->reader->copy_out(data, arg->to);
    }
    return result;
}

/*     ReturnCode_t
 *     get_discovered_topic_data (
 *         in InstanceHandle_t handle,
 *         inout TopicBuiltinTopicData *topic_data);
 */
DDS_ReturnCode_t
DDS_DomainParticipant_get_discovered_topic_data (
    DDS_DomainParticipant _this,
    DDS_TopicBuiltinTopicData *topic_data,
    DDS_InstanceHandle_t  handle)
{
    DDS_ReturnCode_t result;
    DDS_Subscriber sub;
    DDS_DataReader r;
    struct copyDiscoveredDataArg arg;

    SAC_REPORT_STACK();

    sub = DDS_DomainParticipant_get_builtin_subscriber(_this);
    if (sub != NULL) {
        r = DDS_Subscriber_lookup_datareader(sub, "DCPSTopic");
        if (r) {
            arg.to = topic_data;
            arg.reader = r;
            result = DDS_DataReader_read_instance_action(r, handle,
                                                         copyDiscoveredTopicData,
                                                         (void *)&arg);
        } else {
            result = DDS_RETCODE_ERROR;
            SAC_REPORT(result, "Unable to resolve builtin \"DCPSTopic\" DataReader");
        }
    } else {
        result = DDS_RETCODE_ERROR;
        SAC_REPORT(result, "No builtin Subscriber available");
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     Boolean
 *     contains_entity (
 *         in InstanceHandle_t a_hande);
 */
struct check_handle_arg {
    DDS_InstanceHandle_t handle;
    DDS_boolean result;
};

static c_bool
publisher_check_handle(
    void *object,
    struct check_handle_arg *arg)
{
    assert(object);
    assert(arg);

    if (!arg->result) {
        arg->result = DDS_Entity_check_handle(DDS_Entity(object), arg->handle);
        if ( !arg->result ) {
            arg->result = DDS_Publisher_contains_entity(DDS_Publisher(object), arg->handle);
        }
    }
    return !arg->result;
}

static c_bool
subscriber_check_handle(
    void *object,
    struct check_handle_arg *arg)
{
    assert(object);
    assert(arg);

    if (!arg->result) {
        arg->result = DDS_Entity_check_handle(DDS_Entity(object), arg->handle);
        if ( !arg->result ) {
            arg->result = DDS_Subscriber_contains_entity(DDS_Subscriber(object), arg->handle);
        }
    }
    return !arg->result;
}

static c_bool
topic_check_handle(
    void *object,
    struct check_handle_arg *arg)
{
    assert(object);
    assert(arg);

    if (!arg->result) {
        arg->result = DDS_Entity_check_handle(DDS_Entity(object), arg->handle);
    }
    return !arg->result;
}

DDS_boolean
DDS_DomainParticipant_contains_entity (
    DDS_DomainParticipant _this,
    DDS_InstanceHandle_t  a_handle)
{
    DDS_ReturnCode_t result;
    _DomainParticipant dp;
    struct check_handle_arg arg;

    SAC_REPORT_STACK();

    arg.handle = a_handle;
    arg.result = FALSE;
    result = DDS_DomainParticipantClaimRead(_this, &dp);
    if (result == DDS_RETCODE_OK) {
        if (!arg.result) {
            c_iterWalkUntil(dp->publisherList, (c_iterAction)publisher_check_handle, &arg);
        }
        if (!arg.result) {
            c_iterWalkUntil(dp->subscriberList, (c_iterAction)subscriber_check_handle, &arg);
        }
        if (!arg.result) {
            c_iterWalkUntil(dp->topicList, (c_iterAction)topic_check_handle, &arg);
        }
        DDS_DomainParticipantRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return arg.result;
}

/*     ReturnCode_t
 *     get_current_time (
 *         inout Time_t current_time);
 */
DDS_ReturnCode_t
DDS_DomainParticipant_get_current_time (
    DDS_DomainParticipant _this,
    DDS_Time_t  *current_time)
{
    DDS_ReturnCode_t result;
    _DomainParticipant p;
    os_timeW t;

    SAC_REPORT_STACK();

    result = DDS_DomainParticipantCheck(_this, &p);
    if (result == DDS_RETCODE_OK) {
        t = os_timeWGet();
        result = DDS_Time_copyOut(&t, current_time);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     TypeSupport
 *     lookup_typesupport (
 *         in string type_name);
 */
DDS_TypeSupport
DDS_DomainParticipant_lookup_typesupport (
    DDS_DomainParticipant _this,
    const DDS_char *type_name)
{
    DDS_ReturnCode_t result;
    DDS_TypeSupport typeSupport = NULL;
    _DomainParticipant p;
    _TypeSupportBinding found;

    SAC_REPORT_STACK();

    result = DDS_DomainParticipantClaimRead(_this, &p);
    if (result == DDS_RETCODE_OK) {
        found = c_iterResolve(p->typeSupportBindings,
                              compareBinding,
                              (c_voidp)type_name);
        if (found != NULL) {
            typeSupport = found->typeSupport;
        } else {
            typeSupport = NULL;
        }
        DDS_DomainParticipantRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return typeSupport;
}

DDS_ReturnCode_t
DDS_DomainParticipant_register_type (
    DDS_DomainParticipant _this,
    const DDS_char *type_name,
    const DDS_TypeSupport type)
{
    DDS_ReturnCode_t result;
    _DomainParticipant p;
    _TypeSupportBinding found;
    _TypeSupportBinding binding;

    SAC_REPORT_STACK();

    result = DDS_DomainParticipantClaim(_this, &p);
    if (result == DDS_RETCODE_OK) {
        found = c_iterResolve(p->typeSupportBindings,
                              compareBinding,
                              (c_voidp)type_name);
        if (found == NULL) {
            binding = os_malloc(C_SIZEOF(_TypeSupportBinding));
            binding->typeSupport = DDS_TypeSupportKeep(type);
            binding->type_name = os_strdup(type_name);
            p->typeSupportBindings = c_iterInsert(p->typeSupportBindings, binding);
        } else {
            if (found->typeSupport != type) {
                /* See if the given type is compatible with the found one
                 * on this domain participant. */
                result = DDS_TypeSupport_compatible(type, _this);
                if (result != DDS_RETCODE_OK) {
                    result = DDS_RETCODE_PRECONDITION_NOT_MET;
                    SAC_REPORT(result, "Type %s does not match already registered definition",
                                type_name);
                }
            }
        }
        DDS_DomainParticipantRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

DDS_ReturnCode_t
DDS_DomainParticipant_find_type_locked (
    _DomainParticipant _this,
    const DDS_char *type_name,
    DDS_TypeSupport *type)
{
    DDS_ReturnCode_t result;
    _TypeSupportBinding found;

    if (type == NULL || type_name == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "TypeSupport holder = 0x%x and type_name = %s",
                    type, type_name?type_name:"NULL");
    } else {
        found = c_iterResolve(_this->typeSupportBindings,
                              compareBinding,
                              (c_voidp)type_name);
        if (found != NULL) {
            *type = found->typeSupport;
        } else {
            *type = NULL;
        }
        result = DDS_RETCODE_OK;
    }
    return result;
}

DDS_ReturnCode_t
DDS_DomainParticipant_find_type (
    DDS_DomainParticipant _this,
    const DDS_char *type_name,
    DDS_TypeSupport *type)
{
    DDS_ReturnCode_t result;
    _DomainParticipant p;
    SAC_REPORT_STACK();
    result = DDS_DomainParticipantClaimRead(_this, &p);
    if (result == DDS_RETCODE_OK) {
        result = DDS_DomainParticipant_find_type_locked(p, type_name, type);
        DDS_DomainParticipantRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}


DDS_ReturnCode_t
DDS_DomainParticipant_delete_historical_data (
    DDS_DomainParticipant _this,
    const DDS_string partition_expression,
    const DDS_string topic_expression)
{
    DDS_ReturnCode_t result;
    u_participant uParticipant;
    u_result uResult;
    _DomainParticipant p;

    SAC_REPORT_STACK();

    result = DDS_DomainParticipantClaim(_this, &p);
    if (result == DDS_RETCODE_OK) {
        uParticipant = DDS_DomainParticipant_get_user_entity(p);
        uResult = u_participantDeleteHistoricalData(uParticipant,
                                                    partition_expression,
                                                    topic_expression);
        result = DDS_ReturnCode_get(uResult);
        DDS_DomainParticipantRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

DDS_ReturnCode_t
DDS_DomainParticipant_notify_listener(
    DDS_DomainParticipant _this,
    v_listenerEvent event)
{
    DDS_ReturnCode_t result;
    u_eventMask triggerMask;
    DDS_Entity entity;
    struct DDS_ExtDomainParticipantListener cb;

    cb = _DomainParticipant(_this)->listener;
    triggerMask = event->kind;
    result = DDS_RETCODE_OK;

    entity = u_observableGetUserData(u_observable(event->source));

    if ((triggerMask & V_EVENT_ON_DATA_ON_READERS) &&
            (cb.on_data_on_readers != NULL))
    {
        result = DDS_Entity_reset_on_data_on_readers_status(DDS_Entity(entity));
        if (result == DDS_RETCODE_OK) {
            cb.on_data_on_readers(cb.listener_data, entity);
        }
    } else {
        if ((triggerMask & V_EVENT_DATA_AVAILABLE) &&
                (cb.on_data_available != NULL))
        {
            result = DDS_Entity_reset_dataAvailable_status(DDS_Entity(entity));
            if (result == DDS_RETCODE_OK) {
                cb.on_data_available(cb.listener_data, entity);
            }
        }
    }
    if ((triggerMask & V_EVENT_INCONSISTENT_TOPIC) &&
            (cb.on_inconsistent_topic != NULL))
    {
        DDS_InconsistentTopicStatus status;
        DDS_InconsistentTopicStatus_init(&status, &((v_topicStatus)event->eventData)->inconsistentTopic);
        cb.on_inconsistent_topic(cb.listener_data, entity, &status);
    }
    if ((triggerMask & V_EVENT_LIVELINESS_LOST) &&
            (cb.on_liveliness_lost != NULL))
    {
        DDS_LivelinessLostStatus status;
        DDS_LivelinessLostStatus_init(&status, &((v_writerStatus)event->eventData)->livelinessLost);
        cb.on_liveliness_lost(cb.listener_data, entity, &status);
    }
    if (triggerMask & V_EVENT_OFFERED_DEADLINE_MISSED) {
        if (cb.on_offered_deadline_missed != NULL) {
            DDS_OfferedDeadlineMissedStatus status;
            DDS_OfferedDeadlineMissedStatus_init(&status, &((v_writerStatus)event->eventData)->deadlineMissed);
            cb.on_offered_deadline_missed(cb.listener_data, entity, &status);
        }
    }
    if (triggerMask & V_EVENT_REQUESTED_DEADLINE_MISSED) {
        if (cb.on_requested_deadline_missed != NULL) {
            DDS_RequestedDeadlineMissedStatus status;
            DDS_RequestedDeadlineMissedStatus_init(&status, &((v_readerStatus)event->eventData)->deadlineMissed);
            cb.on_requested_deadline_missed(cb.listener_data, entity, &status);
        }
    }
    if ((triggerMask & V_EVENT_SAMPLE_REJECTED) &&
            (cb.on_sample_rejected != NULL))
    {
        DDS_SampleRejectedStatus status;
        DDS_SampleRejectedStatus_init(&status, &((v_readerStatus)event->eventData)->sampleRejected);
        cb.on_sample_rejected(cb.listener_data, entity, &status);
    }
    if ((triggerMask & V_EVENT_LIVELINESS_CHANGED) &&
            (cb.on_liveliness_changed != NULL))
    {
        DDS_LivelinessChangedStatus status;
        DDS_LivelinessChangedStatus_init(&status, &((v_readerStatus)event->eventData)->livelinessChanged);
        cb.on_liveliness_changed(cb.listener_data, entity, &status);
    }
    if (triggerMask & V_EVENT_OFFERED_INCOMPATIBLE_QOS) {
        if (cb.on_offered_incompatible_qos != NULL) {
            DDS_OfferedIncompatibleQosStatus status;
            DDS_OfferedIncompatibleQosStatus_init(&status, &((v_writerStatus)event->eventData)->incompatibleQos);
            cb.on_offered_incompatible_qos(cb.listener_data, entity, &status);
        }
    }
    if (triggerMask & V_EVENT_REQUESTED_INCOMPATIBLE_QOS) {
        if (cb.on_requested_incompatible_qos != NULL) {
            DDS_RequestedIncompatibleQosStatus status;
            DDS_RequestedIncompatibleQosStatus_init(&status, &((v_readerStatus)event->eventData)->incompatibleQos);
            cb.on_requested_incompatible_qos(cb.listener_data, entity, &status);
        }
    }
    if ((triggerMask & V_EVENT_SAMPLE_LOST) &&
            (cb.on_sample_lost != NULL))
    {
        DDS_SampleLostStatus status;
        DDS_SampleLostStatus_init(&status, &((v_readerStatus)event->eventData)->sampleLost);
        cb.on_sample_lost(cb.listener_data, entity, &status);
    }
    if (triggerMask & V_EVENT_PUBLICATION_MATCHED) {
        if (cb.on_publication_matched != NULL) {
            DDS_PublicationMatchedStatus status;
            DDS_PublicationMatchedStatus_init(&status, &((v_writerStatus)event->eventData)->publicationMatch);
            cb.on_publication_matched(cb.listener_data, entity, &status);
        }
    }
    if (triggerMask & V_EVENT_SUBSCRIPTION_MATCHED) {
        if (cb.on_subscription_matched != NULL) {
            DDS_SubscriptionMatchedStatus status;
            DDS_SubscriptionMatchedStatus_init(&status, &((v_readerStatus)event->eventData)->subscriptionMatch);
            cb.on_subscription_matched(cb.listener_data, entity, &status);
        }
    }
    if (triggerMask & V_EVENT_ALL_DATA_DISPOSED ) {
        if (cb.on_all_data_disposed != NULL) {
            cb.on_all_data_disposed(cb.listener_data, entity);
        }
    }

    return result;
}

