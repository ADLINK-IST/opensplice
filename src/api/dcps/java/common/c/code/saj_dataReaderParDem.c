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

#include "saj__fooDataReader.h"
#include "saj__readerContext.h"
#include "saj_copyOut.h"
#include "v_dataReaderInstance.h"
#include "v_dataReaderSample.h"
#include "v_dataViewSample.h"
#include "v_state.h"
#include "os_report.h"
#include "saj_dataReaderParDem.h"
#include "cmn_samplesList.h"
#include "os_atomics.h"

typedef struct {
    C_STRUCT(saj_dstInfo) dst;
    sajParDemContext pdc;
    jobject info_element;
    JNIEnv *env;
    saj_returnCode retcode;
} copy_data_arg;


static void
copy_data(
    void *sample,
    cmn_sampleInfo info,
    void *arg)
{
    copy_data_arg *copy_arg = (copy_data_arg *)arg;

    copy_arg->retcode = saj_sampleInfoCopyOut(copy_arg->env, info, &copy_arg->info_element);
    copy_arg->pdc->copy_info.copyOut(sample, &copy_arg->dst);
}

static void
saj_dataReaderParallelCopy(
    JNIEnv *env,
    sajParDemContext pdc)
{
    os_uint32 i;
    os_int32  len;
    u_result uResult;
    copy_data_arg copy_arg;

    assert(pdc);
    assert(pdc->heuristics.block > 0U);
    assert(pdc->copy_info.len);
    assert(pdc->copy_info.dataSeq);
    assert(pdc->copy_info.infoSeq);
    assert(pdc->copy_info.copyCache);
    assert(pdc->copy_info.copyOut);
    assert(pdc->copy_info.samplesList);
    assert(pdc->copy_info.jreader);
    assert(pdc->copy_info.uEntity);

    copy_arg.pdc = pdc;
    copy_arg.env = env;
    copy_arg.dst.javaEnv = env;
    copy_arg.dst.copyProgram = pdc->copy_info.copyCache;
    copy_arg.dst.jreader = pdc->copy_info.jreader;

    while((i = pa_inc32_nv(&pdc->copy_info.nextIndex) - 1) < pdc->copy_info.len){
        copy_arg.dst.javaObject = GET_OBJECTARRAY_ELEMENT(env, pdc->copy_info.dataSeq, i);
        copy_arg.info_element = GET_OBJECTARRAY_ELEMENT(env, pdc->copy_info.infoSeq, i);
        copy_arg.retcode = SAJ_RETCODE_OK;

        uResult = u_readerProtectCopyOutEnter(SAJ_VOIDP(pdc->copy_info.uEntity));
        if (uResult == U_RESULT_OK) {
            len = cmn_samplesList_read(pdc->copy_info.samplesList, i, copy_data, &copy_arg);
            u_readerProtectCopyOutExit(SAJ_VOIDP(pdc->copy_info.uEntity));
            if ((copy_arg.retcode != SAJ_RETCODE_OK) || (len != 1)) {
                OS_REPORT(OS_ERROR, "DataReader::saj_dataReaderParallelCopy", 0,
                        "cmn_samplesList_read failed.");
            }
        }
        SET_OBJECTARRAY_ELEMENT(env, pdc->copy_info.infoSeq, i, copy_arg.info_element);
        SET_OBJECTARRAY_ELEMENT(env, pdc->copy_info.dataSeq, i, copy_arg.dst.javaObject);
        DELETE_LOCAL_REF(env, copy_arg.dst.javaObject);
        DELETE_LOCAL_REF(env, copy_arg.info_element);
    }
    CATCH_EXCEPTION:;
}

/* Call with pcd->mtx locked */
static void pdc_readyCndBroadcast(
    JNIEnv *env,
    sajParDemContext pdc)
{
    assert(env);
    assert(pdc);

    DELETE_WEAK_GLOBAL_REF(env, pdc->copy_info.dataSeq);
    DELETE_WEAK_GLOBAL_REF(env, pdc->copy_info.infoSeq);
    pdc->copy_info.dataSeq = NULL;
    pdc->copy_info.infoSeq = NULL;
    pdc->parity = !pdc->parity;
    os_condBroadcast(&pdc->readyCnd);
}

