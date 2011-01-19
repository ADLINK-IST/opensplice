/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2010 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#include "v__builtin.h"

#include "os.h"
#include "os_report.h"

#include "c_stringSupport.h"
#include "sd_serializerXMLTypeinfo.h"

#include "v_kernel.h"
#include "v__topic.h"
#include "v__writer.h"
#include "v__publisher.h"
#include "v__subscriber.h"
#include "v__reader.h"
#include "v_dataReaderEntry.h"
#include "v_public.h"
#include "v_time.h"
#include "v_observer.h"
#include "v_participant.h"
#include "v_topicQos.h"
#include "v_writerQos.h"
#include "v_publisherQos.h"

/**************************************************************
 * Private functions
 **************************************************************/

/**************************************************************
 * constructor/destructor
 **************************************************************/

/**************************************************************
 * Protected functions
 **************************************************************/
/**************************************************************
 * Public functions
 **************************************************************/

void
v_builtinWritersDisable(
    v_builtin _this)
{
    /* set builtin writer to NULL, to prevent using those writers
       while publishing builtin topic information
    */
    _this->writers[V_PUBLICATIONINFO_ID] = NULL;
    _this->writers[V_TOPICINFO_ID] = NULL;
    _this->writers[V_PARTICIPANTINFO_ID] = NULL;
    _this->writers[V_SUBSCRIPTIONINFO_ID] = NULL;
    _this->writers[V_C_AND_M_COMMAND_ID] = NULL;
    _this->writers[V_HEARTBEATINFO_ID] = NULL;
    _this->writers[V_DELIVERYINFO_ID] = NULL;
}

