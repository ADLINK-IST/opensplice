
#ifndef V__DATAREADER_H
#define V__DATAREADER_H

#include "v_misc.h"
#include "v_event.h"
#include "v_dataReader.h"
#include "v_dataReaderQuery.h"
#include "v__status.h"

#ifdef OSPL_BUILD_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define v_dataReaderLock(_this) \
        v_observerLock(v_dataReader(_this))

#define v_dataReaderUnLock(_this) \
        v_observerUnlock(v_dataReader(_this))

#ifdef _MSG_STAMP_
void
v_dataReaderLogMessage(
    v_dataReader _this,
    v_message msg);
#endif

void
v_dataReaderDeinit(
    v_dataReader _this);

c_char *
v_dataReaderKeyExpr(
    v_dataReader reader);

/**
 * \brief This operation retrieves the dataReader instance keyList.
 *
 * The keyList is an array of c_field objects that specifies the instance
 * key fields.
 *
 * \param _this The dataReader this method operates on.
 *
 * \return A successful operation will return the dataReader instance keyList.
 *         otherwise the operation will return NULL.
 */
c_array
v_dataReaderKeyList(
    v_dataReader _this);

c_field
v_dataReaderIndexField(
    v_dataReader reader,
    const c_char *name);

c_field
v_dataReaderField(
    v_dataReader reader,
    const c_char *name);

c_bool
v_dataReaderSubscribe(
    v_dataReader reader,
    v_domain domain);

c_bool
v_dataReaderUnSubscribe(
    v_dataReader reader,
     v_domain domain);

c_bool
v_dataReaderSubscribeGroup(
    v_dataReader reader,
    v_group group);

c_bool
v_dataReaderUnSubscribeGroup(
    v_dataReader reader,
    v_group group);

#define v_dataReaderAddEntry(_this,entry) \
        v_dataReaderEntry(v_readerAddEntry(v_reader(_this),v_entry(entry)))

#define v_dataReaderNextInstance(_this,_inst) \
        v_dataReaderInstance( \
            c_tableNext(v_dataReader(_this)->index->notEmptyList, \
                        v_dataReaderInstance(_inst)))

void
v_dataReaderUpdatePurgeLists(
    v_dataReader _this);

void
v_dataReaderNotify(
    v_dataReader _this,
    v_event event,
    c_voidp userData);

void
v_dataReaderNotifyDataAvailable(
    v_dataReader _this,
    v_dataReaderSample sample);

void
v_dataReaderTriggerDataAvailable(
    v_dataReader _this);

void
v_dataReaderNotifySampleRejected(
    v_dataReader _this,
    v_sampleRejectedKind kind,
    v_gid instanceHandle);

void
v_dataReaderNotifyIncompatibleQos(
    v_dataReader _this,
    v_policyId id,
    v_gid writerGID);

typedef struct v_dataReaderNotifyChangedQosArg_s {
    /* the following fields are set when the partitionpolicy has changed. */
    c_iter addedDomains;
    c_iter removedDomains;
} v_dataReaderNotifyChangedQosArg;

void
v_dataReaderNotifyChangedQos(
    v_dataReader _this,
     v_dataReaderNotifyChangedQosArg *arg);

void
v_dataReaderNotifyLivelinessChanged(
    v_dataReader _this,
    v_gid wGID,
    enum v_statusLiveliness oldLivState,
    enum v_statusLiveliness newLivState,
    v_message publicationInfo);

void
v_dataReaderInsertView(
    v_dataReader _this,
    v_dataView view);

void
v_dataReaderRemoveView(
    v_dataReader _this,
    v_dataView view);

void
v_dataReaderRemoveViewUnsafe(
    v_dataReader _this,
    v_dataView view);

void
v_dataReaderRemoveExpiredSamples(
    v_dataReader _this,
    v_readerSampleAction action,
    c_voidp arg);

void
v_dataReaderRemoveInstanceNotAlive(
    v_dataReader _this,
    v_dataReaderInstance instance);

void
v_dataReaderRemoveInstance(
    v_dataReader _this,
    v_dataReaderInstance instance);

void
v_dataReaderCheckDeadlineMissed(
    v_dataReader _this,
    c_time now);

c_long
v_dataReaderInstanceCount(
    v_dataReader _this);

v_dataReaderInstance
v_dataReaderAllocInstance(
    v_dataReader _this);

void
v_dataReaderDeallocInstance(
    v_dataReaderInstance _instance);

OS_API v_topic
v_dataReaderGetTopic(
    v_dataReader _this);

#undef OS_API

#endif