int
saj_fooDataReaderParallelDemarshallingMain(
    JNIEnv * env,
    sajParDemContext pdc)
{
    os_ushort mustTerminate;

    assert(env);
    assert(pdc);

    os_mutexLock(&pdc->superMtx);
    os_mutexLock(&pdc->mtx);
    pdc->nrofWorkers++;
    os_condSignal(&pdc->workerCnd);
    os_mutexUnlock(&pdc->mtx);
    os_mutexUnlock(&pdc->superMtx);

    do {
        os_mutexLock(&pdc->mtx);
        while(!pdc->terminate && !pdc->copy_info.dataSeq){
            os_condWait(&pdc->startCnd, &pdc->mtx);
        }
        mustTerminate = pdc->terminate;
        os_mutexUnlock(&pdc->mtx);

        if(mustTerminate) break;

        saj_dataReaderParallelCopy(env, pdc);

        os_mutexLock(&pdc->mtx);
        pdc->readyCnt++;
        if(pdc->readyCnt > pdc->nrofWorkers){
            /* Main thread was already done, so notify */
            pdc_readyCndBroadcast(env, pdc);
        } else {
            const os_boolean parity = pdc->parity;
            while(parity == pdc->parity){
                os_condWait(&pdc->readyCnd, &pdc->mtx);
            }
        }
        os_mutexUnlock(&pdc->mtx);
    } while(TRUE);

    os_mutexLock(&pdc->superMtx);
    os_mutexLock(&pdc->mtx);
    pdc->nrofWorkers--;
    os_mutexUnlock(&pdc->mtx);
    os_mutexUnlock(&pdc->superMtx);

    return SAJ_RETCODE_OK;
}

static void
saj_parDemContext_signal_terminate(
    sajParDemContext pdc)
{
    assert(pdc);

    os_mutexLock(&pdc->superMtx);
    os_mutexLock(&pdc->mtx);
    pdc->terminate = 1U;
    os_condBroadcast(&pdc->startCnd);
    os_mutexUnlock(&pdc->mtx);
    os_mutexUnlock(&pdc->superMtx);
}

static void
saj_parDemContext_free(
    sajParDemContext pdc)
{
    if(pdc){
        os_condDestroy(&pdc->readyCnd);
        os_condDestroy(&pdc->startCnd);
        os_condDestroy(&pdc->workerCnd);
        os_mutexDestroy(&pdc->mtx);
        os_mutexDestroy(&pdc->superMtx);
        os_free(pdc);
    }
}

static sajParDemContext
saj_parDemContext_new()
{
    sajParDemContext pdc;

    pdc = os_malloc(sizeof(*pdc));

    pdc->nrofWorkers = 0U;
    pdc->heuristics.threshold = 2U; /* Parallelization only helps for at least 2 samples */
    pdc->heuristics.block = 1U;     /* A block of 1 is the bare minimum */
    pa_st32 (&pdc->copy_info.nextIndex, 0U);
    pdc->copy_info.len = 0U;
    pdc->copy_info.dataSeq = NULL;
    pdc->copy_info.infoSeq = NULL;
    pdc->copy_info.copyCache = NULL;
    pdc->copy_info.copyOut = NULL;
    pdc->copy_info.samplesList = NULL;
    pdc->copy_info.uEntity = 0;

    if(os_mutexInit(&pdc->superMtx, NULL) != os_resultSuccess) goto err_supermtx_init;
    if(os_mutexInit(&pdc->mtx, NULL) != os_resultSuccess) goto err_mtx_init;
    if(os_condInit(&pdc->workerCnd, &pdc->mtx, NULL) != os_resultSuccess) goto err_workercnd_init;
    if(os_condInit(&pdc->startCnd, &pdc->mtx, NULL) != os_resultSuccess) goto err_startcnd_init;
    if(os_condInit(&pdc->readyCnd, &pdc->mtx, NULL) != os_resultSuccess) goto err_readycnd_init;

    pdc->readyCnt = 0U;
    pdc->parity = OS_FALSE;
    pdc->terminate = 0u;

    return pdc;

/* Error-handling */
err_readycnd_init:
    (void) os_condDestroy(&pdc->startCnd);
err_startcnd_init:
    (void) os_condDestroy(&pdc->workerCnd);
err_workercnd_init:
    (void) os_mutexDestroy(&pdc->mtx);
err_mtx_init:
    (void) os_mutexDestroy(&pdc->superMtx);
err_supermtx_init:
    os_free(pdc);
    return NULL;
}


