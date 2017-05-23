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
#include "saj_FooDataReaderView.h"
#include "saj_copyIn.h"
#include "saj_copyOut.h"
#include "saj_utilities.h"
#include "saj__readerContext.h"
#include "u_dataView.h"
#include "u_instanceHandle.h"
#include "v_dataViewSample.h"
#include "v_dataReaderSample.h"
#include "saj_dataReaderParDem.h"
#include "cmn_samplesList.h"
#include "cmn_reader.h"

#include "os_heap.h"
#include "saj__report.h"

/* Defines the package of the java implementation classes */
#define SAJ_PACKAGENAME "org/opensplice/dds/dcps/"
#define SAJ_FUNCTION(name) Java_org_opensplice_dds_dcps_FooDataReaderViewImpl_##name

static jint
fillReaderContext(
    JNIEnv *env,
    saj_copyCache copyCache,
    jobject received_data,
    jobject info_seq,
    jint max_samples,
    jobject view,
    jlong uView,
    saj_readerContext *ctx)
{
    int result;
    jobject dataReader;
    sajReaderCopyCache *rc = saj_copyCacheReaderCache(copyCache);

    assert (ctx != NULL);
    ctx->samplesList = NULL;

    if (received_data == NULL) {
        result = SAJ_RETCODE_BAD_PARAMETER;
        SAJ_REPORT(result, "data_values 'null' is invalid.");
        return result;
    }
    if (info_seq == NULL) {
        result = SAJ_RETCODE_BAD_PARAMETER;
        SAJ_REPORT(result, "info_seq 'null' is invalid.");
        return result;
    }

    ctx->javaEnv = env;
    ctx->copyCache = copyCache;
    ctx->dataSeqHolder = received_data;
    ctx->infoSeqHolder = info_seq;

    ctx->dataSeq = (*env)->GetObjectField(env, ctx->dataSeqHolder, rc->dataSeqHolder_value_fid);
    CHECK_EXCEPTION(env);
    ctx->infoSeq = GET_OBJECT_FIELD(env, ctx->infoSeqHolder, sampleInfoSeqHolder_value);
    ctx->max_samples = max_samples;
    if (ctx->dataSeq) {
        ctx->dataSeqLen = GET_ARRAY_LENGTH(env, ctx->dataSeq);
    } else {
        ctx->dataSeqLen = 0;
    }
    if (ctx->infoSeq) {
        ctx->infoSeqLen = GET_ARRAY_LENGTH(env, ctx->infoSeq);
    } else {
        ctx->infoSeqLen = 0;
    }

    /* Rule 1 : Both sequences must have equal len, max_len and owns properties.*/
    if (ctx->dataSeqLen != ctx->infoSeqLen) {
        result = SAJ_RETCODE_PRECONDITION_NOT_MET;
        SAJ_REPORT(result, "lengths of data_values(%d) and info_seq(%d) sequences are not equal", ctx->dataSeqLen, ctx->infoSeqLen);
        return result;
    }

    /*Rule 5: when max_samples != LENGTH_UNLIMITED, then the following condition
    needs to be met: maxSamples <= max_len*/
    if ((ctx->dataSeqLen > 0) &&
        (ctx->max_samples > ctx->dataSeqLen) &&
        (ctx->max_samples != (os_uint32)-1))
    {
        SAJ_REPORT(SAJ_RETCODE_PRECONDITION_NOT_MET, "Creation of the sampleslist failed (maxSamples(%d) > max_len(%d)).",ctx->max_samples,ctx->dataSeqLen);
        return SAJ_RETCODE_PRECONDITION_NOT_MET;
    }

    if (max_samples == 0) {
            /* Optimization to avoid going into the kernel. */
            return SAJ_RETCODE_NO_DATA;
        }

    ctx->max_samples = max_samples;
    ctx->CDRCopy = FALSE;
    /* get pardemCtx from parent reader */
    dataReader = GET_OBJECT_FIELD(env, view, dataReaderViewImplClassReader);
    if (dataReader != NULL) {
        ctx->pardemCtx = (sajParDemContext)(PA_ADDRCAST)GET_LONG_FIELD(
            env, dataReader, dataReaderImplClassParallelDemarshallingContext);
    } else {
        ctx->pardemCtx = NULL;
    }
    ctx->jreader = view;
    ctx->uReader = uView;

    ctx->samplesList = cmn_samplesList_new(TRUE);
    if (ctx->samplesList == NULL) {
        return SAJ_RETCODE_ERROR;
    }
    cmn_samplesList_reset(ctx->samplesList, (os_int32)max_samples);

    return SAJ_RETCODE_OK;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
}

