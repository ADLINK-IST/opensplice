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

#include "saj_FooDataWriter.h"
#include "saj_copyIn.h"
#include "saj_copyOut.h"
#include "saj_copyCache.h"
#include "saj_utilities.h"

#include "gapi.h"

/* Defines the package of the java implementation classes */
#define SAJ_PACKAGENAME "org/opensplice/dds/dcps/"
#define SAJ_FUNCTION(name) Java_org_opensplice_dds_dcps_FooDataWriterImpl_##name

/*
 * Class:     org_opensplice_dds_dcps_FooDataWriterImpl
 * Method:    jniRegisterInstance
 * Signature: (Ljava/lang/Object;Ljava/lang/Object;)I
 */
/*
    private native static long jniRegisterInstance (
        Object DataWriter,
	long copyCache,
        Object instance_data);
*/
JNIEXPORT jlong JNICALL
SAJ_FUNCTION(jniRegisterInstance) (
    JNIEnv *env,
    jclass object,
    jobject DataWriter,
    jlong copyCache,
    jobject instance_data)
{
    C_STRUCT(saj_srcInfo) srcInfo;
    const gapi_foo *src = NULL;

    OS_UNUSED_ARG(object);

    if (instance_data != NULL) {
	srcInfo.javaEnv = env;
        srcInfo.javaObject = instance_data;
        srcInfo.copyProgram = (saj_copyCache)(PA_ADDRCAST)copyCache;
	src = (const gapi_foo *)&srcInfo;
    }

    return (jlong)
	gapi_fooDataWriter_register_instance (
	    (gapi_fooDataWriter)saj_read_gapi_address (env, DataWriter),
	    src);
}

/*
 * Class:     org_opensplice_dds_dcps_FooDataWriterImpl
 * Method:    jniRegisterInstanceWTimestamp
 * Signature: (Ljava/lang/Object;Ljava/lang/Object;LDDS/Time_t;)I
 */
/*
    private native static long jniRegisterInstanceWTimestamp (
        Object DataWriter,
	long copyCache,
        Object instance_data,
        DDS.Time_t source_timestamp);
*/
JNIEXPORT jlong JNICALL
SAJ_FUNCTION(jniRegisterInstanceWTimestamp) (
    JNIEnv *env,
    jclass object,
    jobject DataWriter,
    jlong copyCache,
    jobject instance_data,
    jobject source_timestamp)
{
    C_STRUCT(saj_srcInfo) srcInfo;
    gapi_time_t timestamp;
    const gapi_time_t *ts = NULL;
    const gapi_foo *src = NULL;

    OS_UNUSED_ARG(object);

    if (instance_data != NULL) {
        srcInfo.javaEnv = env;
        srcInfo.javaObject = instance_data;
        srcInfo.copyProgram = (saj_copyCache)(PA_ADDRCAST)copyCache;
        src = (const gapi_foo *)&srcInfo;
    }
    if (source_timestamp != NULL) {
        if (saj_timeCopyIn (env, source_timestamp, &timestamp) == SAJ_RETCODE_OK) {
            ts = (const gapi_time_t *)&timestamp;
	}
    }

    return (jlong)
	gapi_fooDataWriter_register_instance_w_timestamp (
	    (gapi_fooDataWriter)saj_read_gapi_address (env, DataWriter),
	    src,
	    ts);
}

/*
 * Class:     org_opensplice_dds_dcps_FooDataWriterImpl
 * Method:    jniUnregisterInstance
 * Signature: (Ljava/lang/Object;Ljava/lang/Object;I)I
 */
/*
    private native static int jniUnregisterInstance (
        Object DataWriter,
	long copyCache,
        Object instance_data,
        long handle);
*/
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniUnregisterInstance) (
    JNIEnv *env,
    jclass object,
    jobject DataWriter,
    jlong copyCache,
    jobject instance_data,
    jlong handle)
{
    C_STRUCT(saj_srcInfo) srcInfo;
    const gapi_foo *src = NULL;

    OS_UNUSED_ARG(object);

    if (instance_data != NULL) {
        srcInfo.javaEnv = env;
        srcInfo.javaObject = instance_data;
        srcInfo.copyProgram = (saj_copyCache)(PA_ADDRCAST)copyCache;
	src = (const gapi_foo *)&srcInfo;
    }

    return (jint)
	gapi_fooDataWriter_unregister_instance (
	    (gapi_fooDataWriter)saj_read_gapi_address (env, DataWriter),
	    src,
	    (const gapi_instanceHandle_t)handle);
}

