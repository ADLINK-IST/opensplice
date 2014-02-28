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
#include "os.h"
#include "os_version.h"
#include "os_stdlib.h"
#include "os_cond.h"
#include "d_misc.h"
#include "d_admin.h"
#include "d__storeMMF.h"
#include "d_storeMMF.h"
#include "d_storeMMFKernel.h"
#include "d_lock.h"
#include "d_store.h"
#include "d_nameSpace.h"
#include "d_configuration.h"
#include "d_groupInfo.h"
#include "d_table.h"
#include "d_actionQueue.h"
#include "d_object.h"
#include "u_user.h"
#include "u_group.h"
#include "u_entity.h"
#include "u_partition.h"
#include "u_topic.h"
#include "v_public.h"
#include "v_entity.h"
#include "v_time.h"
#include "v_topicQos.h"
#include "v_topic.h"
#include "v_message.h"
#include "v_partition.h"
#include "v_state.h"
#include "c_base.h"
#include "c_laptime.h"
#include "c_iterator.h"
#include "os_heap.h"
#include "os_time.h"
#include "os_stdlib.h"
#include "os_report.h"
#include "c_base.h"

#include <limits.h>


#define MMF_STORE_SNAPSHOT      "mmfStore"OSPL_VERSION_STR".ospl"
#define MMF_STORE_FILENAME      "mmfStore"OSPL_VERSION_STR".ospl"
#define MMF_STORE_BACKUP_EXT    ".bak"
#define MMF_STORE_DATABASE_NAME "OsplDurabilityDatabase"
#define MMF_STORE_KERNEL_NAME   "OsplDurabilityKernel"
#define UNKNOWN   "Unknown"

c_bool  action_started = FALSE;
os_time first_time;
os_time last_time;

static void
initStoreFilePath(
    d_storeMMF storeMMF,
    c_char* directoryName,
    c_char* fileName)
{
    c_long dirPathLen, pathSepLen, filenameLen;
    d_store store;
    c_char *storeFileName, *storeDirName;

    store = d_store(storeMMF);

    if(fileName){
        storeFileName = fileName;
    } else {
        storeFileName = MMF_STORE_FILENAME;
    }
    if(directoryName){
        storeDirName = directoryName;
    } else {
        storeDirName = store->config->persistentStoreDirectory;
    }

    /* storeFilePath is "<persistentStoreDirectory>/<MMF_STORE_FILENAME>" */
    dirPathLen  = (c_long)strlen(storeDirName);
    pathSepLen  = (c_long)strlen(os_fileSep());
    filenameLen = (c_long)strlen(storeFileName);
    storeMMF->storeFilePath = (c_char *)os_malloc((os_uint32)
                                 dirPathLen + pathSepLen + filenameLen +1);
    os_strncpy(storeMMF->storeFilePath, storeDirName,
            (os_uint32)dirPathLen);
    os_strncpy(storeMMF->storeFilePath+dirPathLen, os_fileSep(),
            (os_uint32)pathSepLen);
    os_strncpy(storeMMF->storeFilePath+dirPathLen+pathSepLen, storeFileName,
            (os_uint32)filenameLen);
    storeMMF->storeFilePath[dirPathLen+pathSepLen+filenameLen] = 0;
}

static d_storeResult
openExistingMMF(
    d_storeMMF storeMMF)
{
    d_storeResult result;
    os_result mmfOpenResult, mmfAttachResult;
    c_base base;
    d_storeMMFKernel kernel;
    d_store store;


    store = d_store(storeMMF);
    mmfAttachResult = os_resultFail;
    base = NULL;
    /* open Memory Mapped File */
    mmfOpenResult = os_mmfOpen(storeMMF->mmfHandle);

    if (mmfOpenResult == os_resultSuccess) {
        /* attach memory */
        mmfAttachResult = os_mmfAttach(storeMMF->mmfHandle);
        /*printMStart(os_mmfAddress(storeMMF->mmfHandle));*/
        if (mmfAttachResult == os_resultSuccess) {
            /* open c_base */
            base = c_open(MMF_STORE_DATABASE_NAME,
                          os_mmfAddress(storeMMF->mmfHandle));
            if (base != NULL) {
                /* get storeKernel */
                kernel = d_storeMMFKernelAttach(base, MMF_STORE_KERNEL_NAME);
                if (kernel != NULL) {
                    storeMMF->storeKernel = kernel;
                    storeMMF->base = base;
                    c_mmResume(c_baseMM(storeMMF->base));
                    result = D_STORE_RESULT_OK;
                } else {
                    d_storeReport(store, D_LEVEL_SEVERE, "Kernel object not found in Memory Mapped File %s.\n", storeMMF->storeFilePath);
                    result = D_STORE_RESULT_MUTILATED;
                }
            } else {
                d_storeReport(store, D_LEVEL_SEVERE, "Failed to open database in Memory Mapped File %s.\n", storeMMF->storeFilePath);
                result = D_STORE_RESULT_MUTILATED;
            }
        } else {
            d_storeReport(store, D_LEVEL_SEVERE, "Failed to attach Memory Mapped File %s in memory.\n", storeMMF->storeFilePath);
            result = D_STORE_RESULT_OUT_OF_RESOURCES;
        }
    } else {
        d_storeReport(store, D_LEVEL_SEVERE, "Failed to open Memory Mapped File %s.\n", storeMMF->storeFilePath);
        result = D_STORE_RESULT_IO_ERROR;
    }

    if (result != D_STORE_RESULT_OK) {
        /* clean up */
        if (mmfOpenResult == os_resultSuccess) {
            if (mmfAttachResult == os_resultSuccess) {
                if (base != NULL) {
                    c_destroy(base);
                }
                os_mmfDetach(storeMMF->mmfHandle);
            }
            os_mmfClose(storeMMF->mmfHandle);
        }
    }
    return result;
}