static void
emptyReaderContext(
    saj_readerContext *ctx )
{
    if (ctx->samplesList != NULL) {
        cmn_samplesList_free(ctx->samplesList);
        ctx->samplesList = NULL;
    }
}

/*
 * Class:     org_opensplice_dds_dcps_FooDataReaderViewImpl
 * Method:    jniRead
 * Signature: (Ljava/lang/Object;Ljava/lang/Object;LDDS/SampleInfoSeqHolder;IIII)I
 */
/*
    private native static int jniRead (
        Object DataReaderView,
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
    jobject DataReaderView,
    jlong uView,
    jlong copyCache,
    jobject received_data,
    jobject info_seq,
    jint max_samples,
    jint sample_states,
    jint view_states,
    jint instance_states)
{
    u_result uResult;
    saj_readerContext ctx;
    u_sampleMask mask;
    saj_returnCode retcode;

    OS_UNUSED_ARG(object);

    retcode = fillReaderContext(env,
                (saj_copyCache)(PA_ADDRCAST)copyCache,
                received_data,
                info_seq,
                max_samples,
                DataReaderView,
                uView,
                &ctx);
    if (retcode == SAJ_RETCODE_OK) {
        retcode = DDS_SAMPLE_MASK_CHECK(sample_states, view_states, instance_states);
        if (retcode == SAJ_RETCODE_BAD_PARAMETER) {
            SAJ_REPORT(retcode, "Invalid sample mask(0x%x),view mask(0x%x) or instance mask(0x%x)", sample_states, view_states, instance_states);
        }
    }
    if (retcode == SAJ_RETCODE_OK) {
        mask = DDS_SAMPLE_MASK(sample_states, view_states, instance_states);
        uResult = u_dataViewRead(SAJ_VOIDP(uView), mask, cmn_reader_action, ctx.samplesList, OS_DURATION_ZERO);
        retcode = saj_retcode_from_user_result(uResult);
        if (retcode == SAJ_RETCODE_OK || retcode == SAJ_RETCODE_NO_DATA) {
            retcode = saj_dataReaderParDemStack_copy_out(&ctx);
        }
    }
    emptyReaderContext(&ctx);
    return (jint)retcode;
}

JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniReadWCondition)(
    JNIEnv *env,
    jclass object,
    jobject DataReaderView,
    jlong uView,
    jlong copyCache,
    jobject received_data,
    jobject info_seq,
    jint max_samples,
    jlong uQuery)
{
    u_result uResult;
    saj_readerContext ctx;
    saj_returnCode retcode;

    OS_UNUSED_ARG(object);
    OS_UNUSED_ARG(uView);

    retcode = fillReaderContext(env,
                (saj_copyCache)(PA_ADDRCAST)copyCache,
                received_data,
                info_seq,
                max_samples,
                DataReaderView,
                uQuery,
                &ctx);
    if (retcode == SAJ_RETCODE_OK) {
        uResult = u_queryRead(SAJ_VOIDP(uQuery), cmn_reader_action, ctx.samplesList, OS_DURATION_ZERO);
        retcode = saj_retcode_from_user_result(uResult);
        if (retcode == SAJ_RETCODE_OK || retcode == SAJ_RETCODE_NO_DATA) {
            retcode = saj_dataReaderParDemStack_copy_out(&ctx);
        }
    }
    emptyReaderContext(&ctx);

    return (jint)retcode;
}


/*
 * Class:     org_opensplice_dds_dcps_FooDataReaderViewImpl
 * Method:    jniTake
 * Signature: (Ljava/lang/Object;Ljava/lang/Object;LDDS/SampleInfoSeqHolder;IIII)I
 */
