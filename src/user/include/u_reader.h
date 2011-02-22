/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef U_READER_H
#define U_READER_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "v_status.h"
#include "os_if.h"

typedef c_bool
(*u_readerAction)(
    c_object o,
    c_voidp copyArg);

typedef c_voidp
(*u_readerCopyList)(
    v_collection c,
    c_iter list,
    c_voidp copyArg);

#include "u_entity.h"

#ifdef OSPL_BUILD_USER
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define u_reader(o) ((u_reader)(o))

OS_API u_result
u_readerInit (
    u_reader _this);

OS_API u_result
u_readerDeinit (
    u_reader _this);

OS_API u_result
u_readerGetDeadlineMissedStatus(
    u_reader _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg);

OS_API u_result
u_readerGetIncompatibleQosStatus(
    u_reader _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg);

OS_API u_result
u_readerGetSampleRejectedStatus(
    u_reader _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg);

OS_API u_result
u_readerGetLivelinessChangedStatus(
    u_reader _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg);

OS_API u_result
u_readerGetSampleLostStatus(
    u_reader _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg);

OS_API u_result
u_readerGetSubscriptionMatchStatus(
    u_reader _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg);

OS_API u_result
u_readerGetMatchedPublications (
	u_reader _this,
    v_statusAction action,
    c_voidp arg);

OS_API u_result
u_readerGetMatchedPublicationData (
    u_reader _this,
    u_instanceHandle publication_handle,
    v_statusAction action,
    c_voidp arg);

OS_API u_result
u_readerRead (
    u_reader _this,
    u_readerAction action,
    c_voidp actionArg);

OS_API u_result
u_readerTake (
    u_reader _this,
    u_readerAction action,
    c_voidp actionArg);

OS_API void *
u_readerReadList (
    u_reader _this,
    c_ulong max,
    u_readerCopyList copy,
    c_voidp copyArg);

OS_API void *
u_readerTakeList (
    u_reader _this,
    c_ulong max,
    u_readerCopyList copy,
    c_voidp copyArg);

OS_API u_result
u_readerReadInstance (
    u_reader _this,
    u_instanceHandle handle,
    u_readerAction action,
    c_voidp actionArg);

OS_API u_result
u_readerTakeInstance (
    u_reader _this,
    u_instanceHandle handle,
    u_readerAction action,
    c_voidp actionArg);

OS_API u_result
u_readerReadNextInstance (
    u_reader _this,
    u_instanceHandle handle,
    u_readerAction action,
    c_voidp actionArg);

OS_API u_result
u_readerTakeNextInstance (
    u_reader _this,
    u_instanceHandle handle,
    u_readerAction action,
    c_voidp actionArg);

OS_API u_result
u_readerAddQuery(
    u_reader _this,
    u_query query);

OS_API u_result
u_readerRemoveQuery(
    u_reader _this,
    u_query query);

OS_API c_bool
u_readerContainsQuery(
    u_reader _this,
    u_query query);

OS_API c_long
u_readerQueryCount(
    u_reader _this);

OS_API c_iter
u_readerLookupQueries(
    u_reader _this);

OS_API c_bool
u_readerWalkQueries(
    u_reader _this,
    u_readerAction action,
    c_voidp actionArg);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