static d_storeResult
createNewMMF(
    d_storeMMF storeMMF)
{
    d_storeResult result;
    os_result mmfCreateResult, mmfAttachResult;
    c_base base;
    d_storeMMFKernel kernel;
    d_store store;
    c_size size;


    store = d_store(storeMMF);
    mmfAttachResult = os_resultFail;
    base = NULL;

    /* create Memory Mapped File */
    size = store->config->persistentMMFStoreSize;

    mmfCreateResult = os_mmfCreate(storeMMF->mmfHandle, size);
    if (mmfCreateResult == os_resultSuccess) {
        /* attach memory */
        mmfAttachResult = os_mmfAttach(storeMMF->mmfHandle);
        /*printMStart(os_mmfAddress(storeMMF->mmfHandle));*/
        if (mmfAttachResult == os_resultSuccess) {
            /* create c_base */
            base = c_create(MMF_STORE_DATABASE_NAME,
                            os_mmfAddress(storeMMF->mmfHandle),
                            size, 0);
            if (base != NULL) {
                /* get storeKernel */
                kernel = d_storeMMFKernelNew(base, MMF_STORE_KERNEL_NAME);
                if (kernel != NULL) {
                    storeMMF->storeKernel = kernel;
                    storeMMF->base = base;
                    result = D_STORE_RESULT_OK;
                } else {
                    d_storeReport(store, D_LEVEL_SEVERE, "Failed to create Kernel object in Memory Mapped File %s.\n", storeMMF->storeFilePath);
                    result = D_STORE_RESULT_MUTILATED;
                }
            } else {
                d_storeReport(store, D_LEVEL_SEVERE, "Failed to create database in Memory Mapped File %s.\n", storeMMF->storeFilePath);
                result = D_STORE_RESULT_MUTILATED;
            }
        } else {
            d_storeReport(store, D_LEVEL_SEVERE, "Failed to attach Memory Mapped File %s in memory.\n", storeMMF->storeFilePath);
            result = D_STORE_RESULT_OUT_OF_RESOURCES;
        }
    } else {
        d_storeReport(store, D_LEVEL_SEVERE, "Failed to create Memory Mapped File %s.\n", storeMMF->storeFilePath);
        result = D_STORE_RESULT_IO_ERROR;
    }

    if (result != D_STORE_RESULT_OK) {
        /* clean up */
        if (mmfCreateResult == os_resultSuccess) {
            if (mmfAttachResult == os_resultSuccess) {
                if (base != NULL) {
                    c_destroy(base);
                }
                os_mmfDetach(storeMMF->mmfHandle);
            }
            os_mmfClose(storeMMF->mmfHandle);
        }
    }


    return result;
}

static void
destroyGroupList(
    d_storeMMF storeMMF)
{
    d_groupList groupList, next;

    groupList = storeMMF->groups;

    while(groupList) {
        next = groupList->next;
        os_free(groupList->partition);
        os_free(groupList->topic);
        os_free(groupList);
        groupList = next;
    }
    storeMMF->groups = NULL;
}

static c_bool
addGroupList(
    c_object o,
    c_voidp arg)
{
    d_groupList newElement;
    c_bool success;

    assert(C_TYPECHECK(o,d_groupInfo));

    /* create new groupList element */
    newElement = d_groupList(os_malloc(C_SIZEOF(d_groupList)));

    /* copy values from groupInfo in new element */
    if (newElement) {
        newElement->partition           = os_strdup(d_groupInfoPartition(o));
        newElement->topic               = os_strdup(d_groupInfoTopicName(o));
        newElement->quality.seconds     = d_groupInfoQuality(o).seconds;
        newElement->quality.nanoseconds = d_groupInfoQuality(o).nanoseconds;
        newElement->completeness        = d_groupInfoCompleteness(o);
        newElement->optimized           = TRUE;
        /* add new element in d_storeMMF's groupList */
        newElement->next                = d_storeMMF(arg)->groups;
        d_storeMMF(arg)->groups         = newElement;
        success = TRUE;
    } else {
        success = FALSE;
    }
    return success;
}

static void
updateGroupList(
    d_storeMMF storeMMF)
{
    destroyGroupList(storeMMF);
    c_walk(storeMMF->storeKernel->groups, addGroupList, storeMMF);
}