/*
 * Class:     org_opensplice_dds_dcps_FooDataWriterImpl
 * Method:    jniUnregisterInstanceWTimestamp
 * Signature: (Ljava/lang/Object;Ljava/lang/Object;ILDDS/Time_t;)I
 */
/*
    private native static int jniUnregisterInstanceWTimestamp (
        Object DataWriter,
	long copyCache,
        Object instance_data,
        long handle,
        DDS.Time_t source_timestamp);
*/
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniUnregisterInstanceWTimestamp) (
    JNIEnv *env,
    jclass object,
    jobject DataWriter,
    jlong copyCache,
    jobject instance_data,
    jlong handle,
    jobject source_timestamp)
{
    C_STRUCT(saj_srcInfo) srcInfo;
    gapi_time_t timestamp;
    const gapi_time_t *ts = NULL;
    const gapi_foo *src = NULL;

    OS_UNUSED_ARG(object);

    srcInfo.javaEnv = env;
    if (instance_data != NULL) {
        srcInfo.javaObject = instance_data;
        srcInfo.copyProgram = (saj_copyCache)(PA_ADDRCAST)copyCache;
	src = (const gapi_foo *)&srcInfo;
    }
    if (source_timestamp != NULL) {
        if (saj_timeCopyIn (env, source_timestamp, &timestamp) == SAJ_RETCODE_OK) {
	    ts = (const gapi_time_t *)&timestamp;
	}
    }

    return (jint)
	gapi_fooDataWriter_unregister_instance_w_timestamp (
	    (gapi_fooDataWriter)saj_read_gapi_address (env, DataWriter),
	    src,
	    (gapi_instanceHandle_t)handle,
	    ts);
}

/*
 * Class:     org_opensplice_dds_dcps_FooDataWriterImpl
 * Method:    jniWrite
 * Signature: (Ljava/lang/Object;Ljava/lang/Object;I)I
 */
/*
    private native static int jniWrite (
        Object DataWriter,
	long copyCache,
        Object instance_data,
        long handle);
*/
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniWrite) (
    JNIEnv *env,
    jclass object,
    jobject DataWriter,
    jlong copyCache,
    jobject instance_data,
    jlong handle)
{
    C_STRUCT(saj_srcInfo) srcInfo;
    const gapi_foo *src = NULL;

    OS_UNUSED_ARG(object);

    if (instance_data != NULL) {
        srcInfo.javaEnv = env;
        srcInfo.javaObject = instance_data;
        srcInfo.copyProgram = (saj_copyCache)(PA_ADDRCAST)copyCache;
	src = (const gapi_foo *)&srcInfo;
    }

    return (jint)
	gapi_fooDataWriter_write (
	    (gapi_fooDataWriter)saj_read_gapi_address (env, DataWriter),
	    src,
	    (gapi_instanceHandle_t)handle);
}

/*
 * Class:     org_opensplice_dds_dcps_FooDataWriterImpl
 * Method:    jniWriteWTimestamp
 * Signature: (Ljava/lang/Object;Ljava/lang/Object;ILDDS/Time_t;)I
 */
/*
    private native static int jniWriteWTimestamp (
        Object DataWriter,
	long copyCache,
        Object instance_data,
        long handle,
        DDS.Time_t source_timestamp);
*/
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniWriteWTimestamp) (
    JNIEnv *env,
    jclass object,
    jobject DataWriter,
    jlong copyCache,
    jobject instance_data,
    jlong handle,
    jobject source_timestamp)
{
    C_STRUCT(saj_srcInfo) srcInfo;
    gapi_time_t timestamp;
    const gapi_time_t *ts = NULL;
    const gapi_foo *src = NULL;

    OS_UNUSED_ARG(object);

    if (instance_data != NULL && source_timestamp != NULL) {
        srcInfo.javaEnv = env;
        srcInfo.javaObject = instance_data;
        srcInfo.copyProgram = (saj_copyCache)(PA_ADDRCAST)copyCache;
        if (saj_timeCopyIn (env, source_timestamp, &timestamp) == SAJ_RETCODE_OK) {
	    src = (const gapi_foo *)&srcInfo;
	    ts = (const gapi_time_t *)&timestamp;
	}
    }

    return (jint)
	gapi_fooDataWriter_write_w_timestamp (
	    (gapi_fooDataWriter)saj_read_gapi_address (env, DataWriter),
	    src,
	    (gapi_instanceHandle_t)handle,
	    ts);
}

