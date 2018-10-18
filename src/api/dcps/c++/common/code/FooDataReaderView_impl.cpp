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
#include "FooDataReaderView_impl.h"
#include "MiscUtils.h"
#include "StatusUtils.h"
#include "ReportUtils.h"
#include "LoanRegistry.h"
#include "cmn_samplesList.h"
#include "cmn_reader.h"
#include "u_dataView.h"
#include "v_dataReaderInstance.h"
#include "ReadCondition.h"
#include "QueryCondition.h"
#include "Constants.h"

typedef struct viewCopyInfo_s {
    DDS::OpenSplice::FooDataReaderView_impl *view;
    void *data;
} viewCopyInfo;

struct DDS::OpenSplice::FooDataReaderView_impl::Implementation {
    DDS::OpenSplice::LoanRegistry *loanRegistry;
    cmn_samplesList samplesList;
    u_entity uEntity;

    DDS::OpenSplice::cxxCopyIn  copyIn;
    DDS::OpenSplice::cxxCopyOut copyOut;

    cxxDataSeqAlloc  dataSeqAlloc;
    cxxDataSeqLength dataSeqLength;

    ::DDS::ReturnCode_t prepareSequences (
        void * received_data,
        ::DDS::SampleInfoSeq & info_seq);

    static v_copyin_result rlReq_copyIn (
        c_type type,
        void *data,
        void *to);
};

DDS::OpenSplice::FooDataReaderView_impl::FooDataReaderView_impl () :
    pimpl (new Implementation)
{
    this->pimpl->loanRegistry = NULL;
    this->pimpl->samplesList = NULL;
    this->pimpl->uEntity = NULL;

    this->pimpl->copyIn = NULL;
    this->pimpl->copyOut = NULL;
    this->pimpl->dataSeqAlloc = NULL;
    this->pimpl->dataSeqLength = NULL;
}

DDS::OpenSplice::FooDataReaderView_impl::~FooDataReaderView_impl ()
{
    delete pimpl;
}

DDS::ReturnCode_t
DDS::OpenSplice::FooDataReaderView_impl::nlReq_init (
    DDS::OpenSplice::DataReader *reader,
    const char *name,
    const DDS::DataReaderViewQos &qos,
    DDS::OpenSplice::cxxCopyIn copyIn,
    DDS::OpenSplice::cxxCopyOut copyOut,
    cxxDataSeqAlloc dataSeqAlloc,
    cxxDataSeqLength dataSeqLength)
{
    DDS::ReturnCode_t result;

    assert(reader != NULL);
    assert(name != NULL);
    assert(copyIn != NULL);
    assert(copyOut != NULL);
    assert(dataSeqAlloc != NULL);
    assert(dataSeqLength != NULL);

    result = DDS::OpenSplice::DataReaderView::nlReq_init(reader, name, qos);
    if (result == DDS::RETCODE_OK) {
        this->pimpl->loanRegistry = new LoanRegistry;
        this->pimpl->samplesList = cmn_samplesList_new((os_boolean)TRUE);
        this->pimpl->uEntity = rlReq_get_user_entity();

        this->pimpl->copyIn = copyIn;
        this->pimpl->copyOut = copyOut;
        this->pimpl->dataSeqAlloc = dataSeqAlloc;
        this->pimpl->dataSeqLength = dataSeqLength;
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::FooDataReaderView_impl::wlReq_deinit ()
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    if (this->pimpl->loanRegistry) {
        if (this->pimpl->loanRegistry->is_empty() == FALSE) {
            result = DDS::RETCODE_PRECONDITION_NOT_MET;
            CPP_REPORT(result, "DataReaderView still contains non returned loans.");
        }
    }

    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::DataReaderView::wlReq_deinit();
    }

    if (result == DDS::RETCODE_OK) {
        if (this->pimpl->loanRegistry) {
            delete this->pimpl->loanRegistry;
            this->pimpl->loanRegistry = NULL;
        }
        if (this->pimpl->samplesList) {
            cmn_samplesList_free(this->pimpl->samplesList);
            this->pimpl->samplesList = NULL;
        }
        this->pimpl->copyIn = NULL;
        this->pimpl->copyOut = NULL;
        this->pimpl->dataSeqAlloc = NULL;
        this->pimpl->dataSeqLength = NULL;
    }

    return result;
}