d_storeMMF
d_storeNewMMF(u_participant participant)
{
    d_storeMMF storeMMF;
    d_store store;

    OS_UNUSED_ARG(participant);
    storeMMF = d_storeMMF(os_malloc(C_SIZEOF(d_storeMMF)));
    store = d_store(storeMMF);
    d_storeInit(store, d_storeDeinitMMF);

    storeMMF->opened                = FALSE;
    storeMMF->storeFilePath         = NULL;
    storeMMF->mmfHandle             = NULL;
    storeMMF->storeKernel           = NULL;
    storeMMF->groups                = NULL;

    store->openFunc                     = d_storeOpenMMF;
    store->closeFunc                    = d_storeCloseMMF;
    store->groupsReadFunc               = d_storeGroupsReadMMF;
    store->groupListFreeFunc            = NULL;
    store->groupInjectFunc              = d_storeGroupInjectMMF;
    store->groupStoreFunc               = d_storeGroupStoreMMF;
    store->getQualityFunc               = d_storeGetQualityMMF;
    store->backupFunc                   = d_storeBackupMMF;
    store->restoreBackupFunc            = d_storeRestoreBackupMMF;
    store->actionStartFunc              = d_storeActionStartMMF;
    store->actionStopFunc               = d_storeActionStopMMF;
    store->messageStoreFunc             = d_storeMessageStoreMMF;
    store->instanceDisposeFunc          = d_storeInstanceDisposeMMF;
    store->instanceExpungeFunc          = d_storeInstanceExpungeMMF;
    store->messageExpungeFunc           = d_storeMessageExpungeMMF;
    store->deleteHistoricalDataFunc     = d_storeDeleteHistoricalDataMMF;
    store->messagesInjectFunc           = d_storeMessagesInjectMMF;
    store->instanceRegisterFunc         = d_storeInstanceRegisterMMF;
    store->createPersistentSnapshotFunc = d_storeCreatePersistentSnapshotMMF;
    store->instanceUnregisterFunc       = d_storeInstanceUnregisterMMF;
    store->optimizeGroupFunc            = d_storeOptimizeGroupMMF;
    store->nsIsCompleteFunc             = d_storeNsIsCompleteMMF;
    store->nsMarkCompleteFunc           = d_storeNsMarkCompleteMMF;

    return storeMMF;
}

void
d_storeDeinitMMF(
    d_object object)
{
    d_storeMMF storeMMF;


    assert(d_objectIsValid(d_object(object), D_STORE));
    assert(d_store(object)->type == D_STORE_TYPE_MEM_MAPPED_FILE);

    if(object){
        storeMMF = d_storeMMF(object);

        if(storeMMF->mmfHandle){
            c_mmSuspend(c_baseMM(storeMMF->base));
            os_mmfDetach(storeMMF->mmfHandle);
            os_mmfClose(storeMMF->mmfHandle);
            os_mmfDestroyHandle(storeMMF->mmfHandle);
            storeMMF->mmfHandle = NULL;
        }
        if(storeMMF->storeFilePath){
            os_free(storeMMF->storeFilePath);
            storeMMF->storeFilePath = NULL;
        }
        d_storeDeinit(object);
    }
}

