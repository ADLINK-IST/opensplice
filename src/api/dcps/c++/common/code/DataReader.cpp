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
#include "DataReader.h"
#include "DataReaderView.h"
#include "Constants.h"
#include "QosUtils.h"
#include "MiscUtils.h"
#include "StatusUtils.h"
#include "ReportUtils.h"
#include "SequenceUtils.h"
#include "Subscriber.h"
#include "ReadCondition.h"
#include "QueryCondition.h"
#include "LoanRegistry.h"
#include "ContentFilteredTopic.h"
#include "dds_builtinTopicsSplDcps.h"
#include "u_observable.h"

struct DDS::OpenSplice::DataReader::Implementation {
    DDS::OpenSplice::TopicDescription *topic;
    DDS::OpenSplice::Subscriber *subscriber;
    DDS::OpenSplice::ObjSet *views;
    DDS::OpenSplice::ObjSet *conditions;
    DataReaderViewQos defaultViewQos;

    DDS::Boolean wlReq_insertView (
        DDS::DataReaderView_ptr view);

    DDS::Boolean wlReq_removeView (
        DDS::DataReaderView_ptr view);

    DDS::Boolean wlReq_insertCondition (
        DDS::Condition_ptr condition);

    DDS::Boolean wlReq_removeCondition (
        DDS::Condition_ptr condition);

    static u_result copy_sample_rejected_status (
        u_status info,
        void *arg);

    static u_result copy_liveliness_changed_status (
        u_status info,
        void *arg);

    static u_result copy_deadline_missed_status (
        u_status info,
        void *arg);

    static u_result copy_incompatible_qos_status (
        u_status info,
        void *arg);

    static u_result copy_subscription_matched_status (
        u_status info,
        void *arg);

    static u_result copy_sample_lost_status (
        u_status info,
        void *arg);

    static v_result copy_matched_publication (
        u_publicationInfo *info,
        void *arg);

    static v_result copy_matched_publication_data (
        u_publicationInfo *info,
        void *arg);

    static u_result copy_instance_handles (
        u_instanceHandle *list,
        os_uint32 length,
        c_voidp arg);

};

DDS::OpenSplice::TopicDescription *
DDS::OpenSplice::DataReader::get_topic () {
    return this->pimpl->topic;
}

DDS::OpenSplice::DataReader::DataReader () :
    DDS::OpenSplice::Entity(DDS::OpenSplice::DATAREADER),
    pimpl(new Implementation)
{
    this->pimpl->topic = NULL;
    this->pimpl->subscriber = NULL;
    this->pimpl->views = new ObjSet(TRUE);
    this->pimpl->conditions = new ObjSet(TRUE);
    this->pimpl->defaultViewQos = DATAREADERVIEW_QOS_DEFAULT;
}

DDS::OpenSplice::DataReader::~DataReader ()
{
    delete this->pimpl->conditions;
    delete this->pimpl->views;
    delete this->pimpl;
}

