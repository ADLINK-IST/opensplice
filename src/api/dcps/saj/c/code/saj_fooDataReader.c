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
#include "saj_FooDataReader.h"
#include "saj__fooDataReader.h"
#include "saj_copyIn.h"
#include "saj_copyOut.h"
#include "saj_utilities.h"

#include "gapi.h"

/* Defines the package of the java implementation classes */
#define SAJ_PACKAGENAME "org/opensplice/dds/dcps/"
#define SAJ_FUNCTION(name) Java_org_opensplice_dds_dcps_FooDataReaderImpl_##name

typedef struct {
    JNIEnv *javaEnv;
    saj_copyCache copyCache;
    jobject dataSeqHolder;
    jobject infoSeqHolder;
    jobject dataSeq;
    jobject infoSeq;
    unsigned int dataSeqLen;
    unsigned int infoSeqLen;
    unsigned int max_samples;
} readerContext;
    
void
saj_dataReaderCopy(
    gapi_dataSampleSeq *samples,
    gapi_readerInfo *arg)
{
    unsigned int i;
    unsigned int len;
    unsigned int copyLen;
    gapi_readerInfo *info = arg;
    readerContext *ctx = info->data_buffer;
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
        dst.copyProgram = copyCache;
        dst.javaEnv = ctx->javaEnv;
        for (i = 0; i < len; i++) {
            dst.javaObject = (*env)->GetObjectArrayElement(env, dataSeq, i);
            de = dst.javaObject;
            info->copy_out(samples->_buffer[i].data, &dst);
            if (dst.javaObject != de) {
                (*env)->SetObjectArrayElement(env, dataSeq, i, dst.javaObject);
            }
            info_element = (*env)->GetObjectArrayElement(env, infoSeq, i);
            ie = info_element;
            saj_sampleInfoCopyOut(env, &samples->_buffer[i].info, &info_element);
            if (info_element != ie) {
                (*env)->SetObjectArrayElement(env, infoSeq, i, info_element);
            }
        }
    }
    info->num_samples = len;
}

static jint
fillReaderContext(
    JNIEnv *env,
    saj_copyCache copyCache,
    jobject received_data,
    jobject info_seq,
    jint max_samples,
    readerContext *ctx)
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
    return GAPI_RETCODE_OK;
}

static jint
inputRulesCheckedOk(
    gapi_fooDataReader dataReader,
    readerContext *ctx)
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
    readerContext ctx;
    gapi_fooDataReader dataReader;

    dataReader = (gapi_fooDataReader)saj_read_gapi_address(env, DataReader);
    result = fillReaderContext(env,
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
    readerContext ctx;
    gapi_fooDataReader dataReader;

    dataReader = (gapi_fooDataReader)saj_read_gapi_address(env, DataReader);
    result = fillReaderContext(env,
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
    readerContext ctx;
    gapi_fooDataReader dataReader;

    dataReader = (gapi_fooDataReader)saj_read_gapi_address(env, DataReader);
    result = fillReaderContext(env,
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
    readerContext ctx;
    gapi_fooDataReader dataReader;

    dataReader = (gapi_fooDataReader)saj_read_gapi_address(env, DataReader);
    result = fillReaderContext(env,
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
    jobject data_element;
    sajReaderCopyCache *rc = saj_copyCacheReaderCache((saj_copyCache)(PA_ADDRCAST)copyCache);

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
    jobject data_element;
    sajReaderCopyCache *rc = saj_copyCacheReaderCache((saj_copyCache)(PA_ADDRCAST)copyCache);

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
    readerContext ctx;
    gapi_fooDataReader dataReader;

    dataReader = (gapi_fooDataReader)saj_read_gapi_address(env, DataReader);
    result = fillReaderContext(env,
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
    readerContext ctx;
    gapi_fooDataReader dataReader;

    dataReader = (gapi_fooDataReader)saj_read_gapi_address(env, DataReader);
    result = fillReaderContext(env,
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
    readerContext ctx;
    gapi_fooDataReader dataReader;

    dataReader = (gapi_fooDataReader)saj_read_gapi_address(env, DataReader);
    result = fillReaderContext(env,
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
    readerContext ctx;
    gapi_fooDataReader dataReader;

    dataReader = (gapi_fooDataReader)saj_read_gapi_address(env, DataReader);
    result = fillReaderContext(env,
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
    readerContext ctx;
    gapi_fooDataReader dataReader;

    dataReader = (gapi_fooDataReader)saj_read_gapi_address(env, DataReader);
    result = fillReaderContext(env,
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
    readerContext ctx;
    gapi_fooDataReader dataReader;

    dataReader = (gapi_fooDataReader)saj_read_gapi_address(env, DataReader);
    result = fillReaderContext(env,
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
    jobject element;
    sajReaderCopyCache *rc = saj_copyCacheReaderCache((saj_copyCache)(PA_ADDRCAST)copyCache);

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