::DDS::ReturnCode_t
DDS::OpenSplice::FooDataReaderView_impl::read (
    void * received_data,
    ::DDS::SampleInfoSeq & info_seq,
    DDS::Long max_samples,
    ::DDS::SampleStateMask sample_states,
    ::DDS::ViewStateMask view_states,
    ::DDS::InstanceStateMask instance_states
) THROW_ORB_EXCEPTIONS
{
    ::DDS::ReturnCode_t result = DDS::RETCODE_BAD_PARAMETER;
    u_sampleMask mask;
    u_dataView uView;
    u_result uResult;

    CPP_REPORT_STACK();

    if (statesMaskIsValid(sample_states, view_states, instance_states) == true) {
        mask = statesMask(sample_states, view_states, instance_states);

        result = this->write_lock();
        if (result == DDS::RETCODE_OK) {
            assert(this->pimpl->samplesList != NULL);
            cmn_samplesList_reset(this->pimpl->samplesList, realMaxSamples(max_samples, info_seq));
            uView = u_dataView(this->rlReq_get_user_entity());
            assert(uView != NULL);

            uResult = u_dataViewRead(
                    uView, mask, cmn_reader_action,
                    this->pimpl->samplesList, OS_DURATION_ZERO);
            /* TODO: when samplesList thread specific unlock, see OSPL-4341 */
            if (uResult == U_RESULT_OK) {
                result = this->flush(this->pimpl->samplesList, received_data, info_seq);
            } else {
                result = this->uResultToReturnCode(uResult);
            }

            this->unlock();
        }
    } else {
        CPP_REPORT(result,
                   "sample_states = 0x%x, view_states = 0x%x, instance_states = 0x%x",
                   sample_states, view_states, instance_states);
    }

    CPP_REPORT_FLUSH(this, (result != DDS::RETCODE_OK) && (result != DDS::RETCODE_NO_DATA));

    return result;
}

::DDS::ReturnCode_t
DDS::OpenSplice::FooDataReaderView_impl::take (
    void * received_data,
    ::DDS::SampleInfoSeq & info_seq,
    DDS::Long max_samples,
    ::DDS::SampleStateMask sample_states,
    ::DDS::ViewStateMask view_states,
    ::DDS::InstanceStateMask instance_states
) THROW_ORB_EXCEPTIONS
{
    ::DDS::ReturnCode_t result = DDS::RETCODE_BAD_PARAMETER;
    u_sampleMask mask;
    u_dataView uView;
    u_result uResult;

    CPP_REPORT_STACK();

    if (statesMaskIsValid(sample_states, view_states, instance_states) == true) {
        mask = statesMask(sample_states, view_states, instance_states);

        result = this->write_lock();
        if (result == DDS::RETCODE_OK) {
            assert(this->pimpl->samplesList != NULL);
            cmn_samplesList_reset(this->pimpl->samplesList, realMaxSamples(max_samples, info_seq));
            uView = u_dataView(this->rlReq_get_user_entity());
            assert(uView != NULL);

            uResult = u_dataViewTake(
                    uView, mask, cmn_reader_action,
                    this->pimpl->samplesList, OS_DURATION_ZERO);
            /* TODO: when samplesList thread specific unlock, see OSPL-4341 */
            if (uResult == U_RESULT_OK) {
                result = this->flush(this->pimpl->samplesList, received_data, info_seq);
            } else {
                result = this->uResultToReturnCode(uResult);
            }

            this->unlock();
        }
    } else {
        CPP_REPORT(result,
                   "sample_states = 0x%x, view_states = 0x%x, instance_states = 0x%x",
                   sample_states, view_states, instance_states);
    }

    CPP_REPORT_FLUSH(this, (result != DDS::RETCODE_OK) && (result != DDS::RETCODE_NO_DATA));

    return result;
}