saj_returnCode
saj_fooDataReaderSetParallelReadThreadCount(
    JNIEnv * env,
    jobject jdatareader,
    int value)
{
    sajParDemContext pdc = NULL;
    jint started;

    if(saj_read_parallelDemarshallingContext_address(env, jdatareader, &pdc) != SAJ_RETCODE_OK){
        goto err_getset_address;
    }

    if(pdc){
        /* Signal eventual current threads to shutdown */
        saj_parDemContext_signal_terminate(pdc);
        /* Join threads */
        CALL_VOID_METHOD(env, jdatareader, GET_CACHED(dataReaderImplClassJoinWorkers_mid));
        /* Assert that all threads are actually stopped, so the terminate flag is
         * reset (last  one to stop will do this). */
        assert(pdc->nrofWorkers == 0);
        /* pdc can be reused if new set number is >1, reset terminate flag */
        pdc->terminate = 0;

        /* If the requested size is 0 or 1, this will both result in a single-threaded
         * copy, so we don't have to start any extra threads. If previously there
         * were threads created, we can now cleanup the context as well. */
        if(value <= 1){
            saj_parDemContext_free(pdc);
            pdc = NULL;
            if(saj_write_parallelDemarshallingContext_address(env, jdatareader, NULL) != SAJ_RETCODE_OK){
                goto err_getset_address;
            }
        }
    }

    if(value > 1){
        if(!pdc){
            /* Create and assign the parallelDemarshalling context. It
             * is OK if saj_parDemContextNew fails. The pointer will
             * then be set to 0. */
            if((pdc = saj_parDemContext_new()) == NULL) { goto err_out_of_resources; }
            if(saj_write_parallelDemarshallingContext_address(env, jdatareader, pdc) != SAJ_RETCODE_OK){
                goto err_getset_address;
            }
        }

        started = CALL_INT_METHOD(env, jdatareader, GET_CACHED(dataReaderImplClassStartWorkers_mid), value > 1 ? value - 1 : 0);

        os_mutexLock(&pdc->mtx);
        while (started > pdc->nrofWorkers) {
            os_condWait(&pdc->workerCnd, &pdc->mtx);
        }
        os_mutexUnlock(&pdc->mtx);
    }
    return SAJ_RETCODE_OK;

err_out_of_resources:
    return SAJ_RETCODE_OUT_OF_RESOURCES;

err_getset_address:
CATCH_EXCEPTION:
    return SAJ_RETCODE_ERROR;
}

struct flushCopyArg {
    C_STRUCT(saj_dstInfo) dst;
    JNIEnv *env;
    jobject dataSeq;
    jobject infoSeq;
    void (*copyOut)(void *, void *);
    os_int32 i;
    saj_returnCode retcode;
};

