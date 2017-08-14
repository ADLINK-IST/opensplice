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
#include "FooDataReader_impl.h"
#include "MiscUtils.h"
#include "StatusUtils.h"
#include "ReportUtils.h"
#include "LoanRegistry.h"
#include "v_dataReaderInstance.h"
#include "ReadCondition.h"
#include "cmn_samplesList.h"
#include "cmn_reader.h"
#include "Constants.h"
#include "os_atomics.h"

using namespace DDS::OpenSplice::Utils;

typedef void (*cxxCopyInfoOut)   (const cmn_sampleInfo from, ::DDS::SampleInfo * to);
typedef void (*cxxCopySampleOut) (void *sample, cmn_sampleInfo info, void *arg);

typedef struct readerCopyInfo_s {
    DDS::OpenSplice::FooDataReader_impl *reader;
    void *data;
} readerCopyInfo;

typedef struct copyArg_s {
    void                *dataSample;
    ::DDS::SampleInfo   *infoSample;
    DDS::OpenSplice::FooDataReader_impl::cxxCopyDataOut copyDataOut;
    cxxCopyInfoOut      copyInfoOut;
    ::DDS::ReturnCode_t result;

    copyArg_s() : dataSample(NULL), infoSample(NULL), copyDataOut(NULL), copyInfoOut(NULL), result(DDS::RETCODE_ERROR) {};
} copyArg_t;

class parallelDemarshaling {
private:
    typedef struct heuristics_s {
        DDS::UShort threshold; /* Number of samples threshold at which parallelization should be performed */
        DDS::UShort block;     /* Number of samples to be read without inter-thread sync */
    } heuristics_t;

    typedef struct copyInfo_s {
        os_uint32               length;
        pa_uint32_t             nextIndex;
        void                    *dataSeq;
        ::DDS::SampleInfoSeq    *infoSeq;
        DDS::OpenSplice::FooDataReader_impl::cxxDataSeqGetBuffer     getDataBuffer;
        DDS::OpenSplice::FooDataReader_impl::cxxCopyDataOut          copyDataOut;
        cmn_samplesList         samplesList;
        u_entity                uEntity;
    } copyInfo_t;

    static void *workerMain(void *);

public:
    parallelDemarshaling();
    ~parallelDemarshaling();

    ::DDS::ReturnCode_t init(
            DDS::OpenSplice::FooDataReader_impl::cxxDataSeqGetBuffer dataSeqGetBuffer,
            DDS::OpenSplice::FooDataReader_impl::cxxCopyDataOut copyDataOut,
            cxxCopySampleOut copySampleOut);
    ::DDS::ReturnCode_t deinit();

    ::DDS::ReturnCode_t start_workers(DDS::UShort count);
    ::DDS::ReturnCode_t stop_workers();

    ::DDS::ReturnCode_t copy (
        cmn_samplesList samplesList,
        u_entity uEntity,
        void * received_data,
        ::DDS::SampleInfoSeq & info_seq);

    DDS::Boolean do_copy();

    DDS::UShort get_workerCount();
    DDS::Boolean is_favorable(DDS::ULong length);

private:
    os_mutex    mtx;
    os_cond     startCond;
    os_cond     readyCond;

    DDS::UShort workerCount;   /* Number of created workers, when 0 parallel demarshaling is not enabled (default) */
    DDS::UShort setProperty;   /* value used to for setting up parallel demarshaling */
    DDS::UShort readyCount;

    heuristics_t heuristics;
    copyInfo_t  copyInfo;

    os_threadId *tids;

    /* The parity is used for the loop-condition for readyCnd. Members of the
     * copy_info struct may be reused, so don't provide a safe loop-condition.
     * The parity toggles every loop and since it is not changed until all threads
     * are waiting on the next loop, it is a safe loop condition. */
    DDS::Boolean parity;
    volatile DDS::Boolean terminate;

    DDS::OpenSplice::FooDataReader_impl::cxxDataSeqGetBuffer dataSeqGetBuffer;
    DDS::OpenSplice::FooDataReader_impl::cxxCopyDataOut copyDataOut;
    cxxCopySampleOut copySampleOut;
};

struct DDS::OpenSplice::FooDataReader_impl::Implementation {
    DDS::OpenSplice::LoanRegistry *loanRegistry;
    cmn_samplesList samplesList;

    DDS::OpenSplice::cxxCopyIn  copyIn;
    DDS::OpenSplice::cxxCopyOut copyOut;

    cxxDataSeqAlloc     dataSeqAlloc;
    cxxDataSeqLength    dataSeqLength;
    cxxDataSeqGetBuffer dataSeqGetBuffer;
    cxxCopyDataOut      copyDataOut;

    parallelDemarshaling *pdc;

    DDS::Boolean ignoreLoansOnDeletion;

    static v_copyin_result rlReq_copyIn (
        c_type type,
        void *data,
        void *to);

    ::DDS::ReturnCode_t prepareSequences (
        void * received_data,
        ::DDS::SampleInfoSeq & info_seq);

