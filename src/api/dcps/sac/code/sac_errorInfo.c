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

#include "dds_dcps.h"
#include "sac_object.h"
#include "sac_common.h"
#include "sac_report.h"
#include "cmn_errorInfo.h"

#define DDS_ErrorInfoClaim(_this, info) \
        DDS_Object_claim(DDS_Object(_this), DDS_ERRORINFO, (_Object *)info)

#define DDS_ErrorInfoClaimRead(_this, info) \
        DDS_Object_claim(DDS_Object(_this), DDS_ERRORINFO, (_Object *)info)

#define DDS_ErrorInfoRelease(_this) \
        DDS_Object_release(DDS_Object(_this))

/*     ReturnCode_t
 *     update( );
 */
DDS_ReturnCode_t
DDS_ErrorInfo_update (
    DDS_ErrorInfo _this)
{
    DDS_ReturnCode_t result;
    _ErrorInfo info;
    os_reportInfo *osInfo;

    SAC_REPORT_STACK();

    result = DDS_ErrorInfoClaim(_this, &info);
    if (result == DDS_RETCODE_OK) {
        osInfo = os_reportGetApiInfo();
        if (osInfo != NULL) {
            if (info->source_line != NULL) {
                DDS_free(info->source_line);
                info->source_line = NULL;
            }
            if (osInfo->sourceLine != NULL) {
                info->source_line = DDS_string_dup(osInfo->sourceLine);
            }
            if (info->stack_trace != NULL) {
                DDS_free(info->stack_trace);
                info->stack_trace = NULL;
            }
            if (osInfo->callStack != NULL) {
                info->stack_trace = DDS_string_dup(osInfo->callStack);
            }
            if (info->message != NULL) {
                DDS_free(info->message);
                info->message = NULL;
            }
            if (osInfo->description != NULL) {
                info->message = DDS_string_dup(osInfo->description);
            }
            if (info->location != NULL) {
                DDS_free(info->location);
                info->location = NULL;
            }
            if (osInfo->reportContext != NULL) {
                info->location = DDS_string_dup(osInfo->reportContext);
            }

            info->code = cmn_errorInfo_reportCodeToCode(osInfo->reportCode);
            info->valid = TRUE;
        } else {
            info->valid = FALSE;
            result = DDS_RETCODE_NO_DATA;
        }
        DDS_ErrorInfoRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, (result != DDS_RETCODE_OK) && (result != DDS_RETCODE_NO_DATA));
    return result;
}

/*     ReturnCode_t
 *     get_code(
 *         out ErrorCode code);
 */
DDS_ReturnCode_t
DDS_ErrorInfo_get_code (
    DDS_ErrorInfo _this,
    DDS_ReturnCode_t * code)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    _ErrorInfo errorinfo;

    SAC_REPORT_STACK();

    result = DDS_ErrorInfoClaimRead(_this, &errorinfo);
    if (result == DDS_RETCODE_OK) {
        if (errorinfo->valid) {
            (*code) = errorinfo->code;
        } else {
            result = DDS_RETCODE_NO_DATA;
        }
        DDS_ErrorInfoRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, (result != DDS_RETCODE_OK) && (result != DDS_RETCODE_NO_DATA));
    return result;
}

/*     ReturnCode_t
 *     get_location(
 *         out String location);
 */
DDS_ReturnCode_t
DDS_ErrorInfo_get_location(
    DDS_ErrorInfo _this,
    DDS_string * location)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    _ErrorInfo errorinfo;

    SAC_REPORT_STACK();

    result = DDS_ErrorInfoClaimRead(_this, &errorinfo);
    if (result == DDS_RETCODE_OK) {
        if (errorinfo->valid) {
            if (*location != NULL) {
                DDS_free(*location);
            }
            if (errorinfo->location != NULL) {
                (*location) = DDS_string_dup(errorinfo->location);
            } else {
                (*location) = NULL;
            }
        } else {
            result = DDS_RETCODE_NO_DATA;;
        }
        DDS_ErrorInfoRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, (result != DDS_RETCODE_OK) && (result != DDS_RETCODE_NO_DATA));
    return result;
}

