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
#include "v__builtin.h"

#include "vortex_os.h"
#include "os_report.h"

#include "c_stringSupport.h"
#include "sd_serializerXMLTypeinfo.h"

#include "v_kernel.h"
#include "v__topic.h"
#include "v__topicImpl.h"
#include "v__writer.h"
#include "v__publisher.h"
#include "v__subscriber.h"
#include "v__reader.h"
#include "v__policy.h"
#include "v_topic.h"
#include "v_dataReader.h"
#include "v_deliveryServiceEntry.h"
#include "v_public.h"
#include "v_observer.h"
#include "v_participant.h"
#include "v_topicQos.h"
#include "v_writerQos.h"
#include "v_publisherQos.h"
#include "v_policy.h"
#include "v_groupSet.h"

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
    unsigned i;
    v_writer w = NULL;

    for (i = 0; i < sizeof (_this->writers) / sizeof (_this->writers[0]); i++) {
        w = _this->writers[i];
        _this->writers[i] = NULL;
        v_writerFree(w);
        c_free(w);
    }
}

v_builtin
v_builtinNew(
    v_kernel kernel)
{
#define KS(field) #field ".localId," #field ".systemId"
    enum tqos_selector {
        TQS_DEFAULT,            /* DDS default topic QoS */
        TQS_STANDARD,           /* "Most" built-in toipcs */
        TQS_HEARTBEAT           /* DCPSHeartbeat built-in topic */
    };
    static const struct {
        const char *topicname;
        const char *typename;
        const char *keylist;
        enum v_infoId id;
        int always; /* some topics/writers are required and exist even when built-in topics disabled */
        enum tqos_selector tqos_selector; /* hearbeat and delivery have default QoS for topic (wr qos always reliable) */
        c_bool autodispose; /* topic remains alive in system also when creating writer/node disappears */
        int transient; /* wr qos: -1: always volatile, 0: transient if builtin enabled, 1: always transient */
        int isreliable; /* DCPSHeartbeat must be best-effort */
        c_long transportPriority;
    } ts[] = {
        { V_PARTICIPANTINFO_NAME,   "v_participantInfo",             KS (key),      V_PARTICIPANTINFO_ID,   0, TQS_STANDARD,  1,  0, 1, 0 },
        { V_CMPARTICIPANTINFO_NAME, "v_participantCMInfo",           KS (key),      V_CMPARTICIPANTINFO_ID, 0, TQS_STANDARD,  1,  0, 1, 0 },
        { V_SUBSCRIPTIONINFO_NAME,  "v_subscriptionInfo",            KS (key),      V_SUBSCRIPTIONINFO_ID,  0, TQS_STANDARD,  1,  0, 1, 0 },
        { V_CMDATAREADERINFO_NAME,  "v_dataReaderCMInfo",            KS (key),      V_CMDATAREADERINFO_ID,  0, TQS_STANDARD,  1,  0, 1, 0 },
        { V_PUBLICATIONINFO_NAME,   "v_publicationInfo",             KS (key),      V_PUBLICATIONINFO_ID,   1, TQS_STANDARD,  1,  0, 1, 0 },
        { V_CMDATAWRITERINFO_NAME,  "v_dataWriterCMInfo",            KS (key),      V_CMDATAWRITERINFO_ID,  0, TQS_STANDARD,  1,  0, 1, 0 },
        { V_CMSUBSCRIBERINFO_NAME,  "v_subscriberCMInfo",            KS (key),      V_CMSUBSCRIBERINFO_ID,  0, TQS_STANDARD,  1,  0, 1, 0 },
        { V_CMPUBLISHERINFO_NAME,   "v_publisherCMInfo",             KS (key),      V_CMPUBLISHERINFO_ID,   0, TQS_STANDARD,  1,  0, 1, 0 },
        { V_TOPICINFO_NAME,         "v_topicInfo",                   KS (key),      V_TOPICINFO_ID,         1, TQS_STANDARD,  0,  1, 1, 0 },
        { V_TYPEINFO_NAME,          "v_typeInfo",                    "name,data_representation_id,type_hash.msb,type_hash.lsb", V_TYPEINFO_ID, 1, TQS_STANDARD, 0,  1, 1, 0 },
        { V_HEARTBEATINFO_NAME,     "v_heartbeatInfo",               "id.systemId", V_HEARTBEATINFO_ID,     1, TQS_HEARTBEAT, 1,  1, 0, C_MAX_LONG },
        { V_C_AND_M_COMMAND_NAME,   "v_controlAndMonitoringCommand", KS (key),      V_C_AND_M_COMMAND_ID,   1, TQS_STANDARD,  1, -1, 1, 0 },
        { V_DELIVERYINFO_NAME,      "v_deliveryInfo",                "",            V_DELIVERYINFO_ID,      1, TQS_DEFAULT,   0, -1, 1, 0 }
    };
#undef KS
    v_builtin _this;
    c_type type;
    c_base base;
    v_topicQos tQos = NULL;
    v_publisherQos pQos = NULL;
    v_writerQos wQos = NULL;
    int trans_threshold;
    unsigned i;

    assert ((int) (sizeof (ts) / sizeof (ts[0])) == V_INFO_ID_COUNT);

    base = c_getBase (kernel);
    if ((type = c_resolve (base, "kernelModuleI::v_builtin")) == NULL) {
        OS_REPORT (OS_ERROR,
                     "kernel::v_builtin::v_builtinNew",V_RESULT_INTERNAL_ERROR,
                     "Operation failed, couldn't resolve type \"kernelModuleI::v_builtin\"."
                     OS_REPORT_NL "kernel = 0x%" PA_PRIxADDR ", base = 0x%" PA_PRIxADDR,
                     (os_address) kernel, (os_address) base);
        assert (FALSE);
        return NULL;
    }
    if ((_this = (v_builtin)c_new (type)) == NULL) {
        OS_REPORT (OS_ERROR,
                   "kernel::v_builtin::v_builtinNew", V_RESULT_INTERNAL_ERROR,
                   "Failed to allocate v_builtin object.");
        c_free(type);
        assert (FALSE);
        return NULL;
    }
    c_free (type);

    _this->kernelQos = c_keep(kernel->qos);

    if ((_this->participant = v_participantNew (kernel, V_BUILT_IN_PARTICIPANT_NAME, NULL, TRUE)) == NULL) {
        OS_REPORT(OS_ERROR,
                  "kernel::v_builtin::v_builtinNew", V_RESULT_INTERNAL_ERROR,
                  "Failed to allocate \"Built-in participant\" object.");
        goto error;
    }

    if ((pQos = v_publisherQosNew (kernel, NULL)) == NULL) {
        OS_REPORT(OS_ERROR,
                    "kernel::v_builtin::v_builtinNew", V_RESULT_INTERNAL_ERROR,
                    "Operation failed because v_publisherQosNew failed for: kernel = 0x%" PA_PRIxADDR,
                    (os_address) kernel);
        goto error;
    }
    pQos->presentation.v.access_scope = V_PRESENTATION_TOPIC;
    pQos->entityFactory.v.autoenable_created_entities = TRUE;
    c_free(pQos->partition.v); /* free default partition "" */
    if ((pQos->partition.v = c_stringNew (c_getBase (c_object(kernel)), V_BUILTIN_PARTITION)) == NULL) {
        OS_REPORT (OS_ERROR,
                     "kernel::v_builtin::v_builtinNew", V_RESULT_INTERNAL_ERROR,
                     "Operation failed because c_stringNew failed for: Partition \"%s\".",
                     V_BUILTIN_PARTITION);
        goto error;
    }
    if ((_this->publisher = v_publisherNew (_this->participant, "Built-in publisher", pQos, TRUE)) == NULL) {
        OS_REPORT(OS_ERROR,
                    "kernel::v_builtin::v_builtinNew", V_RESULT_INTERNAL_ERROR,
                    "Operation failed because v_publisherNew failed for: Participant \"%s\".",
                    v_entityName(_this->participant));
        goto error;
    }

    if ((tQos = v_topicQosNew (kernel, NULL)) == NULL) {
        OS_REPORT (OS_ERROR,
                     "kernel::v_builtin::v_builtinNew", V_RESULT_INTERNAL_ERROR,
                     "Operation failed because v_topicQosNew failed for: kernel = 0x%" PA_PRIxADDR,
                     (os_address) kernel);
        goto error;
    }

    if ((wQos = v_writerQosNew (kernel, NULL)) == NULL) {
        OS_REPORT (OS_ERROR,
                     "kernel::v_builtin::v_builtinNew", V_RESULT_INTERNAL_ERROR,
                     "Operation failed because v_writerQosNew failed for: kernel = 0x%" PA_PRIxADDR,
                     (os_address) kernel);
        goto error;
    }
    wQos->durability.v.kind = V_DURABILITY_TRANSIENT;
    wQos->reliability.v.kind = V_RELIABILITY_RELIABLE;
    wQos->history.v.kind = V_HISTORY_KEEPALL;
    wQos->history.v.depth = V_LENGTH_UNLIMITED;

    trans_threshold = (_this->kernelQos->builtin.v.enabled ? 0 : 1);
    for (i = 0; i < sizeof (ts) / sizeof (ts[0]); i++) {
        if (ts[i].always || _this->kernelQos->builtin.v.enabled) {
            char typename[64], wrname[64];
            if (snprintf (typename, sizeof (typename), "kernelModule::%s", ts[i].typename) >= (int) sizeof (typename)) {
                assert (0);
            }
            if (snprintf (wrname, sizeof (wrname), "%sWriter", ts[i].topicname) >= (int) sizeof (wrname)) {
                assert (0);
            }

            switch (ts[i].tqos_selector) {
                case TQS_DEFAULT:
                    tQos->durability.v.kind = V_DURABILITY_VOLATILE;
                    tQos->reliability.v.kind = V_RELIABILITY_BESTEFFORT;
                    tQos->reliability.v.max_blocking_time = OS_DURATION_ZERO;
                    tQos->durabilityService.v.service_cleanup_delay = OS_DURATION_ZERO;
                    tQos->history.v.kind = V_HISTORY_KEEPLAST;
                    tQos->history.v.depth = 1;
                    break;
                case TQS_STANDARD:
                    tQos->durability.v.kind = V_DURABILITY_TRANSIENT;
                    tQos->reliability.v.kind = V_RELIABILITY_RELIABLE;
                    tQos->reliability.v.max_blocking_time = OS_DURATION_INIT(0,100000000);
                    tQos->durabilityService.v.service_cleanup_delay = OS_DURATION_ZERO;
                    /* make the history qos to have keep all and unlimited depth so
                       that samples that are marked for resend don't get squeezed out
                       of the writer's history */
                    tQos->history.v.kind = V_HISTORY_KEEPALL;
                    tQos->history.v.depth = V_LENGTH_UNLIMITED;
                    break;
                case TQS_HEARTBEAT:
                    tQos->durability.v.kind = V_DURABILITY_TRANSIENT;
                    tQos->reliability.v.kind = V_RELIABILITY_BESTEFFORT;
                    tQos->reliability.v.max_blocking_time = OS_DURATION_ZERO;
                    tQos->durabilityService.v.service_cleanup_delay = OS_DURATION_INIT(10,0);
                    tQos->history.v.kind = V_HISTORY_KEEPLAST;
                    tQos->history.v.depth = 1;
                    break;
            }

            _this->topics[ts[i].id] = v_topic(v_topicImplNew (kernel, ts[i].topicname, typename, ts[i].keylist, tQos, TRUE));
            if (_this->topics[ts[i].id] == NULL) {
                OS_REPORT (OS_ERROR,
                             "kernel::v_builtin::v_builtinNew", V_RESULT_INTERNAL_ERROR,
                             "Operation failed because v_topicNew failed for:"
                             OS_REPORT_NL "Topic: \"%s"
                             OS_REPORT_NL "Type: \"%s\""
                             OS_REPORT_NL "Key(s): \"%s\"",
                             ts[i].topicname, typename, ts[i].keylist);
                goto error;
            }
            /* Overrule TopicAccess for builtin topics see OSPL-6437 */
            v_topicImpl(_this->topics[ts[i].id])->accessMode = V_ACCESS_MODE_READ_WRITE;
            if (ts[i].transient >= trans_threshold) {
                wQos->durability.v.kind = V_DURABILITY_TRANSIENT;
            } else {
                wQos->durability.v.kind = V_DURABILITY_VOLATILE;
            }
            wQos->orderby.v.kind = (ts[i].tqos_selector == TQS_HEARTBEAT) ? V_ORDERBY_SOURCETIME : V_ORDERBY_RECEPTIONTIME;
            wQos->lifecycle.v.autodispose_unregistered_instances = ts[i].autodispose;
            wQos->reliability.v.kind = ts[i].isreliable ? V_RELIABILITY_RELIABLE : V_RELIABILITY_BESTEFFORT;
            wQos->transport.v.value = ts[i].transportPriority;
            _this->writers[ts[i].id] = v_writerNew (_this->publisher, wrname, _this->topics[ts[i].id], wQos);
            if (_this->writers[ts[i].id] == NULL) {
                OS_REPORT (OS_ERROR,
                             "kernel::v_builtin::v_builtinNew", V_RESULT_INTERNAL_ERROR,
                             "Operation failed because v_writerNew failed for:"
                             OS_REPORT_NL "Name: \"%s"
                             OS_REPORT_NL "Topic: \"%s",
                             wrname, ts[i].topicname);
                goto error;
            } else {
                v_entityEnable(v_entity(_this->writers[ts[i].id]));
            }
        }
    }
    c_free (wQos);
    c_free (tQos);
    c_free (pQos);

    /* We have to solve a bootstrapping problem here! The kernel
       entities created above have notified their existence to the
       builtin interface.  Unfortunately the implementation does not
       yet exists as being in the construction of it :) Therefore this
       information is published below. */
    {
        os_timeW time = os_timeWGet();
        v_message msg, msg2;

        if (_this->kernelQos->builtin.v.enabled) {
            msg = v_builtinCreateParticipantInfo (_this, _this->participant);
            v_writerWrite (_this->writers[V_PARTICIPANTINFO_ID], msg, time, NULL);
            c_free (msg);
            msg = v_builtinCreateCMParticipantInfo (_this, _this->participant);
            v_writerWrite (_this->writers[V_CMPARTICIPANTINFO_ID], msg, time, NULL);
            c_free (msg);
            msg = v_builtinCreateCMPublisherInfo (_this, _this->publisher);
            v_writerWrite (_this->writers[V_CMPUBLISHERINFO_ID], msg, time, NULL);
            c_free (msg);
        }
        for (i = 0; i < V_INFO_ID_COUNT; i++) {
            /* Do not use v_topicAnnounce (_this->topics[i]) as
               kernel->builtin is not assigned yet! */
            if (_this->topics[i] != NULL) {
                msg = v_builtinCreateTopicInfo (_this, _this->topics[i]);
                v_writerWrite (_this->writers[V_TOPICINFO_ID], msg, time, NULL);
                c_free (msg);
            }
        }
        if (_this->kernelQos->builtin.v.enabled) {
            for (i = 0; i < V_INFO_ID_COUNT; i++) {
                if (_this->writers[i]) {
                    v_observerLock (v_observer(_this->writers[i]));
                    msg = v_builtinCreatePublicationInfo (_this, _this->writers[i]);
                    msg2 = v_builtinCreateCMDataWriterInfo (_this, _this->writers[i]);
                    v_observerUnlock (v_observer(_this->writers[i]));
                    v_writerWrite (_this->writers[V_PUBLICATIONINFO_ID], msg, time, NULL);
                    v_writerWrite (_this->writers[V_CMDATAWRITERINFO_ID], msg2, time, NULL);
                    c_free (msg);
                    c_free (msg2);
                }
            }
        }
    }
    return _this;

 error:
    c_free (wQos);
    c_free (tQos);
    c_free (pQos);
    c_free (_this);
    return NULL;
}

