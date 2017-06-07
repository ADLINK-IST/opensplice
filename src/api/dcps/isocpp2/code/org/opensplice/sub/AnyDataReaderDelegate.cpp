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


/**
 * @file
 */

#include <dds/sub/AnyDataReader.hpp>
#include <org/opensplice/sub/SubscriberDelegate.hpp>

#include <org/opensplice/sub/QueryDelegate.hpp>
#include <org/opensplice/sub/AnyDataReaderDelegate.hpp>
#include <org/opensplice/core/ReportUtils.hpp>
#include <org/opensplice/core/ScopedLock.hpp>
#include <dds/sub/status/detail/DataStateImpl.hpp>
#include <org/opensplice/topic/BuiltinTopicCopy.hpp>
#include <org/opensplice/core/TimeUtils.hpp>

#include <v_status.h>

#include "cmn_reader.h"

/* My compiler substitutes the v_status() function of the Status delegates
 * with the v_status cast macro of the kernel. This prevents compilation.
 * To solve the issue, undefine the marcro: */
#undef v_status

namespace org
{
namespace opensplice
{
namespace sub
{

struct ReaderCopyInfo {
    const org::opensplice::sub::AnyDataReaderDelegate *helper;
    const void *key;
};


/* For dynamic casting to AnyDataReaderDelegate to work for a few older compilers,
 * it is needed that (at least) the constructor is moved to the cpp file. */
AnyDataReaderDelegate::AnyDataReaderDelegate(
        const dds::sub::qos::DataReaderQos& qos,
        const dds::topic::TopicDescription& td)
    : copyIn(NULL), copyOut(NULL), qos_(qos), td_(td)
{
}

AnyDataReaderDelegate::~AnyDataReaderDelegate()
{
}

const dds::topic::TopicDescription&
AnyDataReaderDelegate::topic_description() const
{
    return this->td_;
}

dds::sub::qos::DataReaderQos
AnyDataReaderDelegate::qos() const
{
    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this);
    org::opensplice::core::ScopedObjectLock scopedLock(*this);
    return qos_;
}


void
AnyDataReaderDelegate::qos(const dds::sub::qos::DataReaderQos& qos)
{
    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this);
    org::opensplice::core::ScopedObjectLock scopedLock(*this);
    qos->check();
    u_readerQos drQos = qos.delegate().u_qos();
    u_result uResult = u_dataReaderSetQos((u_dataReader)(this->userHandle), drQos);
    u_readerQosFree(drQos);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "Could not set reader qos.");
    this->qos_ = qos;
}

void
AnyDataReaderDelegate::wait_for_historical_data(const dds::core::Duration& timeout)
{
    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this);
    this->check();
    os_duration uTimeout = org::opensplice::core::timeUtils::convertDuration(timeout);
    u_result uResult = u_dataReaderWaitForHistoricalData((u_dataReader)(this->userHandle), uTimeout);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_dataReaderWaitForHistoricalData failed.");
}

void
AnyDataReaderDelegate::wait_for_historical_data_w_condition (
    const std::string& filter_expression,
    const std::vector<std::string>& filter_parameters,
    const dds::core::Time& min_source_timestamp,
    const dds::core::Time& max_source_timestamp,
    const dds::core::policy::ResourceLimits& resource_limits,
    const dds::core::Duration& timeout)
{
    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this);

    u_result uResult;
    const os_char *filter = NULL;
    os_timeW uMinTimestamp = org::opensplice::core::timeUtils::convertTime(min_source_timestamp, maxSupportedSeconds_);
    os_timeW uMaxTimestamp = org::opensplice::core::timeUtils::convertTime(max_source_timestamp, maxSupportedSeconds_);
    os_duration uTimeout = org::opensplice::core::timeUtils::convertDuration(timeout);

    if (filter_expression.length() != 0) {
        filter = filter_expression.c_str();
    }

    if (filter_parameters.empty()) {
        uResult = u_dataReaderWaitForHistoricalDataWithCondition(
                                        (u_dataReader)(this->userHandle),
                                        filter,
                                        NULL,
                                        0,
                                        uMinTimestamp,
                                        uMaxTimestamp,
                                        resource_limits.max_samples(),
                                        resource_limits.max_instances(),
                                        resource_limits.max_samples_per_instance(),
                                        uTimeout);
    } else {
        os_uint32 psize = filter_parameters.size();
        const char **plist = new const char *[psize];

        int i;
        std::vector<std::string>::const_iterator it;
        for (i = 0, it = filter_parameters.begin(); it != filter_parameters.end(); ++it, ++i) {
            plist[i] = (*it).c_str();
        }

        uResult = u_dataReaderWaitForHistoricalDataWithCondition(
                                        (u_dataReader)(this->userHandle),
                                        filter,
                                        plist,
                                        psize,
                                        uMinTimestamp,
                                        uMaxTimestamp,
                                        resource_limits.max_samples(),
                                        resource_limits.max_instances(),
                                        resource_limits.max_samples_per_instance(),
                                        uTimeout);

        delete[] plist;

    }
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_dataReaderWaitForHistoricalDataWithCondition failed.");
}


