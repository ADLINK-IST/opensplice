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
#include "DataReader.h"
#include "DataReaderView.h"
#include "Constants.h"
#include "ReadCondition.h"
#include "QueryCondition.h"
#include "QosUtils.h"
#include "ReportUtils.h"
#include "u_dataView.h"
#include "u_dataViewQos.h"

struct DDS::OpenSplice::DataReaderView::Implementation {
    DDS::OpenSplice::DataReader *reader;
    DDS::OpenSplice::ObjSet *conditions;

    DDS::Boolean
    wlReq_insertCondition (
        DDS::Condition_ptr);

    DDS::Boolean
    wlReq_removeCondition (
        DDS::Condition_ptr);

    static DDS::Boolean wlReq_deinitCondition (
        DDS::Object_ptr element,
        DDS::ReturnCode_t *result);
};

DDS::OpenSplice::DataReaderView::DataReaderView () :
    DDS::OpenSplice::Entity(DDS::OpenSplice::DATAREADERVIEW),
    pimpl(new Implementation)
{
    this->pimpl->reader = NULL;
    this->pimpl->conditions = new ObjSet(TRUE);
}

DDS::OpenSplice::DataReaderView::~DataReaderView ()
{
    delete this->pimpl->conditions;
    delete pimpl;
}

DDS::ReturnCode_t
DDS::OpenSplice::DataReaderView::nlReq_init (
    DDS::OpenSplice::DataReader *reader,
    const char *name,
    const DDS::DataReaderViewQos &qos)
{
    DDS::ReturnCode_t result;
    u_dataViewQos uQos = NULL;
    u_dataView uView = NULL;

    assert (reader != NULL);
    assert (name != NULL);
    /* qos consistency already checked in create_view */
    assert (DDS::OpenSplice::Utils::qosIsConsistent(qos) == DDS::RETCODE_OK);

    uQos = u_dataViewQosNew(NULL);
    if (uQos) {
        result = DDS::OpenSplice::Utils::copyQosIn(qos, uQos);
        if (result == DDS::RETCODE_OK) {

            uView = u_dataViewNew(
                    u_dataReader(reader->rlReq_get_user_entity()),
                    name,
                    uQos);

            if (uView) {
                result = DDS::OpenSplice::Entity::nlReq_init (u_entity (uView));
                if (result == DDS::RETCODE_OK) {
                    (void) DDS::DataReader::_duplicate(reader);
                    this->pimpl->reader = reader;
                    setDomainId(reader->getDomainId());
                }
                if (result == DDS::RETCODE_OK) {
                    result = this->pimpl->conditions->init();
                }
            } else {
                result = DDS::RETCODE_OUT_OF_RESOURCES;
                CPP_REPORT(result, "Could not create DataReaderView.");
            }
        }
        u_dataViewQosFree(uQos);
    } else {
        result = DDS::RETCODE_OUT_OF_RESOURCES;
        CPP_REPORT(result, "Could not copy DataReaderViewQos.");
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DataReaderView::wlReq_deinit ()
{
    DDS::ReturnCode_t result;

    if (this->pimpl->conditions->getNrElements() != 0) {
        result = DDS::RETCODE_PRECONDITION_NOT_MET;
        CPP_REPORT(result, "DataReaderView still contains '%d' Condition entities.",
            this->pimpl->conditions->getNrElements());
    } else {
        if (this->pimpl->reader != NULL) {
            DDS::release (this->pimpl->reader);
            this->pimpl->reader = NULL;
        }
        result = DDS::OpenSplice::Entity::wlReq_deinit();
    }

    return result;
}

DDS::Boolean
DDS::OpenSplice::DataReaderView::Implementation::wlReq_insertCondition (
    DDS::Condition_ptr condition)
{
    return this->conditions->insertElement(condition);
}

DDS::Boolean
DDS::OpenSplice::DataReaderView::Implementation::wlReq_removeCondition (
    DDS::Condition_ptr condition)
{
    return this->conditions->removeElement(condition);
}

DDS::ReturnCode_t
DDS::OpenSplice::DataReaderView::deinit_condition(
    DDS::OpenSplice::ReadCondition *readCondition)
{
    DDS::ReturnCode_t result = DDS::RETCODE_BAD_PARAMETER;

    if (readCondition) {
        result = readCondition->deinit();
    }

    return result;
}

DDS::ReadCondition_ptr
DDS::OpenSplice::DataReaderView::create_readcondition (
    DDS::SampleStateMask sample_states,
    DDS::ViewStateMask view_states,
    DDS::InstanceStateMask instance_states
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    DDS::OpenSplice::ReadCondition *readCondition = NULL;

    CPP_REPORT_STACK();

    result = this->write_lock();
    if (result == DDS::RETCODE_OK) {
        readCondition = new DDS::OpenSplice::ReadCondition();
        if (readCondition == NULL) {
            result = DDS::RETCODE_OUT_OF_RESOURCES;
            CPP_REPORT(result, "Could not create ReadCondition.");
        } else {
            result = readCondition->init (
                this, sample_states, view_states, instance_states);
            if (result == DDS::RETCODE_OK) {
                this->pimpl->wlReq_insertCondition (readCondition);
            } else {
                delete readCondition;
                readCondition = NULL;
            }
        }
        this->unlock();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return readCondition;
}

DDS::QueryCondition_ptr
DDS::OpenSplice::DataReaderView::create_querycondition (
    DDS::SampleStateMask sample_states,
    DDS::ViewStateMask view_states,
    DDS::InstanceStateMask instance_states,
    const char *query_expression,
    const DDS::StringSeq & query_parameters
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    DDS::OpenSplice::QueryCondition *queryCondition = NULL;

    CPP_REPORT_STACK();

    if (query_expression == NULL) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "query_expression '<NULL>' is invalid.");
    }

    if (result == DDS::RETCODE_OK) {
        result = this->write_lock ();
    }

    if (result == DDS::RETCODE_OK) {
        queryCondition = new DDS::OpenSplice::QueryCondition();
        if (queryCondition == NULL) {
            result = DDS::RETCODE_OUT_OF_RESOURCES;
            CPP_REPORT(result, "Could not create QueryCondition.");
        } else {
            result = queryCondition->init (this, sample_states, view_states, instance_states, query_expression, &query_parameters);
            if (result == DDS::RETCODE_OK) {
                this->pimpl->wlReq_insertCondition (queryCondition);
            } else {
                delete queryCondition;
                queryCondition = NULL;
            }
        }
        this->unlock();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return queryCondition;
}

DDS::ReturnCode_t
DDS::OpenSplice::DataReaderView::delete_readcondition (
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
            CPP_REPORT(result, "ReadCondition not created by DataReaderView.");
        }
        this->unlock();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DataReaderView::delete_contained_entities (
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;

    CPP_REPORT_STACK();

    result = this->write_lock();
    if (result == DDS::RETCODE_OK) {
        result = wlReq_deleteEntityList<DDS::OpenSplice::ReadCondition *>(this->pimpl->conditions);
        this->unlock();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DataReaderView::set_qos (
    const DDS::DataReaderViewQos & qos
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    u_dataViewQos uQos = NULL;
    u_dataView uView;
    u_result uResult;

    CPP_REPORT_STACK();

    result = DDS::OpenSplice::Utils::qosIsConsistent(qos);
    if (result == DDS::RETCODE_OK) {
        uQos = u_dataViewQosNew(NULL);
        if (uQos) {
            result = DDS::OpenSplice::Utils::copyQosIn(qos, uQos);
            if (result == DDS::RETCODE_OK) {
                result = this->write_lock();
                if (result == DDS::RETCODE_OK) {
                    uView = u_dataView(this->rlReq_get_user_entity());
                    assert(uView);
                    uResult = u_dataViewSetQos(uView, uQos);
                    this->unlock();
                    result = this->uResultToReturnCode(uResult);
                    if (result != DDS::RETCODE_OK) {
                        CPP_REPORT(result, "Could not apply DataReaderViewQos.");
                    }
                }
            }
            u_dataViewQosFree(uQos);
        } else {
            result = DDS::RETCODE_OUT_OF_RESOURCES;
            CPP_REPORT(result, "Could not copy DataReaderViewQos.");
        }
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DataReaderView::get_qos (
    DDS::DataReaderViewQos & qos
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    u_dataViewQos uQos = NULL;
    u_dataView uView;
    u_result uResult;

    CPP_REPORT_STACK();

    if (&qos == &DATAREADERVIEW_QOS_DEFAULT) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "QoS 'DATAREADERVIEW_QOS_DEFAULT' is read-only.");
    } else {
        result = this->check();
    }

    if (result == DDS::RETCODE_OK) {
        uView = u_dataView(this->rlReq_get_user_entity());
        assert(uView);
        uResult = u_dataViewGetQos(uView, &uQos);
        if (uResult == U_RESULT_OK) {
            result = DDS::OpenSplice::Utils::copyQosOut(uQos, qos);
            u_dataViewQosFree(uQos);
        } else {
            result = uResultToReturnCode(uResult);
            CPP_REPORT(result, "Could not copy DataReaderViewQos.");
        }
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

void
DDS::OpenSplice::DataReaderView::nlReq_notify_listener (
    DDS::OpenSplice::Entity *sourceEntity,
    DDS::ULong               triggerMask,
    void                    *eventData)
{
    OS_UNUSED_ARG(sourceEntity);
    OS_UNUSED_ARG(triggerMask);
    OS_UNUSED_ARG(eventData);
}

DDS::StatusCondition_ptr
DDS::OpenSplice::DataReaderView::get_statuscondition (
) THROW_ORB_EXCEPTIONS
{
    return NULL;
}

DDS::StatusMask
DDS::OpenSplice::DataReaderView::get_status_changes (
) THROW_ORB_EXCEPTIONS
{
    assert(this->pimpl->reader != NULL);

    return this->pimpl->reader->get_status_changes();
}

DDS::DataReader_ptr
DDS::OpenSplice::DataReaderView::get_datareader (
) THROW_ORB_EXCEPTIONS
{
    DDS::DataReader_ptr reader = NULL;
    DDS::ReturnCode_t result;

    CPP_REPORT_STACK();

    result = this->check();
    if (result == DDS::RETCODE_OK) {
        reader = DDS::DataReader::_duplicate(this->pimpl->reader);
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return reader;
}

DDS::DataReader_ptr
DDS::OpenSplice::DataReaderView::rlReq_get_datareader ()
{
    return this->pimpl->reader;
}
