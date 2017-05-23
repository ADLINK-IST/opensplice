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

os_char *
os_scopeAttrImage(
    os_scopeAttr _this)
{
    os_char *image;
    switch (_this) {
    case OS_SCOPE_SHARED      : image = "OS_SCOPE_SHARED";      break;
    case OS_SCOPE_PRIVATE     : image = "OS_SCOPE_PRIVATE";     break;
    default                   : image = "<undefined value>";    break;
    }
    return image;
}

os_char *
os_lockPolicyImage(
    os_lockPolicy _this)
{
    os_char *image;
    switch (_this) {
    case OS_LOCK_DEFAULT      : image = "OS_LOCK_DEFAULT";      break;
    case OS_LOCKED            : image = "OS_LOCKED";            break;
    case OS_UNLOCKED          : image = "OS_UNLOCKED";          break;
    default                   : image = "<undefined value>";    break;
    }
    return image;
}

os_char *
os_schedClassImage(
    os_schedClass _this)
{
    os_char *image;
    switch (_this) {
    case OS_SCHED_DEFAULT     : image = "OS_SCHED_DEFAULT";     break;
    case OS_SCHED_REALTIME    : image = "OS_SCHED_REALTIME";    break;
    case OS_SCHED_TIMESHARE   : image = "OS_SCHED_TIMESHARE";   break;
    default                   : image = "<undefined value>";    break;
    }
    return image;
}

os_char *
os_compareImage(
    os_compare _this)
{
    os_char *image;
    switch (_this) {
    case OS_LESS              : image = "OS_LESS";              break;
    case OS_EQUAL             : image = "OS_EQUAL";             break;
    case OS_MORE              : image = "OS_MORE";              break;
    default                   : image = "<undefined value>";    break;
    }
    return image;
}

os_char *
os_resultImage(
    os_result _this)
{
    os_char *image;
    switch (_this) {
    case os_resultSuccess     : image = "os_resultSuccess";     break;
    case os_resultUnavailable : image = "os_resultUnavailable"; break;
    case os_resultTimeout     : image = "os_resultTimeout";     break;
    case os_resultBusy        : image = "os_resultBusy";        break;
    case os_resultInvalid     : image = "os_resultInvalid";     break;
    case os_resultFail        : image = "os_resultFail";        break;
    default                   : image = "<undefined value>";    break;
    }
    return image;
}

os_char *
os_booleanImage(
    os_boolean _this)
{
    os_char *image;
    switch (_this) {
    case OS_FALSE             : image = "OS_FALSE";             break;
    case OS_TRUE              : image = "OS_TRUE";              break;
    default                   : image = "<undefined value>";    break;
    }
    return image;
}

union ptr_to_fptr {
    os_fptr fptr;
    void* ptr;
};

os_fptr
os_fptr(
    void* ptr)
{
    union ptr_to_fptr u;
    u.ptr = ptr;
    return u.fptr;
}

os_int
os_resultToReturnCode(
    os_result result)
{
    os_int code = OS_RETCODE_ERROR;

    switch (result) {
        case os_resultSuccess:
            code = OS_RETCODE_OK;
            break;
        case os_resultUnavailable:
            code = OS_RETCODE_PRECONDITION_NOT_MET;
            break;
        case os_resultTimeout:
            code = OS_RETCODE_TIMEOUT;
            break;
        case os_resultBusy:
            code = OS_RETCODE_PRECONDITION_NOT_MET;
            break;
        case os_resultInvalid:
            code = OS_RETCODE_BAD_PARAMETER;
            break;
        default:
            assert (result == os_resultFail);
            break;
    }

    return code;
}

const os_char *
os_returnCodeImage(
    os_int32 code)
{
    const os_char *image;

#define OS__CASE__(code) case code: image = # code; break;
    switch (code) {
        OS__CASE__(OS_RETCODE_OK)
        OS__CASE__(OS_RETCODE_ERROR)
        OS__CASE__(OS_RETCODE_UNSUPPORTED)
        OS__CASE__(OS_RETCODE_BAD_PARAMETER)
        OS__CASE__(OS_RETCODE_PRECONDITION_NOT_MET)
        OS__CASE__(OS_RETCODE_OUT_OF_RESOURCES)
        OS__CASE__(OS_RETCODE_NOT_ENABLED)
        OS__CASE__(OS_RETCODE_IMMUTABLE_POLICY)
        OS__CASE__(OS_RETCODE_INCONSISTENT_POLICY)
        OS__CASE__(OS_RETCODE_ALREADY_DELETED)
        OS__CASE__(OS_RETCODE_TIMEOUT)
        OS__CASE__(OS_RETCODE_NO_DATA)
        OS__CASE__(OS_RETCODE_ILLEGAL_OPERATION)
        default:
            image = "<undefined value>";
            break;
    }
#undef OS__CASE__

    return image;
}
