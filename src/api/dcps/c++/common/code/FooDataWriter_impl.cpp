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
#include "sd_cdr.h"
#include "FooDataWriter_impl.h"
#include "MiscUtils.h"
#include "ReportUtils.h"
#include "Constants.h"
#include "ccpp_dds_cdrBlob.h"

typedef struct writerCopyInfo_s {
    DDS::OpenSplice::FooDataWriter_impl *writer;
    void *data;
} writerCopyInfo;


DDS::OpenSplice::FooDataWriter_impl::FooDataWriter_impl() :
        DDS::OpenSplice::DataWriter(),
        copyIn(NULL),
        copyOut(NULL),
        cdrMarshaler(NULL),
        participant(NULL),
        writerCopy(NULL)
{
    /* Empty */
}

DDS::OpenSplice::FooDataWriter_impl::~FooDataWriter_impl()
{
    /* Empty */
}

DDS::ReturnCode_t
DDS::OpenSplice::FooDataWriter_impl::nlReq_init(
    DDS::OpenSplice::Publisher *publisher,
    DDS::OpenSplice::DomainParticipant *participant,
    const DDS::DataWriterQos &qos,
    DDS::OpenSplice::Topic *a_topic,
    const char *name,
    DDS::OpenSplice::cxxCopyIn copyIn,
    DDS::OpenSplice::cxxCopyOut copyOut,
    u_writerCopy writerCopy,
    void *cdrMarshaler)
{
    DDS::ReturnCode_t result;

    assert(copyIn);
    assert(copyOut);

    result = DDS::OpenSplice::DataWriter::nlReq_init(publisher, qos, a_topic, name);
    if (result == DDS::RETCODE_OK) {
        (void) DDS::DomainParticipant::_duplicate(participant);
        this->participant = participant;
        this->copyIn = copyIn;
        this->copyOut = copyOut;
        this->writerCopy = writerCopy;
        this->cdrMarshaler = cdrMarshaler;
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::FooDataWriter_impl::wlReq_deinit()
{
    DDS::ReturnCode_t result;

    result = DDS::OpenSplice::DataWriter::wlReq_deinit();
    if (result == DDS::RETCODE_OK) {
        if (this->participant) {
            DDS::release(this->participant);
            this->participant = NULL;
        }
    }

    return result;
}


/*
 * Alternative function for DDS::OpenSplice::Utils::copyTimeIn, which can also handle
 * the proprietary DDS::TIMESTAMP_CURRENT.
 */
DDS::ReturnCode_t
copyTimeIn (
    const DDS::Time_t &from,
    os_timeW &to,
    os_int64 maxSupportedSeconds)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    if (&from == &DDS::TIMESTAMP_CURRENT) {
        /* Proprietary time extension, used to indicate that current time
         * needs to be used. Kernel translates invalid to current.
         */
        to = OS_TIMEW_INVALID;
    } else if (DDS::OpenSplice::Utils::timeIsValid(from, maxSupportedSeconds) == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyTimeIn(from, to, maxSupportedSeconds);
    } else {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "Bad parameter: supplied invalid time.");
    }
    return result;
}

::DDS::InstanceHandle_t
DDS::OpenSplice::FooDataWriter_impl::register_instance(
    const void * instance_data
) THROW_ORB_EXCEPTIONS
{
    return register_instance_w_timestamp(instance_data, DDS::TIMESTAMP_CURRENT);
}

