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
#include "saj_FooDataReader.h"
#include "saj_copyIn.h"
#include "saj_copyOut.h"
#include "saj_utilities.h"
#include "saj__readerContext.h"

#include "gapi.h"

#include "os_heap.h"

/* Defines the package of the java implementation classes */
#define SAJ_PACKAGENAME "org/opensplice/dds/dcps/"
#define SAJ_FUNCTION(name) Java_org_opensplice_dds_dcps_FooDataReaderImpl_##name

struct sajParDemContext_s{
    os_ushort       nrofWorkers; /* Number of worker threads. This is configured #threads - 1 (application thread participates as well) */
    struct {
        os_ushort threshold; /* Number of samples threshold at which parallelization should be performed */
        os_ushort block;     /* Number of samples to be read without inter-thread sync */
    } heuristics;
    struct {
        os_uint32       nextIndex; /* Next index to be read by worker thread */
        unsigned int    len;       /* The number of samples to be copied */
        jobject         dataSeq;   /* Weak-ref to the data-sqeuence to copy-out to */
        jobject         infoSeq;   /* Weak-ref to the info-sqeuence to copy-out to */
        saj_copyCache   copyCache; /* Reference to the copyCache */
        gapi_copyOut    copyOut;   /* Copy-out function pointer */
        gapi_dataSampleSeq *samples;
        jobject         jreader;   /* Weak-ref to the reader Object */
    } copy_info;
    os_mutex        superMtx;  /* Mutex that should be held by the 'main' read thread in order to serialize concurrent reads on a parralelized reader */
    os_mutex        mtx;
    os_cond         startCnd;  /* Condition for notifying worker threads that work is to be done */
    os_cond         readyCnd;  /* Condition for notifying application thread that all work is done */
    os_ushort       readyCnt;  /* Counter for readyCnd broadcast */
    /* The parity is used for the loop-condition for readyCnd. Members of the
     * copy_info struct may be reused, so don't provide a safe loop-condition.
     * The parity toggles every loop and since it is not changed until all threads
     * are waiting on the next loop, it is a safe loop condition. */
    os_boolean      parity;
    os_ushort       terminate; /* Flag used to signal blocked threads to stop. */
};

static sajParDemContext
saj_parDemContext_new()
{
    sajParDemContext pdc;
    os_mutexAttr mtxAttrs;
    os_condAttr cndAttrs;

    if(os_mutexAttrInit(&mtxAttrs) != os_resultSuccess) goto err_attrs_init;
    mtxAttrs.scopeAttr = OS_SCOPE_PRIVATE;

    if(os_condAttrInit(&cndAttrs) != os_resultSuccess) goto err_attrs_init;
    cndAttrs.scopeAttr = OS_SCOPE_PRIVATE;

    if((pdc = os_malloc(sizeof(*pdc))) == NULL) goto err_malloc;

    pdc->nrofWorkers = 0U;
    pdc->heuristics.threshold = 2U; /* Parallelization only helps for at least 2 samples */
    pdc->heuristics.block = 1U;     /* A block of 1 is the bare minimum */
    pdc->copy_info.nextIndex = 0U;
    pdc->copy_info.len = 0U;
    pdc->copy_info.dataSeq = NULL;
    pdc->copy_info.infoSeq = NULL;
    pdc->copy_info.copyCache = NULL;
    pdc->copy_info.copyOut = NULL;
    pdc->copy_info.samples = NULL;

    if(os_mutexInit(&pdc->superMtx, &mtxAttrs) != os_resultSuccess) goto err_supermtx_init;
    if(os_mutexInit(&pdc->mtx, &mtxAttrs) != os_resultSuccess) goto err_mtx_init;
    if(os_condInit(&pdc->startCnd, &pdc->mtx, &cndAttrs) != os_resultSuccess) goto err_startcnd_init;
    if(os_condInit(&pdc->readyCnd, &pdc->mtx, &cndAttrs) != os_resultSuccess) goto err_readycnd_init;

    pdc->readyCnt = 0U;
    pdc->parity = OS_FALSE;
    pdc->terminate = 0u;

    return pdc;

/* Error-handling */
err_readycnd_init:
    os_condDestroy(&pdc->startCnd); /* Ignore result */
err_startcnd_init:
    os_mutexDestroy(&pdc->mtx); /* Ignore result */
err_mtx_init:
    os_mutexDestroy(&pdc->superMtx); /* Ignore result */
err_supermtx_init:
    os_free(pdc);
err_malloc:
err_attrs_init:
    return NULL;
}

static void
saj_parDemContext_free(
    sajParDemContext pdc)
{
    if(pdc){
        os_condDestroy(&pdc->readyCnd);
        os_condDestroy(&pdc->startCnd);
        os_mutexDestroy(&pdc->mtx);
        os_mutexDestroy(&pdc->superMtx);
        os_free(pdc);
    }
}

static void
saj_parDemContext_signal_terminate(
    sajParDemContext pdc)
{
    assert(pdc);

    if(os_mutexLock(&pdc->superMtx) == os_resultSuccess){
        if(os_mutexLock(&pdc->mtx) == os_resultSuccess){
            pdc->terminate = 1U;
            os_condBroadcast(&pdc->startCnd);
            os_mutexUnlock(&pdc->mtx);
        }
        os_mutexUnlock(&pdc->superMtx);
    }
}