/*
    private native static int jniTake (
        Object DataReaderView,
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
    jobject DataReaderView,
    jlong uView,
    jlong copyCache,
    jobject received_data,
    jobject info_seq,
    jint max_samples,
    jint sample_states,
    jint view_states,
    jint instance_states)
{
    u_result uResult;
    saj_readerContext ctx;
    u_sampleMask mask;
    saj_returnCode retcode;

    OS_UNUSED_ARG(object);

    retcode = fillReaderContext(env,
        (saj_copyCache)(PA_ADDRCAST)copyCache,
        received_data,
        info_seq,
        max_samples,
        DataReaderView,
        uView,
        &ctx);

    if (retcode == SAJ_RETCODE_OK) {
        retcode = DDS_SAMPLE_MASK_CHECK(sample_states, view_states, instance_states);
        if (retcode == SAJ_RETCODE_BAD_PARAMETER) {
            SAJ_REPORT(retcode, "Invalid sample mask(0x%x),view mask(0x%x) or instance mask(0x%x)", sample_states, view_states, instance_states);
        }
    }
    if (retcode == SAJ_RETCODE_OK) {
        mask = DDS_SAMPLE_MASK(sample_states, view_states, instance_states);
        uResult = u_dataViewTake(SAJ_VOIDP(uView), mask, cmn_reader_action, ctx.samplesList, OS_DURATION_ZERO);
        retcode = saj_retcode_from_user_result(uResult);
        if (retcode == SAJ_RETCODE_OK || retcode == SAJ_RETCODE_NO_DATA) {
            retcode = saj_dataReaderParDemStack_copy_out(&ctx);
        }
    }
    emptyReaderContext(&ctx);

    return retcode;
}

JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniTakeWCondition)(
    JNIEnv *env,
    jclass object,
    jobject DataReaderView,
    jlong uView,
    jlong copyCache,
    jobject received_data,
    jobject info_seq,
    jint max_samples,
    jlong uQuery)
{
    u_result uResult;
    saj_readerContext ctx;
    saj_returnCode retcode;

    OS_UNUSED_ARG(object);
    OS_UNUSED_ARG(uView);

    retcode = fillReaderContext(env,
        (saj_copyCache)(PA_ADDRCAST)copyCache,
        received_data,
        info_seq,
        max_samples,
        DataReaderView,
        uQuery,
        &ctx);

    if (retcode == SAJ_RETCODE_OK) {
        uResult = u_queryTake(SAJ_VOIDP(uQuery), cmn_reader_action, ctx.samplesList, OS_DURATION_ZERO);
        retcode = saj_retcode_from_user_result(uResult);
        if (retcode == SAJ_RETCODE_OK || retcode == SAJ_RETCODE_NO_DATA) {
            retcode = saj_dataReaderParDemStack_copy_out(&ctx);
        }
    }
    emptyReaderContext(&ctx);

    return retcode;
}

/*
 * Class:     org_opensplice_dds_dcps_FooDataReaderViewImpl
 * Method:    jniReadInstance
 * Signature: (Ljava/lang/Object;Ljava/lang/Object;LDDS/SampleInfoSeqHolder;IIIII)I
 */
/*
    private native static int jniReadInstance (
        Object DataReaderView,
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
    jobject DataReaderView,
    jlong uView,
    jlong copyCache,
    jobject received_data,
    jobject info_seq,
    jint max_samples,
    jlong a_handle,
    jint sample_states,
    jint view_states,
    jint instance_states)
{
    u_result uResult;
    saj_readerContext ctx;
    u_sampleMask mask;
    saj_returnCode retcode;

    OS_UNUSED_ARG(object);

    retcode = fillReaderContext(env,
                (saj_copyCache)(PA_ADDRCAST)copyCache,
                received_data,
                info_seq,
                max_samples,
                DataReaderView,
                uView,
                &ctx);

    if (retcode == SAJ_RETCODE_OK) {
        retcode = DDS_SAMPLE_MASK_CHECK(sample_states, view_states, instance_states);
        if (retcode == SAJ_RETCODE_BAD_PARAMETER) {
            SAJ_REPORT(retcode, "Invalid sample mask(0x%x),view mask(0x%x) or instance mask(0x%x)", sample_states, view_states, instance_states);
        }
    }
    if (retcode == SAJ_RETCODE_OK) {
        mask = DDS_SAMPLE_MASK(sample_states, view_states, instance_states);
        uResult = u_dataViewReadInstance(SAJ_VOIDP(uView), (u_instanceHandle)a_handle, mask, cmn_reader_action, ctx.samplesList, OS_DURATION_ZERO);
        retcode = saj_retcode_from_user_result(uResult);
        if (retcode == SAJ_RETCODE_OK || retcode == SAJ_RETCODE_NO_DATA) {
            retcode = saj_dataReaderParDemStack_copy_out(&ctx);
        }
    }
    emptyReaderContext(&ctx);

    return retcode;
}

/*
 * Class:     org_opensplice_dds_dcps_FooDataReaderViewImpl
 * Method:    jniTakeInstance
 * Signature: (Ljava/lang/Object;Ljava/lang/Object;LDDS/SampleInfoSeqHolder;IIIII)I
 */