    ::DDS::ReturnCode_t singleThreadedCopy (
        cmn_samplesList samplesList,
        u_entity uEntity,
        void * received_data,
        ::DDS::SampleInfoSeq & info_seq);

    static void copyInfoOut(
        const cmn_sampleInfo from,
        ::DDS::SampleInfo * to);

    static void copySampleOut (
        void *sample,
        cmn_sampleInfo info,
        void *arg);
};

DDS::OpenSplice::FooDataReader_impl::FooDataReader_impl() :
    pimpl(new Implementation)
{
    this->pimpl->pdc = NULL;
    this->pimpl->loanRegistry = NULL;
    this->pimpl->samplesList = NULL;
    this->pimpl->copyIn = NULL;
    this->pimpl->copyOut = NULL;
    this->pimpl->dataSeqAlloc = NULL;
    this->pimpl->dataSeqLength = NULL;
    this->pimpl->dataSeqGetBuffer = NULL;
    this->pimpl->copyDataOut = NULL;
    this->pimpl->ignoreLoansOnDeletion = false;
}

DDS::OpenSplice::FooDataReader_impl::~FooDataReader_impl()
{
    delete pimpl;
}

DDS::ReturnCode_t
DDS::OpenSplice::FooDataReader_impl::nlReq_init(
    DDS::OpenSplice::Subscriber *subscriber,
    const DDS::DataReaderQos &qos,
    DDS::OpenSplice::TopicDescription *a_topic,
    const char *name,
    DDS::OpenSplice::cxxCopyIn copyIn,
    DDS::OpenSplice::cxxCopyOut copyOut,
    cxxDataSeqAlloc dataSeqAlloc,
    cxxDataSeqLength dataSeqLength,
    cxxDataSeqGetBuffer dataSeqGetBuffer,
    cxxCopyDataOut copyDataOut)
{
    DDS::ReturnCode_t result = DDS::RETCODE_ERROR;

    assert(copyIn);
    assert(copyOut);
    assert(dataSeqAlloc);
    assert(dataSeqLength);
    assert(dataSeqGetBuffer);
    assert(copyDataOut);

    result = DDS::OpenSplice::DataReader::nlReq_init(subscriber, qos, a_topic, name);

    if (result == DDS::RETCODE_OK) {
        this->pimpl->loanRegistry = new LoanRegistry;
        this->pimpl->samplesList = cmn_samplesList_new((os_boolean)FALSE);

        this->pimpl->copyIn = copyIn;
        this->pimpl->copyOut = copyOut;

        this->pimpl->dataSeqAlloc = dataSeqAlloc;
        this->pimpl->dataSeqLength = dataSeqLength;
        this->pimpl->dataSeqGetBuffer = dataSeqGetBuffer;
        this->pimpl->copyDataOut = copyDataOut;
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::FooDataReader_impl::wlReq_deinit ()
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    DDS::StatusMask mask;
    os_char *name;

    mask = this->rlReq_get_listener_mask();
    if (mask != 0) {
        result = this->wlReq_set_listener_mask(0);
        if (result != DDS::RETCODE_OK) {
            result = DDS::RETCODE_ERROR;
            name = u_entityName(rlReq_get_user_entity());
            CPP_REPORT(result, "DataReader %s failed to disable listener.", name);
            os_free(name);
        }
    }

    if (result == DDS::RETCODE_OK) {
        if (this->pimpl->loanRegistry && (this->pimpl->ignoreLoansOnDeletion == false)) {
            if (this->pimpl->loanRegistry->is_empty() == FALSE) {
                result = DDS::RETCODE_PRECONDITION_NOT_MET;
                name = u_entityName(rlReq_get_user_entity());
                CPP_REPORT(result, "DataReader %s still contains non returned loans.", name);
                os_free(name);
            }
        }
    }

    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::DataReader::wlReq_deinit();
    }
    if ((result == DDS::RETCODE_PRECONDITION_NOT_MET) && (mask != 0)) {
        DDS::ReturnCode_t retcode;
        /* Rollback only possible for precondition not met, reset the listener.
         * Might have missed some events. */
        retcode = this->wlReq_set_listener_mask(mask);
        if (retcode != DDS::RETCODE_OK) {
            name = u_entityName(rlReq_get_user_entity());
            CPP_REPORT(retcode, "DataReader %s failed to reset the listener, no more callbacks", name);
            os_free(name);
        }
    }

    if (result == DDS::RETCODE_OK) {
        if (this->pimpl->pdc) {
            this->pimpl->pdc->deinit();
            delete this->pimpl->pdc;
            this->pimpl->pdc = NULL;
        }
        if (this->pimpl->loanRegistry) {
            delete this->pimpl->loanRegistry;
            this->pimpl->loanRegistry = NULL;
        }
        if (this->pimpl->samplesList) {
            cmn_samplesList_free(this->pimpl->samplesList);
            this->pimpl->samplesList = NULL;
        }
    }

    return result;
}