static void
saj_dataReaderParallelCopy(
    JNIEnv *env,
    sajParDemContext pdc)
{
    os_uint32 i;
    C_STRUCT(saj_dstInfo) dst;
    jobject info_element;

    assert(pdc);
    assert(pdc->heuristics.block > 0U);
    assert(pdc->copy_info.len);
    assert(pdc->copy_info.dataSeq);
    assert(pdc->copy_info.infoSeq);
    assert(pdc->copy_info.copyCache);
    assert(pdc->copy_info.copyOut);
    assert(pdc->copy_info.samples);
    assert(pdc->copy_info.jreader);

    dst.copyProgram = pdc->copy_info.copyCache;
    dst.javaEnv = env;
    dst.jreader = pdc->copy_info.jreader;

    while((i = pa_increment(&pdc->copy_info.nextIndex) - 1) < pdc->copy_info.len){
        dst.javaObject = (*env)->GetObjectArrayElement(env, pdc->copy_info.dataSeq, i);
        pdc->copy_info.copyOut(pdc->copy_info.samples->_buffer[i].data, &dst);
        (*env)->SetObjectArrayElement(env, pdc->copy_info.dataSeq, i, dst.javaObject);
        (*env)->DeleteLocalRef(env, dst.javaObject);
        info_element = (*env)->GetObjectArrayElement(env, pdc->copy_info.infoSeq, i);
        saj_sampleInfoCopyOut(env, &pdc->copy_info.samples->_buffer[i].info, &info_element);
        (*env)->SetObjectArrayElement(env, pdc->copy_info.infoSeq, i, info_element);
        (*env)->DeleteLocalRef(env, info_element);
    }
}

/* Call with pcd->mtx locked */
static void pdc_readyCndBroadcast(
    JNIEnv *env,
    sajParDemContext pdc)
{
    assert(env);
    assert(pdc);

    (*env)->DeleteWeakGlobalRef(env, pdc->copy_info.dataSeq);
    (*env)->DeleteWeakGlobalRef(env, pdc->copy_info.infoSeq);
    pdc->copy_info.dataSeq = NULL;
    pdc->copy_info.infoSeq = NULL;
    pdc->parity = !pdc->parity;
    os_condBroadcast(&pdc->readyCnd);
}

void
saj_dataReaderCopy(
    gapi_dataSampleSeq *samples,
    gapi_readerInfo *arg)
{
    unsigned int i;
    unsigned int len;
    unsigned int copyLen;
    unsigned int singleThreadedCopy = 1U;
    gapi_readerInfo *info = arg;
    saj_readerContext *ctx = info->data_buffer;
    JNIEnv *env = ctx->javaEnv;
    saj_copyCache copyCache = ctx->copyCache;
    sajReaderCopyCache *rc = saj_copyCacheReaderCache(copyCache);
    jobject dataSeq;
    jobject infoSeq;
    jobject info_element;
    jobject ie;
    jobject de;
    C_STRUCT(saj_dstInfo) dst;

    if (samples) {
        len = samples->_length;
        if ((info->max_samples != (unsigned int)GAPI_LENGTH_UNLIMITED) && (len > info->max_samples)) {
            len = info->max_samples;
        }
        if (ctx->dataSeqLen != len) {
            if (ctx->dataSeqLen > len) {
                copyLen = len;
            } else {
                copyLen = ctx->dataSeqLen;
            }
            dataSeq = (*env)->NewObjectArray(env, len, rc->dataClass, NULL);
            for (i = 0; i < copyLen; i++) {
                de = (*env)->GetObjectArrayElement(env, ctx->dataSeq, i);
                (*env)->SetObjectArrayElement(env, dataSeq, i, de);
                (*env)->DeleteLocalRef(env, de);
            }
            (*env)->SetObjectField(env, ctx->dataSeqHolder, rc->dataSeqHolder_value_fid, dataSeq);
            ctx->dataSeq = dataSeq;
            infoSeq = (*env)->NewObjectArray(env, len, GET_CACHED (sampleInfo_class), NULL);
            for (i = 0; i < copyLen; i++) {
                ie = (*env)->GetObjectArrayElement(env, ctx->infoSeq, i);
                (*env)->SetObjectArrayElement(env, infoSeq, i, ie);
                (*env)->DeleteLocalRef(env, ie);
            }
            (*env)->SetObjectField(env, ctx->infoSeqHolder, GET_CACHED(sampleInfoSeqHolder_value_fid), infoSeq);
            ctx->infoSeq = infoSeq;
        } else {
            dataSeq = (*env)->GetObjectField(env, ctx->dataSeqHolder, rc->dataSeqHolder_value_fid);
            infoSeq = (*env)->GetObjectField(env, ctx->infoSeqHolder, GET_CACHED(sampleInfoSeqHolder_value_fid));
        }
    } else {
        len = 0;
        /* No data available, claim and set zero sized arrays */
        dataSeq = (*env)->NewObjectArray(env, len, rc->dataClass, NULL);
        (*env)->SetObjectField(env, ctx->dataSeqHolder, rc->dataSeqHolder_value_fid, dataSeq);
        ctx->dataSeq = dataSeq;
        infoSeq = (*env)->NewObjectArray(env, len, GET_CACHED (sampleInfo_class), NULL);
        (*env)->SetObjectField(env, ctx->infoSeqHolder, GET_CACHED(sampleInfoSeqHolder_value_fid), infoSeq);
        ctx->infoSeq = infoSeq;
    }

    if (len > 0) {
        if(ctx->pardemCtx && len >= ctx->pardemCtx->heuristics.threshold){
            sajParDemContext pdc = ctx->pardemCtx;
            /* By keeping the pdc->superMtx locked, concurrent reads on
             * a parallelized reader be serialized. It furthermore
             * prevents new workers to be added/removed while copying. */
            if(os_mutexLock(&pdc->superMtx) == os_resultSuccess){
                if(pdc->nrofWorkers > 0){
                    if(os_mutexLock(&pdc->mtx) == os_resultSuccess){
                        pdc->readyCnt = 0U;
                        pdc->copy_info.nextIndex = 0U;
                        pdc->copy_info.len = len;
                        pdc->copy_info.dataSeq = (*env)->NewWeakGlobalRef(env, dataSeq);
                        pdc->copy_info.infoSeq = (*env)->NewWeakGlobalRef(env, infoSeq);
                        pdc->copy_info.copyCache = copyCache;
                        pdc->copy_info.copyOut = ctx->CDRCopy ? saj_CDROutStruct : info->copy_out;
                        pdc->copy_info.samples = samples;
                        pdc->copy_info.jreader = (*env)->NewWeakGlobalRef(env, ctx->jreader);
                        os_condBroadcast(&pdc->startCnd);
                        os_mutexUnlock(&pdc->mtx);
                    }

                    /* Perform the copy */
                    saj_dataReaderParallelCopy(env, pdc);

                    if(os_mutexLock(&pdc->mtx) == os_resultSuccess){
                        pdc->readyCnt++;
                        if(pdc->readyCnt > pdc->nrofWorkers){
                            /* I am the last one to finish, notify workers */
                            pdc_readyCndBroadcast(env, pdc);
                        } else {
                            const os_boolean parity = pdc->parity;
                            while(parity == pdc->parity){
                                os_condWait(&pdc->readyCnd, &pdc->mtx);
                            }
                        }
                        os_mutexUnlock(&pdc->mtx);
                    }
                    /* Parallel copy succeeded; disable singeThreadedCopy fallback */
                    singleThreadedCopy = 0;
                }
                os_mutexUnlock(&pdc->superMtx);
            }
        }

        if(singleThreadedCopy) {
            dst.copyProgram = copyCache;
            dst.javaEnv = env;
            dst.jreader = ctx->jreader;
            for (i = 0; i < len; i++) {
                dst.javaObject = (*env)->GetObjectArrayElement(env, dataSeq, i);
                if (ctx->CDRCopy) {
                    saj_CDROutStruct(samples->_buffer[i].data, &dst);
                } else {
                    info->copy_out(samples->_buffer[i].data, &dst);
                }
                (*env)->SetObjectArrayElement(env, dataSeq, i, dst.javaObject);
                (*env)->DeleteLocalRef(env, dst.javaObject);
                info_element = (*env)->GetObjectArrayElement(env, infoSeq, i);
                saj_sampleInfoCopyOut(env, &samples->_buffer[i].info, &info_element);
                (*env)->SetObjectArrayElement(env, infoSeq, i, info_element);
                (*env)->DeleteLocalRef(env, info_element);
            }
        }
    }
    info->num_samples = len;
}


