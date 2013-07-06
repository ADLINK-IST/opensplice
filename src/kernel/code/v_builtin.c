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
    _this->writers[V_CMPARTICIPANTINFO_ID] = NULL;
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
    c_bool error;

    base = c_getBase(kernel);
    type = c_resolve(base,"kernelModule::v_builtin");

    if (type) {
        error = FALSE;
        _this = (v_builtin)c_new(type);
        c_free(type);
    } else {
        error = TRUE;
        _this = NULL;
        OS_REPORT_2(OS_ERROR,
                    "kernel::v_builtin::v_builtinNew",0,
                    "Operation failed, couldn't resolve type \"kernelModule::v_builtin\"."
                    OS_REPORT_NL "kernel = 0x%x, base = 0x%x",
                    kernel, base);
        assert(FALSE);
    }
    if (_this) {
        _this->kernelQos = c_keep(kernel->qos);
        /* create QoS's */
        tQos = v_topicQosNew(kernel, NULL);
        if (tQos != NULL) {
            tQos->durability.kind = V_DURABILITY_TRANSIENT;
            tQos->reliability.kind = V_RELIABILITY_RELIABLE;
            tQos->reliability.max_blocking_time.seconds = 0;
            tQos->reliability.max_blocking_time.nanoseconds = 100000000;
            tQos->durabilityService.service_cleanup_delay = C_TIME_ZERO;

            /* make the history qos to have keep all and unlimited depth so
             * that samples that are marked for resend don't get squeezed
             * out of the writer's history
             */
            tQos->history.kind = V_HISTORY_KEEPALL;
            tQos->history.depth = V_LENGTH_UNLIMITED;

            if (_this->kernelQos->builtin.enabled) {
                if (!error) {
                    _this->topics[V_PARTICIPANTINFO_ID] =
                           v_topicNew(kernel, V_PARTICIPANTINFO_NAME,
                                      "kernelModule::v_participantInfo",
                                      "key.localId,key.systemId", tQos);
                    error = (_this->topics[V_PARTICIPANTINFO_ID] == NULL);
                    if (error) {
                        OS_REPORT_1(OS_ERROR,
                                    "kernel::v_builtin::v_builtinNew", 0,
                                    "Operation failed because v_topicNew failed for:"
                                    OS_REPORT_NL "Topic: \"%s"
                                    OS_REPORT_NL "Type: \"kernelModule::v_participantInfo\""
                                    OS_REPORT_NL "Key(s): \"key.localId,key.systemId\"",
                                    V_PARTICIPANTINFO_NAME);
                    }
                }
                if (!error) {
                   _this->topics[V_CMPARTICIPANTINFO_ID] =
                          v_topicNew(kernel, V_CMPARTICIPANTINFO_NAME,
                                     "kernelModule::v_participantCMInfo",
                                     "key.localId,key.systemId", tQos);
                   error = (_this->topics[V_CMPARTICIPANTINFO_ID] == NULL);
                   if (error) {
                       OS_REPORT_1(OS_ERROR,
                                   "kernel::v_builtin::v_builtinNew", 0,
                                   "Operation failed because v_topicNew failed for:"
                                   OS_REPORT_NL "Topic: \"%s"
                                   OS_REPORT_NL "Type: \"kernelModule::v_participantCMInfo\""
                                   OS_REPORT_NL "Key(s): \"key.localId,key.systemId\"",
                                   V_CMPARTICIPANTINFO_NAME);
                   }
                }
                if (!error) {
                    _this->topics[V_SUBSCRIPTIONINFO_ID] =
                           v_topicNew(kernel, V_SUBSCRIPTIONINFO_NAME,
                                      "kernelModule::v_subscriptionInfo",
                                      "key.localId,key.systemId", tQos);
                    error = (_this->topics[V_SUBSCRIPTIONINFO_ID] == NULL);
                    if (error) {
                        OS_REPORT_1(OS_ERROR,
                                    "kernel::v_builtin::v_builtinNew", 0,
                                    "Operation failed because v_topicNew failed for:"
                                    OS_REPORT_NL "Topic: \"%s"
                                    OS_REPORT_NL "Type: \"kernelModule::v_subscriptionInfo\""
                                    OS_REPORT_NL "Key(s): \"key.localId,key.systemId\"",
                                    V_SUBSCRIPTIONINFO_NAME);
                    }
                }
            }
            if (!error) {
                _this->topics[V_PUBLICATIONINFO_ID] =
                   v_topicNew(kernel, V_PUBLICATIONINFO_NAME,
                              "kernelModule::v_publicationInfo",
                              "key.localId,key.systemId", tQos);
                error = (_this->topics[V_PUBLICATIONINFO_ID] == NULL);
                if (error) {
                    OS_REPORT_1(OS_ERROR,
                                "kernel::v_builtin::v_builtinNew", 0,
                                "Operation failed because v_topicNew failed for:"
                                OS_REPORT_NL "Topic: \"%s"
                                OS_REPORT_NL "Type: \"kernelModule::v_publicationInfo\""
                                OS_REPORT_NL "Key(s): \"key.localId,key.systemId\"",
                                V_PUBLICATIONINFO_NAME);
                }
            }
            if (!error) {
                _this->topics[V_TOPICINFO_ID] =
                       v_topicNew(kernel, V_TOPICINFO_NAME,
                                  "kernelModule::v_topicInfo",
                                  "key.localId,key.systemId", tQos);
                error = (_this->topics[V_TOPICINFO_ID] == NULL);
                if (error) {
                    OS_REPORT_1(OS_ERROR,
                                "kernel::v_builtin::v_builtinNew", 0,
                                "Operation failed because v_topicNew failed for:"
                                OS_REPORT_NL "Topic: \"%s"
                                OS_REPORT_NL "Type: \"kernelModule::v_topicInfo\""
                                OS_REPORT_NL "Key(s): \"key.localId,key.systemId\"",
                                V_TOPICINFO_NAME);
                }
            }
            if (!error) {
                _this->topics[V_HEARTBEATINFO_ID] =
                       v_topicNew(kernel, V_HEARTBEATINFO_NAME,
                                  "kernelModule::v_heartbeatInfo",
                                  "id.localId,id.systemId", NULL);
                error = (_this->topics[V_HEARTBEATINFO_ID] == NULL);
                if (error) {
                    OS_REPORT_1(OS_ERROR,
                                "kernel::v_builtin::v_builtinNew", 0,
                                "Operation failed because v_topicNew failed for:"
                                OS_REPORT_NL "Topic: \"%s"
                                OS_REPORT_NL "Type: \"kernelModule::v_heartbeatInfo\""
                                OS_REPORT_NL "Key(s): \"key.localId,key.systemId\"",
                                V_HEARTBEATINFO_NAME);
                }
            }
            if (!error) {
                _this->topics[V_C_AND_M_COMMAND_ID] =
                       v_topicNew(kernel, V_C_AND_M_COMMAND_NAME,
                                  "kernelModule::v_controlAndMonitoringCommand",
                                  "key.localId,key.systemId", tQos);
                error = (_this->topics[V_C_AND_M_COMMAND_ID] == NULL);
                if (error) {
                    OS_REPORT_1(OS_ERROR,
                                "kernel::v_builtin::v_builtinNew", 0,
                                "Operation failed because v_topicNew failed for:"
                                OS_REPORT_NL "Topic: \"%s"
                                OS_REPORT_NL "Type: \"kernelModule::v_controlAndMonitoringCommand\""
                                OS_REPORT_NL "Key(s): \"key.localId,key.systemId\"",
                                V_C_AND_M_COMMAND_NAME);
                }
            }
            if (!error) {
                _this->topics[V_DELIVERYINFO_ID] =
                       v_topicNew(kernel, V_DELIVERYINFO_NAME,
                                  "kernelModule::v_deliveryInfo",
                                  NULL, NULL);
                error = (_this->topics[V_DELIVERYINFO_ID] == NULL);
                if (error) {
                    OS_REPORT_1(OS_ERROR,
                                "kernel::v_builtin::v_builtinNew", 0,
                                "Operation failed because v_topicNew failed for:"
                                OS_REPORT_NL "Topic: \"%s"
                                OS_REPORT_NL "Type: \"kernelModule::v_deliveryInfo\""
                                OS_REPORT_NL "Key(s): \"key.localId,key.systemId\"",
                                V_DELIVERYINFO_NAME);
                }
            }
            c_free(tQos);
        } else {
            error = TRUE;
            OS_REPORT_1(OS_ERROR,
                        "kernel::v_builtin::v_builtinNew", 0,
                        "Operation failed because v_topicQosNew failed for: kernel = 0x%x",
                        kernel);
        }
        if (!error) {
            _this->participant = v_participantNew(kernel,
                                                  V_BUILT_IN_PARTICIPANT_NAME,
                                                  NULL,NULL,TRUE);
            error = (_this->participant == NULL);
            if (error) {
                OS_REPORT(OS_ERROR,
                          "kernel::v_builtin::v_builtinNew", 0,
                          "Failed to allocate \"Built-in participant\" object.");
            }
        }
        if (!error) {
            pQos = v_publisherQosNew(kernel, NULL);
            if (pQos) {
                pQos->presentation.access_scope = V_PRESENTATION_TOPIC;
                pQos->entityFactory.autoenable_created_entities = TRUE;
                c_free(pQos->partition); /* free default partition "" */
                pQos->partition = c_stringNew(c_getBase(c_object(kernel)),
                                              V_BUILTIN_PARTITION);
                error = (pQos->partition == NULL);
                if (error) {
                    OS_REPORT_1(OS_ERROR,
                                "kernel::v_builtin::v_builtinNew", 0,
                                "Operation failed because c_stringNew failed for: Partition \"%s\".",
                                V_BUILTIN_PARTITION);
                } else {
                    _this->publisher = v_publisherNew(_this->participant,
                                                      "Built-in publisher",
                                                      pQos, TRUE);
                    error = (_this->publisher == NULL);
                    if (error) {
                        OS_REPORT_1(OS_ERROR,
                                    "kernel::v_builtin::v_builtinNew", 0,
                                    "Operation failed because v_publisherNew failed for: Participant \"%s\".",
                                    v_entityName(_this->participant));
                    }
                }
                c_free(pQos);
            } else {
                error = TRUE;
                OS_REPORT_1(OS_ERROR,
                            "kernel::v_builtin::v_builtinNew", 0,
                            "Operation failed because v_publisherQosNew failed for: kernel = 0x%x",
                            kernel);
            }
        }
        if (!error) {
            wQos = v_writerQosNew(kernel, NULL);
            if (wQos) {
                wQos->durability.kind = V_DURABILITY_TRANSIENT;
                wQos->reliability.kind = V_RELIABILITY_RELIABLE;
                /* make the history qos to have keep all and unlimited depth so
                 * that samples that are marked for resend don't get squeezed
                 * out of the writer's history
                 */
                wQos->history.kind = V_HISTORY_KEEPALL;
                wQos->history.depth = V_LENGTH_UNLIMITED;

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
                error = (_this->writers[V_TOPICINFO_ID] == NULL);
                if (error) {
                    OS_REPORT_2(OS_ERROR,
                                "kernel::v_builtin::v_builtinNew", 0,
                                "Operation failed because v_writerNew failed for:"
                                OS_REPORT_NL "Name: \"%s"
                                OS_REPORT_NL "Topic: \"%s",
                                V_TOPICINFO_NAME "TopicInfoWriter",
                                V_TOPICINFO_NAME);
                } else {
                    wQos->lifecycle.autodispose_unregistered_instances = TRUE;

                    if (kernel->qos->builtin.enabled) {
                        _this->writers[V_PUBLICATIONINFO_ID] =
                             v_writerNew(_this->publisher,
                                         V_PUBLICATIONINFO_NAME "PublicationInfoWriter",
                                         _this->topics[V_PUBLICATIONINFO_ID],
                                         wQos, TRUE);
                        error = (_this->writers[V_PUBLICATIONINFO_ID] == NULL);
                        if (error) {
                            OS_REPORT_2(OS_ERROR,
                                        "kernel::v_builtin::v_builtinNew", 0,
                                        "Operation failed because v_writerNew failed for:"
                                        OS_REPORT_NL "Name: \"%s"
                                        OS_REPORT_NL "Topic: \"%s",
                                        V_PUBLICATIONINFO_NAME "PublicationInfoWriter",
                                        V_PUBLICATIONINFO_NAME);
                        }
                        _this->writers[V_PARTICIPANTINFO_ID] =
                             v_writerNew(_this->publisher,
                                         V_PARTICIPANTINFO_NAME "ParticipantInfoWriter",
                                         _this->topics[V_PARTICIPANTINFO_ID],
                                         wQos, TRUE);
                        error = (_this->writers[V_PARTICIPANTINFO_ID] == NULL);
                        if (error) {
                            OS_REPORT_2(OS_ERROR,
                                        "kernel::v_builtin::v_builtinNew", 0,
                                        "Operation failed because v_writerNew failed for:"
                                        OS_REPORT_NL "Name: \"%s"
                                        OS_REPORT_NL "Topic: \"%s",
                                        V_PARTICIPANTINFO_NAME "ParticipantInfoWriter",
                                        V_PARTICIPANTINFO_NAME);
                        }
                        _this->writers[V_CMPARTICIPANTINFO_ID] =
                            v_writerNew(_this->publisher,
                                     V_CMPARTICIPANTINFO_NAME "ParticipantCMInfoWriter",
                                     _this->topics[V_CMPARTICIPANTINFO_ID],
                                     wQos, TRUE);
                        error = (_this->writers[V_CMPARTICIPANTINFO_ID] == NULL);
                        if (error) {
                            OS_REPORT_2(OS_ERROR,
                                    "kernel::v_builtin::v_builtinNew", 0,
                                    "Operation failed because v_writerNew failed for:"
                                    OS_REPORT_NL "Name: \"%s"
                                    OS_REPORT_NL "Topic: \"%s",
                                    V_CMPARTICIPANTINFO_NAME "CMParticipantCMInfoWriter",
                                    V_CMPARTICIPANTINFO_NAME);
                        }
                        if (!error) {
                            _this->writers[V_SUBSCRIPTIONINFO_ID] =
                                 v_writerNew(_this->publisher,
                                             V_SUBSCRIPTIONINFO_NAME "SubscriptionInfoWriter",
                                             _this->topics[V_SUBSCRIPTIONINFO_ID],
                                             wQos, TRUE);
                            error = (_this->writers[V_SUBSCRIPTIONINFO_ID] == NULL);
                            if (error) {
                                OS_REPORT_2(OS_ERROR,
                                            "kernel::v_builtin::v_builtinNew", 0,
                                            "Operation failed because v_writerNew failed for:"
                                            OS_REPORT_NL "Name: \"%s"
                                            OS_REPORT_NL "Topic: \"%s",
                                            V_SUBSCRIPTIONINFO_NAME "SubscriptionInfoWriter",
                                            V_SUBSCRIPTIONINFO_NAME);
                            }
                        }
                    } else {
                        _this->writers[V_PARTICIPANTINFO_ID] = NULL;
                        _this->writers[V_CMPARTICIPANTINFO_ID] = NULL;
                        _this->writers[V_SUBSCRIPTIONINFO_ID] = NULL;

                        /* If the resources consumed by builtin topics are unwanted,
                         * then do not write the v_publicationInfo samples as TRANSIENT
                         * data but as VOLATILE data instead.
                         */
                        wQos->durability.kind = V_DURABILITY_VOLATILE;

                        _this->writers[V_PUBLICATIONINFO_ID] =
                             v_writerNew(_this->publisher,
                                         V_PUBLICATIONINFO_NAME "PublicationInfoWriter",
                                         _this->topics[V_PUBLICATIONINFO_ID],
                                         wQos, TRUE);
                        wQos->durability.kind = V_DURABILITY_TRANSIENT;
                        error = (_this->writers[V_PUBLICATIONINFO_ID] == NULL);
                        if (error) {
                            OS_REPORT_2(OS_ERROR,
                                        "kernel::v_builtin::v_builtinNew", 0,
                                        "Operation failed because v_writerNew failed for:"
                                        OS_REPORT_NL "Name: \"%s"
                                        OS_REPORT_NL "Topic: \"%s",
                                        V_PUBLICATIONINFO_NAME "PublicationInfoWriter",
                                        V_PUBLICATIONINFO_NAME);
                        }
                    }
                }
                if (!error) {
                    wQos->durability.kind = V_DURABILITY_VOLATILE;
                    _this->writers[V_HEARTBEATINFO_ID] =
                         v_writerNew(_this->publisher,
                                     V_HEARTBEATINFO_NAME "HeartbeatInfoWriter",
                                     _this->topics[V_HEARTBEATINFO_ID],
                                     NULL /* default qos */, TRUE);
                    error = (_this->writers[V_HEARTBEATINFO_ID] == NULL);
                    if (error) {
                        OS_REPORT_2(OS_ERROR,
                                    "kernel::v_builtin::v_builtinNew", 0,
                                    "Operation failed because v_writerNew failed for:"
                                    OS_REPORT_NL "Name: \"%s"
                                    OS_REPORT_NL "Topic: \"%s",
                                    V_HEARTBEATINFO_NAME "HeartbeatInfoWriter",
                                    V_HEARTBEATINFO_NAME);
                    }
                }
                if (!error) {
                    wQos->durability.kind = V_DURABILITY_VOLATILE;
                    _this->writers[V_C_AND_M_COMMAND_ID] =
                         v_writerNew(_this->publisher,
                                     V_C_AND_M_COMMAND_NAME "Writer",
                                     _this->topics[V_C_AND_M_COMMAND_ID],
                                     wQos, TRUE);
                    error = (_this->writers[V_C_AND_M_COMMAND_ID] == NULL);
                    if (error) {
                        OS_REPORT_2(OS_ERROR,
                                    "kernel::v_builtin::v_builtinNew", 0,
                                    "Operation failed because v_writerNew failed for:"
                                    OS_REPORT_NL "Name: \"%s"
                                    OS_REPORT_NL "Topic: \"%s",
                                    V_C_AND_M_COMMAND_NAME "Writer",
                                    V_C_AND_M_COMMAND_NAME);
                    }
                }
                if (!error) {
                    wQos->lifecycle.autodispose_unregistered_instances = FALSE;

                    _this->writers[V_DELIVERYINFO_ID] =
                         v_writerNew(_this->publisher,
                                     V_DELIVERYINFO_NAME "DeliveryInfoWriter",
                                     _this->topics[V_DELIVERYINFO_ID],
                                     wQos, TRUE);
                    error = (_this->writers[V_DELIVERYINFO_ID] == NULL);
                    if (error) {
                        OS_REPORT_2(OS_ERROR,
                                    "kernel::v_builtin::v_builtinNew", 0,
                                    "Operation failed because v_writerNew failed for:"
                                    OS_REPORT_NL "Name: \"%s"
                                    OS_REPORT_NL "Topic: \"%s",
                                    V_DELIVERYINFO_NAME "DeliveryInfoWriter",
                                    V_DELIVERYINFO_NAME);
                    }
                }
                c_free(wQos);
            } else {
                OS_REPORT_1(OS_ERROR,
                            "kernel::v_builtin::v_builtinNew", 0,
                            "Operation failed because v_writerQosNew failed for: kernel = 0x%x",
                            kernel);
            }
            if (!error) {
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
                    msg = v_builtinCreateCMParticipantInfo(_this,
                                                         _this->participant);
                    v_writerWrite(_this->writers[V_CMPARTICIPANTINFO_ID],
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
        }
        if (error) {
            c_free(_this);
            _this = NULL;
        }
    } else {
        OS_REPORT(OS_ERROR,
                  "kernel::v_builtin::v_builtinNew", 0,
                  "Failed to allocate v_builtin object.");
        assert(FALSE);
        _this = NULL;
    }
    return _this;
}

#define PROCNAME_SIZE (128)
#define XML_SIZE (1024)
v_message
v_builtinCreateCMParticipantInfo (
    v_builtin _this,
    v_participant p)
{
    v_message msg;
    v_topic topic;
    struct v_participantCMInfo *info;
    char *xml_description, *participantName;
    char procName[PROCNAME_SIZE];
    os_int32 parNameLength =0;
    os_int32 procNameLength =0;

    c_base base = c_getBase(c_object(p));
    os_int pid =0;

    assert(p != NULL);
    assert(C_TYPECHECK(p,v_participant));
    assert(C_TYPECHECK(_this,v_builtin));

    /* Pre condition checking */
    if (p == NULL) {
        OS_REPORT_2(OS_ERROR,
                    "kernel::v_builtin::v_builtinCreateCMParticipantInfo", 0,
                    "Operation failed pre condition not met. _this = 0x%x, participant = 0x%x",
                    _this,p);
        return NULL;
    }

    if (_this && (_this->kernelQos->builtin.enabled)) {
        if (p->qos != NULL) {
            topic = v_builtinTopicLookup(_this, V_CMPARTICIPANTINFO_ID);
            if (topic) {
                msg = v_topicMessageNew(topic);
                if (msg != NULL) {
                    procNameLength = os_procGetProcessName(procName,PROCNAME_SIZE);
                    /* If truncation occurred procNameLength >= PROCNAME_SIZE; we don't
                     * care about truncation, but want to use the proper (truncated)
                     * length in that case. */
                    if(procNameLength >= PROCNAME_SIZE){
                        procNameLength = PROCNAME_SIZE - 1;
                    }
                    participantName = v_entityName(p);
                    pid = os_procIdToInteger(os_procIdSelf());
                    info = v_builtinParticipantCMInfoData(_this,msg);
                    info->key = v_publicGid(v_public(p));

                    /* generate xml for productDataPolicy
                     * xml_description buffer needs at least 120 characters for xml tags
                     * this includes the PID value.
                     * ExecName contains at least strlen(procName) characters
                     * ParticipantName contains at lease strlen(pTempName) characters
                     */
                    if (participantName) {
                        parNameLength = strlen(participantName);
                    }
                    xml_description = (char*)os_malloc(procNameLength+parNameLength+120);
                    os_sprintf (xml_description,
                                "<Product><ExecName><![CDATA[%s]]></ExecName>"
                                "<ParticipantName><![CDATA[%s]]></ParticipantName>"
                                "<PID>%i</PID></Product>",
                                procName,
                                participantName ? participantName : "",
                                pid ? pid : 0
                                );
                    info->product.value = c_stringNew(base,xml_description);

                    if (xml_description) {
                        os_free(xml_description);
                        xml_description = NULL;
                    }
                } else {
                    OS_REPORT(OS_ERROR,
                              "kernel::v_builtin::v_builtinCreateCMParticipantInfo", 0,
                              "Failed to create built-in CMParticipant topic message");
                    msg = NULL;
                }
            } else {
                OS_REPORT(OS_ERROR,
                          "kernel::v_builtin::v_builtinCreateCMParticipantInfo", 0,
                          "Failed to lookup built-in CMParticipant topic");
                msg = NULL;
            }
        } else {
            OS_REPORT(OS_ERROR,
                      "kernel::v_builtin::v_builtinCreateCMParticipantInfo", 0,
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
    c_type type;
    c_long size;
    struct v_participantInfo *info;

    assert(p != NULL);
    assert(C_TYPECHECK(p,v_participant));
    assert(C_TYPECHECK(_this,v_builtin));

    /* Pre condition checking */
    if (p == NULL) {
        OS_REPORT_2(OS_ERROR,
                    "kernel::v_builtin::v_builtinCreateParticipantInfo", 0,
                    "Operation failed pre condition not met. _this = 0x%x, participant = 0x%x",
                    _this,p);
        return NULL;
    }

    if (_this && (_this->kernelQos->builtin.enabled)) {
        if (p->qos != NULL) {
            topic = v_builtinTopicLookup(_this, V_PARTICIPANTINFO_ID);
            if (topic) {
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
                        if (info->user_data.value) {
                            memcpy(info->user_data.value,
                                   p->qos->userData.value,
                                   size);
                        } else {
                            c_free(msg);
                            msg = NULL;
                            OS_REPORT(OS_ERROR,
                                      "kernel::v_builtin::v_builtinCreateParticipantInfo", 0,
                                      "Failed to allocate built-in ParticipantInfo topic message user data");
                        }
                    } else {
                        info->user_data.value = NULL;
                    }
                } else {
                    OS_REPORT(OS_ERROR,
                              "kernel::v_builtin::v_builtinCreateParticipantInfo", 0,
                              "Failed to create built-in ParticipantInfo topic message");
                    msg = NULL;
                }
            } else {
                OS_REPORT(OS_ERROR,
                          "kernel::v_builtin::v_builtinCreateParticipantInfo", 0,
                          "Failed to lookup built-in ParticipantInfo topic");
                msg = NULL;
            }
        } else {
            OS_REPORT(OS_ERROR,
                      "kernel::v_builtin::v_builtinCreateParticipantInfo", 0,
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

    /* Operation pre condition checking */
    if (topic == NULL) {
        OS_REPORT_2(OS_ERROR,
                    "kernel::v_builtin::v_builtinCreateTopicInfo", 0,
                    "Operation failed pre condition not met."
                    OS_REPORT_NL "_this = 0x%x, topic = 0x%x",
                    _this,topic);
        assert(FALSE);
        return NULL;
    }
    /*
     * In this case, as opposed to the other builtin topics, no check if builtinTopic is enabled is done.
     * This is because the topicInfo topic is always available. Disabling builtin topics only disables
     * the DCPSParticipant, DCPSSubscription and DCPSPublication topics.
     */
    if (_this) {
        if (topic->qos == NULL) {
            OS_REPORT_2(OS_ERROR,
                        "kernel::v_builtin::v_builtinCreateTopicInfo", 0,
                        "Operation failed topic->qos == NULL."
                        OS_REPORT_NL "_this = 0x%x, topic = 0x%x",
                        _this,topic);
            assert(FALSE);
            return NULL;
        }
        builtinTopic = v_builtinTopicLookup(_this, V_TOPICINFO_ID);
        if (builtinTopic == NULL) {
            OS_REPORT_3(OS_ERROR,
                        "kernel::v_builtin::v_builtinCreateTopicInfo", 0,
                        "Operation v_builtinTopicLookup(\"%s\") failed."
                        OS_REPORT_NL "_this = 0x%x, topic = 0x%x",
                        V_TOPICINFO_ID, _this,topic);
            assert(FALSE);
            return NULL;
        }
        str = c_metaScopedName(c_metaObject(v_topicDataType(topic)));
        if (str == NULL) {
            OS_REPORT_2(OS_ERROR,
                        "kernel::v_builtin::v_builtinCreateTopicInfo", 0,
                        "Operation c_metaScopedName(topic->type) failed."
                        OS_REPORT_NL "_this = 0x%x, topic = 0x%x",
                        _this,topic);
            assert(FALSE);
        }
        msg = v_topicMessageNew(builtinTopic);
        if (msg == NULL) {
            OS_REPORT_1(OS_ERROR,
                        "kernel::v_builtin::v_builtinCreateTopicInfo", 0,
                        "Failed to create built-in \"%s\" topic message",
                        V_TOPICINFO_ID);
            assert(FALSE);
            return NULL;
        }
        if (msg != NULL) {
            info = v_builtinTopicInfoData(_this,msg);
            base = c_getBase(c_object(topic));
            info->type_name = c_stringNew(base, str);
            os_free(str);
            if (info->type_name == NULL) {
                OS_REPORT_4(OS_ERROR,
                            "kernel::v_builtin::v_builtinCreateTopicInfo", 0,
                            "Operation c_stringNew(base=0x%x, str=\"%s\") failed."
                            OS_REPORT_NL "_this = 0x%x, topic = 0x%x",
                            base, str, _this,topic);
                c_free(msg);
                msg = NULL;
                assert(FALSE);
            }
        }
        if (msg != NULL) {
            /* Initialize Message Topic Qos */
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

            if (topicQos->topicData.size > 0) {
                type = c_octet_t(base);
                info->topic_data.value = c_arrayNew(type,topicQos->topicData.size);
                if (info->topic_data.value) {
                    memcpy(info->topic_data.value,
                           topicQos->topicData.value,
                           topicQos->topicData.size);
                } else {
                    c_free(msg);
                    msg = NULL;
                    OS_REPORT_3(OS_ERROR,
                                "kernel::v_builtin::v_builtinCreateTopicInfo", 0,
                                "Operation c_arrayNew(type=0x%x, size=%d)"
                                OS_REPORT_NL "failed for built-in \"%s\" topic message",
                                type, topicQos->topicData.size, V_TOPICINFO_ID);
                    assert(FALSE);
                }
                c_free(type);
            }
        }
        if (msg != NULL) {
            /* Initialize Message Type Description */
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
                    OS_REPORT_3(OS_ERROR,
                                "kernel::v_builtin::v_builtinCreateTopicInfo", 0,
                                "Operation sd_serializerToString(serializer=0x%x, meta_data=0x%x)"
                                OS_REPORT_NL "failed for built-in \"%s\" topic message.",
                                serializer, meta_data, V_TOPICINFO_ID);
                    c_free(msg);
                    msg = NULL;
                    assert(FALSE);
                }
                sd_serializedDataFree(meta_data);
                sd_serializerFree(serializer);
            } else {
                OS_REPORT_2(OS_ERROR,
                            "kernel::v_builtin::v_builtinCreateTopicInfo", 0,
                            "Operation sd_serializerXMLTypeinfoNew(base=0x%x, FALSE)"
                            OS_REPORT_NL "failed for built-in \"%s\" topic message.",
                            base, V_TOPICINFO_ID);
                c_free(msg);
                msg = NULL;
                assert(FALSE);
            }
        }
        if (msg != NULL) {
            /* Initialize message key info */
            info->key.systemId = topic->crcOfName;
            info->key.localId = topic->crcOfTypeName;
            info->key.serial = 0;

            info->name = c_keep(v_topicName(topic));
            info->key_list = c_keep(v_topicKeyExpr(topic));
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

    /* Operation pre condition checking */
    if (writer == NULL) {
        OS_REPORT_2(OS_ERROR,
                    "kernel::v_builtin::v_builtinCreatePublicationInfo", 0,
                    "Operation failed pre condition not met. _this = 0x%x, writer = 0x%x",
                    _this,writer);
        assert(FALSE);
        return NULL;
    }
    publisher = v_publisher(writer->publisher);
    if (publisher == NULL) {
        OS_REPORT(OS_ERROR,
                  "kernel::v_builtin::v_builtinCreatePublicationInfo", 0,
                  "Internal error DataWriter has no Publisher reference.");
        assert(FALSE);
        return NULL;
    }
    builtinTopic = v_builtinTopicLookup(_this,V_PUBLICATIONINFO_ID);
    if (builtinTopic == NULL) {
        OS_REPORT_2(OS_ERROR,
                    "kernel::v_builtin::v_builtinCreatePublicationInfo", 0,
                    "Operation v_builtinTopicLookup(\"V_PUBLICATIONINFO_ID\") failed."
                    OS_REPORT_NL "_this = 0x%x, topic = 0x%x",
                   _this, writer);
        assert(FALSE);
        return NULL;
    }
    str = c_metaScopedName(c_metaObject(v_topicDataType(writer->topic)));
    if (str == NULL) {
        OS_REPORT_2(OS_ERROR,
                    "kernel::v_builtin::v_builtinCreatePublicationInfo", 0,
                    "Operation c_metaScopedName(writer->topic->type) failed."
                    OS_REPORT_NL "_this = 0x%x, writer = 0x%x",
                    _this,writer);
        assert(FALSE);
        return NULL;
    }
    msg = v_topicMessageNew(builtinTopic);
    if (msg == NULL) {
        OS_REPORT(OS_ERROR,
                    "kernel::v_builtin::v_builtinCreatePublicationInfo", 0,
                    "Failed to create built-in \"V_TOPICINFO_ID\" topic message");
        assert(FALSE);
        return NULL;
    }
    info = v_builtinPublicationInfoData(_this,msg);
    base = c_getBase(c_object(writer));
    info->type_name = c_stringNew(base, str);
    os_free(str);
    if (info->type_name == NULL) {
        OS_REPORT_4(OS_ERROR,
                    "kernel::v_builtin::v_builtinCreatePublicationInfo", 0,
                    "Operation c_stringNew(base=0x%x, str=\"%s\") failed."
                    OS_REPORT_NL "_this = 0x%x, writer = 0x%x",
                    base, str, _this, writer);
        c_free(msg);
        msg = NULL;
        assert(FALSE);
    }
    if (msg != NULL) {
        partitions = c_splitString(publisher->qos->partition, ",");
        length = c_iterLength(partitions);
        type = c_string_t(base);
        if (length > 0) {
            info->partition.name = c_arrayNew(type, length);
            if (info->partition.name != NULL) {
                i = 0;
                str = (c_char *)c_iterTakeFirst(partitions);
                while (str != NULL) {
                    assert(i < length);
                    info->partition.name[i++] = c_stringNew(base, str);
                    os_free(str);
                    str = (c_char *)c_iterTakeFirst(partitions);
                }
            } else {
                OS_REPORT_2(OS_ERROR,
                            "kernel::v_builtin::v_builtinCreatePublicationInfo", 0,
                            "c_arrayNew(type=0x%x, length=%d) failed",
                            type, length);
                c_free(msg);
                msg = NULL;
                assert(FALSE);
            }
        } else {
            length = 1;
            info->partition.name = c_arrayNew(type, length);
            if (info->partition.name != NULL) {
                info->partition.name[0] = c_stringNew(base, "");
            } else {
                OS_REPORT_2(OS_ERROR,
                            "kernel::v_builtin::v_builtinCreatePublicationInfo", 0,
                            "c_arrayNew(type=0x%x, length=%d) failed",
                            type, length);
                c_free(msg);
                msg = NULL;
                assert(FALSE);
            }
        }
        c_iterFree(partitions);
        c_free(type);
    }

    if (msg != NULL) {
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
            if (info->user_data.value != NULL) {
                memcpy(info->user_data.value,
                       writerQos->userData.value,
                       writerQos->userData.size);
            } else {
                OS_REPORT_2(OS_ERROR,
                            "kernel::v_builtin::v_builtinCreatePublicationInfo", 0,
                            "c_arrayNew(type=0x%x, length=%d) failed",
                            type, writerQos->userData.size);
                c_free(msg);
                msg = NULL;
                assert(FALSE);
            }
            c_free(type);
        } else {
            info->user_data.value = NULL;
        }
    }
    if (msg != NULL) {
        info->presentation = publisher->qos->presentation;
        info->group_data.value = c_keep(publisher->qos->groupData.value);

        p = v_participant(publisher->participant);
        /* p == NULL : is a valid use-case (according to legacy code) */
        info->participant_key = v_publicGid(v_public(p));

        topicQos = v_topicGetQos(writer->topic);
        info->topic_data.value  = c_keep(topicQos->topicData.value);
        info->key = v_publicGid(v_public(writer));
        info->topic_name = c_keep(v_topicName(writer->topic));

        c_free(topicQos);

        info->alive = writer->alive;
    }

    topicQos = v_topicGetQos(writer->topic);
    info->topic_data.value  = c_keep(topicQos->topicData.value);

    c_free(topicQos);

    info->alive = writer->alive;
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
    if(_this && !_this->kernelQos->builtin.enabled){
        msg = NULL;
    }
    else if ((_this != NULL) && (dataReader != NULL)) {
        base = c_getBase(c_object(dataReader));
        builtinTopic = v_builtinTopicLookup(_this,V_SUBSCRIPTIONINFO_ID);
        msg = v_topicMessageNew(builtinTopic);
        if (msg == NULL) {
            OS_REPORT(OS_ERROR,
                      "kernel::v_builtin::v_builtinCreateSubscriptionInfo", 0,
                      "Failed to create built-in PublicationInfo topic message");
            return NULL;
        }
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
        } else {
            OS_REPORT(OS_ERROR,
                      "kernel::v_builtin::v_builtinCreateSubscriptionInfo", 0,
                      "Internal error DataReader has no Subscriber reference.");
        }
    } else {
        msg = NULL;
        OS_REPORT_2(OS_ERROR,
                    "kernel::v_builtin::v_builtinCreateSubscriptionInfo", 0,
                    "Operation failed pre condition not met. _this = 0x%x, DataReader = 0x%x",
                    _this,dataReader);
    }
    return msg;
}