/*
 * Class:     org_opensplice_dds_dcps_FooDataWriterImpl
 * Method:    jniDispose
 * Signature: (Ljava/lang/Object;Ljava/lang/Object;I)I
 */
/*
    private native static int jniDispose (
        Object DataWriter,
	long copyCache,
        Object instance_data,
        long instance_handle);
*/
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniDispose) (
    JNIEnv *env,
    jclass object,
    jobject DataWriter,
    jlong copyCache,
    jobject instance_data,
    jlong instance_handle)
{
    C_STRUCT(saj_srcInfo) srcInfo;
    const gapi_foo *src = NULL;

    OS_UNUSED_ARG(object);

    if (instance_data != NULL) {
        srcInfo.javaEnv = env;
        srcInfo.javaObject = instance_data;
        srcInfo.copyProgram = (saj_copyCache)(PA_ADDRCAST)copyCache;
	src = (const gapi_foo *)&srcInfo;
    }

    return (jint)
	gapi_fooDataWriter_dispose (
	    (gapi_fooDataWriter)saj_read_gapi_address (env, DataWriter),
	    src,
	    (gapi_instanceHandle_t)instance_handle);
}

/*
 * Class:     org_opensplice_dds_dcps_FooDataWriterImpl
 * Method:    jniDisposeWTimestamp
 * Signature: (Ljava/lang/Object;Ljava/lang/Object;ILDDS/Time_t;)I
 */
/*
    private native static int jniDisposeWTimestamp (
        Object DataWriter,
	long copyCache,
        Object instance_data,
        long instance_handle,
        DDS.Time_t source_timestamp);
*/
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniDisposeWTimestamp) (
    JNIEnv *env,
    jclass object,
    jobject DataWriter,
    jlong copyCache,
    jobject instance_data,
    jlong instance_handle,
    jobject source_timestamp)
{
    C_STRUCT(saj_srcInfo) srcInfo;
    gapi_time_t timestamp;
    const gapi_time_t *ts = NULL;
    const gapi_foo *src = NULL;

    OS_UNUSED_ARG(object);

    if (instance_data != NULL && source_timestamp != NULL) {
        srcInfo.javaEnv = env;
        srcInfo.javaObject = instance_data;
        srcInfo.copyProgram = (saj_copyCache)(PA_ADDRCAST)copyCache;
        if (saj_timeCopyIn (env, source_timestamp, &timestamp) == SAJ_RETCODE_OK) {
	    src = (const gapi_foo *)&srcInfo;
	    ts = (const gapi_time_t *)&timestamp;
	}
    }
    return (jint)
	gapi_fooDataWriter_dispose_w_timestamp (
	    (gapi_fooDataWriter)saj_read_gapi_address (env, DataWriter),
	    src,
	    (gapi_instanceHandle_t)instance_handle,
	    ts);
}

/*
 * Class:     org_opensplice_dds_dcps_FooDataWriterImpl
 * Method:    jniWritedispose
 * Signature: (Ljava/lang/Object;Ljava/lang/Object;I)I
 */
/*
    private native static int jniWritedispose (
        Object DataWriter,
        long copyCache,
        Object instance_data,
        long handle);
*/
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniWritedispose) (
    JNIEnv *env,
    jclass object,
    jobject DataWriter,
    jlong copyCache,
    jobject instance_data,
    jlong handle)
{
    C_STRUCT(saj_srcInfo) srcInfo;
    const gapi_foo *src = NULL;

    OS_UNUSED_ARG(object);

    if (instance_data != NULL) {
        srcInfo.javaEnv = env;
        srcInfo.javaObject = instance_data;
        srcInfo.copyProgram = (saj_copyCache)(PA_ADDRCAST)copyCache;
        src = (const gapi_foo *)&srcInfo;
    }

    return (jint)
        gapi_fooDataWriter_writedispose(
            (gapi_fooDataWriter)saj_read_gapi_address (env, DataWriter),
            src,
            (gapi_instanceHandle_t)handle);
}