int
saj_fooDataReaderParallelDemarshallingMain(
    JNIEnv * env,
    sajParDemContext pdc)
{
    if(os_mutexLock(&pdc->superMtx) != os_resultSuccess) goto err_mtx_lock;
    if(os_mutexLock(&pdc->mtx) != os_resultSuccess) goto err_double_mtx_lock;
    pdc->nrofWorkers++;
    os_mutexUnlock(&pdc->mtx);
    os_mutexUnlock(&pdc->superMtx);

    do {
        if(os_mutexLock(&pdc->mtx) != os_resultSuccess) goto err_mtx_lock;
        while(!pdc->terminate && !pdc->copy_info.dataSeq){
            if(os_condWait(&pdc->startCnd, &pdc->mtx) != os_resultSuccess) goto err_condwait;
        }
        os_mutexUnlock(&pdc->mtx);
        /* pdc->terminate is guaranteed to only toggle to TRUE in a lock,
         * so reading outside the lock is OK. */
        if(pdc->terminate) break;

        saj_dataReaderParallelCopy(env, pdc);

        if(os_mutexLock(&pdc->mtx) != os_resultSuccess) goto err_mtx_lock;
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

    if(os_mutexLock(&pdc->superMtx) != os_resultSuccess) goto err_mtx_lock;
    if(os_mutexLock(&pdc->mtx) != os_resultSuccess) goto err_double_mtx_lock;
    pdc->nrofWorkers--;
    os_mutexUnlock(&pdc->mtx);
    os_mutexUnlock(&pdc->superMtx);

    return SAJ_RETCODE_OK;

err_double_mtx_lock:
    os_mutexUnlock(&pdc->superMtx);
err_mtx_lock:
    return SAJ_RETCODE_ERROR;

err_condwait:
    os_mutexUnlock(&pdc->mtx);
    return SAJ_RETCODE_ERROR;
}

int
saj_fooDataReaderParallelDemarshallingContextFinalize(
    JNIEnv * env,
    jobject jdatareader,
    sajParDemContext pdc)
{
    if(pdc){
        saj_parDemContext_signal_terminate(pdc);
        (*env)->CallVoidMethod(env, jdatareader, GET_CACHED(dataReaderImplClassJoinWorkers_mid));
        /* Assert that all threads are actually stopped. */
        assert(pdc->nrofWorkers == 0);

        saj_parDemContext_free(pdc);
    }

    return SAJ_RETCODE_OK;
}

int
saj_fooDataReaderSetParallelReadThreadCount(
    JNIEnv * env,
    jobject jdatareader,
    int value)
{
    int started = 0;
    sajParDemContext pdc = NULL;


    if(saj_read_parallelDemarshallingContext_address(env, jdatareader, &pdc) != SAJ_RETCODE_OK){
        goto err_get_address;
    }

    if(pdc){
        /* Signal eventual current threads to shutdown */
        saj_parDemContext_signal_terminate(pdc);
        /* Join threads */
        (*env)->CallVoidMethod(env, jdatareader, GET_CACHED(dataReaderImplClassJoinWorkers_mid));
        /* Assert that all threads are actually stopped */
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
                goto fatal_set_address;
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
                goto fatal_set_address;
            }
        }

        started = (*env)->CallIntMethod(env, jdatareader, GET_CACHED(dataReaderImplClassStartWorkers_mid), value > 1 ? value - 1 : 0);
    }

    return started;