#define XML_SIZE (1024)
#define NODENAME_SIZE (64)
v_message
v_builtinCreateCMParticipantInfo (
    v_builtin _this,
    v_participant p)
{
    v_message msg;
    v_topic topic;
    struct v_participantCMInfo *info;
    char *xml_description, *participantName;
    char nodeName[NODENAME_SIZE];
    os_size_t parNameLength =0;
    os_size_t procNameLength =0;
    os_size_t nodeNameLength =0;
    enum v_serviceType serviceType;

    c_base base = c_getBase(c_object(p));
    os_int pid =0;

    assert(p != NULL);
    assert(C_TYPECHECK(p,v_participant));
    assert(C_TYPECHECK(_this,v_builtin));

    /* Pre condition checking */
    if (p == NULL) {
        OS_REPORT(OS_CRITICAL,
                  "kernel::v_builtin::v_builtinCreateCMParticipantInfo", V_RESULT_PRECONDITION_NOT_MET,
                  "Operation failed pre condition not met. _this = 0x%" PA_PRIxADDR ", participant = 0x%" PA_PRIxADDR,
                  (os_address)_this,(os_address)p);
        return NULL;
    }

    if (!c_instanceOf(c_object(p), "v_service")) {
        serviceType = V_SERVICETYPE_NONE;
    } else {
        v_service s = (v_service) p;
        serviceType = s->serviceType;
    }

    if (_this && (_this->kernelQos->builtin.v.enabled)) {
        if (p->qos != NULL) {
            topic = v_builtinTopicLookup(_this, V_CMPARTICIPANTINFO_ID);
            if (topic) {
                msg = v_topicMessageNew(topic);
                if (msg != NULL) {
                    procNameLength = strlen (p->processName);
                    participantName = v_entityName(p);
                    pid = os_procIdSelf();
                    memset(nodeName, 0, sizeof (nodeName));
                    if (os_gethostname(nodeName, sizeof (nodeName) - 1) != os_resultSuccess) {
                        nodeName[0] = 0;
                    }
                    nodeNameLength = strlen (nodeName);

                    info = v_builtinParticipantCMInfoData(msg);
                    info->key = v_publicGid(v_public(p));

                    /* generate xml for productDataPolicy
                     * xml_description buffer needs many characters for xml tags and PID value,
                     * currently 200 suffices
                     * ExecName contains at least strlen(procName) characters
                     * ParticipantName contains at lease strlen(pTempName) characters
                     * NodeName contains at lease strlen(nodeName) characters
                     */
                    if (participantName) {
                        parNameLength = strlen(participantName);
                    }
                    /* Note: the template has the vendor ID hardcoded to the DDSI vendor ID code
                       allocated by the OMG. */
#define T "<Product>" \
          "<ExecName><![CDATA[%s]]></ExecName>" \
          "<ParticipantName><![CDATA[%s]]></ParticipantName>" \
          "<PID>%i</PID>" \
          "<NodeName><![CDATA[%s]]></NodeName>" \
          "<FederationId>%x</FederationId>" \
          "<VendorId>1.2</VendorId>" \
          "<ServiceType>%u</ServiceType>" \
          "</Product>"
#define N_INTS_IN_T 3
                    {
                        os_size_t size = procNameLength+parNameLength+nodeNameLength+strlen(T)+11*N_INTS_IN_T+1;
                        int actlen;
                        xml_description = (char*)os_malloc(size);
                        actlen = snprintf (
                                    xml_description, size, T,
                                    p->processName,
                                    participantName ? participantName : "",
                                    pid ? pid : 0,
                                    nodeName,
                                    info->key.systemId,
                                    (unsigned) serviceType);
                        assert (actlen > 0 && (os_size_t) actlen < size);
                        (void)actlen;
                    }
#undef N_INTS_IN_T
#undef T
                    info->product.value = c_stringNew(base, xml_description);

                    if (xml_description) {
                        os_free(xml_description);
                        xml_description = NULL;
                    }

                    if (!info->product.value) {
                        c_free(msg);
                        msg = NULL;
                        OS_REPORT(OS_CRITICAL,
                                "kernel::v_builtin::v_builtinCreateCMParticipantInfo", V_RESULT_OUT_OF_MEMORY,
                                "Failed to create built-in CMParticipant topic message");
                    }

                } else {
                    OS_REPORT(OS_CRITICAL,
                              "kernel::v_builtin::v_builtinCreateCMParticipantInfo", V_RESULT_OUT_OF_MEMORY,
                              "Failed to create built-in CMParticipant topic message");
                    msg = NULL;
                }
            } else {
                OS_REPORT(OS_CRITICAL,
                          "kernel::v_builtin::v_builtinCreateCMParticipantInfo", V_RESULT_INTERNAL_ERROR,
                          "Failed to lookup built-in CMParticipant topic");
                msg = NULL;
            }
        } else {
            OS_REPORT(OS_CRITICAL,
                      "kernel::v_builtin::v_builtinCreateCMParticipantInfo", V_RESULT_INTERNAL_ERROR,
                      "Failed to produce built-in CMParticipant topic");
            msg = NULL;
        }
    }
    else
    {
        msg = NULL;
    }
    return msg;
}