v_builtin
v_builtinNew(
    v_kernel kernel)
{
    v_builtin _this;
    c_type type;
    c_base base;
    c_time time;
    v_message msg;
    v_topicQos tQos;
    v_publisherQos pQos;
    v_writerQos wQos;
    c_long i;

    base = c_getBase(kernel);
    type = c_resolve(base,"kernelModule::v_builtin");

    if (type) {
        _this = (v_builtin)c_new(type);
        c_free(type);
        if (_this) {
            _this->kernelQos = c_keep(kernel->qos);
            /* create QoS's */
            tQos = v_topicQosNew(kernel, NULL);
            tQos->durability.kind = V_DURABILITY_TRANSIENT;
            tQos->reliability.kind = V_RELIABILITY_RELIABLE;
            tQos->reliability.max_blocking_time.seconds = 0;
            tQos->reliability.max_blocking_time.nanoseconds = 100000000;
            tQos->durabilityService.service_cleanup_delay = C_TIME_ZERO;

            if (_this->kernelQos->builtin.enabled) {
                _this->topics[V_PARTICIPANTINFO_ID] =
                       v_topicNew(kernel, V_PARTICIPANTINFO_NAME,
                                  "kernelModule::v_participantInfo",
                                  "key.localId,key.systemId", tQos);
                _this->topics[V_SUBSCRIPTIONINFO_ID] =
                       v_topicNew(kernel, V_SUBSCRIPTIONINFO_NAME,
                                  "kernelModule::v_subscriptionInfo",
                                  "key.localId,key.systemId", tQos);
                _this->topics[V_PUBLICATIONINFO_ID] =
                   v_topicNew(kernel, V_PUBLICATIONINFO_NAME,
                              "kernelModule::v_publicationInfo",
                              "key.localId,key.systemId", tQos);
            }
            _this->topics[V_TOPICINFO_ID] =
                   v_topicNew(kernel, V_TOPICINFO_NAME,
                              "kernelModule::v_topicInfo",
                              "key.localId,key.systemId", tQos);
            _this->topics[V_HEARTBEATINFO_ID] =
                   v_topicNew(kernel, V_HEARTBEATINFO_NAME,
                              "kernelModule::v_heartbeatInfo",
                              "id.localId,id.systemId", NULL);
            _this->topics[V_C_AND_M_COMMAND_ID] =
                   v_topicNew(kernel, V_C_AND_M_COMMAND_NAME,
                              "kernelModule::v_controlAndMonitoringCommand",
                              "key.localId,key.systemId", tQos);
            _this->topics[V_DELIVERYINFO_ID] =
                   v_topicNew(kernel, V_DELIVERYINFO_NAME,
                              "kernelModule::v_deliveryInfo",
                              NULL, NULL);
            c_free(tQos);

            _this->participant = v_participantNew(kernel,
                                                  "Built-in participant",
                                                  NULL,NULL,TRUE);

            if (_this->participant) {
                pQos = v_publisherQosNew(kernel, NULL);
                pQos->presentation.access_scope = V_PRESENTATION_TOPIC;
                c_free(pQos->partition); /* free default partition "" */
                pQos->partition = c_stringNew(c_getBase(c_object(kernel)),
                                              V_BUILTIN_PARTITION);
                pQos->entityFactory.autoenable_created_entities = TRUE;

                _this->publisher = v_publisherNew(_this->participant,
                                                  "Built-in publisher",
                                                  pQos, TRUE);
                c_free(pQos);
                wQos = v_writerQosNew(kernel, NULL);
                wQos->durability.kind = V_DURABILITY_TRANSIENT;
                wQos->reliability.kind = V_RELIABILITY_RELIABLE;

                /* for the built-in topic DCPSTopic the built-in writer must
                 * define wQos->lifecycle.autodispose_unregistered_instance =
                 * FALSE as there will be at most 1 publisher of each DCPSTopic
                 * instance and this information must remain when the node
                 * containing the producer is down.
                 */
                wQos->lifecycle.autodispose_unregistered_instances = FALSE;
                _this->writers[V_TOPICINFO_ID] =
                     v_writerNew(_this->publisher,
                                 V_TOPICINFO_NAME "TopicInfoWriter",
                                 _this->topics[V_TOPICINFO_ID],
                                 wQos, TRUE);

                wQos->lifecycle.autodispose_unregistered_instances = TRUE;

                if (kernel->qos->builtin.enabled) {
                    _this->writers[V_PUBLICATIONINFO_ID] =
                     v_writerNew(_this->publisher,
                                 V_PUBLICATIONINFO_NAME "PublicationInfoWriter",
                                 _this->topics[V_PUBLICATIONINFO_ID],
                                 wQos, TRUE);

                    _this->writers[V_PARTICIPANTINFO_ID] =
                         v_writerNew(_this->publisher,
                                     V_PARTICIPANTINFO_NAME "ParticipantInfoWriter",
                                     _this->topics[V_PARTICIPANTINFO_ID],
                                     wQos, TRUE);

                    _this->writers[V_SUBSCRIPTIONINFO_ID] =
                         v_writerNew(_this->publisher,
                                     V_SUBSCRIPTIONINFO_NAME "SubscriptionInfoWriter",
                                     _this->topics[V_SUBSCRIPTIONINFO_ID],
                                     wQos, TRUE);
                } else {
                    _this->writers[V_PUBLICATIONINFO_ID] = NULL;
                    _this->writers[V_PARTICIPANTINFO_ID] = NULL;
                    _this->writers[V_SUBSCRIPTIONINFO_ID] = NULL;
                }
                _this->writers[V_HEARTBEATINFO_ID] =
                     v_writerNew(_this->publisher,
                                 V_HEARTBEATINFO_NAME "HeartbeatInfoWriter",
                                 _this->topics[V_HEARTBEATINFO_ID],
                                 NULL /* default qos */, TRUE);

                wQos = v_writerQosNew(kernel, NULL);
                wQos->durability.kind = V_DURABILITY_TRANSIENT;
                wQos->reliability.kind = V_RELIABILITY_RELIABLE;

                _this->writers[V_C_AND_M_COMMAND_ID] =
                     v_writerNew(_this->publisher,
                                 V_C_AND_M_COMMAND_NAME "Writer",
                                 _this->topics[V_C_AND_M_COMMAND_ID],
                                 wQos, TRUE);

                wQos->durability.kind = V_DURABILITY_VOLATILE;
                wQos->reliability.kind = V_RELIABILITY_RELIABLE;
                wQos->lifecycle.autodispose_unregistered_instances = FALSE;

                _this->writers[V_DELIVERYINFO_ID] =
                     v_writerNew(_this->publisher,
                                 V_DELIVERYINFO_NAME "DeliveryInfoWriter",
                                 _this->topics[V_DELIVERYINFO_ID],
                                 wQos, TRUE);
                c_free(wQos);
                /* We have to solve a bootstrapping problem here!
                 * The kernel entities created above have notified
                 * their existence to the builtin interface.
                 * Unfortunately the implementation does not yet exists as
                 * being in the construction of it:)
                 * Therefore this information is published below.
                 */
                time = v_timeGet();

                if (_this->kernelQos->builtin.enabled) {
                    msg = v_builtinCreateParticipantInfo(_this,
                                                         _this->participant);
                    v_writerWrite(_this->writers[V_PARTICIPANTINFO_ID],
                                  msg,time,NULL);
                    c_free(msg);
                }
                for (i = 0; i < V_INFO_ID_COUNT; i++) {
                  /* Do not use v_topicAnnounce(_this->topics[i]);
                   * as kernel->builtin is not assigned yet!
                   */
                    if (_this->topics[i] != NULL) {
                        msg = v_builtinCreateTopicInfo(_this, _this->topics[i]);
                        v_writerWrite(_this->writers[V_TOPICINFO_ID],
                                      msg, time, NULL);
                        c_free(msg);
                    }
                }
                if (_this->kernelQos->builtin.enabled) {
                    for (i = 0; i < V_INFO_ID_COUNT; i++) {
                        if (_this->writers[i]) {
                            v_observerLock(v_observer(_this->writers[i]));
                            msg = v_builtinCreatePublicationInfo(_this,
                                                                 _this->writers[i]);
                            v_observerUnlock(v_observer(_this->writers[i]));
                            v_writerWrite(_this->writers[V_PUBLICATIONINFO_ID],
                                          msg,time,NULL);
                            c_free(msg);
                        }
                    }
                }
            }
        } else {
            _this = NULL;
        }
    } else {
        _this = NULL;
    }
    return _this;
}