err_get_address:
    return -1;
err_out_of_resources:
    return -2; /* TODO: map on SAJ_RETCODE_OUT_OF_RESOURCES when merging to newer version */
fatal_set_address:
    return -3;
}

int
saj_fooDataReaderSetCDRCopy(
    JNIEnv * env,
    jobject jdatareader,
    int value)
{
    if(saj_write_CDRCopy_value(env, jdatareader, value) != SAJ_RETCODE_OK){
        return -1;
    }
    return 0;
}

static jint
fillReaderContext(
    JNIEnv *env,
    jobject dataReader,
    saj_copyCache copyCache,
    jobject received_data,
    jobject info_seq,
    jint max_samples,
    saj_readerContext *ctx)
{
    sajReaderCopyCache *rc = saj_copyCacheReaderCache(copyCache);

    ctx->javaEnv = env;
    ctx->copyCache = copyCache;
    ctx->dataSeqHolder = received_data;
    ctx->infoSeqHolder = info_seq;
    if ((ctx->dataSeqHolder == NULL) || (ctx->infoSeqHolder == NULL)) {
        return GAPI_RETCODE_BAD_PARAMETER;
    }
    ctx->dataSeq = (*env)->GetObjectField(env, ctx->dataSeqHolder, rc->dataSeqHolder_value_fid);
    ctx->infoSeq = (*env)->GetObjectField(env, ctx->infoSeqHolder, GET_CACHED(sampleInfoSeqHolder_value_fid));
    if (ctx->dataSeq) {
        ctx->dataSeqLen = (*env)->GetArrayLength(env, ctx->dataSeq);
    } else {
        ctx->dataSeqLen = 0;
    }
    if (ctx->infoSeq) {
        ctx->infoSeqLen = (*env)->GetArrayLength (env, ctx->infoSeq);
    } else {
        ctx->infoSeqLen = 0;
    }
    ctx->max_samples = max_samples;
    assert(GET_CACHED(dataReaderImplClassParallelDemarshallingContext_fid));
    ctx->pardemCtx = (sajParDemContext)(PA_ADDRCAST)(*env)->GetLongField(env, dataReader, GET_CACHED(dataReaderImplClassParallelDemarshallingContext_fid));
    assert(GET_CACHED(dataReaderImplClassCDRCopy_fid));
    ctx->CDRCopy = (*env)->GetLongField(env, dataReader, GET_CACHED(dataReaderImplClassCDRCopy_fid));
    ctx->jreader = dataReader;
    return GAPI_RETCODE_OK;
}

static jint
inputRulesCheckedOk(
    gapi_fooDataReader dataReader,
    saj_readerContext *ctx)
{
    /* Rule 1 : Both sequences must have equal len, max_len and owns properties.*/
    if (ctx->dataSeqLen != ctx->infoSeqLen) {
        return FALSE;
    }

    /* Rule 4: When max_len > 0, then own must be true.*/
    if ((ctx->dataSeqLen > 0) &&
        gapi_fooDataReader_is_loan(dataReader, ctx->dataSeq, ctx->infoSeq)) {
        return FALSE;
    }

    /* Rule 5: when max_samples != LENGTH_UNLIMITED, then the following condition
    // needs to be met: maxSamples <= max_len  */
    if ((ctx->dataSeqLen > 0) &&
         (ctx->max_samples > ctx->dataSeqLen) &&
         (ctx->max_samples != (unsigned int)GAPI_LENGTH_UNLIMITED)) {
        return FALSE;
    }

    /* In all other cases, the provided sequences are valid.*/
    return TRUE;
}

/*
 * Class:     org_opensplice_dds_dcps_FooDataReaderImpl
 * Method:    jniRead
 * Signature: (Ljava/lang/Object;Ljava/lang/Object;LDDS/SampleInfoSeqHolder;IIII)I
 */
/*
    private native static int jniRead (
        Object DataReader,
        long copyCache,
        Object received_data,
        DDS.SampleInfoSeqHolder info_seq,
        int max_samples,
        int sample_states,
        int view_states,
        int instance_states);
*/
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniRead)(
    JNIEnv *env,
    jclass object,
    jobject DataReader,
    jlong copyCache,
    jobject received_data,
    jobject info_seq,
    jint max_samples,
    jint sample_states,
    jint view_states,
    jint instance_states)
{
    jint result;
    saj_readerContext ctx;
    gapi_fooDataReader dataReader;

    OS_UNUSED_ARG(object);

    dataReader = (gapi_fooDataReader)saj_read_gapi_address(env, DataReader);
    result = fillReaderContext(env, DataReader,
                (saj_copyCache)(PA_ADDRCAST)copyCache,
                received_data,
                info_seq,
                max_samples,
                &ctx);

    if (result == GAPI_RETCODE_OK) {
        if (inputRulesCheckedOk (dataReader, &ctx)) {
            result = (jint)gapi_fooDataReader_read (
                dataReader,
                (gapi_fooSeq *)&ctx,
                (gapi_sampleInfoSeq *)info_seq,
                max_samples,
                sample_states,
                view_states,
                instance_states);
        } else {
            result = GAPI_RETCODE_PRECONDITION_NOT_MET;
        }
    }

    return result;
}

/*
 * Class:     org_opensplice_dds_dcps_FooDataReaderImpl
 * Method:    jniTake
 * Signature: (Ljava/lang/Object;Ljava/lang/Object;LDDS/SampleInfoSeqHolder;IIII)I
 */