void
AnyDataReaderDelegate::reset_data_available()
{
    u_result uResult;

    /* TODO: This is a bit tricky, the entity may already been deleted and in that case
    * this operation will perform a dirty memory read.
    * It may be better to wipe all pending events belonging to an entity when it is deleted or
    * if that is too intrusive find another way to safely detect/avoid deletion.
    * NOTE: This was copied from SAC.
    */
    uResult = u_observableAction(
                        u_observable(this->userHandle),
                        AnyDataReaderDelegate::reset_data_available_callback,
                        NULL);

    if (uResult != U_RESULT_OK) {
        ISOCPP_REPORT_WARNING("Could not reset data available status.");
    }
}

void
AnyDataReaderDelegate::reset_data_available_callback(
   v_public p,
   c_voidp arg)
{
    OS_UNUSED_ARG(arg);

    v_statusReset(v_entity(p)->status, V_EVENT_DATA_AVAILABLE);
}

u_sampleMask
AnyDataReaderDelegate::getUserMask(const dds::sub::status::DataState& state)
{
    u_sampleMask mask = 0;
    const unsigned long sampleMask   = 0x3;
    const unsigned long viewMask     = 0x3;
    const unsigned long instanceMask = 0x7;

    unsigned long uSampleState   = state.sample_state().to_ulong() & sampleMask;
    unsigned long uViewState     = state.view_state().to_ulong() & viewMask;
    unsigned long uInstanceState = state.instance_state().to_ulong() & instanceMask;

    mask = uSampleState | (uViewState << 2) | (uInstanceState << 4);

    return mask;
}


void
AnyDataReaderDelegate::read(
    const u_dataReader reader,
    const dds::sub::status::DataState& mask,
    dds::sub::detail::SamplesHolder& samples,
    uint32_t max_samples)
{
    cmn_samplesList cmnSampleList = cmn_samplesList_new((os_boolean)0);
    cmn_samplesList_reset(cmnSampleList, max_samples);

    u_sampleMask uMask = getUserMask(mask);

    u_result uResult = u_dataReaderRead(reader, uMask, cmn_reader_action, cmnSampleList, OS_DURATION_ZERO);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_dataReaderRead failed.");

    uint32_t length = (uint32_t) cmn_samplesList_length(cmnSampleList);
    if (length > 0) {
        samples.set_length(length);

        uResult = u_readerProtectCopyOutEnter(u_entity(reader));
        ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_dataReaderRead failed.");

        FlushActionArguments flushArgs = {*this, samples};
        int32_t testlength = cmn_samplesList_flush(cmnSampleList, flush_action, &flushArgs);
        u_readerProtectCopyOutExit(u_entity(reader));

        if (testlength < 0) {
            ISOCPP_U_RESULT_CHECK_AND_THROW(U_RESULT_ALREADY_DELETED, "u_dataReaderRead failed.");
        }
        assert(length == (uint32_t)testlength);
    }
    cmn_samplesList_free(cmnSampleList);
}