::DDS::ReturnCode_t
DDS::OpenSplice::FooDataReader_impl::read (
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
    u_dataReader uReader;
    u_result uResult;

    CPP_REPORT_STACK();

    if (statesMaskIsValid(sample_states, view_states, instance_states) == true) {
        mask = statesMask(sample_states, view_states, instance_states);

        result = this->write_lock();
        if (result == DDS::RETCODE_OK) {
            cmn_samplesList_reset(this->pimpl->samplesList, realMaxSamples(max_samples, info_seq));
            uReader = u_dataReader(this->rlReq_get_user_entity());
            assert(uReader != NULL);

            uResult = u_dataReaderRead(
                    uReader, mask, cmn_reader_action,
                    this->pimpl->samplesList, OS_DURATION_ZERO);
            /* TODO: when samplesList thread specific unlock, see OSPL-4341 */
            if (uResult == U_RESULT_OK) {
                result = this->flush(this->pimpl->samplesList, received_data, info_seq);
            } else {
                result = uResultToReturnCode(uResult);
            }

            this->unlock();
        }
    } else {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result,
                   "sample_states = 0x%x, view_states = 0x%x, instance_states = 0x%x",
                   sample_states, view_states, instance_states);
    }

    CPP_REPORT_FLUSH(this, (result != DDS::RETCODE_OK) && (result != DDS::RETCODE_NO_DATA));

    return result;
}

::DDS::ReturnCode_t
DDS::OpenSplice::FooDataReader_impl::take (
    void * received_data,
    ::DDS::SampleInfoSeq & info_seq,
    DDS::Long max_samples,
    ::DDS::SampleStateMask sample_states,
    ::DDS::ViewStateMask view_states,
    ::DDS::InstanceStateMask instance_states
) THROW_ORB_EXCEPTIONS
{
    ::DDS::ReturnCode_t result;
    u_sampleMask mask;
    u_dataReader uReader;
    u_result uResult;

    CPP_REPORT_STACK();

    if (statesMaskIsValid(sample_states, view_states, instance_states) == true) {
        mask = statesMask(sample_states, view_states, instance_states);

        result = this->write_lock();
        if (result == DDS::RETCODE_OK) {
            cmn_samplesList_reset(this->pimpl->samplesList, realMaxSamples(max_samples, info_seq));
            uReader = u_dataReader(this->rlReq_get_user_entity());
            assert(uReader != NULL);

            uResult = u_dataReaderTake(
                    uReader, mask, cmn_reader_action,
                    this->pimpl->samplesList, OS_DURATION_ZERO);
            /* TODO: when samplesList thread specific unlock, see OSPL-4341 */
            if (uResult == U_RESULT_OK) {
                result = this->flush(this->pimpl->samplesList, received_data, info_seq);
            } else {
                result = uResultToReturnCode(uResult);
            }

            this->unlock();
        }
    } else {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result,
                   "sample_states = 0x%x, view_states = 0x%x, instance_states = 0x%x",
                   sample_states, view_states, instance_states);
    }

    CPP_REPORT_FLUSH(this, (result != DDS::RETCODE_OK) && (result != DDS::RETCODE_NO_DATA));

    return result;
}