/*     ReturnCode_t
 *     get_source_line(
 *         out String source_line);
 */
DDS_ReturnCode_t
DDS_ErrorInfo_get_source_line(
    DDS_ErrorInfo _this,
    DDS_string * source_line)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    _ErrorInfo errorinfo;

    SAC_REPORT_STACK();

    result = DDS_ErrorInfoClaimRead(_this, &errorinfo);
    if (result == DDS_RETCODE_OK) {
        if (errorinfo->valid) {
            if (*source_line != NULL) {
                DDS_free(*source_line);
            }
            if (errorinfo->source_line != NULL) {
                (*source_line) = DDS_string_dup(errorinfo->source_line);
            } else {
                (*source_line) = NULL;
            }
        } else {
            result = DDS_RETCODE_NO_DATA;;
        }
        DDS_ErrorInfoRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, (result != DDS_RETCODE_OK) && (result != DDS_RETCODE_NO_DATA));
    return result;
}

/*     ReturnCode_t
 *     get_stack_trace(
 *         out String stack_trace);
 */
DDS_ReturnCode_t
DDS_ErrorInfo_get_stack_trace(
    DDS_ErrorInfo _this,
    DDS_string *stack_trace)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    _ErrorInfo errorinfo;

    SAC_REPORT_STACK();

    result = DDS_ErrorInfoClaimRead(_this, &errorinfo);
    if (result == DDS_RETCODE_OK) {
        if (errorinfo->valid) {
            if (*stack_trace != NULL) {
                DDS_free(*stack_trace);
            }
            if (errorinfo->stack_trace != NULL) {
                (*stack_trace) = DDS_string_dup(errorinfo->stack_trace);
            } else {
                (*stack_trace) = NULL;
            }
        } else {
            result = DDS_RETCODE_NO_DATA;;
        }
        DDS_ErrorInfoRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, (result != DDS_RETCODE_OK) && (result != DDS_RETCODE_NO_DATA));
    return result;
}

/*     ReturnCode_t
 *     get_message(
 *         out String message);
 */
DDS_ReturnCode_t
DDS_ErrorInfo_get_message(
    DDS_ErrorInfo _this,
    DDS_string * message)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    _ErrorInfo errorinfo;

    SAC_REPORT_STACK();

    result = DDS_ErrorInfoClaimRead(_this, &errorinfo);
    if (result == DDS_RETCODE_OK) {
        if (errorinfo->valid) {
            if (*message != NULL) {
                DDS_free(*message);
            }
            if (errorinfo->message != NULL) {
                (*message) = DDS_string_dup(errorinfo->message);
            } else {
                (*message) = NULL;
            }
        } else {
            result = DDS_RETCODE_NO_DATA;;
        }
        DDS_ErrorInfoRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, (result != DDS_RETCODE_OK) && (result != DDS_RETCODE_NO_DATA));
    return result;
}


void
DDS_ErrorInfo_report (
    DDS_ErrorInfo _this)
{
    OS_UNUSED_ARG(_this);
}

DDS_ReturnCode_t
_ErrorInfo_Deinit(
    _Object o)
{
    _ErrorInfo    info = (_ErrorInfo)o;

    if (info->source_line != NULL) {
        DDS_free(info->source_line);
        info->source_line = NULL;
    }
    if (info->stack_trace != NULL) {
        DDS_free(info->stack_trace);
        info->stack_trace = NULL;
    }
    if (info->message != NULL) {
        DDS_free(info->message);
        info->message = NULL;
    }
    if (info->location != NULL) {
        DDS_free(info->location);
        info->location = NULL;
    }

    return DDS_RETCODE_OK;
}

/*     ErrorInfo
 *     ErrorInfo__alloc (
 *         void);
 */
DDS_ErrorInfo
DDS_ErrorInfo__alloc (
    void)
{
    DDS_ErrorInfo _this = NULL;
    DDS_ReturnCode_t result;

    result = DDS_Object_new(DDS_ERRORINFO, _ErrorInfo_Deinit, (_Object *)&_this);
    if (result == DDS_RETCODE_OK) {
    }
    return _this;
}



