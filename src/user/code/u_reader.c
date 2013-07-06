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
#include "v_dataReader.h"
#include "v_spliced.h"
#include "v_participant.h"

#include "os_report.h"

u_result
u_readerInit(
    u_reader _this)
{
    u_result result;
    os_result osResult;
    os_mutexAttr osMutexAttr;

    if (_this != NULL) {
        result = u_dispatcherInit(u_dispatcher(_this));
        if (result == U_RESULT_OK) {
            _this->queries = NULL;
            osResult = os_mutexAttrInit(&osMutexAttr);
            if (osResult == os_resultSuccess) {
                osMutexAttr.scopeAttr = OS_SCOPE_PRIVATE;
                osResult = os_mutexInit(&_this->mutex, &osMutexAttr);
                if (osResult != os_resultSuccess) {
                    result = U_RESULT_INTERNAL_ERROR;
                }
            }
            u_entity(_this)->flags |= U_ECREATE_INITIALISED;
        }
    } else {
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_readerDeinit(
    u_reader _this)
{
    u_result result;
    u_query query;

    if (_this != NULL) {
        result = u_dispatcherDeinit(u_dispatcher(_this));
        if (result == U_RESULT_OK) {
            os_mutexLock(&_this->mutex);
            if (_this->queries) {
                query = c_iterObject(_this->queries,0);
                while (query) {
                    os_mutexUnlock(&_this->mutex);
                    result = u_queryFree(query);
                    os_mutexLock(&_this->mutex);
                    query = c_iterObject(_this->queries,0);
                }
                c_iterFree(_this->queries);
                _this->queries = NULL;
            }
            os_mutexUnlock(&_this->mutex);
            os_mutexDestroy(&_this->mutex);
        }
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
        result = u_entityReadClaim(u_entity(_this), (v_entity*)(&reader));
        if (result == U_RESULT_OK){
            result = u_resultFromKernel(
                         v_readerGetDeadlineMissedStatus(reader,
                                                         reset,
                                                         (v_statusAction)action,
                                                         arg));
            u_entityRelease(u_entity(_this));
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
        result = u_entityReadClaim(u_entity(_this), (v_entity*)(&reader));
        if (result == U_RESULT_OK){
            result = u_resultFromKernel(
                         v_readerGetIncompatibleQosStatus(reader,
                                                          reset,
                                                          (v_statusAction)action,
                                                          arg));
            u_entityRelease(u_entity(_this));
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
        result = u_entityReadClaim(u_entity(_this), (v_entity*)(&reader));
        if (result == U_RESULT_OK){
            result = u_resultFromKernel(
                         v_readerGetSampleRejectedStatus(reader,
                                                         reset,
                                                         (v_statusAction)action,
                                                         arg));
            u_entityRelease(u_entity(_this));
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
        result = u_entityReadClaim(u_entity(_this), (v_entity*)(&reader));
        if (result == U_RESULT_OK){
            result = u_resultFromKernel(
                         v_readerGetLivelinessChangedStatus(reader,
                                                            reset,
                                                            (v_statusAction)action,
                                                            arg));
            u_entityRelease(u_entity(_this));
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
        result = u_entityReadClaim(u_entity(_this), (v_entity*)(&reader));
        if (result == U_RESULT_OK){
            result = u_resultFromKernel(
                         v_readerGetSampleLostStatus(reader,
                                                     reset,
                                                     (v_statusAction)action,
                                                     arg));
            u_entityRelease(u_entity(_this));
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
        result = u_entityReadClaim(u_entity(_this), (v_entity*)(&reader));
        if (result == U_RESULT_OK){
            result = u_resultFromKernel(
                         v_readerGetTopicMatchStatus(reader,
                                                     reset,
                                                     (v_statusAction)action,
                                                     arg));
            u_entityRelease(u_entity(_this));
        } else {
            OS_REPORT(OS_ERROR, "u_readerGetTopicMatchStatus", 0,
                      "Illegal handle detected");
        }
    }
    return result;
}

u_result
u_readerGetMatchedPublications (
    u_reader _this,
    v_statusAction action,
    c_voidp arg)
{
    v_dataReader reader;
    v_spliced spliced;
    v_kernel kernel;
    u_result result;
    c_iter participants;
    v_participant participant;

    result = U_RESULT_PRECONDITION_NOT_MET;
    if (_this != NULL) {
        result = u_entityReadClaim(u_entity(_this), (v_entity*)(&reader));

        if ((result == U_RESULT_OK) && (reader != NULL)) {
            kernel = v_objectKernel(reader);

            participants = v_resolveParticipants(kernel, V_SPLICED_NAME);
            assert(c_iterLength(participants) == 1);
            participant = v_participant(c_iterTakeFirst(participants));
            spliced = v_spliced(participant);
            c_free(participant);
            c_iterFree(participants);

            result = u_resultFromKernel(
                         v_splicedGetMatchedPublications(spliced, v_dataReader(reader), action, arg));
            u_entityRelease(u_entity(_this));
        }
    }
    return result;
}

u_result
u_readerGetMatchedPublicationData (
    u_reader _this,
    u_instanceHandle publication_handle,
    v_statusAction action,
    c_voidp arg)
{
    v_dataReader reader;
    v_spliced spliced;
    v_kernel kernel;
    u_result result;
    c_iter participants;
    v_participant participant;

    result = U_RESULT_PRECONDITION_NOT_MET;
    if (_this != NULL) {
        result = u_entityReadClaim(u_entity(_this), (v_entity*)(&reader));

        if ((result == U_RESULT_OK) && (reader != NULL)) {
            kernel = v_objectKernel(reader);

            participants = v_resolveParticipants(kernel, V_SPLICED_NAME);
            assert(c_iterLength(participants) == 1);
            participant = v_participant(c_iterTakeFirst(participants));
            spliced = v_spliced(participant);
            c_free(participant);
            c_iterFree(participants);

            result = u_resultFromKernel(
                         v_splicedGetMatchedPublicationData(spliced, v_dataReader(reader), u_instanceHandleToGID(publication_handle), action, arg));
            u_entityRelease(u_entity(_this));
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

u_result
u_readerAddQuery(
    u_reader _this,
    u_query query)
{
    os_result r;
    u_result result = U_RESULT_PRECONDITION_NOT_MET;

    if (_this && query) {
        if(u_entityOwner(u_entity(_this))) {
            r = os_mutexLock(&_this->mutex);
            if (r == os_resultSuccess) {
                _this->queries = c_iterInsert(_this->queries, query);
                os_mutexUnlock(&_this->mutex);
                result = U_RESULT_OK;
            } else {
                OS_REPORT(OS_WARNING,
                          "u_readerAddQuery",0,
                          "Failed to lock Reader.");
                result = U_RESULT_ILL_PARAM;
            }
        } else {
            result = U_RESULT_OK;
        }
    } else {
        OS_REPORT(OS_WARNING,
                  "u_readerAddQuery",0,
                  "Illegal parameter.");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_readerRemoveQuery(
    u_reader _this,
    u_query query)
{
    u_query found;
    u_result result;
    os_result r;

    if (_this && query) {
        if(u_entityOwner(u_entity(_this))) {
            r = os_mutexLock(&_this->mutex);
            if (r == os_resultSuccess) {
                found = c_iterTake(_this->queries,query);
                os_mutexUnlock(&_this->mutex);
                if (found) {
                    result = U_RESULT_OK;
                } else {
                    OS_REPORT(OS_WARNING,"u_readerRemoveQuery",0,
                              "The specified Querie is not related to the given Reader.");
                    result = U_RESULT_PRECONDITION_NOT_MET;
                }
            } else {
                OS_REPORT(OS_WARNING,
                          "u_readerRemoveQuery",0,
                          "Failed to lock Reader.");
                result = U_RESULT_ILL_PARAM;
            }
        } else {
            result = U_RESULT_OK;
        }
    } else {
        OS_REPORT(OS_WARNING,
                  "u_readerRemoveQuery",0,
                  "Illegal parameter.");
        result = U_RESULT_ILL_PARAM;
    }

    return result;
}

c_bool
u_readerContainsQuery(
    u_reader _this,
    u_query query)
{
    c_bool found = FALSE;
    os_result r;

    if (_this && query) {
        if(u_entityOwner(u_entity(_this))) {
            r = os_mutexLock(&_this->mutex);
            if (r == os_resultSuccess) {
                found = c_iterContains(_this->queries,query);
                os_mutexUnlock(&_this->mutex);
            } else {
                OS_REPORT(OS_WARNING,
                          "u_readerContainsQuery",0,
                          "Failed to lock Reader.");
            }
        }
    } else {
        OS_REPORT(OS_WARNING,
                  "u_readerRemoveQuery",0,
                  "Illegal parameter.");
    }
    return found;
}

static void
collect_queries(
    c_voidp object,
    c_voidp arg)
{
    c_iter *queries = (c_iter *)arg;
    u_query q = (u_query)object;

    *queries = c_iterInsert(*queries, q);
}

c_iter
u_readerLookupQueries(
    u_reader _this)
{
    c_iter queries = NULL;
    os_result r;

    if (_this) {
        if(u_entityOwner(u_entity(_this))) {
            r = os_mutexLock(&_this->mutex);
            if (r == os_resultSuccess) {
                c_iterWalk(_this->queries, collect_queries, &queries);
                os_mutexUnlock(&_this->mutex);
            } else {
                OS_REPORT(OS_WARNING,
                          "u_readerLookupQueries",0,
                          "Failed to lock Reader.");
            }
        }
    } else {
        OS_REPORT(OS_WARNING,
                  "u_readerLookupQueries",0,
                  "No Reader specified.");
    }
    return queries;
}

c_long
u_readerQueryCount(
    u_reader _this)
{
    c_long length = -1;
    os_result r;

    if (_this) {
        if(u_entityOwner(u_entity(_this))) {
            r = os_mutexLock(&_this->mutex);
            if (r == os_resultSuccess) {
                length = c_iterLength(_this->queries);
                os_mutexUnlock(&_this->mutex);
            } else {
                OS_REPORT(OS_WARNING,
                          "u_readerRemoveQuerie",0,
                          "Failed to lock Reader.");
            }
        }
    } else {
        OS_REPORT(OS_WARNING,
                  "u_readerQueryCount",0,
                  "No Reader specified.");
    }
    return length;
}

c_bool
u_readerWalkQueries(
    u_reader _this,
    u_readerAction action,
    c_voidp actionArg)
{
    c_bool result = U_RESULT_OK;
    os_result r;

    if (_this) {
        if(u_entityOwner(u_entity(_this))) {
            r = os_mutexLock(&_this->mutex);
            if (r == os_resultSuccess) {
                c_iterWalkUntil(_this->queries, (c_iterAction)action, actionArg);
                os_mutexUnlock(&_this->mutex);
            } else {
                OS_REPORT(OS_WARNING,
                          "u_readerWalkQueries",0,
                          "Failed to lock Reader.");
                result = U_RESULT_ILL_PARAM;
            }
        }
    } else {
        OS_REPORT(OS_WARNING,
                  "u_readerWalkQueries",0,
                  "No Reader specified.");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