::DDS::ReturnCode_t
DDS::OpenSplice::FooDataReader_impl::read_w_condition (
    void * received_data,
    ::DDS::SampleInfoSeq & info_seq,
    DDS::Long max_samples,
    ::DDS::ReadCondition_ptr a_condition
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
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
DDS::OpenSplice::FooDataReader_impl::take_w_condition (
    void * received_data,
    ::DDS::SampleInfoSeq & info_seq,
    DDS::Long max_samples,
    ::DDS::ReadCondition_ptr a_condition
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
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
DDS::OpenSplice::FooDataReader_impl::read_next_sample (
    void * received_data,
    ::DDS::SampleInfo & sample_info
) THROW_ORB_EXCEPTIONS
{
    OS_UNUSED_ARG(received_data);
    OS_UNUSED_ARG(sample_info);
    return DDS::RETCODE_UNSUPPORTED;
}

::DDS::ReturnCode_t
DDS::OpenSplice::FooDataReader_impl::take_next_sample (
    void * received_data,
    ::DDS::SampleInfo & sample_info
) THROW_ORB_EXCEPTIONS
{
    OS_UNUSED_ARG(received_data);
    OS_UNUSED_ARG(sample_info);
    return DDS::RETCODE_UNSUPPORTED;
}

::DDS::ReturnCode_t
DDS::OpenSplice::FooDataReader_impl::read_instance (
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
    u_sampleMask mask;
    u_dataReader uReader;
    u_result uResult;
    bool noReport = false;

    CPP_REPORT_STACK();

    if (statesMaskIsValid(sample_states, view_states, instance_states) == true) {
        mask = statesMask(sample_states, view_states, instance_states);

        result = this->write_lock();
        if (result == DDS::RETCODE_OK) {
            cmn_samplesList_reset(this->pimpl->samplesList, realMaxSamples(max_samples, info_seq));
            uReader = u_dataReader(this->rlReq_get_user_entity());
            assert(uReader != NULL);

            uResult = u_dataReaderReadInstance(
                    uReader, (u_instanceHandle) a_handle,
                    mask, cmn_reader_action,
                    this->pimpl->samplesList, OS_DURATION_ZERO);
            noReport = (uResult == U_RESULT_HANDLE_EXPIRED);
            /* TODO: when samplesList thread specific unlock, see OSPL-4341 */
            if (uResult == U_RESULT_OK) {
                result = this->flush(this->pimpl->samplesList, received_data, info_seq);
            } else {
                result = uResultToReturnCode(uResult);
            }

            this->unlock();
        }
    } else {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result,
                   "sample_states = 0x%x, view_states = 0x%x, instance_states = 0x%x",
                   sample_states, view_states, instance_states);
    }

    CPP_REPORT_FLUSH(this, (result != DDS::RETCODE_OK) && (result != DDS::RETCODE_NO_DATA) && !noReport);

    return result;
}

::DDS::ReturnCode_t
DDS::OpenSplice::FooDataReader_impl::take_instance (
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
    u_sampleMask mask;
    u_dataReader uReader;
    u_result uResult;
    bool noReport = false;

    CPP_REPORT_STACK();

    if (statesMaskIsValid(sample_states, view_states, instance_states) == true) {
        mask = statesMask(sample_states, view_states, instance_states);

        result = this->write_lock();
        if (result == DDS::RETCODE_OK) {
            cmn_samplesList_reset(this->pimpl->samplesList, realMaxSamples(max_samples, info_seq));
            uReader = u_dataReader(this->rlReq_get_user_entity());
            assert(uReader != NULL);

            uResult = u_dataReaderTakeInstance(
                    uReader, (u_instanceHandle) a_handle,
                    mask, cmn_reader_action,
                    this->pimpl->samplesList, OS_DURATION_ZERO);
            noReport = (uResult == U_RESULT_HANDLE_EXPIRED);
            /* TODO: when samplesList thread specific unlock, see OSPL-4341 */
            if (uResult == U_RESULT_OK) {
                result = this->flush(this->pimpl->samplesList, received_data, info_seq);
            } else {
                result = uResultToReturnCode(uResult);
            }

            this->unlock();
        }
    } else {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result,
                   "sample_states = 0x%x, view_states = 0x%x, instance_states = 0x%x",
                   sample_states, view_states, instance_states);
    }

    CPP_REPORT_FLUSH(this, (result != DDS::RETCODE_OK) && (result != DDS::RETCODE_NO_DATA) && !noReport);

    return result;
}

::DDS::ReturnCode_t
DDS::OpenSplice::FooDataReader_impl::read_next_instance_internal (
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
    u_sampleMask mask;
    u_dataReader uReader;
    u_result uResult;

    if (statesMaskIsValid(sample_states, view_states, instance_states) == true) {
        mask = statesMask(sample_states, view_states, instance_states);

        result = this->write_lock();
        if (result == DDS::RETCODE_OK) {
            cmn_samplesList_reset(this->pimpl->samplesList, realMaxSamples(max_samples, info_seq));
            uReader = u_dataReader(this->rlReq_get_user_entity());
            assert(uReader != NULL);

            uResult = u_dataReaderReadNextInstance(
                    uReader, (u_instanceHandle) a_handle,
                    mask, cmn_reader_nextInstanceAction,
                    this->pimpl->samplesList, OS_DURATION_ZERO);
            /* TODO: when samplesList thread specific unlock, see OSPL-4341 */
            if (uResult == U_RESULT_OK) {
                result = this->flush(this->pimpl->samplesList, received_data, info_seq);
            } else if (uResult == U_RESULT_HANDLE_EXPIRED) {
                result = DDS::RETCODE_HANDLE_EXPIRED;
            } else {
                result = uResultToReturnCode(uResult);
            }

            this->unlock();
        }
    } else {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result,
                   "sample_states = 0x%x, view_states = 0x%x, instance_states = 0x%x",
                   sample_states, view_states, instance_states);
    }

    return result;
}

::DDS::ReturnCode_t
DDS::OpenSplice::FooDataReader_impl::read_next_instance (
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

    CPP_REPORT_FLUSH(this, (result != DDS::RETCODE_OK) && (result != DDS::RETCODE_NO_DATA) && !noReport);

    return result;
}