/*
    private native static int jniTakeInstance (
        Object DataReaderView,
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
    jobject DataReaderView,
    jlong uView,
    jlong copyCache,
    jobject received_data,
    jobject info_seq,
    jint max_samples,
    jlong a_handle,
    jint sample_states,
    jint view_states,
    jint instance_states)
{
    u_result uResult;
    saj_readerContext ctx;
    u_sampleMask mask;
    saj_returnCode retcode;

    OS_UNUSED_ARG(object);

    retcode = fillReaderContext(env,
                (saj_copyCache)(PA_ADDRCAST)copyCache,
                received_data,
                info_seq,
                max_samples,
                DataReaderView,
                uView,
                &ctx);

    if (retcode == SAJ_RETCODE_OK) {
        retcode = DDS_SAMPLE_MASK_CHECK(sample_states, view_states, instance_states);
        if (retcode == SAJ_RETCODE_BAD_PARAMETER) {
            SAJ_REPORT(retcode, "Invalid sample mask(0x%x),view mask(0x%x) or instance mask(0x%x)", sample_states, view_states, instance_states);
        }
    }
    if (retcode == SAJ_RETCODE_OK) {
        mask = DDS_SAMPLE_MASK(sample_states, view_states, instance_states);
        uResult = u_dataViewTakeInstance(SAJ_VOIDP(uView), (u_instanceHandle)a_handle, mask, cmn_reader_action, ctx.samplesList, OS_DURATION_ZERO);
        retcode = saj_retcode_from_user_result(uResult);
        if (retcode == SAJ_RETCODE_OK || retcode == SAJ_RETCODE_NO_DATA) {
            retcode = saj_dataReaderParDemStack_copy_out(&ctx);
        }
    }
    emptyReaderContext(&ctx);

    return retcode;
}

/*
 * Class:     org_opensplice_dds_dcps_FooDataReaderViewImpl
 * Method:    jniReadNextInstance
 * Signature: (Ljava/lang/Object;Ljava/lang/Object;LDDS/SampleInfoSeqHolder;IIIII)I
 */
/*
    private native static int jniReadNextInstance (
        Object DataReaderView,
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
    jobject DataReaderView,
    jlong uView,
    jlong copyCache,
    jobject received_data,
    jobject info_seq,
    jint max_samples,
    jlong a_handle,
    jint sample_states,
    jint view_states,
    jint instance_states)
{
    u_result uResult;
    saj_readerContext ctx;
    u_sampleMask mask;
    saj_returnCode retcode;

    OS_UNUSED_ARG(object);

    retcode = fillReaderContext(env,
                (saj_copyCache)(PA_ADDRCAST)copyCache,
                received_data,
                info_seq,
                max_samples,
                DataReaderView,
                uView,
                &ctx);

    if (retcode == SAJ_RETCODE_OK) {
        retcode = DDS_SAMPLE_MASK_CHECK(sample_states, view_states, instance_states);
        if (retcode == SAJ_RETCODE_BAD_PARAMETER) {
            SAJ_REPORT(retcode, "Invalid sample mask(0x%x),view mask(0x%x) or instance mask(0x%x)", sample_states, view_states, instance_states);
        }
    }
    if (retcode == SAJ_RETCODE_OK) {
        mask = DDS_SAMPLE_MASK(sample_states, view_states, instance_states);
        uResult = u_dataViewReadNextInstance(SAJ_VOIDP(uView), (u_instanceHandle)a_handle, mask, cmn_reader_nextInstanceAction_OSPL3588, ctx.samplesList, OS_DURATION_ZERO);
        retcode = saj_retcode_from_user_result(uResult);
        if (retcode == SAJ_RETCODE_OK || retcode == SAJ_RETCODE_NO_DATA) {
            retcode = saj_dataReaderParDemStack_copy_out(&ctx);
        }
    }
    emptyReaderContext(&ctx);

    return retcode;
}

JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniReadNextInstanceWCondition)(
    JNIEnv *env,
    jclass object,
    jobject DataReaderView,
    jlong uView,
    jlong copyCache,
    jobject received_data,
    jobject info_seq,
    jint max_samples,
    jlong a_handle,
    jlong uQuery)
{
    u_result uResult;
    saj_readerContext ctx;
    saj_returnCode retcode;

    OS_UNUSED_ARG(object);
    OS_UNUSED_ARG(uView);

    retcode = fillReaderContext(env,
                (saj_copyCache)(PA_ADDRCAST)copyCache,
                received_data,
                info_seq,
                max_samples,
                DataReaderView,
                uQuery,
                &ctx);

    if (retcode == SAJ_RETCODE_OK) {
        uResult = u_queryReadNextInstance(SAJ_VOIDP(uQuery), (u_instanceHandle)a_handle, cmn_reader_action, ctx.samplesList, OS_DURATION_ZERO);
        retcode = saj_retcode_from_user_result(uResult);
        if (retcode == SAJ_RETCODE_OK || retcode == SAJ_RETCODE_NO_DATA) {
            retcode = saj_dataReaderParDemStack_copy_out(&ctx);
        }
    }
    emptyReaderContext(&ctx);

    return retcode;
}

/*
 * Class:     org_opensplice_dds_dcps_FooDataReaderViewImpl
 * Method:    jniTakeNextInstance
 * Signature: (Ljava/lang/Object;Ljava/lang/Object;LDDS/SampleInfoSeqHolder;IIIII)I
 */