/*
 * Class:     org_opensplice_dds_dcps_FooDataWriterImpl
 * Method:    jniWritedisposeWTimestamp
 * Signature: (Ljava/lang/Object;Ljava/lang/Object;ILDDS/Time_t;)I
 */
/*
    private native static int jniWritedisposeWTimestamp (
        Object DataWriter,
        long copyCache,
        Object instance_data,
        long handle,
        DDS.Time_t source_timestamp);
*/
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniWritedisposeWTimestamp) (
    JNIEnv *env,
    jclass object,
    jobject DataWriter,
    jlong copyCache,
    jobject instance_data,
    jlong handle,
    jobject source_timestamp)
{
    C_STRUCT(saj_srcInfo) srcInfo;
    gapi_time_t timestamp;
    const gapi_time_t *ts = NULL;
    const gapi_foo *src = NULL;

    OS_UNUSED_ARG(object);

    if (instance_data != NULL && source_timestamp != NULL) {
        srcInfo.javaEnv = env;
        srcInfo.javaObject = instance_data;
        srcInfo.copyProgram = (saj_copyCache)(PA_ADDRCAST)copyCache;
        if (saj_timeCopyIn (env, source_timestamp, &timestamp) == SAJ_RETCODE_OK) {
            src = (const gapi_foo *)&srcInfo;
            ts = (const gapi_time_t *)&timestamp;
        }
    }

    return (jint)gapi_fooDataWriter_writedispose_w_timestamp(
            (gapi_fooDataWriter)saj_read_gapi_address (env, DataWriter),
            src,
            (gapi_instanceHandle_t)handle,
            ts);
}

/*
 * Class:     org_opensplice_dds_dcps_FooDataWriterImpl
 * Method:    jniGetKeyValue
 * Signature: (Ljava/lang/Object;Ljava/lang/Object;I)I
 */
/*
    private native static int jniGetKeyValue (
        Object DataWriter,
	long copyCache,
        Object key_holder,
        long handle);
*/
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetKeyValue) (
    JNIEnv *env,
    jclass object,
    jobject DataWriter,
    jlong copyCache,
    jobject key_holder,
    jlong handle)
{
    C_STRUCT(saj_dstInfo) dstInfo;
    gapi_foo *dst = NULL;
    jint result;
    jobject element = NULL;
    sajReaderCopyCache *rc = saj_copyCacheReaderCache ((saj_copyCache)(PA_ADDRCAST)copyCache);

    OS_UNUSED_ARG(object);

    if (key_holder != NULL) {
        element = (*env)->GetObjectField (env, key_holder, rc->dataHolder_value_fid);
        dstInfo.javaEnv = env;
        dstInfo.javaObject = element;
        dstInfo.copyProgram = (saj_copyCache)(PA_ADDRCAST)copyCache;
        dst = (gapi_foo *)&dstInfo;
    }

    result = gapi_fooDataWriter_get_key_value (
            (gapi_fooDataWriter)saj_read_gapi_address (env, DataWriter),
            (gapi_foo *)dst,
            (gapi_instanceHandle_t)handle);

    if ((key_holder != NULL) && (dstInfo.javaObject != element)) {
        (*env)->SetObjectField (env, key_holder, rc->dataHolder_value_fid, dstInfo.javaObject);
    }
    return result;
}

/*
 * Class:     org_opensplice_dds_dcps_FooDataWriterImpl
 * Method:    jniLookupInstance
 * Signature: (Ljava/lang/Object;JLjava/lang/Object;)J
 */
JNIEXPORT jlong JNICALL 
SAJ_FUNCTION(jniLookupInstance)(
    JNIEnv *env, 
    jclass object, 
    jobject jwriter, 
    jlong copyCache, 
    jobject instanceData)
{
    C_STRUCT(saj_srcInfo) srcInfo;
    const gapi_foo *src = NULL;

    OS_UNUSED_ARG(object);

    if (instanceData != NULL) {
        srcInfo.javaEnv = env;
        srcInfo.javaObject = instanceData;
        srcInfo.copyProgram = (saj_copyCache)(PA_ADDRCAST)copyCache;
        src = (const gapi_foo *)&srcInfo;
    }

    return (jlong)gapi_fooDataWriter_lookup_instance (
        (gapi_fooDataWriter)saj_read_gapi_address (env, jwriter),
            src);
}