/*
    private native static int jniTake (
        Object DataReader,
        long copyCache,
        Object received_data,
        DDS.SampleInfoSeqHolder info_seq,
        int max_samples,
        int sample_states,
        int view_states,
        int instance_states);
*/
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniTake)(
    JNIEnv *env,
    jclass object,
    jobject DataReader,
    jlong copyCache,
    jobject received_data,
    jobject info_seq,
    jint max_samples,
    jint sample_states,
    jint view_states,
    jint instance_states)
{
    jint result;
    saj_readerContext ctx;
    gapi_fooDataReader dataReader;

    OS_UNUSED_ARG(object);

    dataReader = (gapi_fooDataReader)saj_read_gapi_address(env, DataReader);
    result = fillReaderContext(env, DataReader,
        (saj_copyCache)(PA_ADDRCAST)copyCache,
        received_data,
        info_seq,
        max_samples,
        &ctx);

    if (result == GAPI_RETCODE_OK) {
        if (inputRulesCheckedOk(dataReader, &ctx)) {
            result = (jint)gapi_fooDataReader_take(
                            (gapi_fooDataReader)saj_read_gapi_address(env, DataReader),
                            (gapi_fooSeq *)&ctx,
                            (gapi_sampleInfoSeq *)info_seq,
                            max_samples,
                            sample_states,
                            view_states,
                            instance_states);
        } else {
            result = GAPI_RETCODE_PRECONDITION_NOT_MET;
        }
    }

    return result;
}

/*
 * Class:     org_opensplice_dds_dcps_FooDataReaderImpl
 * Method:    jniReadWCondition
 * Signature: (Ljava/lang/Object;Ljava/lang/Object;LDDS/SampleInfoSeqHolder;ILDDS/ReadCondition;)I
 */
/*
    private native static int jniReadWCondition (
        Object DataReader,
        long copyCache,
        Object received_data,
        DDS.SampleInfoSeqHolder info_seq,
        int max_samples,
        DDS.ReadCondition a_condition);
*/
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniReadWCondition)(
    JNIEnv *env,
    jclass object,
    jobject DataReader,
    jlong copyCache,
    jobject received_data,
    jobject info_seq,
    jint max_samples,
    jobject a_condition)
{
    jint result;
    saj_readerContext ctx;
    gapi_fooDataReader dataReader;

    OS_UNUSED_ARG(object);

    dataReader = (gapi_fooDataReader)saj_read_gapi_address(env, DataReader);
    result = fillReaderContext(env, DataReader,
                (saj_copyCache)(PA_ADDRCAST)copyCache,
                received_data,
                info_seq,
                max_samples,
                &ctx);

    if (result == GAPI_RETCODE_OK) {
        if (inputRulesCheckedOk (dataReader, &ctx)) {
            result = (jint)gapi_fooDataReader_read_w_condition(
                            (gapi_fooDataReader)saj_read_gapi_address(env, DataReader),
                            (gapi_fooSeq *)&ctx,
                            (gapi_sampleInfoSeq *)info_seq,
                            max_samples,
                            (gapi_readCondition)saj_read_gapi_address(env, a_condition));
        } else {
            result = GAPI_RETCODE_PRECONDITION_NOT_MET;
        }
    }

    return result;
}

/*
 * Class:     org_opensplice_dds_dcps_FooDataReaderImpl
 * Method:    jniTakeWCondition
 * Signature: (Ljava/lang/Object;Ljava/lang/Object;LDDS/SampleInfoSeqHolder;ILDDS/ReadCondition;)I
 */
/*
    private native static int jniTakeWCondition (
        Object DataReader,
        long copyCache,
        Object received_data,
        DDS.SampleInfoSeqHolder info_seq,
        int max_samples,
        DDS.ReadCondition a_condition);
*/
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniTakeWCondition)(
    JNIEnv *env,
    jclass object,
    jobject DataReader,
    jlong copyCache,
    jobject received_data,
    jobject info_seq,
    jint max_samples,
    jobject a_condition)
{
    jint result;
    saj_readerContext ctx;
    gapi_fooDataReader dataReader;

    OS_UNUSED_ARG(object);

    dataReader = (gapi_fooDataReader)saj_read_gapi_address(env, DataReader);
    result = fillReaderContext(env, DataReader,
                (saj_copyCache)(PA_ADDRCAST)copyCache,
                received_data,
                info_seq,
                max_samples,
                &ctx);

    if (result == GAPI_RETCODE_OK) {
        if (inputRulesCheckedOk (dataReader, &ctx)) {
            result = (jint)gapi_fooDataReader_take_w_condition(
                            (gapi_fooDataReader)saj_read_gapi_address(env, DataReader),
                            (gapi_fooSeq *)&ctx,
                            (gapi_sampleInfoSeq *)info_seq,
                            max_samples,
                            (gapi_readCondition)saj_read_gapi_address(env, a_condition));
        } else {
            result = GAPI_RETCODE_PRECONDITION_NOT_MET;
        }
    }

    return result;
}

/*
 * Class:     org_opensplice_dds_dcps_FooDataReaderImpl
 * Method:    jniReadNextSample
 * Signature: (Ljava/lang/Object;Ljava/lang/Object;LDDS/SampleInfoHolder;)I
 */
