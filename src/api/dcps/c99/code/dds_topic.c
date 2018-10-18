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
#include <dds_dcps_private.h>
#include <dds.h>
#include <dds_report.h>
#include <dds__qos.h>

struct TopicInfo {
    struct DDS_EntityUserData_s _parent;
    dds_topiclistener_t *listener;

    bool ownParticipant;
};


static void
on_inconsistent_topic (
    void *listener_data,
    DDS_Topic topic,
    const DDS_InconsistentTopicStatus *status)
{
    struct TopicInfo *info = listener_data;
    dds_topiclistener_t *listener = NULL;

    if (info) {
        listener = info->listener;
    }

    if (listener && status && listener->on_inconsistent_topic) {
        dds_inconsistent_topic_status_t s;
        s.total_count = status->total_count;
        s.total_count_change = status->total_count_change;
        listener->on_inconsistent_topic(topic, &s);
    }
}

static void
dds_topic_info_free(
    DDS_EntityUserData data)
{
    struct TopicInfo *info = (struct TopicInfo *)data;
    if (info) {
        if (info->listener) {
            os_free(info->listener);
        }
        os_free(info);
    }
}

static struct TopicInfo *
dds_topic_info_new(
    void)
{
    struct TopicInfo *info;

    info = os_malloc(sizeof(*info));
    DDS_Entity_user_data_init((DDS_EntityUserData)info, dds_topic_info_free);
    info->listener = NULL;

    return info;
}

static void
dds_topic_listener_init(
    struct DDS_TopicListener *listener,
    void *data)
{
    listener->listener_data = data;
    listener->on_inconsistent_topic = on_inconsistent_topic;
}


int
dds_topic_create (
    dds_entity_t pp,
    dds_entity_t * topic,
    const dds_topic_descriptor_t * descriptor,
    const char * name,
    const dds_qos_t * qos,
    const dds_topiclistener_t * listener)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    struct TopicInfo *info = NULL;
    struct DDS_TopicListener dpl;
    struct DDS_TopicListener *lp = NULL;
    DDS_StatusMask mask = (listener) ? DDS_STATUS_MASK_ANY : DDS_STATUS_MASK_NONE;
    DDS_TopicQos *tQos;
    bool ownParticipant = false;

    DDS_REPORT_STACK();

    if (!topic) {
        result = DDS_RETCODE_BAD_PARAMETER;
        DDS_REPORT(result, "Given topic parameter is NULL.");
    }
    if (!name) {
        result = DDS_RETCODE_BAD_PARAMETER;
        DDS_REPORT(result, "Given topic name is NULL.");
    }
    if (!descriptor) {
        result = DDS_RETCODE_BAD_PARAMETER;
        DDS_REPORT(result, "Given descriptor is NULL.");
    }
    if (result == DDS_RETCODE_OK) {
        if (!pp) {
            pp = dds_participant_lookup (DDS_DOMAIN_ID_DEFAULT);
            if (!pp) {
                result = dds_participant_create(&pp, DDS_DOMAIN_ID_DEFAULT, qos, NULL);
                if (result != DDS_RETCODE_OK){
                    DDS_REPORT(result, "Failed to create an implicit DomainParticipant.");
                } else {
                  ownParticipant = true;
                }
            }
        }
    }
    if (result == DDS_RETCODE_OK) {
        result = descriptor->register_type(pp, (void *)descriptor);
        if (result != DDS_RETCODE_OK) {
          DDS_REPORT(result, "Failed to register type.");
        }
    }
    if (result == DDS_RETCODE_OK) {
        info = dds_topic_info_new();
        if (!info) {
            result = DDS_RETCODE_ERROR;
            DDS_REPORT(result, "Failed to create topic info.");
        }
    }
    if (result == DDS_RETCODE_OK) {

        *topic = NULL;
        info->ownParticipant = ownParticipant;

        if (listener) {
            info->listener = os_malloc(sizeof(dds_topiclistener_t));
            *info->listener = *listener;
            lp = &dpl;
            dds_topic_listener_init(&dpl, info);
        }

        if (qos) {
            tQos = DDS_TopicQos__alloc();
            result = DDS_DomainParticipant_get_default_topic_qos(pp, tQos);
            if (result == DDS_RETCODE_OK) {
                dds_qos_to_topic_qos(tQos, qos);
                *topic = DDS_DomainParticipant_create_topic(pp, name, descriptor->type_name, tQos, lp, mask);
            }
            DDS_free(tQos);
        } else {
            *topic = DDS_DomainParticipant_create_topic(pp, name, descriptor->type_name, DDS_TOPIC_QOS_DEFAULT, lp, mask);
        }
        if (*topic) {
            result = DDS_Entity_set_user_data(*topic, (DDS_EntityUserData)info);
        } else {
            result = dds_report_get_error_code();
        }
        DDS_Entity_release_user_data((DDS_EntityUserData)info);
    }

    DDS_REPORT_FLUSH(pp, result != DDS_RETCODE_OK);

    return DDS_ERRNO(result, DDS_MOD_KERNEL, DDS_ERR_Mx);
}

dds_entity_t
dds_topic_find(
    dds_entity_t pp,
    const char * name)
{
    const DDS_Duration_t w = DDS_DURATION_INFINITE;
    return (dds_entity_t)DDS_DomainParticipant_find_topic(pp, name, &w);
}

char *
dds_topic_get_name (
    dds_entity_t topic)
{
    char *name;

    DDS_REPORT_STACK();

    name = DDS_TopicDescription_get_name(topic);

    DDS_REPORT_FLUSH(topic, !name);

    return name;
}