v_message
v_builtinCreateParticipantInfo (
    v_builtin _this,
    v_participant p)
{
    v_message msg;
    v_topic topic;
    c_type type;
    c_long size;
    struct v_participantInfo *info;

    assert(p != NULL);
    assert(C_TYPECHECK(p,v_participant));
    assert(C_TYPECHECK(_this,v_builtin));

    if ((_this != NULL) &&
        (_this->kernelQos->builtin.enabled)) {
        if (p->qos != NULL) {
            topic = v_builtinTopicLookup(_this, V_PARTICIPANTINFO_ID);
            msg = v_topicMessageNew(topic);
            if (msg != NULL) {
                size = p->qos->userData.size;
                info = v_builtinParticipantInfoData(_this,msg);
                info->key = v_publicGid(v_public(p));
                info->user_data.size = size;
                info->user_data.value = NULL;
                type = c_octet_t(c_getBase(c_object(p)));
                if (size > 0) {
                    info->user_data.value = c_arrayNew(type, size);
                    memcpy(info->user_data.value,
                           p->qos->userData.value,
                           size);
                } else {
                    info->user_data.value = NULL;
                }
            }
        } else {
            OS_REPORT(OS_WARNING, "v_builtin", 0,
                      "Failed to produce built-in topic");
            msg = NULL;
        }
    } else {
        msg = NULL;
    }
    return msg;
}

v_message
v_builtinCreateTopicInfo (
    v_builtin _this,
    v_topic topic)
{
    v_message msg;
    v_topic builtinTopic;
    v_topicQos topicQos;
    struct v_topicInfo *info;
    c_type type;
    c_char *str;
    c_base base;
    sd_serializer serializer;
    sd_serializedData meta_data;

    assert(topic != NULL);
    assert(C_TYPECHECK(topic,v_topic));
    assert(C_TYPECHECK(_this,v_builtin));

    if (_this != NULL) {
        builtinTopic = v_builtinTopicLookup(_this, V_TOPICINFO_ID);
        if (builtinTopic != NULL) {
            msg = v_topicMessageNew(builtinTopic);
        } else {
            msg = NULL;
        }
        if (msg != NULL) {
            base = c_getBase(c_object(topic));
            info = v_builtinTopicInfoData(_this,msg);
            info->key.systemId = topic->crcOfName;
            info->key.localId = topic->crcOfTypeName;
            info->key.serial = 0;
            info->name = c_keep(v_topicName(topic));

            str = c_metaScopedName(c_metaObject(v_topicDataType(topic)));
            info->type_name = c_stringNew(base, str);
            os_free(str);

            /* copy qos */
            topicQos = topic->qos;
            info->durability         = topicQos->durability;
            info->durabilityService  = topicQos->durabilityService;
            info->deadline           = topicQos->deadline;
            info->latency_budget     = topicQos->latency;
            info->liveliness         = topicQos->liveliness;
            info->reliability        = topicQos->reliability;
            info->transport_priority = topicQos->transport;
            info->lifespan           = topicQos->lifespan;
            info->destination_order  = topicQos->orderby;
            info->history            = topicQos->history;
            info->resource_limits    = topicQos->resource;
            info->ownership          = topicQos->ownership;
            type                     = c_octet_t(base);
            assert(type != NULL);
            info->topic_data.value   = c_arrayNew(type,topicQos->topicData.size);
            memcpy(info->topic_data.value,
                   topicQos->topicData.value,
                   topicQos->topicData.size);


            info->key_list = c_keep(v_topicKeyExpr(topic));

            info->meta_data = NULL;
            serializer = sd_serializerXMLTypeinfoNew(base, FALSE);
            if (serializer != NULL) {
                meta_data = sd_serializerSerialize(serializer,
                                c_object(v_topicDataType(topic)));
                str = sd_serializerToString(serializer, meta_data);
                if (str != NULL) {
                    info->meta_data = c_stringNew(base, str);
                    os_free(str);
                } else {
                    OS_REPORT(OS_WARNING,
                              "v_builtinCreateTopicInfo", 0,
                              "Failed to serialize topic type.");
                }
                sd_serializedDataFree(meta_data);
                sd_serializerFree(serializer);
            } else {
                OS_REPORT(OS_WARNING,
                          "v_builtinCreateTopicInfo", 0,
                          "Failed to create serializer for topic type.");
            }
        }
    } else {
        msg = NULL;
    }

    return msg;
}