/*
    private native static int jniReadNextSample (
        Object DataReader,
        long copyCache,
        Object received_data,
        DDS.SampleInfoHolder sample_info);
*/
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniReadNextSample)(
    JNIEnv *env,
    jclass object,
    jobject DataReader,
    jlong copyCache,
    jobject received_data,
    jobject sample_info)
{
    jint result;
    gapi_sampleInfo sampleInfo;
    gapi_sampleInfo *si = NULL;
    C_STRUCT(saj_dstInfo) dstInfo;
    gapi_foo *dst = NULL;
    jobject data_element = NULL;
    sajReaderCopyCache *rc = saj_copyCacheReaderCache((saj_copyCache)(PA_ADDRCAST)copyCache);

    OS_UNUSED_ARG(object);
    assert (DataReader);

    if (received_data != NULL) {
        data_element = (*env)->GetObjectField(env, received_data, rc->dataHolder_value_fid);
        dstInfo.javaEnv = env;
        dstInfo.javaObject = received_data;
        dstInfo.copyProgram = (saj_copyCache)(PA_ADDRCAST)copyCache;
        dst = (gapi_foo *)&dstInfo;
    }
    if (sample_info != NULL) {
        si = &sampleInfo;
    }

    result = (jint)gapi_fooDataReader_read_next_sample(
        (gapi_fooDataReader)saj_read_gapi_address(env, DataReader),
        (gapi_foo *)dst,
        si);
    if (result == GAPI_RETCODE_OK) {
        if (data_element != dstInfo.javaObject) {
            (*env)->SetObjectField(env, received_data, rc->dataHolder_value_fid, dstInfo.javaObject);
        }
        if (saj_sampleInfoHolderCopyOut(env, &sampleInfo, &sample_info) != SAJ_RETCODE_OK) {
            result = GAPI_RETCODE_ERROR;
        }
    }
    return result;
}

/*
 * Class:     org_opensplice_dds_dcps_FooDataReaderImpl
 * Method:    jniTakeNextSample
 * Signature: (Ljava/lang/Object;Ljava/lang/Object;LDDS/SampleInfoHolder;)I
 */
/*
    private native static int jniTakeNextSample (
        Object DataReader,
        long copyCache,
        Object received_data,
        DDS.SampleInfoHolder sample_info);
*/
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniTakeNextSample)(
    JNIEnv *env,
    jclass object,
    jobject DataReader,
    jlong copyCache,
    jobject received_data,
    jobject sample_info)
{
    jint result;
    gapi_sampleInfo sampleInfo;
    gapi_sampleInfo *si = NULL;
    C_STRUCT(saj_dstInfo) dstInfo;
    gapi_foo *dst = NULL;
    jobject data_element = NULL;
    sajReaderCopyCache *rc = saj_copyCacheReaderCache((saj_copyCache)(PA_ADDRCAST)copyCache);

    OS_UNUSED_ARG(object);

    if (received_data != NULL) {
        data_element = (*env)->GetObjectField(env, received_data, rc->dataHolder_value_fid);
        dstInfo.javaEnv = env;
        dstInfo.javaObject = data_element;
        dstInfo.copyProgram = (saj_copyCache)(PA_ADDRCAST)copyCache;
        dst = (gapi_foo *)&dstInfo;
    }
    if (sample_info != NULL) {
        si = &sampleInfo;
    }

    assert (DataReader);

    result = (jint)gapi_fooDataReader_take_next_sample(
        (gapi_fooDataReader)saj_read_gapi_address (env, DataReader),
        dst,
        si);
    if (result == GAPI_RETCODE_OK) {
        if (data_element != dstInfo.javaObject) {
            (*env)->SetObjectField(env, received_data, rc->dataHolder_value_fid, dstInfo.javaObject);
        }
        if (saj_sampleInfoHolderCopyOut(env, &sampleInfo, &sample_info) != SAJ_RETCODE_OK) {
            result = GAPI_RETCODE_ERROR;
        }
    }
    return result;
}

/*
 * Class:     org_opensplice_dds_dcps_FooDataReaderImpl
 * Method:    jniReadInstance
 * Signature: (Ljava/lang/Object;Ljava/lang/Object;LDDS/SampleInfoSeqHolder;IIIII)I
 */
/*
    private native static int jniReadInstance (
        Object DataReader,
        long copyCache,
        Object received_data,
        DDS.SampleInfoSeqHolder info_seq,
        int max_samples,
        long a_handle,
        int sample_states,
        int view_states,
        int instance_states);
*/
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniReadInstance)(
    JNIEnv *env,
    jclass object,
    jobject DataReader,
    jlong copyCache,
    jobject received_data,
    jobject info_seq,
    jint max_samples,
    jlong a_handle,
    jint sample_states,
    jint view_states,
    jint instance_states)
{
    jint result;
    saj_readerContext ctx;
    gapi_fooDataReader dataReader;

    OS_UNUSED_ARG(object);

    dataReader = (gapi_fooDataReader)saj_read_gapi_address(env, DataReader);
    result = fillReaderContext(env, DataReader,
                (saj_copyCache)(PA_ADDRCAST)copyCache,
                received_data,
                info_seq,
                max_samples,
                &ctx);

    if (result == GAPI_RETCODE_OK) {
        if (inputRulesCheckedOk (dataReader, &ctx)) {
            result = (jint)gapi_fooDataReader_read_instance(
                            (gapi_fooDataReader)saj_read_gapi_address (env, DataReader),
                            (gapi_fooSeq *)&ctx,
                            (gapi_sampleInfoSeq *)info_seq,
                            max_samples,
                            (gapi_instanceHandle_t)a_handle,
                            sample_states,
                            view_states,
                            instance_states);
        } else {
            result = GAPI_RETCODE_PRECONDITION_NOT_MET;
        }
    }

    return result;
}

/*
 * Class:     org_opensplice_dds_dcps_FooDataReaderImpl
 * Method:    jniTakeInstance
 * Signature: (Ljava/lang/Object;Ljava/lang/Object;LDDS/SampleInfoSeqHolder;IIIII)I
 */