::DDS::ReturnCode_t
DDS::OpenSplice::FooDataReaderView_impl::read_w_condition (
    void * received_data,
    ::DDS::SampleInfoSeq & info_seq,
    DDS::Long max_samples,
    ::DDS::ReadCondition_ptr a_condition
) THROW_ORB_EXCEPTIONS
{
    ::DDS::ReturnCode_t result = DDS::RETCODE_OK;
    DDS::OpenSplice::ReadCondition *condition;

    CPP_REPORT_STACK();

    if (a_condition == NULL) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "a_condition '<NULL>' is invalid.");
    } else {
        condition = dynamic_cast<DDS::OpenSplice::ReadCondition*>(a_condition);
        if (condition == NULL) {
            result = DDS::RETCODE_BAD_PARAMETER;
            CPP_REPORT(result, "a_condition is invalid, not of type '%s'.",
                "DDS::OpenSplice::ReadCondition");
        }
    }

    if (result == DDS::RETCODE_OK) {
        result = condition->read(this, received_data, info_seq, max_samples, (void*)this->pimpl->samplesList);
    }

    CPP_REPORT_FLUSH(this, (result != DDS::RETCODE_OK) && (result != DDS::RETCODE_NO_DATA));

    return result;
}

::DDS::ReturnCode_t
DDS::OpenSplice::FooDataReaderView_impl::take_w_condition (
    void * received_data,
    ::DDS::SampleInfoSeq & info_seq,
    DDS::Long max_samples,
    ::DDS::ReadCondition_ptr a_condition
) THROW_ORB_EXCEPTIONS
{
    ::DDS::ReturnCode_t result = DDS::RETCODE_OK;
    DDS::OpenSplice::ReadCondition *condition;

    CPP_REPORT_STACK();

    if (a_condition == NULL) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "a_condition '<NULL>' is invalid.");
    } else {
        condition = dynamic_cast<DDS::OpenSplice::ReadCondition*>(a_condition);
        if (condition == NULL) {
            result = DDS::RETCODE_BAD_PARAMETER;
            CPP_REPORT(result, "a_condition is invalid, not of type '%s'.",
                "DDS::OpenSplice::ReadCondition");
        }
    }

    if (result == DDS::RETCODE_OK) {
        result = condition->take(this, received_data, info_seq, max_samples, (void*)this->pimpl->samplesList);
    }

    CPP_REPORT_FLUSH(this, (result != DDS::RETCODE_OK) && (result != DDS::RETCODE_NO_DATA));

    return result;
}

::DDS::ReturnCode_t
DDS::OpenSplice::FooDataReaderView_impl::read_next_sample (
    void * received_data,
    ::DDS::SampleInfo & sample_info
) THROW_ORB_EXCEPTIONS
{
    OS_UNUSED_ARG(received_data);
    OS_UNUSED_ARG(sample_info);
    return DDS::RETCODE_UNSUPPORTED;
}

::DDS::ReturnCode_t
DDS::OpenSplice::FooDataReaderView_impl::take_next_sample (
    void * received_data,
    ::DDS::SampleInfo & sample_info
) THROW_ORB_EXCEPTIONS
{
    OS_UNUSED_ARG(received_data);
    OS_UNUSED_ARG(sample_info);
    return DDS::RETCODE_UNSUPPORTED;
}

::DDS::ReturnCode_t
DDS::OpenSplice::FooDataReaderView_impl::read_instance (
    void * received_data,
    ::DDS::SampleInfoSeq & info_seq,
    DDS::Long max_samples,
    ::DDS::InstanceHandle_t a_handle,
    ::DDS::SampleStateMask sample_states,
    ::DDS::ViewStateMask view_states,
    ::DDS::InstanceStateMask instance_states
) THROW_ORB_EXCEPTIONS
{
    ::DDS::ReturnCode_t result = DDS::RETCODE_BAD_PARAMETER;
    u_sampleMask mask;
    u_dataView uView;
    u_result uResult;
    bool noReport = false;

    CPP_REPORT_STACK();

    if (statesMaskIsValid(sample_states, view_states, instance_states) == true) {
        mask = statesMask(sample_states, view_states, instance_states);

        result = this->write_lock();
        if (result == DDS::RETCODE_OK) {
            assert(this->pimpl->samplesList != NULL);
            cmn_samplesList_reset(this->pimpl->samplesList, realMaxSamples(max_samples, info_seq));
            uView = u_dataView(this->rlReq_get_user_entity());
            assert(uView != NULL);

            uResult = u_dataViewReadInstance(
                    uView, a_handle, mask, cmn_reader_action,
                    this->pimpl->samplesList, OS_DURATION_ZERO);
            noReport = (uResult == U_RESULT_HANDLE_EXPIRED);
            /* TODO: when samplesList thread specific unlock, see OSPL-4341 */
            if (uResult == U_RESULT_OK) {
                result = this->flush(this->pimpl->samplesList, received_data, info_seq);
            } else {
                result = this->uResultToReturnCode(uResult);
            }

            this->unlock();
        }
    } else {
        CPP_REPORT(result,
                   "sample_states = 0x%x, view_states = 0x%x, instance_states = 0x%x",
                   sample_states, view_states, instance_states);
    }

    CPP_REPORT_FLUSH(this, (result != DDS::RETCODE_OK) && (result != DDS::RETCODE_NO_DATA) && !noReport);

    return result;
}