void
AnyDataReaderDelegate::take(
    const u_dataReader reader,
    const dds::sub::status::DataState& mask,
    dds::sub::detail::SamplesHolder& samples,
    uint32_t max_samples)
{
    cmn_samplesList cmnSampleList = cmn_samplesList_new((os_boolean)0);
    cmn_samplesList_reset(cmnSampleList, max_samples);

    u_sampleMask uMask = getUserMask(mask);

    u_result uResult = u_dataReaderTake(reader, uMask, cmn_reader_action, cmnSampleList, OS_DURATION_ZERO);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_dataReaderTake failed.");

    uint32_t length = (uint32_t) cmn_samplesList_length(cmnSampleList);
    if (length > 0) {
        samples.set_length(length);

        uResult = u_readerProtectCopyOutEnter(u_entity(reader));
        ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_dataReaderTake failed.");

        FlushActionArguments flushArgs = {*this, samples};
        int32_t testlength = cmn_samplesList_flush(cmnSampleList, flush_action, &flushArgs);
        u_readerProtectCopyOutExit(u_entity(reader));

        if (testlength < 0) {
            ISOCPP_U_RESULT_CHECK_AND_THROW(U_RESULT_ALREADY_DELETED, "u_dataReaderTake failed.");
        }
        assert(length == (uint32_t)testlength);
    }
    cmn_samplesList_free(cmnSampleList);
}


void
AnyDataReaderDelegate::read_instance(
    const u_dataReader reader,
    const dds::core::InstanceHandle& handle,
    const dds::sub::status::DataState& mask,
    dds::sub::detail::SamplesHolder& samples,
    uint32_t max_samples)
{
    cmn_samplesList cmnSampleList = cmn_samplesList_new((os_boolean)0);
    cmn_samplesList_reset(cmnSampleList, max_samples);

    u_sampleMask uMask = getUserMask(mask);

    u_result uResult = u_dataReaderReadInstance(
                           reader, handle.delegate().handle(), uMask,
                           cmn_reader_action, cmnSampleList, OS_DURATION_ZERO);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_dataReaderReadInstance failed.");

    uint32_t length = (uint32_t) cmn_samplesList_length(cmnSampleList);
    if (length > 0) {
        samples.set_length(length);

        uResult = u_readerProtectCopyOutEnter(u_entity(reader));
        ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_dataReaderReadInstance failed.");

        FlushActionArguments flushArgs = {*this, samples};
        int32_t testlength = cmn_samplesList_flush(cmnSampleList, flush_action, &flushArgs);
        u_readerProtectCopyOutExit(u_entity(reader));

        if (testlength < 0) {
            ISOCPP_U_RESULT_CHECK_AND_THROW(U_RESULT_ALREADY_DELETED, "u_dataReaderReadInstance failed.");
        }
        assert(length == (uint32_t)testlength);
    }
    cmn_samplesList_free(cmnSampleList);
}

void
AnyDataReaderDelegate::take_instance(
    const u_dataReader reader,
    const dds::core::InstanceHandle& handle,
    const dds::sub::status::DataState& mask,
    dds::sub::detail::SamplesHolder& samples,
    uint32_t max_samples)
{
    cmn_samplesList cmnSampleList = cmn_samplesList_new((os_boolean)0);
    cmn_samplesList_reset(cmnSampleList, max_samples);

    u_sampleMask uMask = getUserMask(mask);

    u_result uResult = u_dataReaderTakeInstance(
                           reader, handle.delegate().handle(), uMask,
                           cmn_reader_action, cmnSampleList, OS_DURATION_ZERO);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_dataReaderTakeInstance failed.");

    uint32_t length = (uint32_t) cmn_samplesList_length(cmnSampleList);
    if (length > 0) {
        samples.set_length(length);

        uResult = u_readerProtectCopyOutEnter(u_entity(reader));
        ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_dataReaderTakeInstance failed.");

        FlushActionArguments flushArgs = {*this, samples};
        int32_t testlength = cmn_samplesList_flush(cmnSampleList, flush_action, &flushArgs);
        u_readerProtectCopyOutExit(u_entity(reader));

        if (testlength < 0) {
            ISOCPP_U_RESULT_CHECK_AND_THROW(U_RESULT_ALREADY_DELETED, "u_dataReaderTakeInstance failed.");
        }
        assert(length == (uint32_t)testlength);
    }
    cmn_samplesList_free(cmnSampleList);
}