v_message
v_builtinCreateParticipantInfo (
    v_builtin _this,
    v_participant p)
{
    v_message msg;
    v_topic topic;
    c_long size;
    struct v_participantInfo *info;

    assert(p != NULL);
    assert(C_TYPECHECK(p,v_participant));
    assert(C_TYPECHECK(_this,v_builtin));

    /* Pre condition checking */
    if (p == NULL) {
        OS_REPORT(OS_CRITICAL,
                  "kernel::v_builtin::v_builtinCreateParticipantInfo", V_RESULT_PRECONDITION_NOT_MET,
                  "Operation failed pre condition not met. _this = 0x%" PA_PRIxADDR ", participant = 0x%" PA_PRIxADDR,
                  (os_address)_this, (os_address)p);
        return NULL;
    }

    if (_this && (_this->kernelQos->builtin.v.enabled)) {
        if (p->qos != NULL) {
            topic = v_builtinTopicLookup(_this, V_PARTICIPANTINFO_ID);
            if (topic) {
                msg = v_topicMessageNew(topic);
                if (msg != NULL) {
                    size = p->qos->userData.v.size;
                    info = (struct v_participantInfo *) (msg + 1);
                    info->key = v_publicGid(v_public(p));
                    info->user_data.size = size;
                    info->user_data.value = NULL;

                    if (size > 0) {
                        info->user_data.value = c_arrayNew_s(c_octet_t(c_getBase(c_object(p))), (c_ulong) size);
                        if (info->user_data.value) {
                            memcpy(info->user_data.value, p->qos->userData.v.value, (size_t) size);
                        } else {
                            OS_REPORT(OS_CRITICAL,
                                      "kernel::v_builtin::v_builtinCreateParticipantInfo", V_RESULT_OUT_OF_MEMORY,
                                      "Failed to allocate built-in ParticipantInfo topic message user data");
                        }
                    } else {
                        info->user_data.value = NULL;
                    }
                } else {
                    OS_REPORT(OS_CRITICAL,
                              "kernel::v_builtin::v_builtinCreateParticipantInfo", V_RESULT_OUT_OF_MEMORY,
                              "Failed to create built-in ParticipantInfo topic message");
                    msg = NULL;
                }
            } else {
                OS_REPORT(OS_CRITICAL,
                          "kernel::v_builtin::v_builtinCreateParticipantInfo", V_RESULT_INTERNAL_ERROR,
                          "Failed to lookup built-in ParticipantInfo topic");
                msg = NULL;
            }
        } else {
            OS_REPORT(OS_CRITICAL,
                      "kernel::v_builtin::v_builtinCreateParticipantInfo", V_RESULT_INTERNAL_ERROR,
                      "Failed to produce built-in ParticipantInfo topic");
            msg = NULL;
        }
    }
    else
    {
        msg = NULL;
    }
    return msg;
}


