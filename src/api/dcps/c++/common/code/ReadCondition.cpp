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
#include "ReadCondition.h"
#include "StatusUtils.h"
#include "ReportUtils.h"
#include "WaitSet.h"
#include "cpp_dcps_if.h"
#include "Constants.h"

DDS::OpenSplice::ReadCondition::ReadCondition(DDS::OpenSplice::ObjectKind kind) :
    DDS::OpenSplice::Condition(kind),
    sample_states(0),
    view_states(0),
    instance_states(0),
    uQuery(0)
{
}

DDS::OpenSplice::ReadCondition::ReadCondition() :
    DDS::OpenSplice::Condition(DDS::OpenSplice::READCONDITION),
    sample_states(0),
    view_states(0),
    instance_states(0),
    uQuery(0)
{
}

DDS::OpenSplice::ReadCondition::~ReadCondition()
{
    if (this->uQuery) {
        u_objectFree(u_object(this->uQuery));
    }
}

DDS::ReturnCode_t
DDS::OpenSplice::ReadCondition::init (
    DDS::OpenSplice::Entity *reader,
    DDS::SampleStateMask sample_states,
    DDS::ViewStateMask view_states,
    DDS::InstanceStateMask instance_states)
{
    return nlReq_init(reader, sample_states, view_states, instance_states);
}

DDS::ReturnCode_t
DDS::OpenSplice::ReadCondition::nlReq_init (
    DDS::OpenSplice::Entity *reader,
    DDS::SampleStateMask sample_states,
    DDS::ViewStateMask view_states,
    DDS::InstanceStateMask instance_states)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    u_reader uReader = NULL;
    u_sampleMask mask = statesMask(sample_states, view_states, instance_states);
    const os_char *kind;

    this->reader = reader;
    this->sample_states = sample_states;
    this->view_states = view_states;
    this->instance_states = instance_states;

    result = DDS::OpenSplice::Condition::nlReq_init();
    if (result == DDS::RETCODE_OK) {
        setDomainId(reader->getDomainId());
        if (reader->rlReq_get_kind() == DATAREADERVIEW) {
            uReader = u_reader(dynamic_cast<DataReaderView*>(reader)->rlReq_get_user_entity ());
            kind = "DataReaderView";
        } else {
            assert (reader->rlReq_get_kind() == DATAREADER);
            uReader = u_reader(dynamic_cast<DataReader*>(reader)->rlReq_get_user_entity ());
            kind = "DataReader";
        }

        if (uReader) {
            if (this->uQuery == NULL) { // Query condition can initialize uQuery before ReadCondition::nlReq_init is called
                                        // from QueryCondition::nlReq_init. In that case, do not create a query here
                this->uQuery = u_queryNew(uReader, NULL, "1=1", NULL, 0, mask);
                if (this->uQuery == NULL) {
                    result = DDS::RETCODE_ERROR;
                    CPP_REPORT(result, "Could not create ReadCondition.");
                }
            }
        } else {
            result = DDS::RETCODE_BAD_PARAMETER;
            CPP_REPORT(result, "%s is not initialized.", kind);
        }
    }
    return result;

}