void
AnyDataReaderDelegate::read_next_instance(
    const u_dataReader reader,
    const dds::core::InstanceHandle& handle,
    const dds::sub::status::DataState& mask,
    dds::sub::detail::SamplesHolder& samples,
    uint32_t max_samples)
{
    cmn_samplesList cmnSampleList = cmn_samplesList_new((os_boolean)0);
    cmn_samplesList_reset(cmnSampleList, max_samples);

    u_sampleMask uMask = getUserMask(mask);

    u_result uResult = u_dataReaderReadNextInstance(
                           reader, handle.delegate().handle(), uMask,
                           cmn_reader_nextInstanceAction, cmnSampleList, OS_DURATION_ZERO);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_dataReaderReadNextInstance failed.");

    uint32_t length = (uint32_t) cmn_samplesList_length(cmnSampleList);
    if (length > 0) {
        samples.set_length(length);

        uResult = u_readerProtectCopyOutEnter(u_entity(reader));
        ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_dataReaderReadNextInstance failed.");

        FlushActionArguments flushArgs = {*this, samples};
        int32_t testlength = cmn_samplesList_flush(cmnSampleList, flush_action, &flushArgs);
        u_readerProtectCopyOutExit(u_entity(reader));

        if (testlength < 0) {
            ISOCPP_U_RESULT_CHECK_AND_THROW(U_RESULT_ALREADY_DELETED, "u_dataReaderReadNextInstance failed.");
        }
        assert(length == (uint32_t)testlength);
    }
    cmn_samplesList_free(cmnSampleList);
}

void
AnyDataReaderDelegate::take_next_instance(
    const u_dataReader reader,
    const dds::core::InstanceHandle& handle,
    const dds::sub::status::DataState& mask,
    dds::sub::detail::SamplesHolder& samples,
    uint32_t max_samples)
{
    cmn_samplesList cmnSampleList = cmn_samplesList_new((os_boolean)0);
    cmn_samplesList_reset(cmnSampleList, max_samples);

    u_sampleMask uMask = getUserMask(mask);

    u_result uResult = u_dataReaderTakeNextInstance(
                           reader, handle.delegate().handle(), uMask,
                           cmn_reader_nextInstanceAction, cmnSampleList, OS_DURATION_ZERO);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_dataReaderTakeNextInstance failed.");

    uint32_t length = (uint32_t) cmn_samplesList_length(cmnSampleList);
    if (length > 0) {
        samples.set_length(length);

        uResult = u_readerProtectCopyOutEnter(u_entity(reader));
        ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_dataReaderTakeNextInstance failed.");

        FlushActionArguments flushArgs = {*this, samples};
        int32_t testlength = cmn_samplesList_flush(cmnSampleList, flush_action, &flushArgs);
        u_readerProtectCopyOutExit(u_entity(reader));

        if (testlength < 0) {
            ISOCPP_U_RESULT_CHECK_AND_THROW(U_RESULT_ALREADY_DELETED, "u_dataReaderTakeNextInstance failed.");
        }
        assert(length == (uint32_t)testlength);
    }
    cmn_samplesList_free(cmnSampleList);
}



void
AnyDataReaderDelegate::read_w_condition(
    const u_query query,
    dds::sub::detail::SamplesHolder& samples,
    uint32_t max_samples)
{
    cmn_samplesList cmnSampleList = cmn_samplesList_new((os_boolean)0);
    cmn_samplesList_reset(cmnSampleList, max_samples);

    u_result uResult = u_queryRead(query, cmn_reader_action, cmnSampleList, OS_DURATION_ZERO);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_queryRead failed.");

    uint32_t length = (uint32_t) cmn_samplesList_length(cmnSampleList);
    if (length > 0) {
        samples.set_length(length);

        uResult = u_readerProtectCopyOutEnter(u_entity(query));
        ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_queryRead failed.");

        FlushActionArguments flushArgs = {*this, samples};
        int32_t testlength = cmn_samplesList_flush(cmnSampleList, flush_action, &flushArgs);
        u_readerProtectCopyOutExit(u_entity(query));

        if (testlength < 0) {
            ISOCPP_U_RESULT_CHECK_AND_THROW(U_RESULT_ALREADY_DELETED, "u_queryRead failed.");
        }
        assert(length == (uint32_t)testlength);
    }
    cmn_samplesList_free(cmnSampleList);
}