v_message
v_builtinCreateTopicInfo (
    v_builtin _this,
    v_topic topic)
{
    v_topic builtinTopic;
    v_message msg;
    v_result result;

    assert (topic != NULL);
    assert (C_TYPECHECK (topic, v_topic));

    if (_this == NULL) {
        return NULL;
    }
    assert (C_TYPECHECK (_this, v_builtin));

    /* In this case, as opposed to the other builtin topics, no check
     * if builtinTopic is enabled is done.  This is because the
     * topicInfo topic is always available. Disabling builtin topics
     * only disables the DCPSParticipant, DCPSSubscription and
     * DCPSPublication topics.
     */
    builtinTopic = v_builtinTopicLookup (_this, V_TOPICINFO_ID);
    if (builtinTopic == NULL) {
        OS_REPORT(OS_ERROR,
                  "kernel::v_builtin::v_builtinCreateTopicInfo", 0,
                  "Operation v_builtinTopicLookup(%d) failed."
                  OS_REPORT_NL "_this = %p, topic = %p",
                  V_TOPICINFO_ID, (void *) _this, (void *) topic);
        assert(FALSE);
        return NULL;
    }

    msg = v_topicMessageNew(builtinTopic);
    if (msg == NULL) {
        OS_REPORT(OS_ERROR,
                  "kernel::v_builtin::v_builtinCreateTopicInfo", 0,
                  "Failed to create built-in topic message");
        assert(FALSE);
        return NULL;
    }
    if ((result = v_topicFillTopicInfo ((struct v_topicInfo *) (msg + 1), topic)) != V_RESULT_OK) {
        OS_REPORT(OS_ERROR,
                  "kernel::v_builtin::v_builtinCreateTopicInfo", 0,
                  "Failed to fill built-in topic message");
        assert(FALSE);
        c_free (msg);
        return NULL;
    }
    return msg;
}