d_storeResult
d_storeFreeMMF(
    d_storeMMF store)
{
    d_storeResult result;

    assert(d_objectIsValid(d_object(store), D_STORE));
    assert(d_store(store)->type == D_STORE_TYPE_MEM_MAPPED_FILE);

    if(store) {
        d_lockLock(d_lock(store));

        if(store->opened == TRUE) {
            d_lockUnlock(d_lock(store));
            result = D_STORE_RESULT_PRECONDITION_NOT_MET;
        } else {
            d_lockUnlock(d_lock(store));
            d_storeFree(d_store(store));
            result = D_STORE_RESULT_OK;
        }
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeOpenMMF(
    d_store store)
{
    d_storeResult result;
    c_char*       storeDir;
    d_storeMMF    storeMMF;
    os_mmfAttr    mmfAttr;
    os_boolean    mmfExistingFile;
    os_condAttr   actionAttr;

    assert(d_objectIsValid(d_object(store), D_STORE));
    assert(d_store(store)->type == D_STORE_TYPE_MEM_MAPPED_FILE);

    if(store != NULL) {
        storeMMF = d_storeMMF(store);
        d_lockLock(d_lock(store));

        if(storeMMF->opened == TRUE) {
            result = D_STORE_RESULT_PRECONDITION_NOT_MET;
        } else if(store->config->persistentStoreDirectory == NULL ){
            result = D_STORE_RESULT_ILL_PARAM;
        } else {
            storeDir = d_storeDirNew(store, store->config->persistentStoreDirectory);

            if(storeDir){
                os_free(store->config->persistentStoreDirectory);
                store->config->persistentStoreDirectory = storeDir;
                d_storeReport(store, D_LEVEL_INFO,
                                "Persistent store directory '%s' openened.\n",
                                store->config->persistentStoreDirectory);
                result = D_STORE_RESULT_OK;
            } else {
                d_storeReport(store, D_LEVEL_SEVERE,
                                "Persistent store directory '%s' could not be created.\n",
                                store->config->persistentStoreDirectory);
                result = D_STORE_RESULT_PRECONDITION_NOT_MET;
            }
        }

        if(result == D_STORE_RESULT_OK) {
            initStoreFilePath(storeMMF, NULL, NULL);

            /* create Handle for Memory Mapped File */
            os_mmfAttrInit(&mmfAttr);
            if(store->config->persistentMMFStoreAddress != NULL)
            {
                mmfAttr.map_address = (void*)store->config->persistentMMFStoreAddress;
            }

            storeMMF->mmfHandle = os_mmfCreateHandle (storeMMF->storeFilePath, &mmfAttr);
            mmfExistingFile = os_mmfFileExist(storeMMF->mmfHandle);
            if (mmfExistingFile) {
                /* open existing Memory Mapped File */
                result = openExistingMMF(storeMMF);
            } else {
                /* create new Memory Mapped File */
                result = createNewMMF(storeMMF);
            }

            if (result == D_STORE_RESULT_OK) {
                storeMMF->opened = TRUE;
            } else {
                assert(FALSE);
                os_mmfDestroyHandle(storeMMF->mmfHandle);
                storeMMF->mmfHandle = NULL;
                os_free(storeMMF->storeFilePath);
                storeMMF->storeFilePath = NULL;
            }
        }
        storeMMF->actionsInProgress = 0;
        os_condAttrInit(&actionAttr);
        actionAttr.scopeAttr = OS_SCOPE_PRIVATE;
        os_condInit(&(storeMMF->actionCondition), &(d_lock(store)->lock), &actionAttr);

        d_lockUnlock(d_lock(store));
    } else {
        d_storeReport(store, D_LEVEL_SEVERE, "Supplied parameter(s) not valid.\n");
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeCloseMMF(
    d_store store)
{
    d_storeResult result;
    d_storeMMF storeMMF;

    assert(d_objectIsValid(d_object(store), D_STORE));
    assert(d_store(store)->type == D_STORE_TYPE_MEM_MAPPED_FILE);

    if(store != NULL){
        storeMMF = d_storeMMF(store);

        d_lockLock(d_lock(store));

        if(storeMMF->opened == FALSE) {
            result = D_STORE_RESULT_PRECONDITION_NOT_MET;
        } else {
            d_storeMMFKernelUpdateQuality(storeMMF->storeKernel, v_timeGet());
            storeMMF->opened = FALSE;
            storeMMF->storeKernel = NULL;

            destroyGroupList(storeMMF);

            os_mmfSync(storeMMF->mmfHandle);
            c_mmSuspend(c_baseMM(storeMMF->base));
            os_mmfDetach(storeMMF->mmfHandle);
            os_mmfClose(storeMMF->mmfHandle);
            os_mmfDestroyHandle(storeMMF->mmfHandle);
            storeMMF->mmfHandle = NULL;

            os_free(storeMMF->storeFilePath);
            storeMMF->storeFilePath = NULL;

            result = D_STORE_RESULT_OK;
        }
        d_lockUnlock(d_lock(store));
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeActionStartMMF(
    const d_store store)
{
    d_storeResult result;
    os_result osr;

    assert(d_objectIsValid(d_object(store), D_STORE));
    assert(d_store(store)->type == D_STORE_TYPE_MEM_MAPPED_FILE);

    if(store != NULL){
        d_lockLock(d_lock(store));
        osr = os_resultSuccess;

        while((d_storeMMF(store)->actionsInProgress != 0) && (osr == os_resultSuccess)){
            osr = os_condWait(&(d_storeMMF(store)->actionCondition), &(d_lock(store)->lock));
        }
        if((d_storeMMF(store)->actionsInProgress == 0) && (osr == os_resultSuccess)){
            /* Code used to benchmark MMF store (DDS1582) */
            if (store->config && (((c_ulong)D_LEVEL_FINEST) >= ((c_ulong)store->config->tracingVerbosityLevel)))
            {
                action_started = TRUE;
                first_time.tv_sec = 0;
                first_time.tv_nsec = 0;
                last_time.tv_sec = 0;
                last_time.tv_nsec = 0;
            }
            /* end of benchmark code */
            /*d_storeReport(store, D_LEVEL_FINEST, "d_storeActionStartMMF.\n");*/
            result = D_STORE_RESULT_OK;
        } else {
            result = D_STORE_RESULT_ERROR;

            if(osr != os_resultSuccess){
                OS_REPORT_1(OS_ERROR, "d_storeActionStartMMF", 0,
                  "os_condWait returned %d", osr);
                assert(FALSE);
            }
            if(d_storeMMF(store)->actionsInProgress != 0){
                OS_REPORT_1(OS_ERROR, "d_storeActionStartMMF", 0,
                  "d_storeMMF(store)->actionsInProgress == %d",
                  d_storeMMF(store)->actionsInProgress);
                assert(FALSE);
            }
        }
        d_lockUnlock(d_lock(store));
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeActionStopMMF(
    const d_store store)
{
    d_storeResult result;
    os_time diff_time;
    os_result osr;
    d_durability durability;

    assert(d_objectIsValid(d_object(store), D_STORE));
    assert(store->type == D_STORE_TYPE_MEM_MAPPED_FILE);

    if(store != NULL){
        osr = os_resultSuccess;

        d_lockLock(d_lock(store));

        while((d_storeMMF(store)->actionsInProgress != 0) && (osr == os_resultSuccess)){
            osr = os_condWait(&(d_storeMMF(store)->actionCondition), &(d_lock(store)->lock));
        }

        if((d_storeMMF(store)->actionsInProgress == 0) && (osr == os_resultSuccess)){
            durability = d_adminGetDurability(store->admin);

            if(d_durabilityGetState(durability) == D_STATE_COMPLETE){
                /* Only sync to disk if alignment is complete. Otherwise the
                 * service will revert to the backup after restart anyway.
                 */
                os_mmfSync(d_storeMMF(store)->mmfHandle);
            }

            /* Code used to benchmark MMF store (DDS1582) */
            if (store->config && (((c_ulong)D_LEVEL_FINEST) >= ((c_ulong)store->config->tracingVerbosityLevel)))
            {
                action_started = FALSE;
                d_storeReport(store, D_LEVEL_FINEST,
                        "Start time %d.%09d\n",
                        first_time.tv_sec, first_time.tv_nsec);
                d_storeReport(store, D_LEVEL_FINEST,
                        "End time   %d.%09d\n",
                        last_time.tv_sec, last_time.tv_nsec);
                diff_time = os_timeSub(last_time, first_time);
                d_storeReport(store, D_LEVEL_FINEST,
                        "Diff time  %d.%09d seconds \n",
                        diff_time.tv_sec, diff_time.tv_nsec);
            }
            /* end of benchmark code */
            /*d_storeReport(store, D_LEVEL_FINEST, "d_storeActionStopMMF.\n");*/
            result = D_STORE_RESULT_OK;
        } else {
            result = D_STORE_RESULT_ERROR;

            if(osr != os_resultSuccess){
                OS_REPORT_1(OS_ERROR, "d_storeActionStartMMF", 0,
                  "os_condWait returned %d", osr);
                assert(FALSE);
            }
            if(d_storeMMF(store)->actionsInProgress != 0){
                OS_REPORT_1(OS_ERROR, "d_storeActionStartMMF", 0,
                  "d_storeMMF(store)->actionsInProgress == %d",
                  d_storeMMF(store)->actionsInProgress);
                assert(FALSE);
            }
        }
        d_lockUnlock(d_lock(store));
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeGroupsReadMMF(
    const d_store store,
    d_groupList *list)
{
    d_storeResult result;
    d_storeMMF storeMMF;

    assert(d_objectIsValid(d_object(store), D_STORE));
    assert(store->type == D_STORE_TYPE_MEM_MAPPED_FILE);

    if(store != NULL){
        d_lockLock(d_lock(store));

        storeMMF = d_storeMMF(store);

        if(storeMMF->opened == FALSE){
            result = D_STORE_RESULT_PRECONDITION_NOT_MET;
        } else if(storeMMF->storeKernel == NULL ||
                storeMMF->storeKernel->groups == NULL){
            result = D_STORE_RESULT_ILL_PARAM;
        } else {
            /* update groupList before to return it */
            updateGroupList(storeMMF);
            *list = storeMMF->groups;
            result = D_STORE_RESULT_OK;
        }
        d_lockUnlock(d_lock(store));
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeGroupInjectMMF(
    const d_store store,
    const c_char* partition,
    const c_char* topic,
    const u_participant participant,
    d_group *group)
{
    d_groupInfo groupInfo;
    d_storeResult result;

    assert(d_objectIsValid(d_object(store), D_STORE));
    assert(store->type == D_STORE_TYPE_MEM_MAPPED_FILE);

    *group = NULL;

    if(store != NULL){
        d_lockLock(d_lock(store));

        if(d_storeMMF(store)->opened == FALSE) {
            result = D_STORE_RESULT_PRECONDITION_NOT_MET;
        } else if((participant == NULL) || (partition == NULL) || (topic == NULL) || (group == NULL)) {
            result = D_STORE_RESULT_ILL_PARAM;
        } else {
            groupInfo = d_storeMMFKernelGetGroupInfo(
                    d_storeMMFGetKernel(store), partition, topic);

            if(groupInfo) {
                result = d_groupInfoInject(groupInfo, store, participant, group);
                c_free(groupInfo);
            } else {
                result = D_STORE_RESULT_PRECONDITION_NOT_MET;
            }
        }
        d_lockUnlock(d_lock(store));
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeGroupStoreMMF(
    const d_store store,
    const d_group dgroup)
{
    d_storeResult result;

    assert(d_objectIsValid(d_object(store), D_STORE));
    assert(store->type == D_STORE_TYPE_MEM_MAPPED_FILE);

    if(store != NULL) {
        d_lockLock(d_lock(store));

        if(d_storeMMF(store)->opened  == FALSE) {
            result = D_STORE_RESULT_PRECONDITION_NOT_MET;
        } else if(dgroup == NULL) {
            result = D_STORE_RESULT_ILL_PARAM;
        } else {
            result = d_storeMMFKernelAddGroupInfo(d_storeMMFGetKernel(store),
                    dgroup);
        }
        d_lockUnlock(d_lock(store));
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeMessageStoreMMF(
    const d_store store,
    const v_groupAction msg)
{
    d_storeResult result;
    d_groupInfo group;
    d_sample sample;

    /* Code used to benchmark MMF store (DDS1582) */
    if (store->config && (((c_ulong)D_LEVEL_FINEST) >= ((c_ulong)store->config->tracingVerbosityLevel)))
    {
        if(action_started)
        {
                first_time = os_timeGet();
                action_started = FALSE;
        }
    }
    /* end of benchmark code */

    assert(d_objectIsValid(d_object(store), D_STORE));
    assert(store->type == D_STORE_TYPE_MEM_MAPPED_FILE);

    if(store != NULL) {
        d_lockLock(d_lock(store));

        if(d_storeMMF(store)->opened == FALSE) {
            result = D_STORE_RESULT_PRECONDITION_NOT_MET;
        } else if((msg == NULL) || (msg->group == NULL) || (msg->message == NULL)) {
            result = D_STORE_RESULT_ILL_PARAM;
        } else {
            group = d_storeMMFKernelGetGroupInfo(d_storeMMFGetKernel(store),
                    v_entity(msg->group->partition)->name,
                    v_entity(msg->group->topic)->name);

            if(group){
                d_storeMMF(store)->actionsInProgress++;
                d_lockUnlock(d_lock(store));
                /*Time consuming action that can be performed outside lock*/
                sample = d_groupInfoSampleNew(group, NULL, msg->message);
                d_lockLock(d_lock(store));
                d_storeMMF(store)->actionsInProgress--;

                if(d_storeMMF(store)->actionsInProgress == 0){
                    os_condSignal(&(d_storeMMF(store)->actionCondition));
                }
                result = d_groupInfoWrite(group, store, msg, sample);
                /*os_mmfSync(d_storeMMF(store)->mmfHandle); -- only sync on stop action. */
                c_free(group);
            } else {
                result = D_STORE_RESULT_PRECONDITION_NOT_MET;
            }
        }
        d_lockUnlock(d_lock(store));
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }

    /* Code used to benchmark MMF store (DDS1582) */
    if (store->config && (((c_ulong)D_LEVEL_FINEST) >= ((c_ulong)store->config->tracingVerbosityLevel)))
    {
        last_time = os_timeGet();
    }
    /* end of benchmark code */

    return result;
}

d_storeResult
d_storeInstanceDisposeMMF(
    const d_store store,
    const v_groupAction msg)
{
    d_storeResult result;
    d_groupInfo group;
    d_sample sample;

    assert(d_objectIsValid(d_object(store), D_STORE));
    assert(store->type == D_STORE_TYPE_MEM_MAPPED_FILE);

    if(store != NULL) {
        d_lockLock(d_lock(store));

        if(d_storeMMF(store)->opened == FALSE) {
            result = D_STORE_RESULT_PRECONDITION_NOT_MET;
        } else if((msg == NULL) || (msg->group == NULL) || (msg->message == NULL)) {
            result = D_STORE_RESULT_ILL_PARAM;
        } else {
            group = d_storeMMFKernelGetGroupInfo(d_storeMMFGetKernel(store),
                    v_entity(msg->group->partition)->name,
                    v_entity(msg->group->topic)->name);

            if(group){

                d_storeMMF(store)->actionsInProgress++;
                d_lockUnlock(d_lock(store));
                /*Time consuming action that can be performed outside lock*/
                sample = d_groupInfoSampleNew(group, NULL, msg->message);

                d_lockLock(d_lock(store));
                d_storeMMF(store)->actionsInProgress--;

                if(d_storeMMF(store)->actionsInProgress == 0){
                    os_condSignal(&(d_storeMMF(store)->actionCondition));
                }
                result = d_groupInfoWrite(group, store, msg, sample);
                /*os_mmfSync(d_storeMMF(store)->mmfHandle);*/
                c_free(group);
            } else {
                result = D_STORE_RESULT_PRECONDITION_NOT_MET;
            }
        }
        d_lockUnlock(d_lock(store));
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeInstanceExpungeMMF(
    const d_store store,
    const v_groupAction msg)
{
    d_storeResult result;
    d_groupInfo group;

    assert(d_objectIsValid(d_object(store), D_STORE));
    assert(store->type == D_STORE_TYPE_MEM_MAPPED_FILE);

    if(store != NULL) {
        d_lockLock(d_lock(store));

        if(d_storeMMF(store)->opened == FALSE) {
            result = D_STORE_RESULT_PRECONDITION_NOT_MET;
        } else if((msg == NULL) || (msg->group == NULL) || (msg->message == NULL)) {
            result = D_STORE_RESULT_ILL_PARAM;
        } else {
            group = d_storeMMFKernelGetGroupInfo(d_storeMMFGetKernel(store),
                    v_entity(msg->group->partition)->name,
                    v_entity(msg->group->topic)->name);

            if(group){
                result = d_groupInfoExpungeInstance(group, store, msg);
                /* os_mmfSync(d_storeMMF(store)->mmfHandle); */
                c_free(group);
            } else {
                result = D_STORE_RESULT_PRECONDITION_NOT_MET;
            }
        }
        d_lockUnlock(d_lock(store));

    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeMessageExpungeMMF(
    const d_store store,
    const v_groupAction msg)
{
    d_storeResult result;
    d_groupInfo group;

    assert(d_objectIsValid(d_object(store), D_STORE));
    assert(store->type == D_STORE_TYPE_MEM_MAPPED_FILE);

    if(store != NULL) {
        d_lockLock(d_lock(store));

        if(d_storeMMF(store)->opened == FALSE) {
            result = D_STORE_RESULT_PRECONDITION_NOT_MET;
        } else if((msg == NULL) || (msg->group == NULL) || (msg->message == NULL)) {
            result = D_STORE_RESULT_ILL_PARAM;
        } else {
            group = d_storeMMFKernelGetGroupInfo(d_storeMMFGetKernel(store),
                    v_entity(msg->group->partition)->name,
                    v_entity(msg->group->topic)->name);

            if(group){
                result = d_groupInfoExpungeSample(group, store, msg);
                /* os_mmfSync(d_storeMMF(store)->mmfHandle); */
                c_free(group);
            } else {
                result = D_STORE_RESULT_PRECONDITION_NOT_MET;
            }
        }
        d_lockUnlock(d_lock(store));
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeDeleteHistoricalDataMMF(
    const d_store store,
    const v_groupAction msg)
{
    d_storeResult result;
    d_groupInfo group;

    assert(d_objectIsValid(d_object(store), D_STORE));
    assert(store->type == D_STORE_TYPE_MEM_MAPPED_FILE);

    if(store != NULL) {
        d_lockLock(d_lock(store));

        if(d_storeMMF(store)->opened == FALSE) {
            result = D_STORE_RESULT_PRECONDITION_NOT_MET;
        } else if((msg == NULL) || (msg->group == NULL)) {
            result = D_STORE_RESULT_ILL_PARAM;
        } else {
            group = d_storeMMFKernelGetGroupInfo(d_storeMMFGetKernel(store),
                    v_entity(msg->group->partition)->name,
                    v_entity(msg->group->topic)->name);

            if(group){
                result = d_groupInfoDeleteHistoricalData(group, store, msg);
                /* os_mmfSync(d_storeMMF(store)->mmfHandle); */
                c_free(group);
            } else {
                result = D_STORE_RESULT_PRECONDITION_NOT_MET;
            }
        }
        d_lockUnlock(d_lock(store));
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeInstanceRegisterMMF(
    const d_store store,
    const v_groupAction msg)
{
    d_storeResult result;

    assert(d_objectIsValid(d_object(store), D_STORE));
    assert(store->type == D_STORE_TYPE_MEM_MAPPED_FILE);

    if(store != NULL) {
        d_lockLock(d_lock(store));

        if(d_storeMMF(store)->opened == FALSE) {
            result = D_STORE_RESULT_PRECONDITION_NOT_MET;
        } else if((msg == NULL) || (msg->group == NULL) || (msg->message == NULL)) {
            result = D_STORE_RESULT_ILL_PARAM;
        } else {
            result = D_STORE_RESULT_OK;
        }
        d_lockUnlock(d_lock(store));
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeInstanceUnregisterMMF(
    const d_store store,
    const v_groupAction msg)
{
    d_storeResult result;

    assert(d_objectIsValid(d_object(store), D_STORE));
    assert(store->type == D_STORE_TYPE_MEM_MAPPED_FILE);

    if(store != NULL) {
        d_lockLock(d_lock(store));

        if(d_storeMMF(store)->opened == FALSE) {
            result = D_STORE_RESULT_PRECONDITION_NOT_MET;
        } else if((msg == NULL) || (msg->group == NULL) || (msg->message == NULL)) {
            result = D_STORE_RESULT_ILL_PARAM;
        } else {
            result = D_STORE_RESULT_OK;
        }
        d_lockUnlock(d_lock(store));
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeMessagesInjectMMF(
    const d_store store,
    const d_group group)
{
    d_storeResult result;
    d_groupInfo groupInfo;
    d_partition partition;
    d_topic topic;

    assert(d_objectIsValid(d_object(store), D_STORE));
    assert(d_store(store)->type == D_STORE_TYPE_MEM_MAPPED_FILE);

    if(store != NULL) {
        d_lockLock(d_lock(store));

        if(d_storeMMF(store)->opened == FALSE) {
            result = D_STORE_RESULT_PRECONDITION_NOT_MET;
        } else if(group == NULL) {
            result = D_STORE_RESULT_ILL_PARAM;
        } else {
            partition = d_groupGetPartition(group);
            topic = d_groupGetTopic(group);
            groupInfo = d_storeMMFKernelGetGroupInfo(
                    d_storeMMFGetKernel(store), partition, topic);
            os_free(partition);
            os_free(topic);

            if(groupInfo){
                result = d_groupInfoDataInject(groupInfo, store, group);
                c_free(groupInfo);
            } else {
                result = D_STORE_RESULT_PRECONDITION_NOT_MET;
            }
        }
        d_lockUnlock(d_lock(store));

    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeOptimizeGroupMMF(
    const d_store store,
    const d_group group)
{
    d_storeResult result;

    assert(d_objectIsValid(d_object(store), D_STORE));
    assert(d_store(store)->type == D_STORE_TYPE_MEM_MAPPED_FILE);

    if(store && group){
        result = D_STORE_RESULT_OK;
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}


d_storeResult
d_storeNsIsCompleteMMF (
    const d_store store,
    const d_nameSpace nameSpace,
    c_bool* isComplete)
{
    d_storeResult result;

    assert(d_objectIsValid(d_object(store), D_STORE));
    assert(d_store(store)->type == D_STORE_TYPE_MEM_MAPPED_FILE);

    if(store){
        d_lockLock(d_lock(store));
        result = d_storeMMFKernelIsNameSpaceComplete(
                d_storeMMFGetKernel(store), nameSpace, isComplete);
        d_lockUnlock(d_lock(store));
    } else{
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeNsMarkCompleteMMF (
    const d_store store,
    const d_nameSpace nameSpace,
    c_bool isComplete)
{
    d_storeResult result;

    assert(d_objectIsValid(d_object(store), D_STORE));
    assert(d_store(store)->type == D_STORE_TYPE_MEM_MAPPED_FILE);

    if(store){
        d_lockLock(d_lock(store));

        result = d_storeMMFKernelMarkNameSpaceComplete(
                d_storeMMFGetKernel(store), nameSpace, isComplete);
        os_mmfSync(d_storeMMF(store)->mmfHandle);
        d_lockUnlock(d_lock(store));
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeRestoreBackupMMF (
    const d_store store,
    const d_nameSpace nameSpace)
{
    d_storeResult result;
    d_storeMMF persistentStore;

    assert(d_objectIsValid(d_object(store), D_STORE));
    assert(store->type == D_STORE_TYPE_MEM_MAPPED_FILE);

    if(store != NULL) {
        persistentStore = d_storeMMF(store);
        d_lockLock(d_lock(persistentStore));

        if(persistentStore->opened == FALSE) {
            result = D_STORE_RESULT_PRECONDITION_NOT_MET;
        } else if(nameSpace == NULL) {
            result = D_STORE_RESULT_ILL_PARAM;
        } else {
            result = d_storeMMFKernelBackupRestore(d_storeMMFGetKernel(store),
                    store, nameSpace);
            os_mmfSync(d_storeMMF(store)->mmfHandle);
        }
        d_lockUnlock(d_lock(persistentStore));
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeBackupMMF(
    const d_store store,
    const d_nameSpace nameSpace)
{
    d_storeResult result;
    d_storeMMF persistentStore;

    assert(d_objectIsValid(d_object(store), D_STORE));
    assert(store->type == D_STORE_TYPE_MEM_MAPPED_FILE);

    if(store != NULL) {
        persistentStore = d_storeMMF(store);
        d_lockLock(d_lock(persistentStore));

        if(persistentStore->opened == FALSE) {
            result = D_STORE_RESULT_PRECONDITION_NOT_MET;
        } else if(nameSpace == NULL) {
            result = D_STORE_RESULT_ILL_PARAM;
        } else {
            result = d_storeMMFKernelBackup(d_storeMMFGetKernel(store),
                    store, nameSpace);
            os_mmfSync(d_storeMMF(store)->mmfHandle);
        }
        d_lockUnlock(d_lock(persistentStore));
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeGetQualityMMF(
    const d_store store,
    const d_nameSpace nameSpace,
    d_quality* quality)
{
    d_storeResult result;

    assert(d_objectIsValid(d_object(store), D_STORE));
    assert(store->type == D_STORE_TYPE_MEM_MAPPED_FILE);

    if(store != NULL){
        d_lockLock(d_lock(store));
        if(d_storeMMF(store)->opened == FALSE) {
            result = D_STORE_RESULT_PRECONDITION_NOT_MET;
        } else if((quality == NULL) || (nameSpace == NULL)) {
            result = D_STORE_RESULT_ILL_PARAM;
        } else {
            result = d_storeMMFKernelGetQuality(d_storeMMFGetKernel(store),
                    nameSpace, quality);
        }
        d_lockUnlock(d_lock(store));
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

d_storeResult
d_storeCreatePersistentSnapshotMMF(
    const d_store store,
    const c_char* partitionExpr,
    const c_char* topicExpr,
    const c_char* uri)
{
    d_storeResult result;
    c_char *storeDir, *originalPath;
    d_storeMMF storeMMF;
    os_mmfAttr mmfAttr;

    assert(store);
    assert(topicExpr);
    assert(partitionExpr);
    assert(uri);
    assert(d_objectIsValid(d_object(store), D_STORE));
    assert(store->type == D_STORE_TYPE_MEM_MAPPED_FILE);

    if(store){
        if(d_storeMMF(store)->opened == FALSE) {
            result = D_STORE_RESULT_PRECONDITION_NOT_MET;
        } else {
            storeMMF = d_storeMMF(store);
            storeDir = d_storeDirNew(store, uri);

            if(storeDir){
                d_lockLock(d_lock(store));

                os_mmfAttrInit(&mmfAttr);

                if(store->config->persistentMMFStoreAddress != NULL){
                    mmfAttr.map_address =
                        (void*)store->config->persistentMMFStoreAddress;
                }
                originalPath = storeMMF->storeFilePath;
                storeMMF->storeFilePath = NULL;
                os_mmfSync(storeMMF->mmfHandle);
                c_mmSuspend(c_baseMM(storeMMF->base));
                os_mmfDetach(storeMMF->mmfHandle);
                os_mmfClose(storeMMF->mmfHandle);
                os_mmfDestroyHandle(storeMMF->mmfHandle);
                storeMMF->mmfHandle = NULL;

                initStoreFilePath(storeMMF, (c_char*)uri, MMF_STORE_SNAPSHOT);
                os_remove(storeMMF->storeFilePath);

                result = d_storeCopyFile(originalPath, storeMMF->storeFilePath);
                os_free(originalPath);

                if(result == D_STORE_RESULT_OK){
                    storeMMF->mmfHandle = os_mmfCreateHandle (
                            storeMMF->storeFilePath, &mmfAttr);

                    result = openExistingMMF(storeMMF);

                    if(result == D_STORE_RESULT_OK){
                        result = d_storeMMFKernelDeleteNonMatchingGroups(
                                d_storeMMFGetKernel(store),
                                (c_string)partitionExpr, (c_string)topicExpr);

                        os_mmfSync(storeMMF->mmfHandle);
                        c_mmSuspend(c_baseMM(storeMMF->base));
                        os_mmfDetach(storeMMF->mmfHandle);
                        os_mmfClose(storeMMF->mmfHandle);
                    } else {
                        d_storeReport(store, D_LEVEL_SEVERE,
                            "Could open file '%s' for snapshot writing.\n",
                            storeMMF->storeFilePath);
                    }
                    os_mmfDestroyHandle(storeMMF->mmfHandle);
                    storeMMF->mmfHandle = NULL;
                } else {
                    d_storeReport(store, D_LEVEL_SEVERE,
                        "Could not create file '%s' for snapshot.\n",
                        storeMMF->storeFilePath);
                    result = D_STORE_RESULT_IO_ERROR;
                }
                os_free(storeMMF->storeFilePath);

                initStoreFilePath(storeMMF, NULL, NULL);
                storeMMF->mmfHandle = os_mmfCreateHandle (
                        storeMMF->storeFilePath, &mmfAttr);
                openExistingMMF(storeMMF);

                d_lockUnlock(d_lock(store));
            } else {
                d_storeReport(store, D_LEVEL_SEVERE,
                        "Could not create directory '%s' for snapshot.\n",
                        uri);

                result = D_STORE_RESULT_IO_ERROR;
            }
        }
    } else {
        result = D_STORE_RESULT_ILL_PARAM;
    }
    return result;
}