void
AnyDataReaderDelegate::take_w_condition(
    const u_query query,
    dds::sub::detail::SamplesHolder& samples,
    uint32_t max_samples)
{
    cmn_samplesList cmnSampleList = cmn_samplesList_new((os_boolean)0);
    cmn_samplesList_reset(cmnSampleList, max_samples);

    u_result uResult = u_queryTake(query, cmn_reader_action, cmnSampleList, OS_DURATION_ZERO);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_queryTake failed.");

    uint32_t length = (uint32_t) cmn_samplesList_length(cmnSampleList);
    if (length > 0) {
        samples.set_length(length);

        uResult = u_readerProtectCopyOutEnter(u_entity(query));
        ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_queryTake failed.");

        FlushActionArguments flushArgs = {*this, samples};
        int32_t testlength = cmn_samplesList_flush(cmnSampleList, flush_action, &flushArgs);
        u_readerProtectCopyOutExit(u_entity(query));

        if (testlength < 0) {
            ISOCPP_U_RESULT_CHECK_AND_THROW(U_RESULT_ALREADY_DELETED, "u_queryTake failed.");
        }
        assert(length == (uint32_t)testlength);
    }
    cmn_samplesList_free(cmnSampleList);
}

void
AnyDataReaderDelegate::read_instance_w_condition(
    const u_query query,
    const dds::core::InstanceHandle& handle,
    dds::sub::detail::SamplesHolder& samples,
    uint32_t max_samples)
{
    cmn_samplesList cmnSampleList = cmn_samplesList_new((os_boolean)0);
    cmn_samplesList_reset(cmnSampleList, max_samples);

    u_result uResult = u_queryReadInstance(query, handle.delegate().handle(), cmn_reader_action, cmnSampleList, OS_DURATION_ZERO);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_queryReadInstance failed.");

    uint32_t length = (uint32_t) cmn_samplesList_length(cmnSampleList);
    if (length > 0) {
        samples.set_length(length);

        uResult = u_readerProtectCopyOutEnter(u_entity(query));
        ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_queryReadInstance failed.");

        FlushActionArguments flushArgs = {*this, samples};
        int32_t testlength = cmn_samplesList_flush(cmnSampleList, flush_action, &flushArgs);
        u_readerProtectCopyOutExit(u_entity(query));

        if (testlength < 0) {
            ISOCPP_U_RESULT_CHECK_AND_THROW(U_RESULT_ALREADY_DELETED, "u_queryReadInstance failed.");
        }
        assert(length == (uint32_t)testlength);
    }
    cmn_samplesList_free(cmnSampleList);
}

void
AnyDataReaderDelegate::take_instance_w_condition(
    const u_query query,
    const dds::core::InstanceHandle& handle,
    dds::sub::detail::SamplesHolder& samples,
    uint32_t max_samples)
{
    cmn_samplesList cmnSampleList = cmn_samplesList_new((os_boolean)0);
    cmn_samplesList_reset(cmnSampleList, max_samples);

    u_result uResult = u_queryTakeInstance(query, handle.delegate().handle(), cmn_reader_action, cmnSampleList, OS_DURATION_ZERO);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_queryTakeInstance failed.");

    uint32_t length = (uint32_t) cmn_samplesList_length(cmnSampleList);
    if (length > 0) {
        samples.set_length(length);

        uResult = u_readerProtectCopyOutEnter(u_entity(query));
        ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_queryTakeInstance failed.");

        FlushActionArguments flushArgs = {*this, samples};
        int32_t testlength = cmn_samplesList_flush(cmnSampleList, flush_action, &flushArgs);
        u_readerProtectCopyOutExit(u_entity(query));

        if (testlength < 0) {
            ISOCPP_U_RESULT_CHECK_AND_THROW(U_RESULT_ALREADY_DELETED, "u_queryTakeInstance failed.");
        }
        assert(length == (uint32_t)testlength);
    }
    cmn_samplesList_free(cmnSampleList);
}


void
AnyDataReaderDelegate::read_next_instance_w_condition(
    const u_query query,
    const dds::core::InstanceHandle& handle,
    dds::sub::detail::SamplesHolder& samples,
    uint32_t max_samples)
{
    cmn_samplesList cmnSampleList = cmn_samplesList_new((os_boolean)0);
    cmn_samplesList_reset(cmnSampleList, max_samples);

    u_result uResult = u_queryReadNextInstance(query, handle.delegate().handle(), cmn_reader_action, cmnSampleList, OS_DURATION_ZERO);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_queryReadNextInstance failed.");

    uint32_t length = (uint32_t) cmn_samplesList_length(cmnSampleList);
    if (length > 0) {
        samples.set_length(length);

        uResult = u_readerProtectCopyOutEnter(u_entity(query));
        ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_queryReadNextInstance failed.");

        FlushActionArguments flushArgs = {*this, samples};
        int32_t testlength = cmn_samplesList_flush(cmnSampleList, flush_action, &flushArgs);
        u_readerProtectCopyOutExit(u_entity(query));

        if (testlength < 0) {
            ISOCPP_U_RESULT_CHECK_AND_THROW(U_RESULT_ALREADY_DELETED, "u_queryReadNextInstance failed.");
        }
        assert(length == (uint32_t)testlength);
    }
    cmn_samplesList_free(cmnSampleList);
}