/*
    private native static int jniTakeInstance (
        Object DataReader,
        long copyCache,
        Object received_data,
        DDS.SampleInfoSeqHolder info_seq,
        int max_samples,
        long a_handle,
        int sample_states,
        int view_states,
        int instance_states);
*/
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniTakeInstance)(
    JNIEnv *env,
    jclass object,
    jobject DataReader,
    jlong copyCache,
    jobject received_data,
    jobject info_seq,
    jint max_samples,
    jlong a_handle,
    jint sample_states,
    jint view_states,
    jint instance_states)
{
    jint result;
    saj_readerContext ctx;
    gapi_fooDataReader dataReader;

    OS_UNUSED_ARG(object);

    dataReader = (gapi_fooDataReader)saj_read_gapi_address(env, DataReader);
    result = fillReaderContext(env, DataReader,
                (saj_copyCache)(PA_ADDRCAST)copyCache,
                received_data,
                info_seq,
                max_samples,
                &ctx);

    if (result == GAPI_RETCODE_OK) {
        if (inputRulesCheckedOk(dataReader, &ctx)) {
            result = (jint)gapi_fooDataReader_take_instance(
                            (gapi_fooDataReader)saj_read_gapi_address (env, DataReader),
                            (gapi_fooSeq *)&ctx,
                            (gapi_sampleInfoSeq *)info_seq,
                            max_samples,
                            (gapi_instanceHandle_t)a_handle,
                            sample_states,
                            view_states,
                            instance_states);
        } else {
            result = GAPI_RETCODE_PRECONDITION_NOT_MET;
        }
    }

    return result;
}

/*
 * Class:     org_opensplice_dds_dcps_FooDataReaderImpl
 * Method:    jniReadNextInstance
 * Signature: (Ljava/lang/Object;Ljava/lang/Object;LDDS/SampleInfoSeqHolder;IIIII)I
 */
/*
    private native static int jniReadNextInstance (
        Object DataReader,
        long copyCache,
        Object received_data,
        DDS.SampleInfoSeqHolder info_seq,
        int max_samples,
        long a_handle,
        int sample_states,
        int view_states,
        int instance_states);
*/
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniReadNextInstance)(
    JNIEnv *env,
    jclass object,
    jobject DataReader,
    jlong copyCache,
    jobject received_data,
    jobject info_seq,
    jint max_samples,
    jlong a_handle,
    jint sample_states,
    jint view_states,
    jint instance_states)
{
    jint result;
    saj_readerContext ctx;
    gapi_fooDataReader dataReader;

    OS_UNUSED_ARG(object);

    dataReader = (gapi_fooDataReader)saj_read_gapi_address(env, DataReader);
    result = fillReaderContext(env, DataReader,
                (saj_copyCache)(PA_ADDRCAST)copyCache,
                received_data,
                info_seq,
                max_samples,
                &ctx);

    if (result == GAPI_RETCODE_OK) {
        if (inputRulesCheckedOk(dataReader, &ctx)) {
            result = (jint)gapi_fooDataReader_read_next_instance(
                            (gapi_fooDataReader)saj_read_gapi_address(env, DataReader),
                            (gapi_fooSeq *)&ctx,
                            (gapi_sampleInfoSeq *)info_seq,
                            max_samples,
                            (gapi_instanceHandle_t)a_handle,
                            sample_states,
                            view_states,
                            instance_states);
        } else {
            result = GAPI_RETCODE_PRECONDITION_NOT_MET;
        }
    }

    return result;
}

/*
 * Class:     org_opensplice_dds_dcps_FooDataReaderImpl
 * Method:    jniTakeNextInstance
 * Signature: (Ljava/lang/Object;Ljava/lang/Object;LDDS/SampleInfoSeqHolder;IIIII)I
 */
/*
    private native static int jniTakeNextInstance (
        Object DataReader,
        long copyCache,
        long copyCache,
        Object received_data,
        DDS.SampleInfoSeqHolder info_seq,
        int max_samples,
        long a_handle,
        int sample_states,
        int view_states,
        int instance_states);
*/
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniTakeNextInstance)(
    JNIEnv *env,
    jclass object,
    jobject DataReader,
    jlong copyCache,
    jobject received_data,
    jobject info_seq,
    jint max_samples,
    jlong a_handle,
    jint sample_states,
    jint view_states,
    jint instance_states)
{
    jint result;
    saj_readerContext ctx;
    gapi_fooDataReader dataReader;

    OS_UNUSED_ARG(object);

    dataReader = (gapi_fooDataReader)saj_read_gapi_address(env, DataReader);
    result = fillReaderContext(env, DataReader,
                (saj_copyCache)(PA_ADDRCAST)copyCache,
                received_data,
                info_seq,
                max_samples,
                &ctx);

    if (result == GAPI_RETCODE_OK) {
        if (inputRulesCheckedOk(dataReader, &ctx)) {
            result = (jint)gapi_fooDataReader_take_next_instance(
                            (gapi_fooDataReader)saj_read_gapi_address(env, DataReader),
                            (gapi_fooSeq *)&ctx,
                            (gapi_sampleInfoSeq *)info_seq,
                            max_samples,
                            (gapi_instanceHandle_t)a_handle,
                            sample_states,
                            view_states,
                            instance_states);
        } else {
            result = GAPI_RETCODE_PRECONDITION_NOT_MET;
        }
    }

    return result;
}

/*
 * Class:     org_opensplice_dds_dcps_FooDataReaderImpl
 * Method:    jniReadNextInstanceWCondition
 * Signature: (Ljava/lang/Object;Ljava/lang/Object;LDDS/SampleInfoSeqHolder;IILDDS/ReadCondition;)I
 */