::DDS::ReturnCode_t
DDS::OpenSplice::FooDataReader_impl::take_next_instance_internal (
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
    u_sampleMask mask;
    u_dataReader uReader;
    u_result uResult;

    if (statesMaskIsValid(sample_states, view_states, instance_states) == true) {
        mask = statesMask(sample_states, view_states, instance_states);

        result = this->write_lock();
        if (result == DDS::RETCODE_OK) {
            cmn_samplesList_reset(this->pimpl->samplesList, realMaxSamples(max_samples, info_seq));
            uReader = u_dataReader(this->rlReq_get_user_entity());
            assert(uReader != NULL);

            uResult = u_dataReaderTakeNextInstance(
                    uReader, (u_instanceHandle) a_handle,
                    mask, cmn_reader_nextInstanceAction,
                    this->pimpl->samplesList, OS_DURATION_ZERO);
            /* TODO: when samplesList thread specific unlock, see OSPL-4341 */
            if (uResult == U_RESULT_OK) {
                result = this->flush(this->pimpl->samplesList, received_data, info_seq);
            } else if (uResult == U_RESULT_HANDLE_EXPIRED) {
                result = DDS::RETCODE_HANDLE_EXPIRED;
            } else {
                result = uResultToReturnCode(uResult);
            }

            this->unlock();
        }
    } else {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result,
                   "sample_states = 0x%x, view_states = 0x%x, instance_states = 0x%x",
                   sample_states, view_states, instance_states);
    }

    return result;
}


