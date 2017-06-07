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
#include "MiscUtils.h"
#include "ReportUtils.h"
#include "Constants.h"


DDS::ReturnCode_t
DDS::OpenSplice::Utils::booleanIsValid(
    const DDS::Boolean value)
{
    DDS::ReturnCode_t result = DDS::RETCODE_BAD_PARAMETER;

    if (value == (DDS::Boolean)TRUE || value == (DDS::Boolean)FALSE) {
        result = DDS::RETCODE_OK;
    } else {
        CPP_REPORT(result, "Boolean invalid");
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::durationIsValid(
    const DDS::Duration_t &duration)
{
    DDS::ReturnCode_t result = DDS::RETCODE_BAD_PARAMETER;

    /* Duration is valid, only when range below 1 billion,
     * or when both fields are equal to DURATION_INFINITE.
     */
    if ((duration.sec == DDS::DURATION_INFINITE_SEC &&
         duration.nanosec == DDS::DURATION_INFINITE_NSEC) ||
        (duration.nanosec < 1000000000ULL) )
    {
        result = DDS::RETCODE_OK;
    } else {
        CPP_REPORT(result, "Duration_t is invalid, seconds '%d', nanoseconds '%d'.",
            duration.sec, duration.nanosec);
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::timeIsValid(
    const DDS::Time_t &time,
    os_int64 maxSupportedSeconds)
{
    DDS::ReturnCode_t result = DDS::RETCODE_BAD_PARAMETER;
    os_int64 sec = time.sec;

    if ((time.sec >= 0) && (C_TIME_NANOS(time.nanosec) < 1000000000ULL)) {
        result = DDS::RETCODE_OK;
    } else if ((time.sec == TIMESTAMP_INVALID_SEC) && (time.nanosec == TIMESTAMP_INVALID_NSEC)) {
        CPP_REPORT(result, "Time_t is invalid");
    } else if (sec > maxSupportedSeconds) {
         result = DDS::RETCODE_BAD_PARAMETER;
         if (sec <= OS_TIME_MAX_VALID_SECONDS) {
             CPP_REPORT(result, "Time value [%" PA_PRId64".%u] is not supported, support for time beyond year 2038 is not enabled",
                        sec, time.nanosec);
         } else {
             CPP_REPORT(result, "Time value [%" PA_PRId64".%u] is not supported the time is too large", sec, time.nanosec);
         }
    } else {
        os_int64 sec = time.sec;
        CPP_REPORT(result, "Time_t is invalid, seconds '%" PA_PRId64"', nanoseconds '%u'", sec, time.nanosec);
    }

    return result;
}

DDS::Boolean
DDS::OpenSplice::Utils::durationIsEqual (
    const DDS::Duration_t &a,
    const DDS::Duration_t &b)
{
    DDS::Boolean equal = TRUE;

    if (a.sec != b.sec || a.nanosec != b.nanosec) {
        equal = FALSE;
    }

    return equal;
}

DDS::Boolean
DDS::OpenSplice::Utils::timeIsEqual (
    const DDS::Time_t &a,
    const DDS::Time_t &b)
{
    DDS::Boolean equal = TRUE;

    if (a.sec != b.sec || a.nanosec != b.nanosec) {
        equal = FALSE;
    }

    return equal;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyDurationIn (
    const DDS::Duration_t &from,
    v_duration &to)
{
    if ( (from.sec     == DDS::DURATION_INFINITE_SEC) &&
         (from.nanosec == DDS::DURATION_INFINITE_NSEC) ) {
         to = C_TIME_INFINITE;
    } else {
        to.seconds     = from.sec;
        to.nanoseconds = from.nanosec;
    }
    return DDS::RETCODE_OK;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyDurationOut (
    const v_duration &from,
    DDS::Duration_t &to)
{
    if ( (from.seconds == C_TIME_INFINITE.seconds) &&
         (from.nanoseconds == C_TIME_INFINITE.nanoseconds) ) {
        to.sec     = DDS::DURATION_INFINITE_SEC;
        to.nanosec = DDS::DURATION_INFINITE_NSEC;
    } else {
        to.sec     = from.seconds;
        to.nanosec = from.nanoseconds;
    }
    return DDS::RETCODE_OK;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyDurationIn(
    const DDS::Duration_t &from,
    os_duration &to)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    if ((from.sec     == DDS::DURATION_INFINITE_SEC) &&
        (from.nanosec == DDS::DURATION_INFINITE_NSEC)) {
        to = OS_DURATION_INFINITE;
    } else if (from.sec >= 0 && from.nanosec < 1000000000) {
        to = OS_DURATION_INIT(from.sec, from.nanosec);
    } else {
        result = DDS::RETCODE_BAD_PARAMETER;
    }
    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyDurationOut (
    const os_duration &from,
    DDS::Duration_t &to)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    assert(!OS_DURATION_ISINVALID(from));

    if (OS_DURATION_ISINFINITE(from)) {
        to.sec     = DDS::DURATION_INFINITE_SEC;
        to.nanosec = DDS::DURATION_INFINITE_NSEC;
    } else {
        assert(OS_DURATION_ISPOSITIVE(from));
        to.sec     = OS_DURATION_GET_SECONDS(from);
        to.nanosec = OS_DURATION_GET_NANOSECONDS(from);
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyTimeIn(
    const DDS::Time_t &from,
    os_timeW &to,
    os_int64 maxSupportedSeconds)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    os_int64 sec = from.sec;

    if ((from.sec == TIMESTAMP_INVALID_SEC) && (from.nanosec == TIMESTAMP_INVALID_NSEC)) {
        to = OS_TIMEW_INVALID;
    } else if (sec > maxSupportedSeconds) {
        result = DDS::RETCODE_BAD_PARAMETER;
        if (sec <= OS_TIME_MAX_VALID_SECONDS) {
            CPP_REPORT(result, "Time value [%" PA_PRId64".%u] is not supported, support for time beyond year 2038 is not enabled",
                      sec, from.nanosec);
        } else {
            CPP_REPORT(result, "Time value [%" PA_PRId64".%u] is not supported the time is too large", sec, from.nanosec);
        }
    } else if ((from.sec >= 0 && from.nanosec < 1000000000)) {
        to = OS_TIMEW_INIT(from.sec, from.nanosec);
    } else {
        result = DDS::RETCODE_BAD_PARAMETER;
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyTimeOut(
    const os_timeW &from,
    DDS::Time_t &to)
{
    assert(!OS_TIMEW_ISINFINITE(from));

    if (OS_TIMEW_ISINVALID(from)) {
        to = DDS::TIMESTAMP_INVALID;
    } else {
        assert(OS_TIMEW_ISNORMALIZED(from));
        to.sec = OS_TIMEW_GET_SECONDS(from);
        to.nanosec = OS_TIMEW_GET_NANOSECONDS(from);
    }

    return DDS::RETCODE_OK;
}

const DDS::Char*
DDS::OpenSplice::Utils::returnCodeToString(
    DDS::ReturnCode_t code)
{
    const DDS::Char* image;

#define _CASE_(code) case code: image = #code; break;
    switch (code) {
    _CASE_(DDS::RETCODE_OK);
    _CASE_(DDS::RETCODE_ERROR);
    _CASE_(DDS::RETCODE_UNSUPPORTED);
    _CASE_(DDS::RETCODE_BAD_PARAMETER);
    _CASE_(DDS::RETCODE_PRECONDITION_NOT_MET);
    _CASE_(DDS::RETCODE_OUT_OF_RESOURCES);
    _CASE_(DDS::RETCODE_NOT_ENABLED);
    _CASE_(DDS::RETCODE_IMMUTABLE_POLICY);
    _CASE_(DDS::RETCODE_INCONSISTENT_POLICY);
    _CASE_(DDS::RETCODE_ALREADY_DELETED);
    _CASE_(DDS::RETCODE_TIMEOUT);
    _CASE_(DDS::RETCODE_NO_DATA);
    _CASE_(DDS::RETCODE_ILLEGAL_OPERATION);
    break;
    default:
        image = "Illegal return code value.";
    break;
    }
#undef _CASE_
    return image;
}

static void
dummy_callback(
    v_public p,
    c_voidp arg)
{
    OS_UNUSED_ARG(p);
    OS_UNUSED_ARG(arg);
}


DDS::ReturnCode_t
DDS::OpenSplice::Utils::observableExists(
    u_observable observable)
{
    DDS::ReturnCode_t result;

    u_result uResult = u_observableAction(observable, &dummy_callback, NULL);
    result = DDS::OpenSplice::CppSuperClass::uResultToReturnCode(uResult);

    return result;
}