::DDS::InstanceHandle_t
DDS::OpenSplice::FooDataWriter_impl::register_instance_w_timestamp(
    const void * instance_data,
    const ::DDS::Time_t & source_timestamp
) THROW_ORB_EXCEPTIONS
{
    ::DDS::InstanceHandle_t handle = DDS::HANDLE_NIL;
    DDS::ReturnCode_t result;
    u_writer uWriter;
    u_result uResult;
    os_timeW timestamp;
    writerCopyInfo data;

    CPP_REPORT_STACK();

    assert(instance_data != NULL);
    result = this->check();
    if (result == DDS::RETCODE_OK) {
        uWriter = u_writer(this->rlReq_get_user_entity());
        assert (uWriter != NULL);
        result = copyTimeIn(source_timestamp, timestamp, maxSupportedSeconds);
        if (result == DDS::RETCODE_OK)
        {
            data.writer = this;
            data.data = (void *)instance_data;
            uResult = u_writerRegisterInstance(
                    uWriter,
                    writerCopy,
                    (void *)&data,
                    timestamp,
                    (u_instanceHandle *) &handle);
            result = uResultToReturnCode(uResult);
        }
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return handle;
}

::DDS::ReturnCode_t
DDS::OpenSplice::FooDataWriter_impl::unregister_instance(
    const void * instance_data,
    ::DDS::InstanceHandle_t handle
) THROW_ORB_EXCEPTIONS
{
    ::DDS::ReturnCode_t result;

    CPP_REPORT_STACK();
    result = unregister_instance_w_timestamp(instance_data, handle, DDS::TIMESTAMP_CURRENT);
    CPP_REPORT_FLUSH(this, (result != DDS::RETCODE_OK) && (result != DDS::RETCODE_TIMEOUT));

    return result;
}

::DDS::ReturnCode_t
DDS::OpenSplice::FooDataWriter_impl::unregister_instance_w_timestamp(
    const void * instance_data,
    ::DDS::InstanceHandle_t handle,
    const ::DDS::Time_t & source_timestamp
) THROW_ORB_EXCEPTIONS
{
    ::DDS::ReturnCode_t result;
    u_writer uWriter;
    u_result uResult;
    os_timeW timestamp;
    writerCopyInfo data, *_data;

    CPP_REPORT_STACK();

    assert(instance_data != NULL);
    result = this->check();
    if (result == DDS::RETCODE_OK) {
        uWriter = u_writer(this->rlReq_get_user_entity());
        assert (uWriter != NULL);
        result = copyTimeIn(source_timestamp, timestamp, maxSupportedSeconds);
        if (result == DDS::RETCODE_OK)
        {
            if (instance_data != NULL) {
                data.writer = this;
                data.data = (void *)instance_data;
                _data = &data;
            } else {
                _data = NULL;
            }
            uResult = u_writerUnregisterInstance(
                    uWriter,
                    writerCopy,
                    (void *)_data,
                    timestamp,
                    handle);
            result = uResultToReturnCode(uResult);
        }
    }

    CPP_REPORT_FLUSH(this, (result != DDS::RETCODE_OK) && (result != DDS::RETCODE_TIMEOUT));

    return result;
}

::DDS::ReturnCode_t
DDS::OpenSplice::FooDataWriter_impl::write(
    const void * instance_data,
    ::DDS::InstanceHandle_t handle
) THROW_ORB_EXCEPTIONS
{
    ::DDS::ReturnCode_t result;

    CPP_REPORT_STACK();
    result = write_w_timestamp(instance_data, handle,DDS::TIMESTAMP_CURRENT);
    CPP_REPORT_FLUSH(this, (result != DDS::RETCODE_OK) && (result != DDS::RETCODE_TIMEOUT));

    return result;
}


::DDS::ReturnCode_t
DDS::OpenSplice::FooDataWriter_impl::write_w_timestamp(
    const void * instance_data,
    ::DDS::InstanceHandle_t handle,
    const ::DDS::Time_t & source_timestamp
) THROW_ORB_EXCEPTIONS
{
    ::DDS::ReturnCode_t result;
    u_writer uWriter;
    u_result uResult;
    os_timeW timestamp;
    writerCopyInfo data;

    CPP_REPORT_STACK();

    assert(instance_data != NULL);
    result = this->check();
    if (result == DDS::RETCODE_OK) {
        uWriter = u_writer(this->rlReq_get_user_entity());
        assert (uWriter != NULL);
        result = copyTimeIn(source_timestamp, timestamp, maxSupportedSeconds);
        if (result == DDS::RETCODE_OK)
        {
            data.writer = this;
            data.data = (void *)instance_data;
            uResult = u_writerWrite(
                    uWriter,
                    writerCopy,
                    (void *)&data,
                    timestamp,
                    handle);
            result = uResultToReturnCode(uResult);
        }
    }
    CPP_REPORT_FLUSH(this, (result != DDS::RETCODE_OK) && (result != DDS::RETCODE_TIMEOUT));

    return result;
}

::DDS::ReturnCode_t
DDS::OpenSplice::FooDataWriter_impl::dispose(
    const void * instance_data,
    ::DDS::InstanceHandle_t handle
) THROW_ORB_EXCEPTIONS
{
    ::DDS::ReturnCode_t result;

    CPP_REPORT_STACK();
    result = dispose_w_timestamp(instance_data, handle, DDS::TIMESTAMP_CURRENT);
    CPP_REPORT_FLUSH(this, (result != DDS::RETCODE_OK) && (result != DDS::RETCODE_TIMEOUT));

    return result;
}

::DDS::ReturnCode_t
DDS::OpenSplice::FooDataWriter_impl::dispose_w_timestamp(
    const void * instance_data,
    ::DDS::InstanceHandle_t handle,
    const ::DDS::Time_t & source_timestamp
) THROW_ORB_EXCEPTIONS
{
    ::DDS::ReturnCode_t result;
    u_writer uWriter;
    u_result uResult;
    os_timeW timestamp;
    writerCopyInfo data;

    CPP_REPORT_STACK();

    assert(instance_data != NULL);
    result = this->check();
    if (result == DDS::RETCODE_OK) {
        uWriter = u_writer(this->rlReq_get_user_entity());
        assert (uWriter != NULL);
        result = copyTimeIn(source_timestamp, timestamp, maxSupportedSeconds);
        if (result == DDS::RETCODE_OK)
        {
            data.writer = this;
            data.data = (void *)instance_data;
            uResult = u_writerDispose(
                    uWriter,
                    writerCopy,
                    (void *)&data,
                    timestamp,
                    handle);
            result = uResultToReturnCode(uResult);
        }
    }

    CPP_REPORT_FLUSH(this, (result != DDS::RETCODE_OK) && (result != DDS::RETCODE_TIMEOUT));

    return result;
}

::DDS::ReturnCode_t
DDS::OpenSplice::FooDataWriter_impl::writedispose(
    const void * instance_data,
    ::DDS::InstanceHandle_t handle
) THROW_ORB_EXCEPTIONS
{
    ::DDS::ReturnCode_t result;

    CPP_REPORT_STACK();
    result = writedispose_w_timestamp(instance_data, handle, DDS::TIMESTAMP_CURRENT);
    CPP_REPORT_FLUSH(this, (result != DDS::RETCODE_OK) && (result != DDS::RETCODE_TIMEOUT));

    return result;
}

::DDS::ReturnCode_t
DDS::OpenSplice::FooDataWriter_impl::writedispose_w_timestamp(
    const void * instance_data,
    ::DDS::InstanceHandle_t handle,
    const ::DDS::Time_t & source_timestamp
) THROW_ORB_EXCEPTIONS
{
    ::DDS::ReturnCode_t result;
    u_writer uWriter;
    u_result uResult;
    os_timeW timestamp;
    writerCopyInfo data;

    CPP_REPORT_STACK();

    assert(instance_data != NULL);
    result = this->check();
    if (result == DDS::RETCODE_OK) {
        uWriter = u_writer(this->rlReq_get_user_entity());
        assert (uWriter != NULL);
        result = copyTimeIn(source_timestamp, timestamp, maxSupportedSeconds);
        if (result == DDS::RETCODE_OK)
        {
            data.writer = this;
            data.data = (void *)instance_data;
            uResult = u_writerWriteDispose(
                    uWriter,
                    writerCopy,
                    (void *)&data,
                    timestamp,
                    handle);
            result = uResultToReturnCode(uResult);
        }
    }

    CPP_REPORT_FLUSH(this, (result != DDS::RETCODE_OK) && (result != DDS::RETCODE_TIMEOUT));

    return result;
}

::DDS::ReturnCode_t
DDS::OpenSplice::FooDataWriter_impl::get_key_value(
    void * key_holder,
    ::DDS::InstanceHandle_t handle
) THROW_ORB_EXCEPTIONS
{
    ::DDS::ReturnCode_t result;
    u_writer uWriter;
    u_result uResult;

    CPP_REPORT_STACK();

    assert(key_holder != NULL);
    result = this->check();
    if (result == DDS::RETCODE_OK) {
        uWriter = u_writer(this->rlReq_get_user_entity());
        assert (uWriter != NULL);
        uResult = u_writerCopyKeysFromInstanceHandle(
                uWriter,
                handle,
                this->copyOut,
                key_holder);
        result = uResultToReturnCode(uResult);
    }
    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

::DDS::InstanceHandle_t
DDS::OpenSplice::FooDataWriter_impl::lookup_instance(
    const void * instance_data
) THROW_ORB_EXCEPTIONS
{
    ::DDS::InstanceHandle_t  handle = DDS::HANDLE_NIL;
    DDS::ReturnCode_t result;
    u_writer uWriter;
    writerCopyInfo data;
    u_result uResult;

    CPP_REPORT_STACK();

    assert(instance_data != NULL);
    result = this->check();
    if (result == DDS::RETCODE_OK) {
        uWriter = u_writer(this->rlReq_get_user_entity());
        assert (uWriter != NULL);
        data.writer = this;
        data.data = (void *)instance_data;
        uResult = u_writerLookupInstance(
                uWriter,
                writerCopy,
                (void *)&data,
                (u_instanceHandle *) &handle);
        result = uResultToReturnCode(uResult);
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return handle;
}

v_copyin_result
DDS::OpenSplice::FooDataWriter_impl::rlReq_copyIn (
    c_type type,
    void *data,
    void *to)
{
    v_copyin_result result;
    c_base base = c_getBase(c_object(type));
    writerCopyInfo *info = (writerCopyInfo *)data;

    /* TODO: Add copy cache, see SAC _DataWriterCopy(). */
    result = info->writer->copyIn (base, info->data, to);

    return result;
}

v_copyin_result
DDS::OpenSplice::FooDataWriter_impl::rlReq_cdrCopyIn (
	c_type type,
	void *data,
	void *to)
{
    v_copyin_result result;
    writerCopyInfo *info = (writerCopyInfo *)data;
    DDS::CDRSample *from = (DDS::CDRSample *) info->data;
    int cdrResult;

    OS_UNUSED_ARG(type);

    cdrResult = sd_cdrDeserializeRaw(to, (sd_cdrInfo *) info->writer->cdrMarshaler, from->blob.length(), from->blob.get_buffer());
    if (cdrResult == SD_CDR_OK) {
    	result = V_COPYIN_RESULT_OK;
    } else if (cdrResult == SD_CDR_OUT_OF_MEMORY) {
    	result = V_COPYIN_RESULT_OUT_OF_MEMORY;
    } else {
    	result = V_COPYIN_RESULT_INVALID;
    }

    return result;
}
