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
#include "saj_FooDataReader.h"
#include "saj__fooDataReader.h"
#include "saj_copyIn.h"
#include "saj_copyOut.h"
#include "saj_utilities.h"
#include "saj__readerContext.h"
#include "v_dataReaderSample.h"
#include "saj_dataReaderParDem.h"
#include "cmn_samplesList.h"
#include "cmn_reader.h"

#include "os_heap.h"
#include "saj__report.h"

/* Defines the package of the java implementation classes */
#define SAJ_PACKAGENAME "org/opensplice/dds/dcps/"
#define SAJ_FUNCTION(name) Java_org_opensplice_dds_dcps_FooDataReaderImpl_##name

static saj_returnCode
fillReaderContext(
    JNIEnv *env,
    jobject dataReader,
    jlong uReader,
    saj_copyCache copyCache,
    jobject data_seq,
    jobject info_seq,
    jint max_samples,
    saj_readerContext *ctx)
{
    int result;
    /* TODO: Implement this function properly. */
    sajReaderCopyCache *rc = saj_copyCacheReaderCache(copyCache);

    ctx->samplesList = NULL;

    if (data_seq == NULL) {
        result = SAJ_RETCODE_BAD_PARAMETER;
        SAJ_REPORT(result, "data_values '<NULL>' is invalid.");
        return result;
    }
    if (info_seq == NULL) {
        result = SAJ_RETCODE_BAD_PARAMETER;
        SAJ_REPORT(result, "info_seq 'null' is invalid.");
        return result;
    }

    ctx->javaEnv = env;
    ctx->copyCache = copyCache;
    ctx->dataSeqHolder = data_seq;
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
    /* Rule 5: when max_samples != LENGTH_UNLIMITED, then the following condition
    // needs to be met: maxSamples <= max_len  */
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
    assert(GET_CACHED(dataReaderImplClassParallelDemarshallingContext_fid));
    ctx->pardemCtx = (sajParDemContext)(PA_ADDRCAST)GET_LONG_FIELD(
        env, dataReader, dataReaderImplClassParallelDemarshallingContext);
    assert(GET_CACHED(dataReaderImplClassCDRCopy_fid));
    ctx->CDRCopy = GET_LONG_FIELD(env, dataReader, dataReaderImplClassCDRCopy);
    ctx->jreader = dataReader;
    ctx->uReader = uReader;

    ctx->samplesList = cmn_samplesList_new(FALSE);
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
    jlong uReader,
    jlong copyCache,
    jobject received_data,
    jobject info_seq,
    jint max_samples,
    jint sample_states,
    jint view_states,
    jint instance_states)
{
    u_sampleMask mask;
    u_result uResult;
    saj_readerContext ctx;
    saj_returnCode retcode;

    OS_UNUSED_ARG(object);

    retcode = fillReaderContext(env, DataReader, uReader,
                     (saj_copyCache)(PA_ADDRCAST)copyCache,
                     received_data,
                     info_seq,
                     max_samples,
                     &ctx);
    if (retcode == SAJ_RETCODE_OK) {
        retcode = DDS_SAMPLE_MASK_CHECK(sample_states, view_states, instance_states);
        if (retcode == SAJ_RETCODE_BAD_PARAMETER) {
            SAJ_REPORT(retcode, "Invalid sample mask(0x%x),view mask(0x%x) or instance mask(0x%x)", sample_states, view_states, instance_states);
        }
    }
    if (retcode == SAJ_RETCODE_OK) {
        mask = DDS_SAMPLE_MASK(sample_states, view_states, instance_states);
        uResult = u_dataReaderRead(SAJ_VOIDP(uReader), mask, cmn_reader_action, ctx.samplesList, OS_DURATION_ZERO);
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
    jobject DataReader,
    jlong uReader,
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
    OS_UNUSED_ARG(uReader);

    retcode = fillReaderContext(env, DataReader, uQuery,
                     (saj_copyCache)(PA_ADDRCAST)copyCache,
                     received_data,
                     info_seq,
                     max_samples,
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
    jlong uReader,
    jlong copyCache,
    jobject received_data,
    jobject info_seq,
    jint max_samples,
    jint sample_states,
    jint view_states,
    jint instance_states)
{
    u_sampleMask mask;
    saj_readerContext ctx;
    saj_returnCode retcode;
    u_result uResult;

    OS_UNUSED_ARG(object);

    retcode = fillReaderContext(env, DataReader, uReader,
        (saj_copyCache)(PA_ADDRCAST)copyCache,
        received_data,
        info_seq,
        max_samples,
        &ctx);

    if (retcode == SAJ_RETCODE_OK) {
        retcode = DDS_SAMPLE_MASK_CHECK(sample_states, view_states, instance_states);
        if (retcode == SAJ_RETCODE_BAD_PARAMETER) {
            SAJ_REPORT(retcode, "Invalid sample mask(0x%x),view mask(0x%x) or instance mask(0x%x)", sample_states, view_states, instance_states);
        }
    }
    if (retcode == SAJ_RETCODE_OK) {
        mask = DDS_SAMPLE_MASK(sample_states, view_states, instance_states);
        uResult = u_dataReaderTake(SAJ_VOIDP(uReader), mask, cmn_reader_action, ctx.samplesList, OS_DURATION_ZERO);
        retcode = saj_retcode_from_user_result(uResult);
        if (retcode == SAJ_RETCODE_OK || retcode == SAJ_RETCODE_NO_DATA) {
            retcode = saj_dataReaderParDemStack_copy_out(&ctx);
        }
    }
    emptyReaderContext(&ctx);

    return (jint)retcode;
}

JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniTakeWCondition)(
    JNIEnv *env,
    jclass object,
    jobject DataReader,
    jlong uReader,
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
    OS_UNUSED_ARG(uReader);

    retcode = fillReaderContext(env, DataReader, uQuery,
                     (saj_copyCache)(PA_ADDRCAST)copyCache,
                     received_data,
                     info_seq,
                     max_samples,
                     &ctx);
    if (retcode == SAJ_RETCODE_OK) {
        uResult = u_queryTake(SAJ_VOIDP(uQuery), cmn_reader_action, ctx.samplesList, OS_DURATION_ZERO);
        retcode = saj_retcode_from_user_result(uResult);
        if (retcode == SAJ_RETCODE_OK || retcode == SAJ_RETCODE_NO_DATA) {
            retcode = saj_dataReaderParDemStack_copy_out(&ctx);
        }
    }
    emptyReaderContext(&ctx);

    return (jint)retcode;
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
    jlong uReader,
    jlong copyCache,
    jobject received_data,
    jobject sample_info)
{
    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(object);
    OS_UNUSED_ARG(DataReader);
    OS_UNUSED_ARG(uReader);
    OS_UNUSED_ARG(copyCache);
    OS_UNUSED_ARG(received_data);
    OS_UNUSED_ARG(sample_info);
    SAJ_REPORT(SAJ_RETCODE_UNSUPPORTED, "ReadNextSample not supported");
    return (jint)SAJ_RETCODE_UNSUPPORTED;
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
    jlong uReader,
    jlong copyCache,
    jobject received_data,
    jobject sample_info)
{
    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(object);
    OS_UNUSED_ARG(DataReader);
    OS_UNUSED_ARG(uReader);
    OS_UNUSED_ARG(copyCache);
    OS_UNUSED_ARG(received_data);
    OS_UNUSED_ARG(sample_info);
    SAJ_REPORT(SAJ_RETCODE_UNSUPPORTED, "TakeNextSample not supported");
    return (jint)SAJ_RETCODE_UNSUPPORTED;
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
    jlong uReader,
    jlong copyCache,
    jobject received_data,
    jobject info_seq,
    jint max_samples,
    jlong a_handle,
    jint sample_states,
    jint view_states,
    jint instance_states)
{
    u_sampleMask mask;
    saj_readerContext ctx;
    saj_returnCode retcode;
    u_result uResult;

    OS_UNUSED_ARG(object);

    retcode = fillReaderContext(env, DataReader, uReader,
        (saj_copyCache)(PA_ADDRCAST)copyCache,
        received_data,
        info_seq,
        max_samples,
        &ctx);

    if (retcode == SAJ_RETCODE_OK) {
        retcode = DDS_SAMPLE_MASK_CHECK(sample_states, view_states, instance_states);
        if (retcode == SAJ_RETCODE_BAD_PARAMETER) {
            SAJ_REPORT(retcode, "Invalid sample mask(0x%x),view mask(0x%x) or instance mask(0x%x)", sample_states, view_states, instance_states);
        }
    }
    if (retcode == SAJ_RETCODE_OK) {
        mask = DDS_SAMPLE_MASK(sample_states, view_states, instance_states);
        uResult = u_dataReaderReadInstance(SAJ_VOIDP(uReader), a_handle, mask, cmn_reader_action, ctx.samplesList, OS_DURATION_ZERO);
        retcode = saj_retcode_from_user_result(uResult);
        if (retcode == SAJ_RETCODE_OK || retcode == SAJ_RETCODE_NO_DATA) {
            retcode = saj_dataReaderParDemStack_copy_out(&ctx);
        }
    }
    emptyReaderContext(&ctx);

    return (jint)retcode;
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
    jlong uReader,
    jlong copyCache,
    jobject received_data,
    jobject info_seq,
    jint max_samples,
    jlong a_handle,
    jint sample_states,
    jint view_states,
    jint instance_states)
{
    u_sampleMask mask;
    saj_readerContext ctx;
    saj_returnCode retcode;
    u_result uResult;

    OS_UNUSED_ARG(object);

    retcode = fillReaderContext(env, DataReader, uReader,
        (saj_copyCache)(PA_ADDRCAST)copyCache,
        received_data,
        info_seq,
        max_samples,
        &ctx);

    if (retcode == SAJ_RETCODE_OK) {
        retcode = DDS_SAMPLE_MASK_CHECK(sample_states, view_states, instance_states);
        if (retcode == SAJ_RETCODE_BAD_PARAMETER) {
            SAJ_REPORT(retcode, "Invalid sample mask(0x%x),view mask(0x%x) or instance mask(0x%x)", sample_states, view_states, instance_states);
        }
    }
    if (retcode == SAJ_RETCODE_OK) {
        mask = DDS_SAMPLE_MASK(sample_states, view_states, instance_states);
        uResult = u_dataReaderTakeInstance(SAJ_VOIDP(uReader), a_handle, mask, cmn_reader_action, ctx.samplesList, OS_DURATION_ZERO);
        retcode = saj_retcode_from_user_result(uResult);
        if (retcode == SAJ_RETCODE_OK || retcode == SAJ_RETCODE_NO_DATA) {
            retcode = saj_dataReaderParDemStack_copy_out(&ctx);
        }
    }
    emptyReaderContext(&ctx);

    return (jint)retcode;
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
    jlong uReader,
    jlong copyCache,
    jobject received_data,
    jobject info_seq,
    jint max_samples,
    jlong a_handle,
    jint sample_states,
    jint view_states,
    jint instance_states)
{
    u_sampleMask mask;
    saj_readerContext ctx;
    saj_returnCode retcode;
    u_result uResult;

    OS_UNUSED_ARG(object);

    retcode = fillReaderContext(env, DataReader, uReader,
        (saj_copyCache)(PA_ADDRCAST)copyCache,
        received_data,
        info_seq,
        max_samples,
        &ctx);

    if (retcode == SAJ_RETCODE_OK) {
        retcode = DDS_SAMPLE_MASK_CHECK(sample_states, view_states, instance_states);
        if (retcode == SAJ_RETCODE_BAD_PARAMETER) {
            SAJ_REPORT(retcode, "Invalid sample mask(0x%x),view mask(0x%x) or instance mask(0x%x)", sample_states, view_states, instance_states);
        }
    }
    if (retcode == SAJ_RETCODE_OK) {
        mask = DDS_SAMPLE_MASK(sample_states, view_states, instance_states);
        uResult = u_dataReaderReadNextInstance(SAJ_VOIDP(uReader), a_handle, mask, cmn_reader_nextInstanceAction, ctx.samplesList, OS_DURATION_ZERO);
        retcode = saj_retcode_from_user_result(uResult);
        if (retcode == SAJ_RETCODE_OK || retcode == SAJ_RETCODE_NO_DATA) {
            retcode = saj_dataReaderParDemStack_copy_out(&ctx);
        }
    }
    emptyReaderContext(&ctx);

    return (jint)retcode;
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
    jlong uReader,
    jlong copyCache,
    jobject received_data,
    jobject info_seq,
    jint max_samples,
    jlong a_handle,
    jint sample_states,
    jint view_states,
    jint instance_states)
{
    u_sampleMask mask;
    saj_readerContext ctx;
    saj_returnCode retcode;
    u_result uResult;

    OS_UNUSED_ARG(object);

    retcode = fillReaderContext(env, DataReader, uReader,
        (saj_copyCache)(PA_ADDRCAST)copyCache,
        received_data,
        info_seq,
        max_samples,
        &ctx);

    if (retcode == SAJ_RETCODE_OK) {
        retcode = DDS_SAMPLE_MASK_CHECK(sample_states, view_states, instance_states);
        if (retcode == SAJ_RETCODE_BAD_PARAMETER) {
            SAJ_REPORT(retcode, "Invalid sample mask(0x%x),view mask(0x%x) or instance mask(0x%x)", sample_states, view_states, instance_states);
        }
    }
    if (retcode == SAJ_RETCODE_OK) {
        mask = DDS_SAMPLE_MASK(sample_states, view_states, instance_states);
        uResult = u_dataReaderTakeNextInstance(SAJ_VOIDP(uReader), a_handle, mask, cmn_reader_nextInstanceAction, ctx.samplesList, OS_DURATION_ZERO);
        retcode = saj_retcode_from_user_result(uResult);
        if (retcode == SAJ_RETCODE_OK || retcode == SAJ_RETCODE_NO_DATA) {
            retcode = saj_dataReaderParDemStack_copy_out(&ctx);
        }
    }
    emptyReaderContext(&ctx);

    return (jint)retcode;
}

JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniReadNextInstanceWCondition)(
    JNIEnv *env,
    jclass object,
    jobject DataReader,
    jlong uReader,
    jlong copyCache,
    jobject received_data,
    jobject info_seq,
    jint max_samples,
    jlong a_handle,
    jlong uQuery)
{
    saj_readerContext ctx;
    saj_returnCode retcode;
    u_result uResult;

    OS_UNUSED_ARG(object);
    OS_UNUSED_ARG(uReader);

    retcode = fillReaderContext(env, DataReader, uQuery,
        (saj_copyCache)(PA_ADDRCAST)copyCache,
        received_data,
        info_seq,
        max_samples,
        &ctx);

    if (retcode == SAJ_RETCODE_OK) {
        uResult = u_queryReadNextInstance(SAJ_VOIDP(uQuery), a_handle, cmn_reader_nextInstanceAction, ctx.samplesList, OS_DURATION_ZERO);
        retcode = saj_retcode_from_user_result(uResult);
        if (retcode == SAJ_RETCODE_OK || retcode == SAJ_RETCODE_NO_DATA) {
            retcode = saj_dataReaderParDemStack_copy_out(&ctx);
        }
    }
    emptyReaderContext(&ctx);

    return (jint)retcode;
}

JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniTakeNextInstanceWCondition)(
    JNIEnv *env,
    jclass object,
    jobject DataReader,
    jlong uReader,
    jlong copyCache,
    jobject received_data,
    jobject info_seq,
    jint max_samples,
    jlong a_handle,
    jlong uQuery)
{
    saj_readerContext ctx;
    saj_returnCode retcode;
    u_result uResult;

    OS_UNUSED_ARG(object);
    OS_UNUSED_ARG(uReader);

    retcode = fillReaderContext(env, DataReader, uQuery,
        (saj_copyCache)(PA_ADDRCAST)copyCache,
        received_data,
        info_seq,
        max_samples,
        &ctx);

    if (retcode == SAJ_RETCODE_OK) {
        uResult = u_queryTakeNextInstance(SAJ_VOIDP(uQuery), a_handle, cmn_reader_nextInstanceAction, ctx.samplesList, OS_DURATION_ZERO);
        retcode = saj_retcode_from_user_result(uResult);
        if (retcode == SAJ_RETCODE_OK || retcode == SAJ_RETCODE_NO_DATA) {
            retcode = saj_dataReaderParDemStack_copy_out(&ctx);
        }
    }
    emptyReaderContext(&ctx);

    return (jint)retcode;
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
    jlong uReader,
    jlong copyCache,
    jobject key_holder,
    jlong handle)
{
    C_STRUCT(saj_dstInfo) dstInfo;
    u_result uResult;
    saj_returnCode retcode;
    sajReaderCopyCache *rc = saj_copyCacheReaderCache((saj_copyCache)(PA_ADDRCAST)copyCache);

    OS_UNUSED_ARG(object);

    if (key_holder != NULL) {
        dstInfo.javaEnv = env;
        dstInfo.javaObject = NULL;
        dstInfo.copyProgram = (saj_copyCache)(PA_ADDRCAST)copyCache;

        uResult = u_dataReaderCopyKeysFromInstanceHandle(
                        SAJ_VOIDP(uReader),
                        (u_instanceHandle)handle,
                        (u_copyOut)saj_copyOutStruct,
                        &dstInfo);
        retcode = saj_retcode_from_user_result(uResult);
        if (retcode == SAJ_RETCODE_ALREADY_DELETED) {
            /* Already deleted is caused by an invalid handle.
             * In the context of this operation this should result
             * in a precondition not met error code.
             */
            retcode = SAJ_RETCODE_PRECONDITION_NOT_MET;
        }
        (*env)->SetObjectField(env, key_holder, rc->dataHolder_value_fid, dstInfo.javaObject);
        CHECK_EXCEPTION(env);
        DELETE_LOCAL_REF(env, dstInfo.javaObject);
    } else {
        retcode = SAJ_RETCODE_BAD_PARAMETER;
    }

    return (jint)retcode;

    CATCH_EXCEPTION:
    return SAJ_RETCODE_ERROR;
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
    jlong uReader,
    jlong copyCache,
    jobject instance)
{
    saj_returnCode result;
    C_STRUCT(saj_srcInfo) srcInfo;
    u_result uResult;
    u_instanceHandle uHandle = U_INSTANCEHANDLE_NIL;

    OS_UNUSED_ARG(object);

    if (instance != NULL) {
        srcInfo.javaEnv = env;
        srcInfo.javaObject = instance;
        srcInfo.copyProgram = (saj_copyCache)(PA_ADDRCAST)copyCache;
        uResult = u_dataReaderLookupInstance(SAJ_VOIDP(uReader), &srcInfo, fooCopyIn, &uHandle);
        result = saj_retcode_from_user_result(uResult);
        if (result != SAJ_RETCODE_OK) {
            SAJ_REPORT(result, "Failed to lookup instance.");
        }
    } else {
        SAJ_REPORT(SAJ_RETCODE_ERROR, "Instance is null.");
    }
    return (jlong)uHandle;
}