void
AnyDataReaderDelegate::take_next_instance_w_condition(
    const u_query query,
    const dds::core::InstanceHandle& handle,
    dds::sub::detail::SamplesHolder& samples,
    uint32_t max_samples)
{
    cmn_samplesList cmnSampleList = cmn_samplesList_new((os_boolean)0);
    cmn_samplesList_reset(cmnSampleList, max_samples);

    u_result uResult = u_queryTakeNextInstance(query, handle.delegate().handle(), cmn_reader_action, cmnSampleList, OS_DURATION_ZERO);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_queryTakeNextInstance failed.");

    uint32_t length = (uint32_t) cmn_samplesList_length(cmnSampleList);
    if (length > 0) {
        samples.set_length(length);

        uResult = u_readerProtectCopyOutEnter(u_entity(query));
        ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_queryTakeNextInstance failed.");

        FlushActionArguments flushArgs = {*this, samples};
        int32_t testlength = cmn_samplesList_flush(cmnSampleList, flush_action, &flushArgs);
        u_readerProtectCopyOutExit(u_entity(query));

        if (testlength < 0) {
            ISOCPP_U_RESULT_CHECK_AND_THROW(U_RESULT_ALREADY_DELETED, "u_queryTakeNextInstance failed.");
        }
        assert(length == (uint32_t)testlength);
    }
    cmn_samplesList_free(cmnSampleList);
}







void
AnyDataReaderDelegate::get_key_value(
    const u_dataReader reader,
    const dds::core::InstanceHandle& handle, void *key)
{
    u_result uResult = u_dataReaderCopyKeysFromInstanceHandle(
                           reader, handle.delegate().handle(), copyOut, key);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_dataReaderCopyKeysFromInstanceHandle failed.");
}

u_instanceHandle
AnyDataReaderDelegate::lookup_instance(
     const u_dataReader reader,
     const void *key) const
{
    u_instanceHandle handle = U_INSTANCEHANDLE_NIL;
    ReaderCopyInfo keyInfo;

    keyInfo.helper = this;
    keyInfo.key = key;

    u_result uResult = u_dataReaderLookupInstance(reader, (void *)&keyInfo, (u_copyIn)copy_key, &handle);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_dataReaderLookupInstance failed.");

    return handle;
}

static v_result
copy_liveliness_changed_status(
    c_voidp info,
    c_voidp arg)
{
    struct v_livelinessChangedInfo *from = (struct v_livelinessChangedInfo *)info;
    ::dds::core::status::LivelinessChangedStatus *to = static_cast< ::dds::core::status::LivelinessChangedStatus * >(arg);
    to->delegate().v_status(*from);
    return V_RESULT_OK;
}


dds::core::status::LivelinessChangedStatus
AnyDataReaderDelegate::liveliness_changed_status()
{
    dds::core::status::LivelinessChangedStatus status;
    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this);
    org::opensplice::core::ScopedObjectLock scopedLock(*this);
    u_result uResult = u_readerGetLivelinessChangedStatus(
                                    (u_reader)(this->userHandle),
                                    TRUE,
                                    copy_liveliness_changed_status,
                                    &status);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_dataReaderGetLivelinessChangedStatus failed.");
    return status;
}

static v_result
copy_sample_rejected_status(
    c_voidp info,
    c_voidp arg)
{
    struct v_sampleRejectedInfo *from = (struct v_sampleRejectedInfo *)info;
    ::dds::core::status::SampleRejectedStatus *to = static_cast< ::dds::core::status::SampleRejectedStatus * >(arg);
    to->delegate().v_status(*from);
    return V_RESULT_OK;
}

dds::core::status::SampleRejectedStatus
AnyDataReaderDelegate::sample_rejected_status()
{
    dds::core::status::SampleRejectedStatus status;
    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this);
    org::opensplice::core::ScopedObjectLock scopedLock(*this);
    u_result uResult = u_readerGetSampleRejectedStatus(
                                    (u_reader)(this->userHandle),
                                    TRUE,
                                    copy_sample_rejected_status,
                                    &status);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_readerGetSampleRejectedStatus failed.");
    return status;
}