::DDS::ReturnCode_t
DDS::OpenSplice::FooDataReaderView_impl::take_instance (
    void * received_data,
    ::DDS::SampleInfoSeq & info_seq,
    DDS::Long max_samples,
    ::DDS::InstanceHandle_t a_handle,
    ::DDS::SampleStateMask sample_states,
    ::DDS::ViewStateMask view_states,
    ::DDS::InstanceStateMask instance_states
) THROW_ORB_EXCEPTIONS
{
    ::DDS::ReturnCode_t result = DDS::RETCODE_BAD_PARAMETER;
    u_sampleMask mask;
    u_dataView uView;
    u_result uResult;
    bool noReport = false;

    CPP_REPORT_STACK();

    if (statesMaskIsValid(sample_states, view_states, instance_states) == true) {
        mask = statesMask(sample_states, view_states, instance_states);

        result = this->write_lock();
        if (result == DDS::RETCODE_OK) {
            assert(this->pimpl->samplesList != NULL);
            cmn_samplesList_reset(this->pimpl->samplesList, realMaxSamples(max_samples, info_seq));
            uView = u_dataView(this->rlReq_get_user_entity());
            assert(uView != NULL);

            uResult = u_dataViewTakeInstance(
                    uView, a_handle, mask, cmn_reader_action,
                    this->pimpl->samplesList, OS_DURATION_ZERO);
            noReport = (uResult == U_RESULT_HANDLE_EXPIRED);
            /* TODO: when samplesList thread specific unlock, see OSPL-4341 */
            if (uResult == U_RESULT_OK) {
                result = this->flush(this->pimpl->samplesList, received_data, info_seq);
            } else {
                result = this->uResultToReturnCode(uResult);
            }

            this->unlock();
        }
    } else {
        CPP_REPORT(result,
                   "sample_states = 0x%x, view_states = 0x%x, instance_states = 0x%x",
                   sample_states, view_states, instance_states);
    }

    CPP_REPORT_FLUSH(this, (result != DDS::RETCODE_OK) && (result != DDS::RETCODE_NO_DATA) && !noReport);

    return result;
}

::DDS::ReturnCode_t
DDS::OpenSplice::FooDataReaderView_impl::read_next_instance_internal (
    void * received_data,
    ::DDS::SampleInfoSeq & info_seq,
    DDS::Long max_samples,
    ::DDS::InstanceHandle_t a_handle,
    ::DDS::SampleStateMask sample_states,
    ::DDS::ViewStateMask view_states,
    ::DDS::InstanceStateMask instance_states
) THROW_ORB_EXCEPTIONS
{
    ::DDS::ReturnCode_t result = DDS::RETCODE_BAD_PARAMETER;
    u_sampleMask mask;
    u_dataView uView;
    u_result uResult;

    if (statesMaskIsValid(sample_states, view_states, instance_states) == true) {
        mask = statesMask(sample_states, view_states, instance_states);

        result = this->write_lock();
        if (result == DDS::RETCODE_OK) {
            assert(this->pimpl->samplesList != NULL);
            cmn_samplesList_reset(this->pimpl->samplesList, realMaxSamples(max_samples, info_seq));
            uView = u_dataView(this->rlReq_get_user_entity());
            assert(uView != NULL);

            uResult = u_dataViewReadNextInstance(
                    uView, a_handle, mask,
                    cmn_reader_nextInstanceAction_OSPL3588,
                    this->pimpl->samplesList, OS_DURATION_ZERO);
            /* TODO: when samplesList thread specific unlock, see OSPL-4341 */
            if (uResult == U_RESULT_OK) {
                result = this->flush(this->pimpl->samplesList, received_data, info_seq);
            } else if (uResult == U_RESULT_HANDLE_EXPIRED) {
                result = DDS::RETCODE_HANDLE_EXPIRED;
            } else {
                result = this->uResultToReturnCode(uResult);
            }

            this->unlock();
        }
    } else {
        CPP_REPORT(result,
                   "sample_states = 0x%x, view_states = 0x%x, instance_states = 0x%x",
                   sample_states, view_states, instance_states);
    }

    return result;
}