DDS::ReturnCode_t
DDS::OpenSplice::ReadCondition::wlReq_deinit()
{
    DDS::ReturnCode_t result;
    result = DDS::OpenSplice::Condition::wlReq_deinit();
    if (result == DDS::RETCODE_OK)
    {
        if (this->uQuery) {
            result = uResultToReturnCode(u_objectClose (u_object (this->uQuery)));
        }
        this->reader = NULL;
        this->sample_states = 0;
        this->view_states = 0;
        this->instance_states = 0;
    }
    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::ReadCondition::attachToWaitset (
    DDS::WaitSet *waitset
)
{
    DDS::ReturnCode_t result;

    result = this->write_lock ();
    if (result == DDS::RETCODE_OK) {
        if (!this->deinitializing) {
            if (!waitsets->containsElement(waitset)) {
                /* Don't _duplicate the waitset to avoid cyclic references messing up the garbage collection;
                 * it will detach itself when it is destructed. */
                result = waitset->wlReq_attachGeneralCondition(this, u_observable(uQuery));
                if (result == DDS::RETCODE_OK) {
                    DDS::Boolean insertOK;
                    insertOK = waitsets->insertElement(waitset);
                    if (!insertOK) {
                        result = DDS::RETCODE_OUT_OF_RESOURCES;
                        CPP_REPORT(result, "Could not attach to Waitset.");
                    }
                    assert(insertOK);
                }
            } else {
                result = DDS::RETCODE_OK;
            }
        } else {
            /* Do not allow a Condition that is already deinitializing to be attached to a WaitSet. */
            result = DDS::RETCODE_ALREADY_DELETED;
            CPP_REPORT(result, "This ReadCondition is being deleted.");
        }
        this->unlock ();
    }
    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::ReadCondition::wlReq_detachFromWaitset (
    DDS::WaitSet *waitset
)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    if (waitsets->removeElement(waitset)) {
        result = waitset->wlReq_detachGeneralCondition(this, u_observable(uQuery));
    } else {
        /* Unable to take the given waitset is not a problem when de-initializing. */
        if (!this->deinitializing) {
            result = DDS::RETCODE_PRECONDITION_NOT_MET;
            CPP_REPORT(result, "This ReadCondition is being deleted.");
        }
    }
    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::ReadCondition::detachFromWaitset (
    DDS::WaitSet *waitset
)
{
    DDS::ReturnCode_t result;

    result = this->write_lock ();
    if (result == DDS::RETCODE_OK) {
        result = wlReq_detachFromWaitset(waitset);
        this->unlock ();
    }
    return result;
}

DDS::SampleStateMask
DDS::OpenSplice::ReadCondition::get_sample_state_mask (
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    DDS::SampleStateMask mask = 0U;

    CPP_REPORT_STACK();

    result = this->check();
    if (result == DDS::RETCODE_OK) {
        mask = this->sample_states;
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return mask;
}

DDS::ViewStateMask
DDS::OpenSplice::ReadCondition::get_view_state_mask (
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    DDS::ViewStateMask mask = 0U;

    CPP_REPORT_STACK();

    result = this->check();
    if (result == DDS::RETCODE_OK) {
        mask = this->view_states;
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return mask;
}

DDS::InstanceStateMask
DDS::OpenSplice::ReadCondition::get_instance_state_mask (
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    DDS::InstanceStateMask mask = 0U;

    CPP_REPORT_STACK();

    result = this->check();
    if (result == DDS::RETCODE_OK) {
        mask = this->instance_states;
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return mask;
}

DDS::DataReader_ptr
DDS::OpenSplice::ReadCondition::get_datareader()
{
    DDS::DataReader_ptr reader = NULL;
    DDS::ReturnCode_t result;

    CPP_REPORT_STACK();

    result = this->check();
    if (result == DDS::RETCODE_OK) {
        assert (this->reader != NULL);
        if (this->reader->rlReq_get_kind() == DATAREADER) {
            reader = dynamic_cast<DataReader*>(this->reader);
            assert (reader != NULL);
        }
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return DDS::DataReader::_duplicate(reader);
}

DDS::DataReaderView_ptr
DDS::OpenSplice::ReadCondition::get_datareaderview()
{
    DDS::DataReaderView_ptr reader = NULL;
    DDS::ReturnCode_t result;

    result = this->read_lock();
    if (result == DDS::RETCODE_OK) {
        assert (this->reader != NULL);
        if (this->reader->rlReq_get_kind() == DATAREADERVIEW) {
            reader = dynamic_cast<DataReaderView*>(this->reader);
        }
        this->unlock();
    } else {
        reader = NULL;
    }
    return DDS::DataReaderView::_duplicate(reader);
}

unsigned char
DDS::OpenSplice::ReadCondition::test_sample_states (
    c_object o,
    c_voidp args)
{
    OS_UNUSED_ARG(o);
    OS_UNUSED_ARG(args);
    return true; /* state evaluation is now in the kernel. */
}


DDS::Boolean
DDS::OpenSplice::ReadCondition::get_trigger_value()
{
    assert(uQuery);
    return u_queryTest(uQuery, DDS::OpenSplice::ReadCondition::test_sample_states, this);
}

u_query
DDS::OpenSplice::ReadCondition::get_user_query()
{
    DDS::ReturnCode_t result;
    u_query uQuery;

    result = this->check();
    if (result == DDS::RETCODE_OK) {
        assert (this->uQuery != NULL);
        uQuery = this->uQuery;
    } else {
        uQuery = NULL;
    }
    return uQuery;
}

DDS::ReturnCode_t
DDS::OpenSplice::ReadCondition::read (
    DDS::OpenSplice::Entity *source,
    void *data_seq,
    DDS::SampleInfoSeq &info_seq,
    const long max_samples,
    void*)
{
    DDS::ReturnCode_t result = DDS::RETCODE_BAD_PARAMETER;

    CPP_REPORT_STACK();

    if (source->rlReq_get_kind() == DATAREADER) {
        result = (dynamic_cast<FooDataReader_impl*>(source))->read(data_seq, info_seq, max_samples, this->get_sample_state_mask(), this->get_view_state_mask(), this->get_instance_state_mask());
    } else if (source->rlReq_get_kind() == DATAREADERVIEW) {
        result = (dynamic_cast<FooDataReaderView_impl*>(source))->read(data_seq, info_seq, max_samples, this->get_sample_state_mask(), this->get_view_state_mask(), this->get_instance_state_mask());
    } else {
        CPP_REPORT(result, "Invalid source Entity kind");
    }

    CPP_REPORT_FLUSH(this, (result != DDS::RETCODE_OK) && (result != DDS::RETCODE_NO_DATA));

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::ReadCondition::take (
    DDS::OpenSplice::Entity *source,
    void *data_seq,
    DDS::SampleInfoSeq &info_seq,
    const long max_samples,
    void*)
{
    DDS::ReturnCode_t result = DDS::RETCODE_BAD_PARAMETER;

    CPP_REPORT_STACK();

    if (source->rlReq_get_kind() == DATAREADER) {
        result = (dynamic_cast<FooDataReader_impl*>(source))->take(data_seq, info_seq, max_samples, this->get_sample_state_mask(), this->get_view_state_mask(), this->get_instance_state_mask());
    } else if (source->rlReq_get_kind() == DATAREADERVIEW) {
        result = (dynamic_cast<FooDataReaderView_impl*>(source))->take(data_seq, info_seq, max_samples, this->get_sample_state_mask(), this->get_view_state_mask(), this->get_instance_state_mask());
    } else {
        CPP_REPORT(result, "Invalid source Entity kind");
    }

    CPP_REPORT_FLUSH(this, (result != DDS::RETCODE_OK) && (result != DDS::RETCODE_NO_DATA));

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::ReadCondition::read_next_instance (
    DDS::OpenSplice::Entity *source,
    void *data_seq,
    DDS::SampleInfoSeq &info_seq,
    const long max_samples,
    DDS::InstanceHandle_t a_handle,
    void*)
{
    DDS::ReturnCode_t result = DDS::RETCODE_BAD_PARAMETER;

    CPP_REPORT_STACK();

    if (source->rlReq_get_kind() == DATAREADER) {
        result = (dynamic_cast<FooDataReader_impl*>(source))->read_next_instance_internal(data_seq, info_seq, max_samples, a_handle, this->get_sample_state_mask(), this->get_view_state_mask(), this->get_instance_state_mask());
    } else if (source->rlReq_get_kind() == DATAREADERVIEW) {
        result = (dynamic_cast<FooDataReaderView_impl*>(source))->read_next_instance_internal(data_seq, info_seq, max_samples, a_handle, this->get_sample_state_mask(), this->get_view_state_mask(), this->get_instance_state_mask());
    } else {
        CPP_REPORT(result, "Invalid source Entity kind");
    }

    CPP_REPORT_FLUSH(this, (result != DDS::RETCODE_OK) && (result != DDS::RETCODE_NO_DATA) && (result != DDS::RETCODE_HANDLE_EXPIRED));

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::ReadCondition::take_next_instance (
    DDS::OpenSplice::Entity *source,
    void *data_seq,
    DDS::SampleInfoSeq &info_seq,
    const long max_samples,
    DDS::InstanceHandle_t a_handle,
    void*)
{
    DDS::ReturnCode_t result = DDS::RETCODE_BAD_PARAMETER;

    CPP_REPORT_STACK();

    if (source->rlReq_get_kind() == DATAREADER) {
        result = (dynamic_cast<FooDataReader_impl*>(source))->take_next_instance_internal(data_seq, info_seq, max_samples, a_handle, this->get_sample_state_mask(), this->get_view_state_mask(), this->get_instance_state_mask());
    } else if (source->rlReq_get_kind() == DATAREADERVIEW) {
        result = (dynamic_cast<FooDataReaderView_impl*>(source))->take_next_instance_internal(data_seq, info_seq, max_samples, a_handle, this->get_sample_state_mask(), this->get_view_state_mask(), this->get_instance_state_mask());
    } else {
        CPP_REPORT(result, "Invalid source Entity kind");
    }

    CPP_REPORT_FLUSH(this, (result != DDS::RETCODE_OK) && (result != DDS::RETCODE_NO_DATA) && (result != DDS::RETCODE_HANDLE_EXPIRED));

    return result;
}


DDS::ReturnCode_t
DDS::OpenSplice::ReadCondition::isAlive()
{
    return DDS::OpenSplice::Utils::observableExists(u_observable(this->uQuery));
}