static void
flushCopy(
    void *sample,
    cmn_sampleInfo sampleInfo,
    void *arg)
{
    JNIEnv *env;
    struct flushCopyArg *a = (struct flushCopyArg *)arg;
    jobject info_element;

    env = a->env;
    a->dst.javaObject = GET_OBJECTARRAY_ELEMENT(env, a->dataSeq, a->i);
    a->copyOut(sample, &a->dst);
    SET_OBJECTARRAY_ELEMENT(env, a->dataSeq, a->i, a->dst.javaObject);
    DELETE_LOCAL_REF(env, a->dst.javaObject);

    info_element = GET_OBJECTARRAY_ELEMENT(env, a->infoSeq, a->i);
    a->retcode = saj_sampleInfoCopyOut(env, sampleInfo, &info_element);
    SET_OBJECTARRAY_ELEMENT(env, a->infoSeq, a->i, info_element);
    DELETE_LOCAL_REF(env, info_element);

    a->i++;

    return;
    CATCH_EXCEPTION: a->retcode = SAJ_RETCODE_ERROR;
}


saj_returnCode
saj_dataReaderParDemStack_copy_out(
    saj_readerContext *ctx)
{
    unsigned int i;
    unsigned int len;
    unsigned int copyLen;
    unsigned int singleThreadedCopy = 1U;
    int r;
    JNIEnv *env = ctx->javaEnv;
    saj_copyCache copyCache = ctx->copyCache;
    sajReaderCopyCache *rc = saj_copyCacheReaderCache(copyCache);
    jobject dataSeq, dataSeqNew;
    jobject infoSeq, infoSeqNew;
    jobject ie;
    jobject de;
    saj_returnCode retcode = SAJ_RETCODE_ERROR;

    len = cmn_samplesList_length(ctx->samplesList);
    if (len > 0) {

        assert(ctx->dataSeqLen == ctx->infoSeqLen);

        if (ctx->dataSeqLen != len) {

            /* Resize output arrays for samples and sampleInfos */

            if (ctx->dataSeqLen > len) {
                copyLen = len;
            } else {
                copyLen = ctx->dataSeqLen;
            }

            dataSeqNew = NEW_OBJECTARRAY(env, len, rc->dataClass, NULL);
            assert(dataSeqNew);
            for (i = 0; i < copyLen; i++) {
                de = GET_OBJECTARRAY_ELEMENT(env, ctx->dataSeq, i);
                SET_OBJECTARRAY_ELEMENT(env, dataSeqNew, i, de);
                DELETE_LOCAL_REF(env, de);
            }
            (*env)->SetObjectField(env, ctx->dataSeqHolder, rc->dataSeqHolder_value_fid, dataSeqNew);
            CHECK_EXCEPTION(env);
            dataSeq = dataSeqNew;

            infoSeqNew = NEW_OBJECTARRAY(env, len, GET_CACHED(sampleInfo_class), NULL);
            assert(infoSeqNew);
            for (i = 0; i < copyLen; i++) {
                ie = GET_OBJECTARRAY_ELEMENT(env, ctx->infoSeq, i);
                SET_OBJECTARRAY_ELEMENT(env, infoSeqNew, i, ie);
                DELETE_LOCAL_REF(env, ie);
            }
            SET_OBJECT_FIELD(env, ctx->infoSeqHolder, sampleInfoSeqHolder_value, infoSeqNew);
            infoSeq = infoSeqNew;
        } else {
            dataSeq = (*env)->GetObjectField(env, ctx->dataSeqHolder, rc->dataSeqHolder_value_fid);
            CHECK_EXCEPTION(env);
            infoSeq = GET_OBJECT_FIELD(env, ctx->infoSeqHolder, sampleInfoSeqHolder_value);
        }
    } else {
        len = 0;
        /* No data available, claim and set zero sized arrays */
        dataSeq = NEW_OBJECTARRAY(env, len, rc->dataClass, NULL);
        (*env)->SetObjectField(env, ctx->dataSeqHolder, rc->dataSeqHolder_value_fid, dataSeq);
        CHECK_EXCEPTION(env);
        infoSeq = NEW_OBJECTARRAY(env, len, GET_CACHED(sampleInfo_class), NULL);
        SET_OBJECT_FIELD(env, ctx->infoSeqHolder, sampleInfoSeqHolder_value, infoSeq);
        retcode = SAJ_RETCODE_NO_DATA;
    }

    if (len > 0) {

        /* Now start copying data */
        if (ctx->pardemCtx && len >= ctx->pardemCtx->heuristics.threshold) {
            sajParDemContext pdc = ctx->pardemCtx;
            /* By keeping the pdc->superMtx locked, concurrent reads on
             * a parallelized reader be serialized. It furthermore
             * prevents new workers to be added/removed while copying. */
            os_mutexLock(&pdc->superMtx);
            if (pdc->nrofWorkers > 0) {
                os_mutexLock(&pdc->mtx);
                pdc->readyCnt = 0U;
                pa_st32 (&pdc->copy_info.nextIndex, 0U);
                pdc->copy_info.len = len;
                pdc->copy_info.dataSeq = NEW_WEAK_GLOBAL_REF(env, dataSeq);
                pdc->copy_info.infoSeq = NEW_WEAK_GLOBAL_REF(env, infoSeq);
                pdc->copy_info.copyCache = copyCache;
                pdc->copy_info.copyOut = ctx->CDRCopy ? saj_CDROutStruct : saj_copyOutStruct;
                pdc->copy_info.samplesList = ctx->samplesList;
                pdc->copy_info.jreader = NEW_WEAK_GLOBAL_REF(env, ctx->jreader);
                pdc->copy_info.uEntity = ctx->uReader;
                os_condBroadcast(&pdc->startCnd);
                os_mutexUnlock(&pdc->mtx);

                /* Perform the copy */
                saj_dataReaderParallelCopy(env, pdc);

                os_mutexLock(&pdc->mtx);
                pdc->readyCnt++;
                if (pdc->readyCnt > pdc->nrofWorkers) {
                    /* I am the last one to finish, notify workers */
                    pdc_readyCndBroadcast(env, pdc);
                } else {
                    const os_boolean parity = pdc->parity;
                    while(parity == pdc->parity){
                        os_condWait(&pdc->readyCnd, &pdc->mtx);
                    }
                }
                os_mutexUnlock(&pdc->mtx);

                /* free the samples on the sample list */
                r = u_readerProtectCopyOutEnter(SAJ_VOIDP(ctx->uReader));
                if (r == U_RESULT_OK) {
                    cmn_samplesList_reset(ctx->samplesList, 0);
                    u_readerProtectCopyOutExit(SAJ_VOIDP(ctx->uReader));
                }

                /* Parallel copy succeeded; disable singeThreadedCopy fallback */
                singleThreadedCopy = 0;
                retcode = SAJ_RETCODE_OK;
            }
            os_mutexUnlock(&pdc->superMtx);
        }

        if (singleThreadedCopy) {
            int r;
            struct flushCopyArg arg;

            arg.dst.copyProgram = copyCache;
            arg.dst.javaEnv = env;
            arg.dst.jreader = ctx->jreader;

            arg.env = env;
            arg.dataSeq = dataSeq;
            arg.infoSeq = infoSeq;
            arg.copyOut = ctx->CDRCopy ? saj_CDROutStruct : saj_copyOutStruct;
            arg.i = 0;
            arg.retcode = SAJ_RETCODE_OK;

            r = u_readerProtectCopyOutEnter(SAJ_VOIDP(ctx->uReader));
            if (r == U_RESULT_OK) {
                r = cmn_samplesList_flush(ctx->samplesList, flushCopy, &arg);
                u_readerProtectCopyOutExit(SAJ_VOIDP(ctx->uReader));

                if (r == 0) {
                    retcode = SAJ_RETCODE_NO_DATA;
                } else if (r < 0) {
                    retcode = SAJ_RETCODE_ALREADY_DELETED;
                } else {
                    retcode = arg.retcode;
                }
            } else {
                retcode = saj_retcode_from_user_result(r);
            }
        }
    }
    return retcode;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}