static v_result
copy_sample_lost_status(
    c_voidp info,
    c_voidp arg)
{
    struct v_sampleLostInfo *from = (struct v_sampleLostInfo *)info;
    ::dds::core::status::SampleLostStatus *to = static_cast< ::dds::core::status::SampleLostStatus * >(arg);
    to->delegate().v_status(*from);
    return V_RESULT_OK;
}

dds::core::status::SampleLostStatus
AnyDataReaderDelegate::sample_lost_status()
{
    dds::core::status::SampleLostStatus status;
    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this);
    org::opensplice::core::ScopedObjectLock scopedLock(*this);
    u_result uResult = u_readerGetSampleLostStatus(
                                    (u_reader)(this->userHandle),
                                    TRUE,
                                    copy_sample_lost_status,
                                    &status);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_readerGetSampleLostStatus failed.");
    return status;
}

static v_result
copy_requested_deadline_missed_status(
    c_voidp info,
    c_voidp arg)
{
    v_result result = V_RESULT_OK;
    struct v_deadlineMissedInfo *from = (struct v_deadlineMissedInfo *)info;
    ::dds::core::status::RequestedDeadlineMissedStatus *to = static_cast< ::dds::core::status::RequestedDeadlineMissedStatus * >(arg);
    try {
        to->delegate().v_status(*from);
    } catch (dds::core::Error&) {
        result = V_RESULT_INTERNAL_ERROR;
    }
    return result;
}

dds::core::status::RequestedDeadlineMissedStatus
AnyDataReaderDelegate::requested_deadline_missed_status()
{
    dds::core::status::RequestedDeadlineMissedStatus status;
    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this);
    org::opensplice::core::ScopedObjectLock scopedLock(*this);
    u_result uResult = u_readerGetDeadlineMissedStatus(
                                    (u_reader)(this->userHandle),
                                    TRUE,
                                    copy_requested_deadline_missed_status,
                                    &status);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_readerGetDeadlineMissedStatus failed.");
    return status;
}

static v_result
copy_requested_incompatible_qos_status(
    c_voidp info,
    c_voidp arg)
{
    v_result result = V_RESULT_OK;
    struct v_incompatibleQosInfo *from = (struct v_incompatibleQosInfo *)info;
    ::dds::core::status::RequestedIncompatibleQosStatus *to = static_cast< ::dds::core::status::RequestedIncompatibleQosStatus * >(arg);
    try {
        to->delegate().v_status(*from);
    } catch (dds::core::Error&) {
        result = V_RESULT_INTERNAL_ERROR;
    }
    return result;
}

dds::core::status::RequestedIncompatibleQosStatus
AnyDataReaderDelegate::requested_incompatible_qos_status()
{
    dds::core::status::RequestedIncompatibleQosStatus status;
    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this);
    org::opensplice::core::ScopedObjectLock scopedLock(*this);
    u_result uResult = u_readerGetIncompatibleQosStatus(
                                    (u_reader)(this->userHandle),
                                    TRUE,
                                    copy_requested_incompatible_qos_status,
                                    &status);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_readerGetIncompatibleQosStatus failed.");
    return status;
}

static v_result
copy_subscription_matched_status(
    c_voidp info,
    c_voidp arg)
{
    struct v_topicMatchInfo *from = (struct v_topicMatchInfo *)info;
    ::dds::core::status::SubscriptionMatchedStatus *to = static_cast< ::dds::core::status::SubscriptionMatchedStatus * >(arg);
    to->delegate().v_status(*from);
    return V_RESULT_OK;
}

dds::core::status::SubscriptionMatchedStatus
AnyDataReaderDelegate::subscription_matched_status()
{
    dds::core::status::SubscriptionMatchedStatus status;
    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this);
    org::opensplice::core::ScopedObjectLock scopedLock(*this);
    u_result uResult = u_readerGetSubscriptionMatchStatus(
                                    (u_reader)(this->userHandle),
                                    TRUE,
                                    copy_subscription_matched_status,
                                    &status);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_readerGetSubscriptionMatchStatus failed.");
    return status;
}