/*
    private native static int jniTakeNextInstance (
        Object DataReaderView,
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
    jobject DataReaderView,
    jlong uView,
    jlong copyCache,
    jobject received_data,
    jobject info_seq,
    jint max_samples,
    jlong a_handle,
    jint sample_states,
    jint view_states,
    jint instance_states)
{
    u_result uResult;
    saj_readerContext ctx;
    u_sampleMask mask;
    saj_returnCode retcode;

    OS_UNUSED_ARG(object);

    retcode = fillReaderContext(env,
                (saj_copyCache)(PA_ADDRCAST)copyCache,
                received_data,
                info_seq,
                max_samples,
                DataReaderView,
                uView,
                &ctx);

    if (retcode == SAJ_RETCODE_OK) {
        retcode = DDS_SAMPLE_MASK_CHECK(sample_states, view_states, instance_states);
        if (retcode == SAJ_RETCODE_BAD_PARAMETER) {
            SAJ_REPORT(retcode, "Invalid sample mask(0x%x),view mask(0x%x) or instance mask(0x%x)", sample_states, view_states, instance_states);
        }
    }
    if (retcode == SAJ_RETCODE_OK) {
        mask = DDS_SAMPLE_MASK(sample_states, view_states, instance_states);
        uResult = u_dataViewTakeNextInstance(SAJ_VOIDP(uView), (u_instanceHandle)a_handle, mask, cmn_reader_nextInstanceAction_OSPL3588, ctx.samplesList, OS_DURATION_ZERO);
        retcode = saj_retcode_from_user_result(uResult);
        if (retcode == SAJ_RETCODE_OK || retcode == SAJ_RETCODE_NO_DATA) {
            retcode = saj_dataReaderParDemStack_copy_out(&ctx);
        }
    }
    emptyReaderContext(&ctx);

    return retcode;
}

JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniTakeNextInstanceWCondition)(
    JNIEnv *env,
    jclass object,
    jobject DataReaderView,
    jlong uView,
    jlong copyCache,
    jobject received_data,
    jobject info_seq,
    jint max_samples,
    jlong a_handle,
    jlong uQuery)
{
    u_result uResult;
    saj_readerContext ctx;
    saj_returnCode retcode;

    OS_UNUSED_ARG(object);
    OS_UNUSED_ARG(uView);

    retcode = fillReaderContext(env,
                (saj_copyCache)(PA_ADDRCAST)copyCache,
                received_data,
                info_seq,
                max_samples,
                DataReaderView,
                uQuery,
                &ctx);

    if (retcode == SAJ_RETCODE_OK) {
        uResult = u_queryTakeNextInstance(SAJ_VOIDP(uQuery), (u_instanceHandle)a_handle, cmn_reader_action, ctx.samplesList, OS_DURATION_ZERO);
        retcode = saj_retcode_from_user_result(uResult);
        if (retcode == SAJ_RETCODE_OK || retcode == SAJ_RETCODE_NO_DATA) {
            retcode = saj_dataReaderParDemStack_copy_out(&ctx);
        }
    }
    emptyReaderContext(&ctx);

    return retcode;
}

/*
 * Class:     org_opensplice_dds_dcps_FooDataReaderViewImpl
 * Method:    jniGetKeyValue
 * Signature: (Ljava/lang/Object;Ljava/lang/Object;I)I
 */