v_message
v_builtinCreatePublicationInfo (
    v_builtin _this,
    v_writer writer)
{
    v_message msg;
    v_topic builtinTopic;
    struct v_publicationInfo *info;
    v_publisher publisher;
    v_participant p;
    c_type type;
    c_base base;
    v_topicQos topicQos;
    v_writerQos writerQos;
    c_char *str;
    c_iter partitions;
    c_long i;
    c_long length;

    assert(writer != NULL);
    assert(C_TYPECHECK(writer,v_writer));
    assert(C_TYPECHECK(_this,v_builtin));

    if ((_this != NULL) &&
        (_this->kernelQos->builtin.enabled)) {
        base = c_getBase(c_object(writer));
        builtinTopic = v_builtinTopicLookup(_this,V_PUBLICATIONINFO_ID);
        msg = v_topicMessageNew(builtinTopic);
        if (msg == NULL) {
            return NULL;
        }
        info = v_builtinPublicationInfoData(_this,msg);
        info->participant_key = v_publicGid(NULL);
        info->group_data.value = NULL;

        publisher = v_publisher(writer->publisher);
        if (publisher != NULL) {
            info->presentation = publisher->qos->presentation;
            info->group_data.value = c_keep(publisher->qos->groupData.value);
            p = v_participant(publisher->participant);
            if (p != NULL) {
                info->participant_key = v_publicGid(v_public(p));
            }

            partitions = c_splitString(publisher->qos->partition, ",");
            length = c_iterLength(partitions);
            type = c_string_t(base);
            if (length > 0) {
                info->partition.name = c_arrayNew(type, length);
                i = 0;
                str = (c_char *)c_iterTakeFirst(partitions);
                while (str != NULL) {
                    assert(i < length);
                    info->partition.name[i++] = c_stringNew(base, str);
                    os_free(str);
                    str = (c_char *)c_iterTakeFirst(partitions);
                }
            } else {
                length = 1;
                info->partition.name = c_arrayNew(type, length);
                info->partition.name[0] = c_stringNew(base, "");
            }
            c_free(type);
            c_iterFree(partitions);
        }

        info->key = v_publicGid(v_public(writer));
        info->topic_name = c_keep(v_topicName(writer->topic));

        str = c_metaScopedName(c_metaObject(v_topicDataType(writer->topic)));
        info->type_name = c_stringNew(base, str);
        os_free(str);

        writerQos = writer->qos;
        /* copy qos */
        info->durability         = writerQos->durability;
        info->deadline           = writerQos->deadline;
        info->latency_budget     = writerQos->latency;
        info->liveliness         = writerQos->liveliness;
        info->reliability        = writerQos->reliability;
        info->lifespan           = writerQos->lifespan;
        info->ownership          = writerQos->ownership;
        info->ownership_strength = writerQos->strength;
        info->destination_order  = writerQos->orderby;
        info->lifecycle          = writerQos->lifecycle;

        if (writerQos->userData.size > 0) {
            type = c_octet_t(base);
            info->user_data.value = c_arrayNew(type, writerQos->userData.size);
            memcpy(info->user_data.value,
                   writerQos->userData.value,
                   writerQos->userData.size);
            c_free(type);
        } else {
            info->user_data.value = NULL;
        }

        topicQos = v_topicGetQos(writer->topic);
        info->topic_data.value  = c_keep(topicQos->topicData.value);

        c_free(topicQos);

        info->alive = writer->alive;
    } else {
        msg = NULL;
    }
    return msg;
}