static v_result
copy_matched_publication(
    u_publicationInfo *info,
    void *arg)
{
    dds::core::InstanceHandleSeq *seq  = reinterpret_cast<dds::core::InstanceHandleSeq *>(arg);
    ::dds::core::InstanceHandle handle = u_instanceHandleFromGID(info->key);
    seq->push_back(handle);
    return V_RESULT_OK;
}

::dds::core::InstanceHandleSeq
AnyDataReaderDelegate::matched_publications()
{
    ::dds::core::InstanceHandleSeq handleSeq;
    u_result uResult = u_readerGetMatchedPublications(
                                (u_reader)(this->userHandle),
                                copy_matched_publication,
                                &handleSeq);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_readerGetMatchedPublications failed.");
    return handleSeq;
}

static v_result
copy_matched_publication_data(
    u_publicationInfo *info,
    void *arg)
{
    dds::topic::PublicationBuiltinTopicData *to = reinterpret_cast<dds::topic::PublicationBuiltinTopicData *>(arg);
    __PublicationBuiltinTopicData__copyOut(info, to);
    return V_RESULT_OK;
}

const dds::topic::PublicationBuiltinTopicData
AnyDataReaderDelegate::matched_publication_data(const ::dds::core::InstanceHandle& h)
{
    dds::topic::PublicationBuiltinTopicData dataSample;
    u_result uResult = u_readerGetMatchedPublicationData(
                                (u_reader)(this->userHandle),
                                h->handle(),
                                copy_matched_publication_data,
                                &dataSample);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_readerGetMatchedPublicationData failed.");
    return dataSample;
}

void
AnyDataReaderDelegate::close()
{
    this->queries.all_close();

    org::opensplice::core::EntityDelegate::close();
}

void
AnyDataReaderDelegate::add_query(
    org::opensplice::sub::QueryDelegate& query)
{
    this->queries.insert(query);
}

void
AnyDataReaderDelegate::remove_query(
    org::opensplice::sub::QueryDelegate& query)
{
    this->queries.erase(query);
}




void
AnyDataReaderDelegate::copy_sample_info(
    cmn_sampleInfo from,
    dds::sub::SampleInfo *to)
{
    org::opensplice::sub::SampleInfoImpl& info = to->delegate();

    info.timestamp(dds::core::Time(OS_TIMEW_GET_SECONDS(from->source_timestamp),
            OS_TIMEW_GET_NANOSECONDS(from->source_timestamp)));

    dds::sub::status::SampleState ss(from->sample_state);
    dds::sub::status::ViewState vs(from->view_state);
    dds::sub::status::InstanceState is(from->instance_state);
    info.state(dds::sub::status::DataState(ss, vs, is));
    info.generation_count().delegate() = org::opensplice::sub::GenerationCountImpl(from->disposed_generation_count, from->no_writers_generation_count);
    info.rank().delegate() = org::opensplice::sub::RankImpl(from->sample_rank, from->generation_rank, from->absolute_generation_rank);
    info.valid(from->valid_data);
    dds::core::InstanceHandle ih(from->instance_handle);
    info.instance_handle(ih);
    dds::core::InstanceHandle ph(from->publication_handle);
    info.publication_handle(ph);
}


void
AnyDataReaderDelegate::flush_action(
    void *sample,
    cmn_sampleInfo sampleInfo,
    void *args)
{
    FlushActionArguments *flushArgs = (FlushActionArguments *) args;

    void *data = flushArgs->samples.data();
    dds::sub::SampleInfo* info = flushArgs->samples.info();

    flushArgs->reader.copyOut(sample, data);
    copy_sample_info(sampleInfo, info);

    flushArgs->samples++;
}

v_copyin_result
AnyDataReaderDelegate::copy_key(c_type t, const void *data, void *to)
{
    c_base base = c_getBase(c_object(t));
    struct ReaderCopyInfo *info = (struct ReaderCopyInfo *)data;

    return info->helper->getCopyIn()(base, info->key, to);
}

dds::sub::TAnyDataReader<AnyDataReaderDelegate>
AnyDataReaderDelegate::wrapper_to_any()
{
    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this);
    AnyDataReaderDelegate::ref_type ref =
            OSPL_CXX11_STD_MODULE::dynamic_pointer_cast<AnyDataReaderDelegate>(this->get_strong_ref());
    dds::sub::AnyDataReader any_reader(ref);
    return any_reader;
}




}
}
}