/*
    private native static int jniReadNextInstanceWCondition (
        Object DataReader,
        long copyCache,
        Object received_data,
        DDS.SampleInfoSeqHolder info_seq,
        int max_samples,
        long a_handle,
        DDS.ReadCondition a_condition);
*/
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniReadNextInstanceWCondition)(
    JNIEnv *env,
    jclass object,
    jobject DataReader,
    jlong copyCache,
    jobject received_data,
    jobject info_seq,
    jint max_samples,
    jlong a_handle,
    jobject a_condition)
{
    jint result;
    saj_readerContext ctx;
    gapi_fooDataReader dataReader;

    OS_UNUSED_ARG(object);

    dataReader = (gapi_fooDataReader)saj_read_gapi_address(env, DataReader);
    result = fillReaderContext(env, DataReader,
                (saj_copyCache)(PA_ADDRCAST)copyCache,
                received_data,
                info_seq,
                max_samples,
                &ctx);

    if (result == GAPI_RETCODE_OK) {
        if (inputRulesCheckedOk(dataReader, &ctx)) {
            result = (jint)gapi_fooDataReader_read_next_instance_w_condition(
                            (gapi_fooDataReader)saj_read_gapi_address(env, DataReader),
                            (gapi_fooSeq *)&ctx,
                            (gapi_sampleInfoSeq *)info_seq,
                            max_samples,
                            (gapi_instanceHandle_t)a_handle,
                            (gapi_readCondition)saj_read_gapi_address (env, a_condition));
        } else {
            result = GAPI_RETCODE_PRECONDITION_NOT_MET;
        }
    }

    return result;
}

/*
 * Class:     org_opensplice_dds_dcps_FooDataReaderImpl
 * Method:    jniTakeNextInstanceWCondition
 * Signature: (Ljava/lang/Object;Ljava/lang/Object;LDDS/SampleInfoSeqHolder;IILDDS/ReadCondition;)I
 */
/*
    private native static int jniTakeNextInstanceWCondition (
        Object DataReader,
        long copyCache,
        Object received_data,
        DDS.SampleInfoSeqHolder info_seq,
        int max_samples,
        long a_handle,
        DDS.ReadCondition a_condition);
*/
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniTakeNextInstanceWCondition)(
    JNIEnv *env,
    jclass object,
    jobject DataReader,
    jlong copyCache,
    jobject received_data,
    jobject info_seq,
    jint max_samples,
    jlong a_handle,
    jobject a_condition)
{
    jint result;
    saj_readerContext ctx;
    gapi_fooDataReader dataReader;

    OS_UNUSED_ARG(object);

    dataReader = (gapi_fooDataReader)saj_read_gapi_address(env, DataReader);
    result = fillReaderContext(env, DataReader,
                (saj_copyCache)(PA_ADDRCAST)copyCache,
                received_data,
                info_seq,
                max_samples,
                &ctx);

    if (result == GAPI_RETCODE_OK) {
        if (inputRulesCheckedOk(dataReader, &ctx)) {
            result = (jint)gapi_fooDataReader_take_next_instance_w_condition(
                            (gapi_fooDataReader)saj_read_gapi_address (env, DataReader),
                            (gapi_fooSeq *)&ctx,
                            (gapi_sampleInfoSeq *)info_seq,
                            max_samples,
                            (gapi_instanceHandle_t)a_handle,
                            (gapi_readCondition)saj_read_gapi_address(env, a_condition));
        } else {
            result = GAPI_RETCODE_PRECONDITION_NOT_MET;
        }
    }

    return result;
}


/*
 * Class:     org_opensplice_dds_dcps_FooDataReaderImpl
 * Method:    jniGetKeyValue
 * Signature: (Ljava/lang/Object;Ljava/lang/Object;I)I
 */
/*
    private native static int jniGetKeyValue (
        Object DataReader,
        long copyCache,
        Object key_holder,
        long handle);
*/
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetKeyValue)(
    JNIEnv *env,
    jclass object,
    jobject DataReader,
    jlong copyCache,
    jobject key_holder,
    jlong handle)
{
    C_STRUCT(saj_dstInfo) dstInfo;
    gapi_foo *dst = NULL;
    jint result;
    jobject element = NULL;
    sajReaderCopyCache *rc = saj_copyCacheReaderCache((saj_copyCache)(PA_ADDRCAST)copyCache);

    OS_UNUSED_ARG(object);

    assert (DataReader);

    if (key_holder != NULL) {
        element = (*env)->GetObjectField(env, key_holder, rc->dataHolder_value_fid);
        dstInfo.javaEnv = env;
        dstInfo.javaObject = element;
        dstInfo.copyProgram = (saj_copyCache)(PA_ADDRCAST)copyCache;
        dst = (gapi_foo *)&dstInfo;
    }

    result = gapi_fooDataReader_get_key_value(
                (gapi_fooDataReader)saj_read_gapi_address(env, DataReader),
                (gapi_foo *)dst,
                (gapi_instanceHandle_t)handle);

    if ((key_holder != NULL) && (dstInfo.javaObject != element)) {
        (*env)->SetObjectField(env, key_holder, rc->dataHolder_value_fid, dstInfo.javaObject);
    }

    return result;
}

/*
 * Class:     org_opensplice_dds_dcps_FooDataReaderImpl
 * Method:    jniLookupInstance
 * Signature: (Ljava/lang/Object;Ljava/lang/Object;I)I
 */
/*
    private native static int jniLookupInstance (
        Object DataReader,
        long copyCache,
        Object instance);
*/
JNIEXPORT jlong JNICALL
SAJ_FUNCTION(jniLookupInstance)(
    JNIEnv *env,
    jclass object,
    jobject DataReader,
    jlong copyCache,
    jobject instance)
{
    C_STRUCT(saj_srcInfo) srcInfo;
    gapi_foo *src = NULL;
    jlong result;

    OS_UNUSED_ARG(object);
    assert (DataReader);

    if (instance != NULL) {
        srcInfo.javaEnv = env;
        srcInfo.javaObject = instance;
        srcInfo.copyProgram = (saj_copyCache)(PA_ADDRCAST)copyCache;
        src = (gapi_foo *)&srcInfo;
    }

    result = (jlong)gapi_fooDataReader_lookup_instance(
                (gapi_fooDataReader)saj_read_gapi_address(env, DataReader),
                (gapi_foo *)src);

    return result;
}