::DDS::ReturnCode_t
DDS::OpenSplice::FooDataReaderView_impl::read_next_instance (
    void * received_data,
    ::DDS::SampleInfoSeq & info_seq,
    DDS::Long max_samples,
    ::DDS::InstanceHandle_t a_handle,
    ::DDS::SampleStateMask sample_states,
    ::DDS::ViewStateMask view_states,
    ::DDS::InstanceStateMask instance_states
) THROW_ORB_EXCEPTIONS
{
    ::DDS::ReturnCode_t result;
    bool noReport = false;

    CPP_REPORT_STACK();

    result = read_next_instance_internal(received_data, info_seq, max_samples,
                                         a_handle, sample_states, view_states, instance_states);

    if (result == DDS::RETCODE_HANDLE_EXPIRED) {
        result = DDS::RETCODE_BAD_PARAMETER;
        noReport = true;
    }

    CPP_REPORT_FLUSH(this, (result != DDS::RETCODE_OK) && (result != DDS::RETCODE_NO_DATA) && !noReport);

    return result;
}

::DDS::ReturnCode_t
DDS::OpenSplice::FooDataReaderView_impl::take_next_instance_internal (
    void * received_data,
    ::DDS::SampleInfoSeq & info_seq,
    DDS::Long max_samples,
    ::DDS::InstanceHandle_t a_handle,
    ::DDS::SampleStateMask sample_states,
    ::DDS::ViewStateMask view_states,
    ::DDS::InstanceStateMask instance_states
) THROW_ORB_EXCEPTIONS
{
    ::DDS::ReturnCode_t result = DDS::RETCODE_BAD_PARAMETER;
    u_sampleMask mask;
    u_dataView uView;
    u_result uResult;

    if (statesMaskIsValid(sample_states, view_states, instance_states) == true) {
        mask = statesMask(sample_states, view_states, instance_states);

        result = this->write_lock();
        if (result == DDS::RETCODE_OK) {
            assert(this->pimpl->samplesList != NULL);
            cmn_samplesList_reset(this->pimpl->samplesList, realMaxSamples(max_samples, info_seq));
            uView = u_dataView(this->rlReq_get_user_entity());
            assert(uView != NULL);

            uResult = u_dataViewTakeNextInstance(
                    uView, a_handle, mask,
                    cmn_reader_nextInstanceAction_OSPL3588,
                    this->pimpl->samplesList, OS_DURATION_ZERO);
            /* TODO: when samplesList thread specific unlock, see OSPL-4341 */
            if (uResult == U_RESULT_OK) {
                result = this->flush(this->pimpl->samplesList, received_data, info_seq);
            } else if (uResult == U_RESULT_HANDLE_EXPIRED) {
                result = DDS::RETCODE_HANDLE_EXPIRED;
            } else {
                result = this->uResultToReturnCode(uResult);
            }

            this->unlock();
        }
    } else {
        CPP_REPORT(result,
                   "sample_states = 0x%x, view_states = 0x%x, instance_states = 0x%x",
                   sample_states, view_states, instance_states);
    }

    return result;
}