static c_bool
getTopic (
    c_object o,
    c_voidp arg)
{
    v_dataReaderEntry entry = v_dataReaderEntry(o);
    v_topic *topic = (v_topic *)arg;
    c_bool result = TRUE;

    if (*topic == NULL) {
        *topic = c_keep(entry->topic);
    } else {
        /* Already a topic was found so this must be a Multi Topic reader.
         * In that case abort and clear the topic.
         */
        c_free(*topic);
        *topic = NULL;
        result = FALSE;
    }
    return result;
}

v_message
v_builtinCreateSubscriptionInfo (
    v_builtin _this,
    v_dataReader dataReader)
{
    v_message msg;
    struct v_subscriptionInfo *info;
    v_participant p;
    v_subscriber s;
    v_topic topic, builtinTopic;
    v_topicQos qos;
    v_readerQos rQos;
    c_base base;
    c_type type;
    c_char *str;
    c_iter partitions;
    c_long i;
    c_long length;

    assert(dataReader != NULL);
    assert(C_TYPECHECK(dataReader,v_dataReader));
    assert(C_TYPECHECK(_this,v_builtin));

    if ((_this != NULL) &&
        (_this->kernelQos->builtin.enabled)) {
        base = c_getBase(c_object(dataReader));
        builtinTopic = v_builtinTopicLookup(_this,V_SUBSCRIPTIONINFO_ID);
        msg = v_topicMessageNew(builtinTopic);
        if (msg != NULL) {
            info = v_builtinSubscriptionInfoData(_this,msg);
            info->partition.name = NULL;
            info->participant_key = v_publicGid(NULL);

            s = v_subscriber(v_reader(dataReader)->subscriber);
            if (s != NULL) {
                info->presentation = s->qos->presentation;
                info->group_data.value = c_keep(s->qos->groupData.value);

                partitions = c_splitString(s->qos->partition, ",");
                length = c_iterLength(partitions);
                type = c_string_t(base);
                if (length > 0) {
                    info->partition.name = c_arrayNew(type, length);
                    i = 0;
                    str = (c_char *)c_iterTakeFirst(partitions);
                    while (str != NULL) {
                        assert(i < length);
                        info->partition.name[i++] = c_stringNew(base, str);
                        os_free(str);
                        str = (c_char *)c_iterTakeFirst(partitions);
                    }
                } else {
                    length = 1;
                    info->partition.name = c_arrayNew(type, length);
                    info->partition.name[0] = c_stringNew(base, "");
                }
                c_iterFree(partitions);

                p = v_participant(s->participant);
                if (p != NULL) {
                    info->participant_key = v_publicGid(v_public(p));
                }
            }

            topic = NULL;
            v_readerWalkEntries(v_reader(dataReader),getTopic,&topic);
            assert(topic);
            rQos = v_reader(dataReader)->qos;

            info->key = v_publicGid(v_public(dataReader));
            info->topic_name = c_keep(v_topicName(topic));

            str = c_metaScopedName(c_metaObject(v_topicDataType(topic)));
            info->type_name = c_stringNew(base, str);
            os_free(str);

            /* copy qos */
            info->durability        = rQos->durability;
            info->deadline          = rQos->deadline;
            info->latency_budget    = rQos->latency;
            info->liveliness        = rQos->liveliness;
            info->reliability       = rQos->reliability;
            info->ownership         = rQos->ownership;
            info->destination_order = rQos->orderby;
            info->time_based_filter = rQos->pacing;
            info->lifespan          = rQos->lifespan;

            type = c_octet_t(base);
            if (rQos->userData.size > 0) {
                info->user_data.value = c_arrayNew(type, rQos->userData.size);
                memcpy(info->user_data.value,
                       rQos->userData.value,
                       rQos->userData.size);
            } else {
                info->user_data.value = NULL;
            }
            c_free(type);

            qos = v_topicGetQos(topic);
            info->topic_data.value = c_keep(qos->topicData.value);
            v_topicQosFree(qos);
        }
    } else {
        msg = NULL;
    }
    return msg;
}