/*
    private native static int jniGetKeyValue (
        Object DataReaderView,
        long copyCache,
        Object key_holder,
        long handle);
*/
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetKeyValue)(
    JNIEnv *env,
    jclass object,
    jlong uView,
    jlong copyCache,
    jobject key_holder,
    jlong handle)
{
#if 0
    saj_returnCode retcode;
    C_STRUCT(saj_dstInfo) dstInfo;
    void *dst = NULL;
    u_result uResult;
    jobject element;
    sajReaderCopyCache *rc = saj_copyCacheReaderCache((saj_copyCache)(PA_ADDRCAST)copyCache);

    if (key_holder != NULL) {
        element = (*env)->GetObjectField(env, key_holder, rc->dataHolder_value_fid);
        CHECK_EXCEPTION(env);
        dstInfo.javaEnv = env;
        dstInfo.javaObject = element;
        dstInfo.copyProgram = (saj_copyCache)(PA_ADDRCAST)copyCache;
        dst = (void *)&dstInfo;
    }

    /* TODO: Solve availability of u_dataView_get_key_value(). */
    uResult = u_dataView_get_key_value(
                SAJ_VOIDP(uView),
                (void *)dst,
                (u_instanceHandle)handle);
    retcode = saj_retcode_from_user_result(uResult);
    if ((key_holder != NULL) && (dstInfo.javaObject != element)) {
        (*env)->SetObjectField(env, key_holder, rc->dataHolder_value_fid, dstInfo.javaObject);
    }

    return retcode;
    CATCH_EXCEPTION: return SAJ_RETCODE_ERROR;
#else
    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(object);
    OS_UNUSED_ARG(uView);
    OS_UNUSED_ARG(copyCache);
    OS_UNUSED_ARG(key_holder);
    OS_UNUSED_ARG(handle);
    SAJ_REPORT(SAJ_RETCODE_UNSUPPORTED, "GetKeyValue not supported");
    return SAJ_RETCODE_UNSUPPORTED;
#endif
}

static v_copyin_result
fooCopyIn (
    c_type type,
    const void *data,
    void *to)
{
    v_copyin_result result;
    os_int32 copyResult = saj_copyInStruct(c_getBase(type), data, to);

    switch (copyResult) {
    case OS_RETCODE_OK:
        result = V_COPYIN_RESULT_OK;
        break;
    case OS_RETCODE_BAD_PARAMETER:
        result = V_COPYIN_RESULT_INVALID;
        break;
    case OS_RETCODE_OUT_OF_RESOURCES:
        result = V_COPYIN_RESULT_OUT_OF_MEMORY;
        break;
    default:
        result = V_COPYIN_RESULT_OK;
        break;
    }

    return result;
}

/*
 * Class:     org_opensplice_dds_dcps_FooDataReaderViewImpl
 * Method:    jniLookupInstance
 * Signature: (Ljava/lang/Object;Ljava/lang/Object;I)I
 */
/*
    private native static int jniLookupInstance (
        Object DataReaderView,
        long copyCache,
        Object instance);
*/
JNIEXPORT jlong JNICALL
SAJ_FUNCTION(jniLookupInstance)(
    JNIEnv *env,
    jclass object,
    jlong uView,
    jlong copyCache,
    jobject instance)
{
    int result = SAJ_RETCODE_OK;
    C_STRUCT(saj_srcInfo) srcInfo;
    u_result uResult;
    u_instanceHandle uHandle = U_INSTANCEHANDLE_NIL;

    OS_UNUSED_ARG(object);

    if (instance != NULL) {
        srcInfo.javaEnv = env;
        srcInfo.javaObject = instance;
        srcInfo.copyProgram = (saj_copyCache)(PA_ADDRCAST)copyCache;
        uResult = u_dataViewLookupInstance(SAJ_VOIDP(uView), &srcInfo, fooCopyIn, &uHandle);
        result = saj_retcode_from_user_result(uResult);
        if (result != SAJ_RETCODE_OK) {
            SAJ_REPORT(result, "Failed to lookup instance.");
        }
    }

    return uHandle;
}