::DDS::ReturnCode_t
DDS::OpenSplice::FooDataReaderView_impl::take_next_instance (
    void * received_data,
    ::DDS::SampleInfoSeq & info_seq,
    DDS::Long max_samples,
    ::DDS::InstanceHandle_t a_handle,
    ::DDS::SampleStateMask sample_states,
    ::DDS::ViewStateMask view_states,
    ::DDS::InstanceStateMask instance_states
) THROW_ORB_EXCEPTIONS
{
    ::DDS::ReturnCode_t result;
    bool noReport = false;

    CPP_REPORT_STACK();

    result = take_next_instance_internal(received_data, info_seq, max_samples,
                                         a_handle, sample_states, view_states, instance_states);

    if (result == DDS::RETCODE_HANDLE_EXPIRED) {
        result = DDS::RETCODE_BAD_PARAMETER;
        noReport = true;
    }

    CPP_REPORT_FLUSH(this, (result != DDS::RETCODE_OK) && (result != DDS::RETCODE_NO_DATA) && !noReport);
    return result;
}


::DDS::ReturnCode_t
DDS::OpenSplice::FooDataReaderView_impl::read_next_instance_w_condition (
    void * received_data,
    ::DDS::SampleInfoSeq & info_seq,
    DDS::Long max_samples,
    ::DDS::InstanceHandle_t a_handle,
    ::DDS::ReadCondition_ptr a_condition
) THROW_ORB_EXCEPTIONS
{
    ::DDS::ReturnCode_t result = DDS::RETCODE_OK;
    DDS::OpenSplice::ReadCondition *condition;
    bool noReport = false;

    CPP_REPORT_STACK();

    if (a_condition == NULL) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "a_condition '<NULL>' is invalid.");
    } else {
        condition = dynamic_cast<DDS::OpenSplice::ReadCondition*>(a_condition);
        if (condition == NULL) {
            result = DDS::RETCODE_BAD_PARAMETER;
            CPP_REPORT(result, "a_condition is invalid, not of type '%s'.",
                "DDS::OpenSplice::ReadCondition");
        }
    }

    if (result == DDS::RETCODE_OK) {
        result = condition->read_next_instance(this, received_data, info_seq, max_samples, a_handle, (void*)this->pimpl->samplesList);
        if (result == DDS::RETCODE_HANDLE_EXPIRED) {
            result = DDS::RETCODE_BAD_PARAMETER;
            noReport = true;
        }
    }

    CPP_REPORT_FLUSH(this, (result != DDS::RETCODE_OK) && (result != DDS::RETCODE_NO_DATA) && !noReport);

    return result;
}

::DDS::ReturnCode_t
DDS::OpenSplice::FooDataReaderView_impl::take_next_instance_w_condition (
    void * received_data,
    ::DDS::SampleInfoSeq & info_seq,
    DDS::Long max_samples,
    ::DDS::InstanceHandle_t a_handle,
    ::DDS::ReadCondition_ptr a_condition
) THROW_ORB_EXCEPTIONS
{
    ::DDS::ReturnCode_t result = DDS::RETCODE_OK;
    DDS::OpenSplice::ReadCondition *condition;
    bool noReport = false;

    CPP_REPORT_STACK();

    if (a_condition == NULL) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "a_condition '<NULL>' is invalid.");
    } else {
        condition = dynamic_cast<DDS::OpenSplice::ReadCondition*>(a_condition);
        if (condition == NULL) {
            result = DDS::RETCODE_BAD_PARAMETER;
            CPP_REPORT(result, "a_condition is invalid, not of type '%s'.",
                "DDS::OpenSplice::ReadCondition");
        }
    }

    if (result == DDS::RETCODE_OK) {
        result = condition->take_next_instance(this, received_data, info_seq, max_samples, a_handle, (void*)this->pimpl->samplesList);
        if (result == DDS::RETCODE_HANDLE_EXPIRED) {
            result = DDS::RETCODE_BAD_PARAMETER;
            noReport = true;
        }
    }

    CPP_REPORT_FLUSH(this, (result != DDS::RETCODE_OK) && (result != DDS::RETCODE_NO_DATA) && !noReport);

    return result;
}