::DDS::ReturnCode_t
DDS::OpenSplice::FooDataReader_impl::take_next_instance (
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
DDS::OpenSplice::FooDataReader_impl::read_next_instance_w_condition (
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
DDS::OpenSplice::FooDataReader_impl::take_next_instance_w_condition (
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

DDS::ReturnCode_t
DDS::OpenSplice::FooDataReader_impl::wlReq_return_loan (
    void *data_buffer,
    void *info_buffer)
{
    DDS::ReturnCode_t result;

    CPP_REPORT_STACK();

    assert(this->pimpl->loanRegistry);
    result = this->pimpl->loanRegistry->deregister_loan(data_buffer, info_buffer);

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

::DDS::ReturnCode_t
DDS::OpenSplice::FooDataReader_impl::get_key_value (
    void * key_holder,
    ::DDS::InstanceHandle_t handle
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    u_dataReader uReader;
    u_result uResult;

    CPP_REPORT_STACK();

    assert(key_holder != NULL);

    result = this->check();
    if (result == DDS::RETCODE_OK) {
        uReader = u_dataReader(this->rlReq_get_user_entity());
        assert(uReader != NULL);

        uResult = u_dataReaderCopyKeysFromInstanceHandle(
                    uReader,
                    (u_instanceHandle)handle,
                    this->pimpl->copyOut,
                    key_holder);
        result = uResultToReturnCode(uResult);

        /* The OpenSplice user-layer may detect that the instance is deleted
         * In that case according to the spec return PRECONDITION_NOT_MET.
         */
        if (result == DDS::RETCODE_ALREADY_DELETED) {
            result = DDS::RETCODE_PRECONDITION_NOT_MET;
            CPP_REPORT(result, "Instance is not registered.");
        }
    }
    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

::DDS::InstanceHandle_t
DDS::OpenSplice::FooDataReader_impl::lookup_instance (
    const void * instance
) THROW_ORB_EXCEPTIONS
{
    ::DDS::InstanceHandle_t handle = DDS::HANDLE_NIL;
    DDS::ReturnCode_t result;
    u_dataReader uReader;
    u_result uResult;
    readerCopyInfo data;

    CPP_REPORT_STACK();

    result = this->check();
    if (result == DDS::RETCODE_OK) {
        uReader = u_dataReader(this->rlReq_get_user_entity());
        assert(uReader != NULL);
        data.reader = this;
        data.data = (void *)instance;
        uResult = u_dataReaderLookupInstance(
                uReader,
                (void *)&data,
                (u_copyIn)this->pimpl->rlReq_copyIn,
                &handle);
        result = uResultToReturnCode(uResult);
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return handle;
}

::DDS::ReturnCode_t
DDS::OpenSplice::FooDataReader_impl::flush (
    void * samplesList,
    void * received_data,
    ::DDS::SampleInfoSeq & info_seq)
{
    ::DDS::ReturnCode_t result;
    OS_UNUSED_ARG(samplesList);
    CPP_REPORT_STACK();

    result = this->pimpl->prepareSequences(received_data, info_seq);
    if (result == DDS::RETCODE_OK) {
        result = this->actualFlush(this->pimpl->samplesList, received_data, info_seq);
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

::DDS::ReturnCode_t
DDS::OpenSplice::FooDataReader_impl::actualFlush (
    void * samplesList,
    void * received_data,
    ::DDS::SampleInfoSeq & info_seq)
{
    ::DDS::ReturnCode_t result = DDS::RETCODE_ERROR;
    u_entity uEntity = rlReq_get_user_entity();
    cmn_samplesList list;
    DDS::ULong length;
    DDS::Boolean single = TRUE;
    assert(samplesList);
    assert(received_data);

    list = reinterpret_cast<cmn_samplesList>(samplesList);

    length = cmn_samplesList_length(list);

    if (this->pimpl->pdc != NULL) {
        if (this->pimpl->pdc->is_favorable(length)) {
            single = FALSE;
            result = this->pimpl->pdc->copy(list, uEntity, received_data, info_seq);
        }
    }
    if (single) {
        result = this->pimpl->singleThreadedCopy(list, uEntity, received_data, info_seq);
    }

    /* Free samples */
    if (u_readerProtectCopyOutEnter(uEntity) == U_RESULT_OK) {
        cmn_samplesList_reset(list, 0);
        u_readerProtectCopyOutExit(uEntity);
    }

    return result;
}


DDS::Long
DDS::OpenSplice::FooDataReader_impl::rlReq_get_workers ()
{
    DDS::Long workerCount = 0;

    if (this->pimpl->pdc != NULL) {
        workerCount = this->pimpl->pdc->get_workerCount();
    }

    return workerCount;
}

DDS::ReturnCode_t
DDS::OpenSplice::FooDataReader_impl::wlReq_set_workers (
    DDS::Long value)
{
    DDS::ReturnCode_t result = DDS::RETCODE_ERROR;
    parallelDemarshaling * pdc = NULL;

    DDS::Boolean start = FALSE;

    if (value <= 1) {
        if (this->pimpl->pdc != NULL) {
            pdc = this->pimpl->pdc;
            this->pimpl->pdc = NULL;
            pdc->deinit();
            delete pdc;
        }

        result = DDS::RETCODE_OK;
    } else {
        if (this->pimpl->pdc != NULL) {
            /* workerCount should always be > 0 when pdc != NULL */
            if (this->pimpl->pdc->get_workerCount() == value) {
                /* Requested amount of threads already active, do nothing.
                 */
                result = DDS::RETCODE_OK;
            } else {
                /* Requested amount of workers NOT active, stop all workers.
                 */
                this->pimpl->pdc->stop_workers();
                start = TRUE;
            }
        } else {
            /* ParallelDemarshaling not yet active, start it now.
             */
            pdc = new parallelDemarshaling();
            pdc->init(this->pimpl->dataSeqGetBuffer, this->pimpl->copyDataOut, this->pimpl->copySampleOut);
            this->pimpl->pdc = pdc;
            start = TRUE;
        }
    }

    if (start) {
        result = this->pimpl->pdc->start_workers(value);

        if ((result != DDS::RETCODE_OK) &&
            (this->pimpl->pdc->get_workerCount() == 0)) {
            pdc = this->pimpl->pdc;
            this->pimpl->pdc = NULL;
            pdc->deinit();
            delete pdc;
        }
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::FooDataReader_impl::wlReq_set_ignoreOpenLoansAtDeletionStatus (
    DDS::Boolean value)
{
    this->pimpl->ignoreLoansOnDeletion = value;
    return DDS::RETCODE_OK;
}

DDS::Boolean
DDS::OpenSplice::FooDataReader_impl::rlReq_get_ignoreOpenLoansAtDeletionStatus ()
{
    return this->pimpl->ignoreLoansOnDeletion;
}

/*
 * Implementation
 */
v_copyin_result
DDS::OpenSplice::FooDataReader_impl::Implementation::rlReq_copyIn (
    c_type type,
    void *data,
    void *to)
{
    v_copyin_result result;
    c_base base = c_getBase(c_object(type));
    readerCopyInfo *info = (readerCopyInfo *)data;

    /* TODO: Add copy cache, see SAC _DataReaderCopyIn(). */
    result = info->reader->pimpl->copyIn (base, info->data, to);

    return result;
}

::DDS::ReturnCode_t
DDS::OpenSplice::FooDataReader_impl::Implementation::prepareSequences (
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

DDS::ReturnCode_t
DDS::OpenSplice::FooDataReader_impl::Implementation::singleThreadedCopy (
    cmn_samplesList samplesList,
    u_entity uEntity,
    void * received_data,
    ::DDS::SampleInfoSeq & info_seq)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    os_uint32 length;
    os_uint32 i;
    os_int32 res;
    u_result uResult;
    copyArg_t copyArg;

    copyArg.copyDataOut = this->copyDataOut;
    copyArg.result = DDS::RETCODE_OK;
    length = cmn_samplesList_length(samplesList);
    for (i = 0; i < length; i++) {
        copyArg.dataSample = this->dataSeqGetBuffer(received_data, i);
        copyArg.infoSample = &(info_seq.get_buffer()[i]);

        uResult = u_readerProtectCopyOutEnter(uEntity);
        if (uResult == U_RESULT_OK) {
            res = cmn_samplesList_read(samplesList, i, this->copySampleOut, &copyArg);
            u_readerProtectCopyOutExit(uEntity);
            if ((copyArg.result != DDS::RETCODE_OK) || (res != 1)) {
                result = copyArg.result;
                break;
            }
        } else {
            if (i == 0) {
                result = uResultToReturnCode(uResult);
            }
        }
    }

    return result;
}

void
DDS::OpenSplice::FooDataReader_impl::Implementation::copySampleOut (
    void *sample,
    cmn_sampleInfo info,
    void *arg)
{
    copyArg_t *copyArg = reinterpret_cast<copyArg_t *>(arg);

    copyArg->copyDataOut(sample, copyArg->dataSample);
    DDS::OpenSplice::FooDataReader_impl::Implementation::copyInfoOut(info, copyArg->infoSample);
}

void
DDS::OpenSplice::FooDataReader_impl::Implementation::copyInfoOut(
    const cmn_sampleInfo from,
    ::DDS::SampleInfo * to)
{
    to->sample_state                 = static_cast<DDS::SampleStateKind>(from->sample_state);
    to->view_state                   = static_cast<DDS::ViewStateKind>(from->view_state);
    to->instance_state               = static_cast<DDS::InstanceStateKind>(from->instance_state);
    to->disposed_generation_count    = static_cast< ::DDS::Long>(from->disposed_generation_count);
    to->no_writers_generation_count  = static_cast< ::DDS::Long>(from->no_writers_generation_count);
    to->sample_rank                  = static_cast< ::DDS::Long>(from->sample_rank);
    to->generation_rank              = static_cast< ::DDS::Long>(from->generation_rank);
    to->absolute_generation_rank     = static_cast< ::DDS::Long>(from->absolute_generation_rank);
    to->instance_handle              = static_cast<DDS::InstanceHandle_t>(from->instance_handle);
    to->publication_handle           = static_cast<DDS::InstanceHandle_t>(from->publication_handle);
    to->valid_data                   = static_cast< ::DDS::Boolean>(from->valid_data);
    DDS::OpenSplice::Utils::copyTimeOut(from->source_timestamp, to->source_timestamp);
    DDS::OpenSplice::Utils::copyTimeOut(from->reception_timestamp, to->reception_timestamp);
}

parallelDemarshaling::parallelDemarshaling () :
    workerCount (0),
    setProperty (0),
    readyCount (0),
    tids(NULL),
    parity(FALSE),
    terminate(FALSE),
    copySampleOut(NULL)
{
    this->heuristics.threshold = 2;
    this->heuristics.block = 1;

    this->copyInfo.length = 0;
    pa_st32(&this->copyInfo.nextIndex, 0);
    this->copyInfo.dataSeq = NULL;
    this->copyInfo.infoSeq = NULL;
    this->copyInfo.getDataBuffer = NULL;
    this->copyInfo.copyDataOut = NULL;
    this->copyInfo.samplesList = NULL;
    this->copyInfo.uEntity = NULL;
};

parallelDemarshaling::~parallelDemarshaling ()
{
    /* NOP */
};

::DDS::ReturnCode_t
parallelDemarshaling::init (
    DDS::OpenSplice::FooDataReader_impl::cxxDataSeqGetBuffer dataSeqGetBuffer,
    DDS::OpenSplice::FooDataReader_impl::cxxCopyDataOut copyDataOut,
    cxxCopySampleOut copySampleOut)
{
    assert(dataSeqGetBuffer);
    assert(copyDataOut);
    assert(copySampleOut);

    if (os_mutexInit(&this->mtx, NULL) != os_resultSuccess) {
        goto err_mtx;
    }

    if (os_condInit(&this->startCond, &this->mtx, NULL) != os_resultSuccess) {
        goto err_startcond;
    }

    if (os_condInit(&this->readyCond, &this->mtx, NULL) != os_resultSuccess) {
        goto err_readycond;
    }

    this->dataSeqGetBuffer = dataSeqGetBuffer;
    this->copyDataOut   = copyDataOut;
    this->copySampleOut = copySampleOut;

    return DDS::RETCODE_OK;

err_readycond:
    os_condDestroy(&this->startCond);
err_startcond:
    os_mutexDestroy(&this->mtx);
err_mtx:

    return DDS::RETCODE_ERROR;
}

::DDS::ReturnCode_t
parallelDemarshaling::deinit ()
{
    this->stop_workers();

    return DDS::RETCODE_ERROR;
}

::DDS::ReturnCode_t
parallelDemarshaling::start_workers (
    DDS::UShort count)
{
    ::DDS::ReturnCode_t result = DDS::RETCODE_ERROR;
    os_result osr;
    os_threadAttr attr;

    if (count > 1) {
        if (os_mutexLock_s(&this->mtx) == os_resultSuccess) {
            this->terminate = FALSE;

            os_threadAttrInit(&attr);

            this->tids = new os_threadId[count - 1];
            result = DDS::RETCODE_OK;

            do {
                osr = os_threadCreate(
                        &this->tids[this->workerCount],
                        "parDemWorker",
                        &attr,
                        this->workerMain,
                        this);
                if (osr == os_resultSuccess) {
                    this->workerCount++;
                } else {
                    result = DDS::RETCODE_ERROR;
                }
            } while ((result == DDS::RETCODE_OK) && (this->workerCount < (count -1)));

            if (result == DDS::RETCODE_ERROR) {
                delete[] this->tids;
            }
            os_mutexUnlock(&this->mtx);
        }
    } else {
        result = DDS::RETCODE_OK;
    }

    return result;
}

::DDS::ReturnCode_t
parallelDemarshaling::stop_workers ()
{
    DDS::UShort i;
    DDS::UShort workers = 0;

    if (os_mutexLock_s(&this->mtx) == os_resultSuccess) {
        workers = this->workerCount;
        this->terminate = TRUE;
        os_condBroadcast(&this->startCond);
        os_mutexUnlock(&this->mtx);
    }

    for (i = 0; i < workers; i++) {
        (void)os_threadWaitExit(this->tids[i], NULL);
    }

    assert(this->workerCount == 0);
    delete[] this->tids;
    this->tids = NULL;

    return DDS::RETCODE_OK;
}

DDS::UShort
parallelDemarshaling::get_workerCount ()
{
    return this->workerCount;
}

DDS::Boolean
parallelDemarshaling::is_favorable (DDS::ULong length)
{
    DDS::Boolean result = FALSE;

    if ((this->terminate == FALSE) &&
        (length >= this->heuristics.threshold) &&
        (this->workerCount > 0)) {
        result = TRUE;
    }

    return result;
}

DDS::Boolean
parallelDemarshaling::do_copy ()
{
    DDS::Boolean result = TRUE;
    os_uint32 i;
    os_int32  res;
    copyArg_t copyArg;

    assert(this->heuristics.block > 0U);
    assert(this->copyInfo.length);
    assert(this->copyInfo.dataSeq);
    assert(this->copyInfo.infoSeq);
    assert(this->copyInfo.samplesList);
    assert(this->copyInfo.copyDataOut);
    assert(this->copyInfo.getDataBuffer);
    assert(this->copyInfo.uEntity);

    copyArg.copyDataOut = this->copyInfo.copyDataOut;
    copyArg.result = DDS::RETCODE_OK;

    while ((i = pa_inc32_nv(&this->copyInfo.nextIndex) - 1) < this->copyInfo.length) {
        copyArg.dataSample = this->copyInfo.getDataBuffer(this->copyInfo.dataSeq, i);
        copyArg.infoSample = &(this->copyInfo.infoSeq->get_buffer()[i]);

        if (u_readerProtectCopyOutEnter(this->copyInfo.uEntity) == U_RESULT_OK) {
            res = cmn_samplesList_read(this->copyInfo.samplesList, i, this->copySampleOut, &copyArg);
            u_readerProtectCopyOutExit(this->copyInfo.uEntity);
            if ((copyArg.result != DDS::RETCODE_OK) || (res != 1)) {
                result = FALSE;
                break;
            }
        }
    }

    if (os_mutexLock_s(&this->mtx) == os_resultSuccess) {
        this->readyCount++;
        if (this->readyCount > this->workerCount) {
            /* I am the last one to finish, notify workers */
            this->copyInfo.dataSeq = NULL;
            this->copyInfo.infoSeq = NULL;
            this->parity = !this->parity;
            os_condBroadcast(&this->readyCond);
        } else {
            const DDS::Boolean parity = this->parity;
            while(parity == this->parity){
                os_condWait(&this->readyCond, &this->mtx);
            }
        }
        os_mutexUnlock(&this->mtx);
    }

    return result;
}

::DDS::ReturnCode_t
parallelDemarshaling::copy (
    cmn_samplesList samplesList,
    u_entity uEntity,
    void * received_data,
    ::DDS::SampleInfoSeq & info_seq)
{
    DDS::ReturnCode_t result = DDS::RETCODE_ERROR;

    if (os_mutexLock_s(&this->mtx) == os_resultSuccess) {
        this->readyCount = 0;
        this->copyInfo.length = cmn_samplesList_length(samplesList);
        pa_st32(&this->copyInfo.nextIndex, 0U);
        this->copyInfo.dataSeq = received_data;
        this->copyInfo.infoSeq = &info_seq;
        this->copyInfo.samplesList = samplesList;
        this->copyInfo.uEntity = uEntity;
        this->copyInfo.getDataBuffer = this->dataSeqGetBuffer;
        this->copyInfo.copyDataOut = this->copyDataOut;

        os_condBroadcast(&this->startCond);
        os_mutexUnlock(&this->mtx);

        if (this->do_copy()) {
            result = DDS::RETCODE_OK;
        }
    }

    return result;
}

void *
parallelDemarshaling::workerMain (
    void *arg)
{
    parallelDemarshaling * pdc = reinterpret_cast<parallelDemarshaling *>(arg);

    while (1) {
        os_mutexLock(&pdc->mtx);
        while ((pdc->terminate == FALSE) &&
               (pdc->copyInfo.dataSeq == NULL)) {
            os_condWait(&pdc->startCond, &pdc->mtx);
        }
        os_mutexUnlock(&pdc->mtx);

        /* pdc->terminate is guaranteed to only toggle to TRUE in a lock,
         * so reading outside the lock is OK. */
        if (pdc->terminate) break;
        (void) pdc->do_copy();
    }

    os_mutexLock(&pdc->mtx);
    pdc->workerCount--;
    os_mutexUnlock(&pdc->mtx);

    return NULL;
}
