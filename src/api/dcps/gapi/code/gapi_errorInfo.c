/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#include "gapi.h"
#include "gapi_entity.h"
#include "gapi_errorInfo.h"
#include "os_report.h"


static gapi_errorCode_t
MapUserErrorInfoCode(
    os_int32 userCode)
{
    gapi_errorCode_t result = GAPI_ERRORCODE_UNDEFINED;

    switch (userCode) {
    default:
        result = GAPI_ERRORCODE_UNDEFINED;
    break;
    }
    result = userCode;

    return result;
}

gapi_boolean
_ErrorInfoFree (
    void * _errorInfo)
{
    _ErrorInfo errorinfo = (_ErrorInfo)_errorInfo;

    if (errorinfo->source_line != NULL) {
        gapi_free(errorinfo->source_line);
    }

    if (errorinfo->stack_trace != NULL) {
        gapi_free(errorinfo->stack_trace);
    }

    if (errorinfo->message != NULL) {
        gapi_free(errorinfo->message);
    }

    if (errorinfo->location != NULL) {
        gapi_free(errorinfo->location);
    }
    return TRUE;
}

_ErrorInfo
_ErrorInfoNew (
    void)
{
    _ErrorInfo newErrorInfo;

    newErrorInfo = _ErrorInfoAlloc();

    return newErrorInfo;
}

/*     ErrorInfo
 *     ErrorInfo__alloc (
 *         void);
 */
gapi_errorInfo
gapi_errorInfo__alloc (
    void)
{
    return (gapi_errorInfo)_EntityRelease(_ErrorInfoNew());
}

/*     ReturnCode_t
 *     update( );
 */
gapi_returnCode_t
gapi_errorInfo_update (
    gapi_errorInfo _this)
{
    gapi_returnCode_t result;
    _ErrorInfo    info;
    os_reportInfo *osInfo;

    info = gapi_errorInfoClaim(_this, &result);
    if ( info != NULL ) {
        osInfo = os_reportGetApiInfo();
        if (osInfo != NULL) {
            if (info->source_line != NULL) {
                gapi_free(info->source_line);
                info->source_line = NULL;
            }
            if (osInfo->sourceLine != NULL) {
                info->source_line = gapi_string_dup(osInfo->sourceLine);
            }
            if (info->stack_trace != NULL) {
                gapi_free(info->stack_trace);
                info->stack_trace = NULL;
            }
            if (osInfo->callStack != NULL) {
                info->stack_trace = gapi_string_dup(osInfo->callStack);
            }
            if (info->message != NULL) {
                gapi_free(info->message);
                info->message = NULL;
            }
            if (osInfo->description != NULL) {
                info->message = gapi_string_dup(osInfo->description);
            }
            if (info->location != NULL) {
                gapi_free(info->location);
                info->location = NULL;
            }
            if (osInfo->reportContext != NULL) {
                info->location = gapi_string_dup(osInfo->reportContext);
            }
            info->code = MapUserErrorInfoCode(osInfo->reportCode);
            info->valid = TRUE;
        } else {
            info->valid = FALSE;
            result = GAPI_RETCODE_NO_DATA;
        }
        _EntityRelease(info);
    }
    return result;
}

/*     ReturnCode_t
 *     get_code(
 *         out ErrorCode code);
 */
gapi_returnCode_t
gapi_errorInfo_get_code (
    gapi_errorInfo _this,
    gapi_errorCode_t *code)
{
    gapi_returnCode_t result      = GAPI_RETCODE_OK;
    _ErrorInfo        errorinfo;

    errorinfo = gapi_errorInfoClaim(_this, &result);

    if (errorinfo->valid) {
        (*code) = errorinfo->code;
    } else {
        result = GAPI_RETCODE_NO_DATA;
    }

    _EntityRelease(errorinfo);

    return result;
}

/*     ReturnCode_t
 *     get_location(
 *         out String location);
 */
gapi_returnCode_t
gapi_errorInfo_get_location(
    gapi_errorInfo _this,
    gapi_string *location)
{
    gapi_returnCode_t result      = GAPI_RETCODE_OK;
    _ErrorInfo        errorinfo;

    errorinfo = gapi_errorInfoClaim(_this, &result);

    if (errorinfo->valid) {
        if (*location != NULL) {
            gapi_free(*location);
        }
        if (errorinfo->location != NULL) {
            (*location) = gapi_string_dup(errorinfo->location);
        } else {
            (*location) = NULL;
        }
    } else {
        result = GAPI_RETCODE_NO_DATA;;
    }

    _EntityRelease(errorinfo);

    return result;
}

/*     ReturnCode_t
 *     get_source_line(
 *         out String source_line);
 */
gapi_returnCode_t
gapi_errorInfo_get_source_line(
    gapi_errorInfo _this,
    gapi_string *source_line)
{
    gapi_returnCode_t result      = GAPI_RETCODE_OK;
    _ErrorInfo        errorinfo;

    errorinfo = gapi_errorInfoClaim(_this, &result);

    if (errorinfo->valid) {
        if (*source_line != NULL) {
            gapi_free(*source_line);
        }
        if (errorinfo->source_line != NULL) {
            (*source_line) = gapi_string_dup(errorinfo->source_line);
        } else {
            (*source_line) = NULL;
        }
    } else {
        result = GAPI_RETCODE_NO_DATA;;
    }

    _EntityRelease(errorinfo);

    return result;
}

/*     ReturnCode_t
 *     get_stack_trace(
 *         out String stack_trace);
 */
gapi_returnCode_t
gapi_errorInfo_get_stack_trace(
    gapi_errorInfo _this,
    gapi_string *stack_trace)
{
    gapi_returnCode_t result      = GAPI_RETCODE_OK;
    _ErrorInfo        errorinfo;

    errorinfo = gapi_errorInfoClaim(_this, &result);

    if (errorinfo->valid) {
        if (*stack_trace != NULL) {
            gapi_free(*stack_trace);
        }
        if (errorinfo->stack_trace != NULL) {
            (*stack_trace) = gapi_string_dup(errorinfo->stack_trace);
        } else {
            (*stack_trace) = NULL;
        }
    } else {
        result = GAPI_RETCODE_NO_DATA;;
    }

    _EntityRelease(errorinfo);

    return result;
}

/*     ReturnCode_t
 *     get_message(
 *         out String message);
 */
gapi_returnCode_t
gapi_errorInfo_get_message(
    gapi_errorInfo _this,
    gapi_string *message)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _ErrorInfo errorinfo;

    errorinfo = gapi_errorInfoClaim(_this, &result);

    if (errorinfo->valid) {
        if (*message != NULL) {
            gapi_free(*message);
        }
        if (errorinfo->message != NULL) {
            (*message) = gapi_string_dup(errorinfo->message);
        } else {
            (*message) = NULL;
        }
    } else {
        result = GAPI_RETCODE_NO_DATA;;
    }

    _EntityRelease(errorinfo);

    return result;
}



