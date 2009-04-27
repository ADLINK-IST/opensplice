/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

#include "u_reader.h"
#include "u__entity.h"
#include "u__dispatcher.h"
#include "u_dataReader.h"
#include "u_dataView.h"
#include "u_query.h"
#include "u__user.h"
#include "u__types.h"

#include "v_public.h"
#include "v_reader.h"

#include "os_report.h"

u_result
u_readerClaim(
    u_reader _this,
    v_reader *reader)
{
    u_result result = U_RESULT_OK;

    if ((_this != NULL) && (reader != NULL)) {
        *reader = v_reader(u_entityClaim(u_entity(_this)));
        if (*reader == NULL) {
            OS_REPORT_2(OS_WARNING, "u_readerClaim", 0,
                        "Claim Reader failed. "
                        "<_this = 0x%x, reader = 0x%x>.",
                         _this, reader);
            result = U_RESULT_INTERNAL_ERROR;
        }
    } else {
        OS_REPORT_2(OS_ERROR,"u_readerClaim",0,
                    "Illegal parameter. "
                    "<_this = 0x%x, reader = 0x%x>.",
                    _this, reader);
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_readerRelease(
    u_reader _this)
{
    u_result result = U_RESULT_OK;

    if (_this != NULL) {
        result = u_entityRelease(u_entity(_this));
    } else {
        OS_REPORT_1(OS_ERROR,"u_readerRelease",0,
                    "Illegal parameter. <_this = 0x%x>.", _this);
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_readerInit(
    u_reader r)
{
    u_result result;

    if (r != NULL) {
        result = u_dispatcherInit(u_dispatcher(r));
        u_entity(r)->flags |= U_ECREATE_INITIALISED;
    } else {
        result = U_RESULT_ILL_PARAM;
    }

    return result;
}

u_result
u_readerDeinit(
    u_reader r)
{
    u_result result;

    if (r != NULL) {
        result = u_dispatcherDeinit(u_dispatcher(r));
    } else {
        result = U_RESULT_ILL_PARAM;
    }

    return result;
}

u_result
u_readerGetDeadlineMissedStatus(
    u_reader _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg)
{
    v_reader reader;
    u_result result;

    result = U_RESULT_PRECONDITION_NOT_MET;
    if (_this != NULL) {
        result = u_readerClaim(_this,&reader);
        if (reader != NULL) {
            result = u_resultFromKernel(
                         v_readerGetDeadlineMissedStatus(reader,
                                                         reset,
                                                         (v_statusAction)action,
                                                         arg));
            u_readerRelease(_this);
        }  else {
            OS_REPORT(OS_ERROR, "u_readerDeadlineMissedStatus", 0,
                      "Illegal handle detected");
        }
    }
    return result;
}

u_result
u_readerGetIncompatibleQosStatus(
    u_reader _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg)
{
    v_reader reader;
    u_result result;

    result = U_RESULT_PRECONDITION_NOT_MET;
    if (_this != NULL) {
        result = u_readerClaim(_this,&reader);
        if (reader != NULL) {
            result = u_resultFromKernel(
                         v_readerGetIncompatibleQosStatus(reader,
                                                          reset,
                                                          (v_statusAction)action,
                                                          arg));
            u_readerRelease(_this);
        } else {
            OS_REPORT(OS_ERROR, "u_readerGetIncompatibleQosStatus", 0,
                      "Illegal handle detected");
        }
    }
    return result;
}

u_result
u_readerGetSampleRejectedStatus(
    u_reader _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg)
{
    v_reader reader;
    u_result result;

    result = U_RESULT_PRECONDITION_NOT_MET;
    if (_this != NULL) {
        result = u_readerClaim(_this,&reader);
        if (reader != NULL) {
            result = u_resultFromKernel(
                         v_readerGetSampleRejectedStatus(reader,
                                                         reset,
                                                         (v_statusAction)action,
                                                         arg));
            u_readerRelease(_this);
        }  else {
            OS_REPORT(OS_ERROR, "u_readerGetSampleRejectedStatus", 0,
                      "Illegal handle detected");
        }
    }
    return result;
}

u_result
u_readerGetLivelinessChangedStatus(
    u_reader _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg)
{
    v_reader reader;
    u_result result;

    result = U_RESULT_PRECONDITION_NOT_MET;
    if (_this != NULL) {
        result = u_readerClaim(_this,&reader);
        if (reader != NULL) {
            result = u_resultFromKernel(
                         v_readerGetLivelinessChangedStatus(reader,
                                                            reset,
                                                            (v_statusAction)action,
                                                            arg));
            u_readerRelease(_this);
        } else {
            OS_REPORT(OS_ERROR, "u_readerGetLivelinessChangedStatus", 0,
                      "Illegal handle detected");
        }
    }
    return result;
}

u_result
u_readerGetSampleLostStatus(
    u_reader _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg)
{
    v_reader reader;
    u_result result;

    result = U_RESULT_PRECONDITION_NOT_MET;
    if (_this != NULL) {
        result = u_readerClaim(_this,&reader);
        if (reader != NULL) {
            result = u_resultFromKernel(
                         v_readerGetSampleLostStatus(reader,
                                                     reset,
                                                     (v_statusAction)action,
                                                     arg));
            u_readerRelease(_this);
        } else {
            OS_REPORT(OS_ERROR, "u_readerGetSampleLostStatus", 0,
                      "Illegal handle detected");
        }
    }
    return result;
}

u_result
u_readerGetSubscriptionMatchStatus(
    u_reader _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg)
{
    v_reader reader;
    u_result result;

    result = U_RESULT_PRECONDITION_NOT_MET;
    if (_this != NULL) {
        result = u_readerClaim(_this,&reader);
        if (reader != NULL) {
            result = u_resultFromKernel(
                         v_readerGetTopicMatchStatus(reader,
                                                     reset,
                                                     (v_statusAction)action,
                                                     arg));
            u_readerRelease(_this);
        } else {
            OS_REPORT(OS_ERROR, "u_readerGetTopicMatchStatus", 0,
                      "Illegal handle detected");
        }
    }
    return result;
}

u_result
u_readerRead(
    u_reader r, 
    u_readerAction action,
    c_voidp actionArg)
{
    u_result result;
    switch (u_entity(r)->kind) {
    case U_READER:
        result = u_dataReaderRead(u_dataReader(r), action, actionArg);
    break;
    case U_DATAVIEW:
        result = u_dataViewRead(u_dataView(r), action, actionArg);
    break;
    case U_QUERY:
        result = u_queryRead(u_query(r), action, actionArg);
    break;
    default:
        result = U_RESULT_ILL_PARAM;
    break;
    }
    return result;
}

u_result
u_readerTake(
    u_reader r, 
    u_readerAction action,
    c_voidp actionArg)
{
    u_result result;

    switch (u_entity(r)->kind) {
    case U_READER:
        result = u_dataReaderTake(u_dataReader(r), action, actionArg);
    break;
    case U_DATAVIEW:
        result = u_dataViewTake(u_dataView(r), action, actionArg);
    break;
    case U_QUERY:
        result = u_queryTake(u_query(r), action, actionArg);
    break;
    default:
        result = U_RESULT_ILL_PARAM;
    break;
    }
    return result;
}

void *
u_readerReadList(
    u_reader r,
    c_ulong max,
    u_readerCopyList copy,
    c_voidp copyArg)
{
    void *result = NULL;

    switch (u_entity(r)->kind) {
    case U_READER:
        result = u_dataReaderReadList(u_dataReader(r), max, copy, copyArg);
    break;
    case U_DATAVIEW:
         assert(FALSE);
    break;
    case U_QUERY:
        result = u_queryReadList(u_query(r), max, copy, copyArg);
    break;
    default:
        result = NULL;
    break;
    }
    return result;
}

void *
u_readerTakeList(
    u_reader r,
    c_ulong max,
    u_readerCopyList copy,
    c_voidp copyArg)
{
    void *result = NULL;

    switch (u_entity(r)->kind) {
    case U_READER:
        result = u_dataReaderTakeList(u_dataReader(r), max, copy, copyArg);
    break;
    case U_DATAVIEW:
        assert(FALSE);
    break;
    case U_QUERY:
        result = u_queryTakeList(u_query(r), max, copy, copyArg);
    break;
    default:
        result = NULL;
    break;
    }
    return result;
}

u_result
u_readerReadInstance(
    u_reader r,
    u_instanceHandle h,
    u_readerAction action,
    c_voidp actionArg)
{
    u_result result;

    switch (u_entity(r)->kind) {
    case U_READER:
        result = u_dataReaderReadInstance(u_dataReader(r), h, action, actionArg);
    break;
    case U_DATAVIEW:
        result = u_dataViewReadInstance(u_dataView(r), h, action, actionArg);
    break;
    case U_QUERY:
        result = u_queryReadInstance(u_query(r), h, action, actionArg);
    break;
    default:
        result = U_RESULT_ILL_PARAM;
    break;
    }
    return result;
}

u_result
u_readerTakeInstance(
    u_reader r,
    u_instanceHandle h,
    u_readerAction action,
    c_voidp actionArg)
{
    u_result result;

    switch (u_entity(r)->kind) {
    case U_READER:
        result = u_dataReaderTakeInstance(u_dataReader(r), h, action, actionArg);
    break;
    case U_DATAVIEW:
        result = u_dataViewTakeInstance(u_dataView(r), h, action, actionArg);
    break;
    case U_QUERY:
        result = u_queryTakeInstance(u_query(r), h, action, actionArg);
    break;
    default:
        result = U_RESULT_ILL_PARAM;
    break;
    }
    return result;
}


u_result
u_readerReadNextInstance(
    u_reader r,
    u_instanceHandle h,
    u_readerAction action,
    c_voidp actionArg)
{
    u_result result;

    switch (u_entity(r)->kind) {
    case U_READER:
        result = u_dataReaderReadNextInstance(u_dataReader(r), h, action, actionArg);
    break;
    case U_DATAVIEW:
        result = u_dataViewReadNextInstance(u_dataView(r), h, action, actionArg);
    break;
    case U_QUERY:
        result = u_queryReadNextInstance(u_query(r), h, action, actionArg);
    break;
    default:
        result = U_RESULT_ILL_PARAM;
    break;
    }
    return result;
}

u_result
u_readerTakeNextInstance(
    u_reader r,
    u_instanceHandle h,
    u_readerAction action,
    c_voidp actionArg)
{
    u_result result;

    switch (u_entity(r)->kind) {
    case U_READER:
        result = u_dataReaderTakeNextInstance(u_dataReader(r), h, action, actionArg);
    break;
    case U_DATAVIEW:
        result = u_dataViewTakeNextInstance(u_dataView(r), h, action, actionArg);
    break;
    case U_QUERY:
        result = u_queryTakeNextInstance(u_query(r), h, action, actionArg);
    break;
    default:
        result = U_RESULT_ILL_PARAM;
    break;
    }
    return result;
}