v_message
v_builtinCreateTypeInfo (
    v_builtin _this,
    v_typeRepresentation tr)
{
    v_message msg = NULL;
    v_topic topic;
    struct v_typeInfo *info;

    if (_this != NULL) {
        topic = v_builtinTopicLookup(_this, V_TYPEINFO_ID);
        if (topic) {
            msg = v_topicMessageNew (topic);
            if (msg != NULL) {
                info = v_builtinTypeInfoData(msg);
                info->name = c_keep(tr->typeName);
                info->data_representation_id = tr->dataRepresentationId;
                info->type_hash = tr->typeHash;
                info->meta_data = c_keep(tr->metaData);
                info->extentions = c_keep(tr->extentions);
            } else {
                OS_REPORT(OS_CRITICAL,
                          "kernel::v_builtin::v_builtinCreateTypeInfo", V_RESULT_INTERNAL_ERROR,
                          "Failed to create built-in TypeInfo topic message");
                assert(FALSE);
            }
        } else {
            OS_REPORT(OS_CRITICAL,
                      "kernel::v_builtin::v_builtinCreateTypeInfo", V_RESULT_INTERNAL_ERROR,
                      "Failed to lookup built-in TypeInfo topic");
            assert(FALSE);
        }
    }

    return msg;
}

static void
free_string_iter (
    c_iter iter)
{
    c_char *str;
    while ((str = (c_char *) c_iterTakeFirst (iter)) != NULL) {
        os_free (str);
    }
    c_iterFree (iter);
}

static v_result
v_setBuiltinPartitionPolicy (
    struct v_builtinPartitionPolicy *bpp,
    c_base base,
    const char *pp)
{
    c_iter partitions = c_splitString (pp, ",");
    c_ulong length = c_iterLength (partitions);
    c_type type = c_string_t (base);
    bpp->name = c_arrayNew (type, length > 0 ? length : 1);
    if (bpp->name == NULL) {
        goto err_out_of_memory;
    } else if (length > 0) {
        c_ulong i = 0;
        c_char *str;
        while ((str = (c_char *) c_iterTakeFirst (partitions)) != NULL) {
            assert(i < length);
            bpp->name[i] = c_stringNew (base, str);
            os_free (str);
            if (bpp->name[i] == NULL) {
                goto err_out_of_memory;
            }
            i++;
        }
    } else {
        if ((bpp->name[0] = c_stringNew (base, "")) == NULL) {
            goto err_out_of_memory;
        }
    }
    c_iterFree (partitions);
    return V_RESULT_OK;
 err_out_of_memory:
    /* caller expected to do c_free(msg) where msg contains bpp */
    free_string_iter (partitions);
    return V_RESULT_OUT_OF_MEMORY;
}

v_message
v_builtinCreatePublicationInfo (
    v_builtin _this,
    v_writer writer)
{
    const c_base base = c_getBase (c_object (writer));
    v_message msg;
    struct v_publicationInfo *info;
    v_participant participant;
    v_publisher publisher;
    v_topic topic, builtinTopic;
    v_writerQos qos;
    v_topicQos topicQos;

    assert (_this);
    assert (writer != NULL);
    assert (C_TYPECHECK (writer, v_writer));
    assert (C_TYPECHECK (_this, v_builtin));

    if ((publisher = v_publisher (writer->publisher)) == NULL) {
        OS_REPORT(OS_ERROR,
                  "kernel::v_builtin::v_builtinCreatePublicationInfo", 0,
                  "Internal error DataWriter has no Publisher reference.");
        assert(FALSE);
        return NULL;
    }
    participant = v_participant (publisher->participant);
    if ((builtinTopic = v_builtinTopicLookup (_this, V_PUBLICATIONINFO_ID)) == NULL) {
        OS_REPORT(OS_ERROR,
                  "kernel::v_builtin::v_builtinCreatePublicationInfo", 0,
                  "Operation v_builtinTopicLookup(\"V_PUBLICATIONINFO_ID\") failed."
                  OS_REPORT_NL "_this = %p, topic = %p",
                  (void *) _this, (void *) writer);
        assert(FALSE);
        return NULL;
    }

    topic = writer->topic;
    qos = writer->qos;
    topicQos = v_topicGetQos (topic);

    msg = v_topicMessageNew(builtinTopic);
    if (msg == NULL) {
        OS_REPORT(OS_ERROR,
                    "kernel::v_builtin::v_builtinCreatePublicationInfo", 0,
                    "Failed to create built-in \"V_TOPICINFO_ID\" topic message");
        goto err_out_of_memory;
    }

    info = (struct v_publicationInfo *) (msg + 1);

    info->key = v_publicGid (v_public (writer));
    info->participant_key = v_publicGid (v_public (participant));
    v_policyConvToExt_topic_name (&info->topic_name, v_topicName (topic));
    if (v_policyConvToExt_type_name (base, &info->type_name, v_topicDataType (topic)) != V_RESULT_OK) {
        goto err_out_of_memory;
    }
    v_policyConvToExt_durability (&info->durability, &qos->durability);
    v_policyConvToExt_deadline (&info->deadline, &qos->deadline);
    v_policyConvToExt_latency_budget (&info->latency_budget, &qos->latency);
    v_policyConvToExt_liveliness (&info->liveliness, &qos->liveliness);
    v_policyConvToExt_reliability (&info->reliability, &qos->reliability);
    v_policyConvToExt_lifespan (&info->lifespan, &qos->lifespan);
    v_policyConvToExt_destination_order (&info->destination_order, &qos->orderby);
    if (v_policyConvToExt_user_data (base, &info->user_data, &qos->userData) != V_RESULT_OK) {
        goto err_out_of_memory;
    }
    v_policyConvToExt_ownership (&info->ownership, &qos->ownership);
    v_policyConvToExt_ownership_strength (&info->ownership_strength, &qos->strength);
    v_policyConvToExt_presentation (&info->presentation, &publisher->qos->presentation);
    if (v_policyConvToExt_partition (base, &info->partition, &publisher->qos->partition) != V_RESULT_OK) {
        goto err_out_of_memory;
    }
    if (v_policyConvToExt_topic_data (base, &info->topic_data, &topicQos->topicData) != V_RESULT_OK) {
        goto err_out_of_memory;
    }
    if (v_policyConvToExt_group_data (base, &info->group_data, &publisher->qos->groupData) != V_RESULT_OK) {
        goto err_out_of_memory;
    }
    v_policyConvToExt_writer_data_lifecycle (&info->lifecycle, &qos->lifecycle);
    info->alive = writer->alive;
    return msg;

 err_out_of_memory:
    v_topicQosFree (topicQos);
    c_free (msg);
    return NULL;
}