::DDS::ReturnCode_t
DDS::OpenSplice::FooDataReaderView_impl::wlReq_return_loan (
    void *data_buffer,
    void *info_buffer
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;

    CPP_REPORT_STACK();

    assert (this->pimpl->loanRegistry != NULL);
    result = this->pimpl->loanRegistry->deregister_loan(data_buffer, info_buffer);

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

::DDS::ReturnCode_t
DDS::OpenSplice::FooDataReaderView_impl::get_key_value (
    void * key_holder,
    ::DDS::InstanceHandle_t handle
) THROW_ORB_EXCEPTIONS
{
    OS_UNUSED_ARG(key_holder);
    OS_UNUSED_ARG(handle);
    return DDS::RETCODE_UNSUPPORTED;
}

::DDS::InstanceHandle_t
DDS::OpenSplice::FooDataReaderView_impl::lookup_instance (
    const void * instance
) THROW_ORB_EXCEPTIONS
{
    ::DDS::InstanceHandle_t handle = DDS::HANDLE_NIL;
    DDS::ReturnCode_t result;
    u_dataView uView;
    u_result uResult;
    viewCopyInfo data;

    CPP_REPORT_STACK();

    result = this->check();
    if (result == DDS::RETCODE_OK) {
        assert(this->pimpl->copyIn != NULL);
        uView = u_dataView(this->rlReq_get_user_entity());
        assert(uView != NULL);
        data.view = this;
        data.data = (void *)instance;
        uResult = u_dataViewLookupInstance(
                uView,
                (void *)&data,
                (u_copyIn)this->pimpl->rlReq_copyIn,
                (u_instanceHandle *) &handle);
        result = uResultToReturnCode(uResult);
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return handle;
}

::DDS::ReturnCode_t
DDS::OpenSplice::FooDataReaderView_impl::flush (
    void * samplesList,
    void * received_data,
    ::DDS::SampleInfoSeq & info_seq)
{
    ::DDS::ReturnCode_t result;
    DDS::OpenSplice::FooDataReader_impl *reader;
    OS_UNUSED_ARG(samplesList);
    CPP_REPORT_STACK();

    result = this->pimpl->prepareSequences(received_data, info_seq);
    if (result == DDS::RETCODE_OK) {
        /* TODO: use get_datareader, when OSPL-4341 implemented */
        reader = dynamic_cast<DDS::OpenSplice::FooDataReader_impl *>(this->rlReq_get_datareader());
        reader->write_lock();
        if (result == DDS::RETCODE_OK) {
            result = reader->actualFlush(this->pimpl->samplesList, received_data, info_seq);
            reader->unlock();
        }
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

/*
 * Implementation
 */
::DDS::ReturnCode_t
DDS::OpenSplice::FooDataReaderView_impl::Implementation::prepareSequences (
    void * received_data,
    ::DDS::SampleInfoSeq & info_seq)
{
    ::DDS::ReturnCode_t result;
    DDS::ULong length;
    void * data;

    length = cmn_samplesList_length(this->samplesList);
    if (length > 0) {
        if (info_seq.maximum() == 0) {
            info_seq.replace(length, length, info_seq.allocbuf(length), false);
            data = this->dataSeqAlloc(received_data, length);

#if 0
            /* TODO: Enable when sampleList is thread specific, see OSPL-4341 */
            this->write_lock();
            if (result == DDS::RETCODE_OK) {
#endif
                result = this->loanRegistry->register_loan(data, info_seq.get_buffer());
#if 0
                this->unlock();
            }
#endif
        } else {
            info_seq.length(length);
            this->dataSeqLength(received_data, length);
            result = DDS::RETCODE_OK;
        }
    } else {
        result = DDS::RETCODE_NO_DATA;
    }

    return result;
}

v_copyin_result
DDS::OpenSplice::FooDataReaderView_impl::Implementation::rlReq_copyIn (
    c_type type,
    void *data,
    void *to)
{
    v_copyin_result result;
    c_base base = c_getBase(c_object(type));
    viewCopyInfo *info = (viewCopyInfo *)data;

    /* TODO: Add copy cache, see SAC _DataReaderViewCopyIn(). */
    result = info->view->pimpl->copyIn (base, info->data, to);

    return result;
}