char *
dds_topic_get_type_name (
    dds_entity_t topic)
{
    char *name;

    DDS_REPORT_STACK();

    name = DDS_Topic_get_type_name(topic);

    DDS_REPORT_FLUSH(topic, !name);

    return name;
}

char *
dds_topic_get_metadescriptor (
		dds_entity_t topic)
{
    char *descriptor;

    DDS_REPORT_STACK();

    descriptor = DDS_Topic_get_metadescription(topic);

    DDS_REPORT_FLUSH(topic, !descriptor);

    return descriptor;
}

char *
dds_topic_get_keylist (
		dds_entity_t topic)
{
    char *keylist;

    DDS_REPORT_STACK();

    keylist = DDS_Topic_get_keylist(topic);

    DDS_REPORT_FLUSH(topic, !keylist);

    return keylist;
}

void
dds_topic_set_filter (
    dds_entity_t topic,
    dds_topic_filter_fn filter)
{
    OS_UNUSED_ARG(topic);
    OS_UNUSED_ARG(filter);

}

dds_topic_filter_fn
dds_topic_get_filter (
    dds_entity_t topic)
{
    OS_UNUSED_ARG(topic);

    return NULL;
}

int
dds_topic_get_listener(
    dds_entity_t e,
    dds_topiclistener_t *listener)
{
    DDS_ReturnCode_t result = DDS_RETCODE_BAD_PARAMETER;
    struct TopicInfo *info = NULL;

    DDS_REPORT_STACK();

    if (listener) {
        result = DDS_Entity_claim_user_data(e, (DDS_EntityUserData *)&info);
        if (result == DDS_RETCODE_OK) {
            if (info->listener) {
                *listener = *info->listener;
            }
            DDS_Entity_release_user_data((DDS_EntityUserData)info);
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        DDS_REPORT(result, "The listener parameter is NULL.");
    }

    DDS_REPORT_FLUSH(e, result != DDS_RETCODE_OK);

    return DDS_ERRNO(result, DDS_MOD_KERNEL, DDS_ERR_Mx);
}

int
dds_topic_set_listener(
    dds_entity_t e,
    const dds_topiclistener_t *listener)
{
    DDS_ReturnCode_t result;
    struct TopicInfo *info = NULL;
    struct DDS_TopicListener dpl;
    dds_topiclistener_t *newListener;
    dds_topiclistener_t *oldListener;
    DDS_StatusMask mask;

    DDS_REPORT_STACK();

    result = DDS_Entity_claim_user_data(e, (DDS_EntityUserData *)&info);
    if (result == DDS_RETCODE_OK) {
        oldListener = info->listener;
        if (listener) {
            newListener = os_malloc(sizeof(dds_topiclistener_t));
            *newListener = *listener;
            mask = dds_status_get_enabled(e);
            info->listener = newListener;
            dds_topic_listener_init(&dpl, info);
        } else {
            mask = 0;
            info->listener = NULL;
        }
        result = DDS_Topic_set_listener(e, &dpl, mask);
        DDS_Entity_release_user_data((DDS_EntityUserData)info);
        os_free(oldListener);
    }

    DDS_REPORT_FLUSH(e, result != DDS_RETCODE_OK);

    return DDS_ERRNO(result, DDS_MOD_KERNEL, DDS_ERR_Mx);
}


int
dds_topic_delete(
    dds_entity_t e)
{
    int result;
    int ud_result;
    struct TopicInfo *info = NULL;
    DDS_DomainParticipant participant;

    ud_result = DDS_Entity_claim_user_data(e, (DDS_EntityUserData *)&info);
    participant = DDS_Topic_get_participant(e);
    if (participant) {
      result = DDS_DomainParticipant_delete_topic(participant, e);
      if (result == DDS_RETCODE_OK) {
          if (ud_result == DDS_RETCODE_OK && info->ownParticipant) {
              dds_entity_delete(participant);
          }
          DDS_Entity_release_user_data((DDS_EntityUserData)info);
      }
    } else {
        result = DDS_RETCODE_ALREADY_DELETED;
    }

    return DDS_ERRNO(result, DDS_MOD_KERNEL, DDS_ERR_Mx);
}

static int
dds_generic_topic_descriptor_register(
    dds_entity_t pp,
    void *arg)
{
    DDS_ReturnCode_t result = DDS_RETCODE_ERROR;
    dds_topic_descriptor_t *descriptor = (dds_topic_descriptor_t *)arg;

    if (descriptor) {
        DDS_TypeSupport ts = (DDS_TypeSupport)descriptor->arg;
        if (ts) {
            result = DDS_TypeSupport_register_type(ts, pp, descriptor->type_name);
        }
    }

    return result;
}

static int
dds_topic_descriptor_deinit(
    void *arg)
{
    dds_topic_descriptor_t *descriptor = (dds_topic_descriptor_t *)arg;

    if (descriptor && descriptor->arg) {
        assert(descriptor->size == 0);

        DDS_free(descriptor->arg);
        DDS_free(descriptor->type_name);
    }

    return DDS_RETCODE_OK;
}


dds_topic_descriptor_t *
dds_topic_descriptor_create(
    const char *name,
    const char *keys,
    const char *spec)
{
    dds_topic_descriptor_t *descriptor;

    descriptor = DDS_alloc(sizeof(*descriptor), (DDS_deallocatorType)dds_topic_descriptor_deinit);
    descriptor->register_type = dds_generic_topic_descriptor_register;
    descriptor->type_name = DDS_string_dup(name);
    descriptor->size = 0;
    descriptor->destructor = NULL;
    descriptor->arg = DDS_TypeSupport__alloc(name, keys, spec);

    return descriptor;
}

void
dds_topic_descriptor_delete(
    dds_topic_descriptor_t *descriptor)
{
    DDS_free(descriptor);
}