v_message
v_builtinCreateCMDataWriterInfo (
    v_builtin _this,
    v_writer writer)
{
    v_message msg;
    v_topic builtinTopic;

    assert (writer != NULL);
    assert (C_TYPECHECK (writer, v_writer));
    assert (C_TYPECHECK (_this, v_builtin));

    if (_this == NULL || !_this->kernelQos->builtin.v.enabled) {
        return NULL;
    }

    if ((builtinTopic = v_builtinTopicLookup (_this, V_CMDATAWRITERINFO_ID)) == NULL) {
        OS_REPORT (OS_CRITICAL, "v_builtinCreateCMDataWriterInfo: ", V_RESULT_INTERNAL_ERROR,
                     "Operation v_builtinTopicLookup(\"CM_DATAWRITERINFO_ID\") failed."
                     OS_REPORT_NL "_this = 0x%" PA_PRIxADDR,
                     (os_address) _this);
        assert (FALSE);
        return NULL;
    }

    if ((msg = v_topicMessageNew (builtinTopic)) == NULL) {
        OS_REPORT(OS_CRITICAL,
                    "kernel::v_builtin::v_builtinCreateCMDataWriterInfo", V_RESULT_INTERNAL_ERROR,
                    "Failed to create built-in topic message");
        assert (FALSE);
        return NULL;
    }

    {
        struct v_dataWriterCMInfo *info = v_builtinDataWriterCMInfoData(msg);
        char transport_priority_str[128];
        v_writerQos qos = writer->qos;
        if (qos->transport.v.value != 0) {
            if (snprintf (transport_priority_str, sizeof (transport_priority_str),
                          "<Product><transport_priority>%ld</transport_priority></Product>",
                          (long) qos->transport.v.value) >= (int) sizeof (transport_priority_str)) {
                assert (0);
            }
        } else {
            transport_priority_str[0] = 0;
        }
        info->key = v_publicGid (v_public (writer));
        info->product.value = c_stringNew (c_getBase (writer), transport_priority_str);
        info->publisher_key = v_publicGid (v_public (writer->publisher));
        info->name = c_keep (v_entity (writer)->name);
        v_policyConvToExt_history (&info->history, &qos->history);
        v_policyConvToExt_resource_limits (&info->resource_limits, &qos->resource);
        v_policyConvToExt_writer_data_lifecycle (&info->writer_data_lifecycle, &qos->lifecycle);
    }
    return msg;
}

static c_bool
getTopic (
    c_object o,
    c_voidp arg)
{
    v_topic *topic = (v_topic *)arg;
    c_bool result = TRUE;

    if (c_instanceOf(o, "v_dataReaderEntry")) {
        v_dataReaderEntry entry = v_dataReaderEntry(o);
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
    } else if (c_instanceOf(o, "v_deliveryServiceEntry")) {
        v_deliveryServiceEntry entry = v_deliveryServiceEntry(o);
        *topic = c_keep(entry->topic);
    } else {
        OS_REPORT(OS_ERROR, "kernel::v_builtin::getTopic", V_RESULT_ILL_PARAM, "Type mismatch: object type is %s, but v_dataReaderEntry or v_deliveryServiceEntry was expected", c_metaObject(c_getType(o))->name);
        assert(FALSE);
    }

    return result;
}