DDS::ReturnCode_t
DDS::OpenSplice::DataReader::nlReq_init (
    DDS::OpenSplice::Subscriber *subscriber,
    const DDS::DataReaderQos &qos,
    DDS::OpenSplice::TopicDescription *a_topic,
    const char *name)
{
    DDS::OpenSplice::ContentFilteredTopic *contentFilteredTopic;
    DDS::ULong i = 0;
    DDS::ULong length = 0;
    DDS::ULong bytes = 0;
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    u_readerQos uReaderQos = NULL;
    u_dataReader uReader = NULL;
    const char *expression = NULL;
    c_value *uParameters = NULL;

    assert (subscriber != NULL);
    /* Only check for QoS consistency here in debug builds. It's unnecessary to
       to verify it twice in release builds. */
    assert (DDS::OpenSplice::Utils::qosIsConsistent (qos) == DDS::RETCODE_OK);
    assert (a_topic != NULL);
    assert (name != NULL);

    uReaderQos = u_readerQosNew (NULL);
    if (uReaderQos != NULL) {
        result = DDS::OpenSplice::Utils::copyQosIn (qos, uReaderQos);
    } else {
        result = DDS::RETCODE_OUT_OF_RESOURCES;
        CPP_REPORT(result, "Could not copy DataReaderQos.");
    }

    if (result == DDS::RETCODE_OK) {
        result = this->pimpl->views->init();
    }

    if (result == DDS::RETCODE_OK) {
        result = this->pimpl->conditions->init();
    }

    if (result == DDS::RETCODE_OK) {
        result = a_topic->write_lock();
        if (result == DDS::RETCODE_OK) {
            expression = a_topic->rlReq_get_topic_expression ();
            assert(expression);
            switch (a_topic->rlReq_get_kind()) {
                case DDS::OpenSplice::CONTENTFILTEREDTOPIC:
                    contentFilteredTopic =
                        dynamic_cast<DDS::OpenSplice::ContentFilteredTopic *>(a_topic);
                    if (contentFilteredTopic != NULL) {
                        length = contentFilteredTopic->filterParameters.length ();
                        if(length > 0) {
                            bytes = length * sizeof (struct c_value);
                            uParameters = (c_value *)os_malloc (bytes);
                            for (i = 0; i < length; i++) {
                                const c_string param = (const c_string) contentFilteredTopic->filterParameters[i].in();
                                uParameters[i] = c_stringValue(param);
                            }
                        }
                    } else {
                        result = DDS::RETCODE_BAD_PARAMETER;
                        CPP_REPORT(result, "a_topic invalid, not of type '%s'",
                            "DDS::OpenSplice::ContentFilteredTopic");
                    }
                    break;
                case DDS::OpenSplice::TOPIC:
                default:
                    uParameters = NULL;
                    break;
            }
        }

        if (result == DDS::RETCODE_OK) {
            uReader = u_dataReaderNew (
                u_subscriber (subscriber->rlReq_get_user_entity ()),
                name,
                expression,
                uParameters,
                length,
                uReaderQos);
            if (uReader == NULL) {
                result = DDS::RETCODE_OUT_OF_RESOURCES;
                CPP_REPORT(result, "Could not create DataReader.");
            }
        }

        if (result == DDS::RETCODE_OK) {
            result = DDS::OpenSplice::Entity::nlReq_init (u_entity (uReader));
            if (result == DDS::RETCODE_OK) {
                (void) DDS::Subscriber::_duplicate(subscriber);
                this->pimpl->subscriber = subscriber;
                (void) DDS::TopicDescription::_duplicate(a_topic);
                this->pimpl->topic = a_topic;
                a_topic->wlReq_incrNrUsers();
                setDomainId(subscriber->getDomainId());

            }
        }

        a_topic->unlock();
    }

    if (uReaderQos) {
        u_readerQosFree (uReaderQos);
    }
    if (uParameters) {
        os_free (uParameters);
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DataReader::wlReq_deinit ()
{
    DDS::ReturnCode_t result;

    if (this->pimpl->views->getNrElements() != 0) {
        result = DDS::RETCODE_PRECONDITION_NOT_MET;
        CPP_REPORT(result, "DataReader still contains %d DataReaderView entities.",
            this->pimpl->views->getNrElements());
    } else if (this->pimpl->conditions->getNrElements() != 0) {
        result = DDS::RETCODE_PRECONDITION_NOT_MET;
        CPP_REPORT(result, "DataReader still contains %d Condition entities.",
            this->pimpl->conditions->getNrElements());
    } else {
        this->disable_callbacks();

        if (this->pimpl->topic != NULL) {
            result = this->pimpl->topic->write_lock();
            if (result == DDS::RETCODE_OK) {
                this->pimpl->topic->wlReq_decrNrUsers();
                this->pimpl->topic->unlock();
            }
            DDS::release (this->pimpl->topic);
            this->pimpl->topic = NULL;
        }
        if (this->pimpl->subscriber != NULL) {
            DDS::release (this->pimpl->subscriber);
            this->pimpl->subscriber = NULL;
        }
        result = DDS::OpenSplice::Entity::wlReq_deinit();
    }

    return result;
}

DDS::ReadCondition_ptr
DDS::OpenSplice::DataReader::create_readcondition (
    DDS::SampleStateMask sample_states,
    DDS::ViewStateMask view_states,
    DDS::InstanceStateMask instance_states
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    DDS::OpenSplice::ReadCondition *condition = NULL;

    CPP_REPORT_STACK();

    result = this->write_lock();
    if (result == DDS::RETCODE_OK) {
        condition = new DDS::OpenSplice::ReadCondition();
        if (condition == NULL) {
            result = DDS::RETCODE_OUT_OF_RESOURCES;
            CPP_REPORT(result, "Could not create ReadCondition.");
        } else {
            result = condition->init (
                this, sample_states, view_states, instance_states);
            if (result == DDS::RETCODE_OK){
                this->pimpl->wlReq_insertCondition (condition);
            } else {
                delete condition;
                condition = NULL;
            }
        }
        this->unlock();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return condition;
}

DDS::QueryCondition_ptr
DDS::OpenSplice::DataReader::create_querycondition (
    DDS::SampleStateMask sample_states,
    DDS::ViewStateMask view_states,
    DDS::InstanceStateMask instance_states,
    const char *query_expression,
    const DDS::StringSeq &query_parameters
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    DDS::OpenSplice::QueryCondition *condition = NULL;

    CPP_REPORT_STACK();

    if (query_expression == NULL) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "query_expression '<NULL>' is invalid.");
    } else {
        result = this->write_lock();
    }

    if (result == DDS::RETCODE_OK) {
        condition = new DDS::OpenSplice::QueryCondition();
        if (condition == NULL) {
            result = DDS::RETCODE_OUT_OF_RESOURCES;
            CPP_REPORT(result, "Could not create QueryCondition.");
        } else {
            result = condition->init (
                this, sample_states, view_states, instance_states, query_expression, &query_parameters);
            if (result == DDS::RETCODE_OK) {
                this->pimpl->wlReq_insertCondition (condition);
            } else {
                delete condition;
                condition = NULL;
            }
        }
        this->unlock();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return condition;
}

DDS::ReturnCode_t
DDS::OpenSplice::DataReader::delete_readcondition (
    DDS::ReadCondition_ptr a_condition
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    DDS::OpenSplice::ReadCondition *condition;

    CPP_REPORT_STACK();

    if (a_condition == NULL) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "a_condition '<NULL>' is invalid.");
    } else {
        condition = dynamic_cast<DDS::OpenSplice::ReadCondition *>(a_condition);
        if (condition == NULL) {
            result = DDS::RETCODE_BAD_PARAMETER;
            CPP_REPORT(result, "a_condition is invalid, not of type '%s'.",
                "DDS::OpenSplice::ReadCondition");
        }
    }

    if (result == DDS::RETCODE_OK) {
        result = this->write_lock();
    }

    if (result == DDS::RETCODE_OK) {
        if (this->pimpl->wlReq_removeCondition(condition)) {
            result = condition->deinit();
        } else {
            result = DDS::RETCODE_PRECONDITION_NOT_MET;
            CPP_REPORT(result, "a_condition not created by DataReader.");
        }
        this->unlock();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DataReader::delete_contained_entities (
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result, endResult;

    CPP_REPORT_STACK();

    endResult = this->write_lock();
    if (endResult == DDS::RETCODE_OK) {
        /* When an error is detected during the deletion of an entity,
         * than continue deleting the next instances so that you delete
         * as much as possible. However, save the error you encountered
         * in the endResult, so that the caller knows that not everything
         * deleted successfully.
         */
        result = wlReq_deleteFactoryList<DDS::OpenSplice::DataReaderView *>(this->pimpl->views);
        if (result != DDS::RETCODE_OK) {
            endResult = result;
        }
        result = wlReq_deleteEntityList<DDS::OpenSplice::ReadCondition *>(this->pimpl->conditions);
        if (result != DDS::RETCODE_OK) {
            endResult = result;
        }
        this->unlock();
    }

    CPP_REPORT_FLUSH(this, endResult != DDS::RETCODE_OK);

    return endResult;
}

DDS::ReturnCode_t
DDS::OpenSplice::DataReader::set_qos (
    const DDS::DataReaderQos &qos
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    DDS::DataReaderQos readerQos;
    DDS::TopicQos topicQos;
    u_dataReader uReader;
    u_readerQos uReaderQos;
    u_result uResult;

    CPP_REPORT_STACK();

    result = DDS::OpenSplice::Utils::qosIsConsistent (qos);

    if (result == DDS::RETCODE_OK) {
        uReaderQos = u_readerQosNew (NULL);
        if (uReaderQos == NULL) {
            result = DDS::RETCODE_OUT_OF_RESOURCES;
            CPP_REPORT(result, "Could not copy DataReaderQos.");
        }
    }

    if (result == DDS::RETCODE_OK) {
        result = this->write_lock ();
        if (result == DDS::RETCODE_OK) {
            if (&qos == &DATAREADER_QOS_DEFAULT) {
                /* QoS consistency is checked by set_default_datareader_qos */
                result = this->pimpl->subscriber->get_default_datareader_qos (readerQos);
                if (result == DDS::RETCODE_OK) {
                    result = DDS::OpenSplice::Utils::copyQosIn (
                        readerQos, uReaderQos);
                }
            } else if (&qos == &DATAREADER_QOS_USE_TOPIC_QOS) {
                result = this->pimpl->subscriber->get_default_datareader_qos (readerQos);
                if (result == DDS::RETCODE_OK) {
                    result = this->pimpl->subscriber->copy_from_topicdescription (
                        readerQos, this->pimpl->topic);
                    if (result == DDS::RETCODE_OK) {
                        result = DDS::OpenSplice::Utils::qosIsConsistent (qos);
                        if (result == DDS::RETCODE_OK) {
                            result = DDS::OpenSplice::Utils::copyQosIn (
                                readerQos, uReaderQos);
                        }
                    }
                }
            } else {
                result = DDS::OpenSplice::Utils::copyQosIn (
                    qos, uReaderQos);
            }

            if (result == DDS::RETCODE_OK) {
                uReader = u_dataReader (this->rlReq_get_user_entity ());
                assert (uReader != NULL);
                uResult = u_dataReaderSetQos (uReader, uReaderQos);
                result = uResultToReturnCode (uResult);
                if (result != DDS::RETCODE_OK) {
                    CPP_REPORT(result, "Could not apply DataReaderQos.");
                }
            }

            this->unlock ();
        }

        u_readerQosFree (uReaderQos);
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DataReader::get_qos (
    DDS::DataReaderQos &qos
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    u_dataReader uReader;
    u_readerQos uReaderQos;
    u_result uResult;

    CPP_REPORT_STACK();

    if (&qos == &DATAREADER_QOS_DEFAULT) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "QoS 'DATAREADER_QOS_DEFAULT' is read-only.");
    } else if (&qos == &DATAREADER_QOS_USE_TOPIC_QOS) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "QoS 'DATAREADER_QOS_USE_TOPIC_QOS' is read-only.");
    } else {
        result = this->check ();
    }

    if (result == DDS::RETCODE_OK) {
        uReader = u_dataReader (this->rlReq_get_user_entity ());
        uResult = u_dataReaderGetQos(uReader, &uReaderQos);
        result = uResultToReturnCode (uResult);
        if (result == DDS::RETCODE_OK) {
            result = DDS::OpenSplice::Utils::copyQosOut (
                uReaderQos, qos);
            u_readerQosFree(uReaderQos);
        } else {
            CPP_REPORT(result, "Could not copy DataReaderQos.");
        }
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DataReader::set_listener (
    DDS::DataReaderListener_ptr a_listener,
    DDS::StatusMask mask
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;

    CPP_REPORT_STACK();
    result = DDS::OpenSplice::Entity::nlReq_set_listener(a_listener, mask);
    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::DataReaderListener_ptr
DDS::OpenSplice::DataReader::get_listener (
) THROW_ORB_EXCEPTIONS
{
    DDS::DataReaderListener_ptr listener;

    CPP_REPORT_STACK();
    listener = dynamic_cast<DDS::DataReaderListener_ptr>(
        DDS::OpenSplice::Entity::nlReq_get_listener());
    CPP_REPORT_FLUSH(this, listener == NULL);

    return listener;
}

DDS::ReturnCode_t
DDS::OpenSplice::DataReader::set_property (
    const ::DDS::Property & a_property
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;

    CPP_REPORT_STACK();

    if (a_property.name != NULL) {
        if (strcmp(a_property.name, "parallelReadThreadCount") == 0) {

            if (a_property.value != NULL) {
                DDS::Long value;
                char * pend;

                value = strtol(a_property.value, &pend, 10);
                if ((*pend == '\0') && (value >= 0)) {
                    result = this->write_lock();
                    if (result == DDS::RETCODE_OK) {
                        result = this->wlReq_set_workers(value);
                        this->unlock();
                    }
                } else {
                    result = DDS::RETCODE_BAD_PARAMETER;
                    CPP_REPORT(result, "Property 'parallelReadThreadCount' value is invalid, not of type 'Long'.");
                }
            } else {
                result = DDS::RETCODE_BAD_PARAMETER;
                CPP_REPORT(result, "Property 'parallelReadThreadCount' value '<NULL>' is invalid.");
            }
        } else if (strcmp(a_property.name, "ignoreLoansOnDeletion") == 0) {
            if (a_property.value != NULL) {
                DDS::Boolean value;
                value = (os_strcasecmp("true",(const char*)a_property.value) == 0);
                result = this->write_lock();
                if (result == DDS::RETCODE_OK) {
                     result = this->wlReq_set_ignoreOpenLoansAtDeletionStatus(value);
                     this->unlock();
                }
            } else {
                result = DDS::RETCODE_BAD_PARAMETER;
                CPP_REPORT(result, "Property 'ignoreLoansOnDeletion' value '<NULL>' is invalid.");
            }
        } else {
            result = DDS::RETCODE_UNSUPPORTED;
            const char *name = a_property.name;
            CPP_REPORT(result, "Property '%s' is not supported.",
                name != NULL ? name : "<NULL>");
        }
    } else {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "Property '<NULL>' is invalid.");
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::TopicDescription_ptr
DDS::OpenSplice::DataReader::get_topicdescription (
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    DDS::TopicDescription_ptr topic = NULL;

    CPP_REPORT_STACK();

    result = this->check();
    if (result == DDS::RETCODE_OK) {
        topic = DDS::TopicDescription::_duplicate(this->pimpl->topic);
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return topic;
}

DDS::Subscriber_ptr
DDS::OpenSplice::DataReader::get_subscriber (
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    DDS::Subscriber_ptr subscriber = NULL;

    CPP_REPORT_STACK();

    result = this->check();
    if (result == DDS::RETCODE_OK) {
        subscriber = DDS::Subscriber::_duplicate(this->pimpl->subscriber);
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return subscriber;
}

DDS::ReturnCode_t
DDS::OpenSplice::DataReader::get_sample_rejected_status (
    DDS::SampleRejectedStatus &status
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    u_reader uReader;
    u_result uResult;

    CPP_REPORT_STACK();

    result = this->check();
    if (result == DDS::RETCODE_OK) {
        uReader = u_reader(this->rlReq_get_user_entity());
        assert(uReader != NULL);
        uResult = u_readerGetSampleRejectedStatus(
                uReader,
                TRUE,
                this->pimpl->copy_sample_rejected_status,
                &status);
        result = uResultToReturnCode(uResult);
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DataReader::get_liveliness_changed_status (
    DDS::LivelinessChangedStatus &status
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    u_reader uReader;
    u_result uResult;

    CPP_REPORT_STACK();

    result = this->check();
    if (result == DDS::RETCODE_OK) {
        uReader = u_reader(this->rlReq_get_user_entity());
        assert(uReader != NULL);
        uResult = u_readerGetLivelinessChangedStatus(
                uReader,
                TRUE,
                this->pimpl->copy_liveliness_changed_status,
                &status);
        result = uResultToReturnCode(uResult);
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DataReader::get_requested_deadline_missed_status (
    DDS::RequestedDeadlineMissedStatus &status
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    u_reader uReader;
    u_result uResult;

    CPP_REPORT_STACK();

    result = this->check();
    if (result == DDS::RETCODE_OK) {
        uReader = u_reader(this->rlReq_get_user_entity());
        assert(uReader != NULL);
        uResult = u_readerGetDeadlineMissedStatus(
                uReader,
                TRUE,
                this->pimpl->copy_deadline_missed_status,
                &status);
        result = uResultToReturnCode(uResult);
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DataReader::get_requested_incompatible_qos_status (
    DDS::RequestedIncompatibleQosStatus &status
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    u_reader uReader;
    u_result uResult;

    CPP_REPORT_STACK();

    result = this->check();
    if (result == DDS::RETCODE_OK) {
        uReader = u_reader(this->rlReq_get_user_entity());
        assert(uReader != NULL);
        uResult = u_readerGetIncompatibleQosStatus(
                uReader,
                TRUE,
                this->pimpl->copy_incompatible_qos_status,
                &status);
        result = uResultToReturnCode(uResult);
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DataReader::get_subscription_matched_status (
    DDS::SubscriptionMatchedStatus &status
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    u_reader uReader;
    u_result uResult;

    CPP_REPORT_STACK();

    result = this->check();
    if (result == DDS::RETCODE_OK) {
        uReader = u_reader(this->rlReq_get_user_entity());
        assert(uReader != NULL);
        uResult = u_readerGetSubscriptionMatchStatus(
                uReader,
                TRUE,
                this->pimpl->copy_subscription_matched_status,
                &status);
        result = uResultToReturnCode(uResult);
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DataReader::get_sample_lost_status (
    DDS::SampleLostStatus &status
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    u_reader uReader;
    u_result uResult;

    CPP_REPORT_STACK();

    result = this->check();
    if (result == DDS::RETCODE_OK) {
        uReader = u_reader(this->rlReq_get_user_entity());
        assert(uReader != NULL);
        uResult = u_readerGetSampleLostStatus(
                uReader,
                TRUE,
                this->pimpl->copy_sample_lost_status,
                &status);
        result = uResultToReturnCode(uResult);
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DataReader::wait_for_historical_data (
    const DDS::Duration_t &max_wait
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    u_dataReader uDataReader;
    u_result uResult;
    os_duration timeout;

    CPP_REPORT_STACK();

    result = DDS::OpenSplice::Utils::durationIsValid(max_wait);
    if (result == DDS::RETCODE_OK) {
        result = this->check();
    }

    if (result == DDS::RETCODE_OK) {
        if (!this->rlReq_is_enabled()) {
            result = DDS::RETCODE_NOT_ENABLED;
        } else {
            uDataReader = u_dataReader(this->rlReq_get_user_entity());
            assert(uDataReader != NULL);
            (void) DDS::OpenSplice::Utils::copyDurationIn(max_wait, timeout);
            uResult = u_dataReaderWaitForHistoricalData( uDataReader, timeout);
            result = uResultToReturnCode(uResult);
        }
    }

    CPP_REPORT_FLUSH(this, (result != DDS::RETCODE_OK) && (result != DDS::RETCODE_TIMEOUT));

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DataReader::wait_for_historical_data_w_condition (
    const char * filter_expression,
    const DDS::StringSeq & filter_parameters,
    const DDS::Time_t & min_source_timestamp,
    const DDS::Time_t & max_source_timestamp,
    const DDS::ResourceLimitsQosPolicy & resource_limits,
    const DDS::Duration_t & max_wait
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    u_dataReader uDataReader;
    u_result uResult;
    os_duration timeout;
    os_timeW minTimestamp;
    os_timeW maxTimestamp;
    c_ulong length;
    const os_char **params;

    CPP_REPORT_STACK();

    result = this->check();
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::durationIsValid(max_wait);
    }
    if (result == RETCODE_OK) {
        result = DDS::OpenSplice::Utils::stringSeqenceIsValid(filter_parameters);
    }
    if (result == RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyTimeIn(min_source_timestamp, minTimestamp, maxSupportedSeconds);
    }
    if (result == RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyTimeIn(max_source_timestamp, maxTimestamp, maxSupportedSeconds);
    }

    if (result == DDS::RETCODE_OK) {
        if (filter_parameters.length() != 0) {
            length = filter_parameters.length();
            params = (const os_char **) DDS::OpenSplice::Utils::stringSeqToStringArray(filter_parameters, FALSE);
            if (params == NULL) {
                result = DDS::RETCODE_OUT_OF_RESOURCES;
            }
        } else {
            length = 0;
            params = NULL;
        }

        if (result == DDS::RETCODE_OK) {
            result = DDS::OpenSplice::Utils::copyDurationIn(max_wait, timeout);
            assert(result == DDS::RETCODE_OK);
            uDataReader = u_dataReader(this->rlReq_get_user_entity());
            assert(uDataReader != NULL);
            uResult = u_dataReaderWaitForHistoricalDataWithCondition(
                uDataReader,
                (os_char*) filter_expression,
                params,
                length,
                minTimestamp,
                maxTimestamp,
                resource_limits.max_samples,
                resource_limits.max_instances,
                resource_limits.max_samples_per_instance,
                timeout);
            result = uResultToReturnCode(uResult);

            DDS::OpenSplice::Utils::freeStringArray((DDS::Char**)params, length);
        }
    }

    CPP_REPORT_FLUSH(this, (result != DDS::RETCODE_OK) && (result != DDS::RETCODE_TIMEOUT));

    return result;
}

DDS::DataReaderView_ptr
DDS::OpenSplice::DataReader::create_view (
    const DDS::DataReaderViewQos &qos
) THROW_ORB_EXCEPTIONS
{
    DDS::OpenSplice::DataReaderView *view = NULL;
    DDS::ReturnCode_t result;
    DDS::OpenSplice::TypeSupportMetaHolder *tsMetaHolder = NULL;
    const DDS::DataReaderViewQos *viewQosPtr;

    CPP_REPORT_STACK();

    tsMetaHolder = this->pimpl->topic->get_typesupport_meta_holder();
    if (tsMetaHolder) {
        view = tsMetaHolder->create_view();
        if (view) {
            result = this->write_lock();
            if (result == DDS::RETCODE_OK) {
                if (&qos == &DATAREADERVIEW_QOS_DEFAULT) {
                    viewQosPtr = &this->pimpl->defaultViewQos;
                } else {
                    result = DDS::OpenSplice::Utils::qosIsConsistent(qos);
                    viewQosPtr = &qos;
                }
                if (result == DDS::RETCODE_OK) {
                    result = view->init(this, "dataReaderView", *viewQosPtr, tsMetaHolder->get_copy_in(), tsMetaHolder->get_copy_out());
                    if ((result == DDS::RETCODE_OK) &&
                        (!this->pimpl->wlReq_insertView(view))) {
                        result = DDS::RETCODE_OUT_OF_RESOURCES;
                    }
                }
                this->unlock();
            }
            if (result != DDS::RETCODE_OK) {
                DDS::release (view);
                view = NULL;
            }
        } else {
            result = DDS::RETCODE_OUT_OF_RESOURCES;
        }
        DDS::release (tsMetaHolder);
    } else {
        result = DDS::RETCODE_BAD_PARAMETER;
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return view;
}

DDS::ReturnCode_t
DDS::OpenSplice::DataReader::delete_view (
    DDS::DataReaderView_ptr a_datareaderview
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    DDS::OpenSplice::DataReaderView *view;

    CPP_REPORT_STACK();

    if (a_datareaderview == NULL) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "a_datareaderview '<NULL>' is invalid.");
    } else {
        view = dynamic_cast<DDS::OpenSplice::DataReaderView *>(a_datareaderview);
        if (view == NULL) {
            result = DDS::RETCODE_BAD_PARAMETER;
            CPP_REPORT(result, "a_datareaderview is invalid, not of type '%s'.",
                "DDS::OpenSplice::DataReaderView");
        }
    }

    if (result == DDS::RETCODE_OK) {
        result = this->write_lock ();
    }

    if (result == DDS::RETCODE_OK) {
        if (this->pimpl->wlReq_removeView(view) == FALSE) {
            if (view->get_kind() == DDS::OpenSplice::DATAREADERVIEW) {
                result = DDS::RETCODE_PRECONDITION_NOT_MET;
            } else {
                result = DDS::RETCODE_BAD_PARAMETER;
            }
        }
        if (result == DDS::RETCODE_OK) {
            result = view->deinit ();
        } else {
            (void)this->pimpl->wlReq_insertView(view);
        }
        this->unlock ();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DataReader::set_default_datareaderview_qos (
    const DDS::DataReaderViewQos & qos
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;

    CPP_REPORT_STACK();

    result = this->write_lock();
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::qosIsConsistent(qos);
        if (result == DDS::RETCODE_OK) {
            this->pimpl->defaultViewQos = qos;
        }
        this->unlock ();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DataReader::get_default_datareaderview_qos (
    DDS::DataReaderViewQos & qos
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    CPP_REPORT_STACK();

    if (&qos == &DATAREADERVIEW_QOS_DEFAULT) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "QoS 'DATAREADERVIEW_QOS_DEFAULT' is read-only.");
    } else {
        result = this->read_lock();
    }

    if (result == DDS::RETCODE_OK) {
        qos = this->pimpl->defaultViewQos;
        this->unlock ();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DataReader::get_matched_publications (
    DDS::InstanceHandleSeq &publication_handles
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    u_reader uReader;
    u_result uResult;

    CPP_REPORT_STACK();

    result = this->check();
    if (result == DDS::RETCODE_OK) {
        uReader = u_reader(this->rlReq_get_user_entity());
        assert(uReader != NULL);
        uResult = u_readerGetMatchedPublications(
                uReader,
                this->pimpl->copy_matched_publication,
                &publication_handles);
        result = uResultToReturnCode(uResult);
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DataReader::get_matched_publication_data (
    DDS::PublicationBuiltinTopicData &publication_data,
    DDS::InstanceHandle_t publication_handle
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result = DDS::RETCODE_BAD_PARAMETER;
    u_reader uReader;
    u_result uResult;

    CPP_REPORT_STACK();

    if (publication_handle != DDS::HANDLE_NIL) {
        result = this->check();
        if (result == DDS::RETCODE_OK) {
            uReader = u_reader(this->rlReq_get_user_entity());
            assert(uReader != NULL);
            uResult = u_readerGetMatchedPublicationData(
                    uReader,
                    (u_instanceHandle)publication_handle,
                    this->pimpl->copy_matched_publication_data,
                    &publication_data);
            result = uResultToReturnCode(uResult);
        }
    } else {
        CPP_REPORT(result, "publication_handle 'HANDLE_NIL' is invalid.");
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

void
DDS::OpenSplice::DataReader::nlReq_notify_listener (
    DDS::OpenSplice::Entity *sourceEntity,
    DDS::ULong               triggerMask,
    void                    *eventData)
{
    DDS::DataReaderListener_ptr listener;
    DDS::ReturnCode_t result;

    /* Using _narrow to cast Listener, this increases the refcount to ensure
     * the ListenerObject is not deleted while notifying. */
    listener = DDS::DataReaderListener::_narrow(this->listener);
    if (listener) {

        if (triggerMask & V_EVENT_DATA_AVAILABLE) {
            result = sourceEntity->reset_dataAvailable_status();
            if (result == DDS::RETCODE_OK) {
                listener->on_data_available(dynamic_cast<DDS::DataReader_ptr>(sourceEntity));
            }
        }

        if (triggerMask & V_EVENT_SAMPLE_REJECTED) {
            DDS::SampleRejectedStatus status;
            DDS::OpenSplice::Utils::copyStatusOut(v_readerStatus(eventData)->sampleRejected,
                                                  status);
            listener->on_sample_rejected(dynamic_cast<DDS::DataReader_ptr>(sourceEntity),
                                         status);
        }

        if (triggerMask & V_EVENT_LIVELINESS_CHANGED) {
            DDS::LivelinessChangedStatus status;
            DDS::OpenSplice::Utils::copyStatusOut(v_readerStatus(eventData)->livelinessChanged,
                                                  status);
            listener->on_liveliness_changed(dynamic_cast<DDS::DataReader_ptr>(sourceEntity),
                                            status);
        }

        if (triggerMask & V_EVENT_REQUESTED_DEADLINE_MISSED) {
            DDS::RequestedDeadlineMissedStatus status;
            DDS::OpenSplice::Utils::copyStatusOut(v_readerStatus(eventData)->deadlineMissed,
                                                  status);
            listener->on_requested_deadline_missed(dynamic_cast<DDS::DataReader_ptr>(sourceEntity),
                                                   status);
        }

        if (triggerMask & V_EVENT_REQUESTED_INCOMPATIBLE_QOS) {
            DDS::RequestedIncompatibleQosStatus status;
            DDS::OpenSplice::Utils::copyStatusOut(v_readerStatus(eventData)->incompatibleQos,
                                                  status);
            listener->on_requested_incompatible_qos(dynamic_cast<DDS::DataReader_ptr>(sourceEntity),
                                                    status);
        }

        if (triggerMask & V_EVENT_SAMPLE_LOST) {
            DDS::SampleLostStatus status;
            DDS::OpenSplice::Utils::copyStatusOut(v_readerStatus(eventData)->sampleLost,
                                                  status);
            listener->on_sample_lost(dynamic_cast<DDS::DataReader_ptr>(sourceEntity),
                                     status);
        }

        if (triggerMask & V_EVENT_SUBSCRIPTION_MATCHED) {
            DDS::SubscriptionMatchedStatus status;
            DDS::OpenSplice::Utils::copyStatusOut(v_readerStatus(eventData)->subscriptionMatch,
                                                  status);
            listener->on_subscription_matched(dynamic_cast<DDS::DataReader_ptr>(sourceEntity),
                                              status);
        }
        DDS::release(listener);
    }
}

DDS::ReturnCode_t
DDS::OpenSplice::DataReader::deinit_view (
    DDS::OpenSplice::DataReaderView * view)
{
    DDS::ReturnCode_t result = DDS::RETCODE_BAD_PARAMETER;

    if (view) {
        result = view->deinit();
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DataReader::deinit_condition(
    DDS::OpenSplice::ReadCondition *readCondition)
{
    DDS::ReturnCode_t result = DDS::RETCODE_BAD_PARAMETER;

    if (readCondition) {
        result = readCondition->deinit();
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DataReader::nlReq_get_instance_handles (
    DDS::InstanceHandleSeq &participant_handles
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    u_dataReader uReader;
    u_result uResult;

    participant_handles.length(0);
    result = this->check();
    if (result == DDS::RETCODE_OK) {
        uReader = u_dataReader(this->rlReq_get_user_entity());
        assert (uReader);
        uResult = u_dataReaderGetInstanceHandles(
                    uReader,
                    DDS::OpenSplice::DataReader::Implementation::copy_instance_handles,
                    &participant_handles);
        result = this->uResultToReturnCode(uResult);
    }

    return result;
}


/*
 * Implementation
 */
DDS::Boolean
DDS::OpenSplice::DataReader::Implementation::wlReq_insertView (
    DDS::DataReaderView_ptr view)
{
    return this->views->insertElement(view);
}

DDS::Boolean
DDS::OpenSplice::DataReader::Implementation::wlReq_removeView (
    DDS::DataReaderView_ptr view)
{
    return this->views->removeElement(view);
}

DDS::Boolean
DDS::OpenSplice::DataReader::Implementation::wlReq_insertCondition (
    DDS::Condition_ptr condition)
{
    return this->conditions->insertElement(condition);
}

DDS::Boolean
DDS::OpenSplice::DataReader::Implementation::wlReq_removeCondition (
    DDS::Condition_ptr condition)
{
    return this->conditions->removeElement(condition);
}

u_result
DDS::OpenSplice::DataReader::Implementation::copy_sample_rejected_status (
    u_status info,
    void *arg)
{
    DDS::ReturnCode_t result;
    struct v_sampleRejectedInfo *from = reinterpret_cast<struct v_sampleRejectedInfo *>(info);
    DDS::SampleRejectedStatus *to = reinterpret_cast<DDS::SampleRejectedStatus *>(arg);

    result = DDS::OpenSplice::Utils::copyStatusOut(*from, *to);

    return DDS::OpenSplice::CppSuperClass::ReturnCodeTouResult(result);
}

u_result
DDS::OpenSplice::DataReader::Implementation::copy_liveliness_changed_status (
    u_status info,
    void *arg)
{
    DDS::ReturnCode_t result;
    struct v_livelinessChangedInfo *from = reinterpret_cast<struct v_livelinessChangedInfo *>(info);
    DDS::LivelinessChangedStatus *to = reinterpret_cast<DDS::LivelinessChangedStatus *>(arg);

    result = DDS::OpenSplice::Utils::copyStatusOut(*from, *to);

    return DDS::OpenSplice::CppSuperClass::ReturnCodeTouResult(result);
}

u_result
DDS::OpenSplice::DataReader::Implementation::copy_deadline_missed_status (
    u_status info,
    void *arg)
{
    DDS::ReturnCode_t result;
    struct v_deadlineMissedInfo *from = reinterpret_cast<struct v_deadlineMissedInfo *>(info);
    DDS::RequestedDeadlineMissedStatus *to = reinterpret_cast<DDS::RequestedDeadlineMissedStatus *>(arg);

    result = DDS::OpenSplice::Utils::copyStatusOut(*from, *to);

    return DDS::OpenSplice::CppSuperClass::ReturnCodeTouResult(result);
}

u_result
DDS::OpenSplice::DataReader::Implementation::copy_incompatible_qos_status (
    u_status info,
    void *arg)
{
    DDS::ReturnCode_t result;
    struct v_incompatibleQosInfo  *from = reinterpret_cast<struct v_incompatibleQosInfo *>(info);
    DDS::RequestedIncompatibleQosStatus *to = reinterpret_cast<DDS::RequestedIncompatibleQosStatus *>(arg);

    result = DDS::OpenSplice::Utils::copyStatusOut(*from, *to);

    return DDS::OpenSplice::CppSuperClass::ReturnCodeTouResult(result);
}

u_result
DDS::OpenSplice::DataReader::Implementation::copy_subscription_matched_status (
    u_status info,
    void *arg)
{
    DDS::ReturnCode_t result;
    struct v_topicMatchInfo *from = reinterpret_cast<struct v_topicMatchInfo *>(info);
    DDS::SubscriptionMatchedStatus *to = reinterpret_cast<DDS::SubscriptionMatchedStatus *>(arg);

    result = DDS::OpenSplice::Utils::copyStatusOut(*from, *to);

    return DDS::OpenSplice::CppSuperClass::ReturnCodeTouResult(result);
}

u_result
DDS::OpenSplice::DataReader::Implementation::copy_sample_lost_status (
    u_status info,
    void *arg)
{
    DDS::ReturnCode_t result;
    struct v_sampleLostInfo *from = reinterpret_cast<struct v_sampleLostInfo *>(info);
    DDS::SampleLostStatus *to = reinterpret_cast<DDS::SampleLostStatus *>(arg);

    result = DDS::OpenSplice::Utils::copyStatusOut(*from, *to);

    return DDS::OpenSplice::CppSuperClass::ReturnCodeTouResult(result);
}

v_result
DDS::OpenSplice::DataReader::Implementation::copy_matched_publication (
    u_publicationInfo *info,
    void *arg)
{
    DDS::InstanceHandleSeq *seq  = reinterpret_cast<DDS::InstanceHandleSeq *>(arg);
    DDS::InstanceHandle_t handle = u_instanceHandleFromGID(info->key);

    DDS::OpenSplice::Utils::appendSequenceItem<
                                        DDS::InstanceHandleSeq,
                                        DDS::InstanceHandle_t>(
                                                        *seq,
                                                        handle);

    return V_RESULT_OK;
}

v_result
DDS::OpenSplice::DataReader::Implementation::copy_matched_publication_data (
    u_publicationInfo *info,
    void *arg)
{
    DDS::PublicationBuiltinTopicData *to = reinterpret_cast<DDS::PublicationBuiltinTopicData *>(arg);
    __DDS_PublicationBuiltinTopicData__copyOut(info, to);
    return V_RESULT_OK;
}

u_result
DDS::OpenSplice::DataReader::Implementation::copy_instance_handles (
    u_instanceHandle *list,
    os_uint32 length,
    c_voidp arg)
{
    DDS::InstanceHandleSeq *seq = (DDS::InstanceHandleSeq *)arg;
    DDS::InstanceHandle_t  *arr;
    os_uint32 i;

    /* Make space. */
    seq->length(length);

    /* Copy info. */
    arr = seq->get_buffer(FALSE);
    for (i = 0; i < length; i++) {
        arr[i] = (DDS::InstanceHandle_t)(list[i]);
    }

    return U_RESULT_OK;
}