v_message
v_builtinCreateSubscriptionInfo (
    v_builtin _this,
    v_reader reader)
{
    const c_base base = c_getBase (c_object (reader));
    v_message msg;
    struct v_subscriptionInfo *info;
    v_participant participant;
    v_subscriber subscriber;
    v_topic topic, builtinTopic;
    v_readerQos qos;
    v_topicQos topicQos;

    assert (_this);
    assert (reader != NULL);
    assert (C_TYPECHECK (reader, v_reader));
    assert (C_TYPECHECK (_this, v_builtin));

    if (!_this->kernelQos->builtin.v.enabled) {
        return NULL;
    }

    if ((subscriber = v_subscriber (reader->subscriber)) == NULL) {
        OS_REPORT(OS_ERROR,
                  "kernel::v_builtin::v_builtinCreateSubscriptionInfo", 0,
                  "Internal error DataReader has no Subscriber reference.");
        assert (FALSE);
        return NULL;
    }
    participant = v_participant (subscriber->participant);
    if ((builtinTopic = v_builtinTopicLookup (_this, V_SUBSCRIPTIONINFO_ID)) == NULL) {
        OS_REPORT(OS_ERROR,
                  "kernel::v_builtin::v_builtinCreateSubscriptionInfo", 0,
                  "Operation v_builtinTopicLookup(\"V_SUBSCRIPTIONINFO_ID\") failed."
                  OS_REPORT_NL "_txhis = %p",
                  (void *) _this);
        assert (FALSE);
        return NULL;
    }

    topic = NULL;
    v_readerWalkEntries(reader,getTopic,&topic);
    assert (topic);
    qos = reader->qos;
    topicQos = v_topicGetQos (topic);

    msg = v_topicMessageNew (builtinTopic);
    if (msg == NULL) {
        OS_REPORT(OS_ERROR,
                  "kernel::v_builtin::v_builtinCreateSubscriptionInfo", V_RESULT_OUT_OF_MEMORY,
                  "Failed to create built-in PublicationInfo topic message");
        goto err_out_of_memory;
    }
    info = (struct v_subscriptionInfo *) (msg + 1);

    info->key = v_publicGid(v_public(reader));
    info->participant_key = v_publicGid(v_public(participant));
    v_policyConvToExt_topic_name (&info->topic_name, v_topicName (topic));
    if (v_policyConvToExt_type_name (base, &info->type_name, v_topicDataType (topic)) != V_RESULT_OK) {
        goto err_out_of_memory;
    }
    v_policyConvToExt_durability (&info->durability, &qos->durability);
    v_policyConvToExt_deadline (&info->deadline, &qos->deadline);
    v_policyConvToExt_latency_budget (&info->latency_budget, &qos->latency);
    v_policyConvToExt_liveliness (&info->liveliness, &qos->liveliness);
    v_policyConvToExt_reliability (&info->reliability, &qos->reliability);
    v_policyConvToExt_ownership (&info->ownership, &qos->ownership);
    v_policyConvToExt_destination_order (&info->destination_order, &qos->orderby);
    if (v_policyConvToExt_user_data (base, &info->user_data, &qos->userData) != V_RESULT_OK) {
        goto err_out_of_memory;
    }
    v_policyConvToExt_time_based_filter (&info->time_based_filter, &qos->pacing);
    v_policyConvToExt_presentation (&info->presentation, &subscriber->qos->presentation);
    if (v_policyConvToExt_partition (base, &info->partition, &subscriber->qos->partition) != V_RESULT_OK) {
        goto err_out_of_memory;
    }
    if (v_policyConvToExt_topic_data (base, &info->topic_data, &topicQos->topicData) != V_RESULT_OK) {
        goto err_out_of_memory;
    }
    v_topicQosFree(topicQos);
    if (v_policyConvToExt_group_data (base, &info->group_data, &subscriber->qos->groupData) != V_RESULT_OK) {
        goto err_out_of_memory;
    }
    v_policyConvToExt_reader_lifespan (&info->lifespan, &qos->lifespan);
    c_free (topic);
    return msg;

err_out_of_memory:
    v_topicQosFree (topicQos);
    c_free (topic);
    c_free (msg);
    return NULL;
}

v_message
v_builtinCreateCMDataReaderInfo (
    v_builtin _this,
    v_reader reader)
{
    v_message msg;
    v_topic builtinTopic;

    assert (reader != NULL);
    assert (C_TYPECHECK (reader, v_reader));
    assert (C_TYPECHECK (_this, v_builtin));

    if (_this == NULL || !_this->kernelQos->builtin.v.enabled) {
        return NULL;
    }

    if ((builtinTopic = v_builtinTopicLookup (_this, V_CMDATAREADERINFO_ID)) == NULL) {
        OS_REPORT (OS_CRITICAL, "v_builtinCreateCMDataReaderInfo: ", V_RESULT_INTERNAL_ERROR,
                   "Operation v_builtinTopicLookup(\"CM_DATAREADERINFO_ID\") failed."
                   OS_REPORT_NL "_this = 0x%" PA_PRIxADDR,
                   (os_address) _this);
        assert (FALSE);
        return NULL;
    }

    if ((msg = v_topicMessageNew (builtinTopic)) == NULL) {
        OS_REPORT(OS_CRITICAL,
                  "kernel::v_builtin::v_builtinCreateCMDataReaderInfo", V_RESULT_INTERNAL_ERROR,
                  "Failed to create built-in topic message");
        assert (FALSE);
        return NULL;
    }

    {
        struct v_dataReaderCMInfo *info = v_builtinDataReaderCMInfoData(msg);
        c_base base = c_getBase (reader);
        v_readerQos qos = reader->qos;
        info->key = v_publicGid (v_public (reader));
        info->product.value = c_stringNew (base, "");
        info->subscriber_key = v_publicGid (v_public (reader->subscriber));
        info->name = c_keep (v_entity (reader)->name);
        v_policyConvToExt_history (&info->history, &qos->history);
        v_policyConvToExt_resource_limits (&info->resource_limits, &qos->resource);
        v_policyConvToExt_reader_data_lifecycle (&info->reader_data_lifecycle, &qos->lifecycle);
        v_policyConvToExt_subscription_keys (&info->subscription_keys, &qos->userKey);
        v_policyConvToExt_reader_lifespan (&info->reader_lifespan, &qos->lifespan);
        v_policyConvToExt_share (base, &info->share, &qos->share);
    }

    return msg;
}

v_message
v_builtinCreateCMPublisherInfo (
    v_builtin _this,
    v_publisher publisher)
{
    v_message msg;
    v_topic builtinTopic;

    assert (publisher != NULL);
    assert (C_TYPECHECK (publisher, v_publisher));
    assert (C_TYPECHECK (_this, v_builtin));

    if (_this == NULL || !_this->kernelQos->builtin.v.enabled) {
        return NULL;
    }

    if ((builtinTopic = v_builtinTopicLookup (_this, V_CMPUBLISHERINFO_ID)) == NULL) {
        OS_REPORT (OS_CRITICAL, "v_builtinCreateCMPublisherInfo: ", V_RESULT_INTERNAL_ERROR,
                     "Operation v_builtinTopicLookup(\"CM_PUBLISHERINFO_ID\") failed."
                     OS_REPORT_NL "_this = 0x%" PA_PRIxADDR,
                     (os_address) _this);
        assert (FALSE);
        return NULL;
    }

    if ((msg = v_topicMessageNew (builtinTopic)) == NULL) {
        OS_REPORT(OS_CRITICAL,
                    "kernel::v_builtin::v_builtinCreateCMPublisherInfo", V_RESULT_OUT_OF_MEMORY,
                    "Failed to create built-in topic message");
        assert (FALSE);
        return NULL;
    }

    {
        v_result result;
        struct v_publisherCMInfo *info = v_builtinPublisherCMInfoData(msg);
        c_base base = c_getBase (publisher);
        v_publisherQos qos = publisher->qos;

        info->key = v_publicGid (v_public (publisher));
        info->product.value = c_stringNew (base, "");
        info->participant_key = v_publicGid (v_public (publisher->participant));
        info->name = c_keep (v_entity (publisher)->name);
        v_policyConvToExt_entity_factory (&info->entity_factory, &qos->entityFactory);

        result = v_setBuiltinPartitionPolicy (&info->partition, base, qos->partition.v);
        if (result != V_RESULT_OK) {
          OS_REPORT(OS_CRITICAL,
                    "kernel::v_builtin::v_builtinCreateCMPublisherInfo", result,
                    "c_setBuiltinPartitionPolicy failed");
          c_free(msg);
          msg = NULL;
          assert(FALSE);
        }
    }
    return msg;
}

v_message
v_builtinCreateCMSubscriberInfo (
    v_builtin _this,
    v_subscriber subscriber)
{
    v_message msg;
    v_topic builtinTopic;

    assert (subscriber != NULL);
    assert (C_TYPECHECK (subscriber, v_subscriber));
    assert (C_TYPECHECK (_this, v_builtin));

    if (_this == NULL || !_this->kernelQos->builtin.v.enabled) {
        return NULL;
    }

    if ((builtinTopic = v_builtinTopicLookup (_this, V_CMSUBSCRIBERINFO_ID)) == NULL) {
        OS_REPORT (OS_CRITICAL, "v_builtinCreateCMSubscriberInfo: ", 0,
                     "Operation v_builtinTopicLookup(\"CM_SUBSCRIBERINFO_ID\") failed."
                     OS_REPORT_NL "_this = 0x%" PA_PRIxADDR,
                     (os_address) _this);
        assert (FALSE);
        return NULL;
    }

    if ((msg = v_topicMessageNew (builtinTopic)) == NULL) {
        OS_REPORT(OS_CRITICAL,
                  "kernel::v_builtin::v_builtinCreateCMSubscriberInfo", 0,
                  "Failed to create built-in topic message");
        assert (FALSE);
        return NULL;
    }

    {
        v_result result;
        struct v_subscriberCMInfo *info = v_builtinSubscriberCMInfoData(msg);
        c_base base = c_getBase (subscriber);
        v_subscriberQos qos = subscriber->qos;

        info->key = v_publicGid (v_public (subscriber));
        info->product.value = c_stringNew (base, "");
        info->participant_key = v_publicGid (v_public (subscriber->participant));
        info->name = c_keep (v_entity (subscriber)->name);
        v_policyConvToExt_entity_factory (&info->entity_factory, &qos->entityFactory);
        v_policyConvToExt_share (base, &info->share, &qos->share);

        result = v_setBuiltinPartitionPolicy (&info->partition, base, qos->partition.v);
        if (result != V_RESULT_OK) {
          OS_REPORT(OS_CRITICAL,
                    "kernel::v_builtin::v_builtinCreateCMSubscriberInfo", result,
                    "c_setBuiltinPartitionPolicy failed");
          c_free(msg);
          msg = NULL;
          assert(FALSE);
        }
    }
    return msg;
}

v_writeResult
v_builtinWriteHeartbeat(
    v_writer writer,
    c_ulong systemId,
    os_timeW timestamp,
    os_duration duration,
    v_state state)
{
    v_group group;
    v_message message = NULL;
    v_resendScope resendScope = V_RESEND_NONE;
    v_kernel kernel = NULL;
    v_writeResult result = V_WRITE_ERROR;

    assert(writer != NULL);

    kernel = v_objectKernel(writer);
    assert(kernel != NULL);
    group = v_groupSetGet(kernel->groupSet, V_BUILTIN_PARTITION, V_HEARTBEATINFO_NAME);
    assert(group != NULL);

    message = v_topicMessageNew(
        v_builtinTopicLookup(kernel->builtin, V_HEARTBEATINFO_ID));
    if (message != NULL) {
        struct v_heartbeatInfo *heartbeat;

        v_nodeState(message) = state;
        message->allocTime = os_timeEGet();
        message->qos = c_keep(writer->relQos);
        message->sequenceNumber = v_writerAllocSequenceNumber(writer);
        message->transactionId = 0;
        message->writeTime = timestamp;
        message->writerGID = v_publicGid(v_public(writer));
        message->writerInstanceGID = v_publicGid(NULL);

        heartbeat = v_builtinHeartbeatInfoData(message);
        heartbeat->period = v_durationFromOsDuration(duration);
        heartbeat->id.systemId = systemId;
        heartbeat->id.localId = 1;
        heartbeat->id.serial = 1;

        result = v_groupWrite(
            group, message, NULL, V_NETWORKID_ANY, &resendScope);
    }

    c_free(message);
    c_free(group);

    if (result != V_WRITE_SUCCESS) {
        OS_REPORT(OS_WARNING, "kernel::v_builtin::v_builtinWriteHeartbeat", result, "%s failed (%s)", OS_FUNCTION, v_writeResultString(result));
    }

    return result;
}

const c_char *
v_builtinInfoIdToName(
    v_infoId infoId)
{
    const c_char *name;

#define V__BUILTIN_CASE(info) case info ## _ID: name = info ## _NAME; break

    switch (infoId) {
        V__BUILTIN_CASE(V_PARTICIPANTINFO);
        V__BUILTIN_CASE(V_TOPICINFO);
        V__BUILTIN_CASE(V_PUBLICATIONINFO);
        V__BUILTIN_CASE(V_SUBSCRIPTIONINFO);
        V__BUILTIN_CASE(V_HEARTBEATINFO);
        V__BUILTIN_CASE(V_DELIVERYINFO);
        V__BUILTIN_CASE(V_C_AND_M_COMMAND);
        V__BUILTIN_CASE(V_CMPARTICIPANTINFO);
        V__BUILTIN_CASE(V_CMDATAWRITERINFO);
        V__BUILTIN_CASE(V_CMDATAREADERINFO);
        V__BUILTIN_CASE(V_CMPUBLISHERINFO);
        V__BUILTIN_CASE(V_CMSUBSCRIBERINFO);
        default:
            name = NULL;
            break;
    }

    assert (name != NULL);

    return name;
}
