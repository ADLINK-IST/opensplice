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
#include "saj_QosProvider.h" /* JNI generated headers */
#include "saj_utilities.h"
#include "saj_copyOut.h"

#include "qp_qosProvider.h"

#include "os.h"
#include "os_report.h"

#define SAJ_FUNCTION(name) Java_DDS_QosProvider_##name
#define SAJ_FUNCTION_STR(name) #name

#define jniQosProviderCheckError(env, jump) do { if((*(env))->ExceptionCheck(env)) goto jump; } while(0)

static void
jniQosProviderHandleException(
        JNIEnv *env,
        const c_char * location)
{
    jclass eCls;
    jthrowable e;
    jmethodID eToStr;
    jstring eMsg;
    const c_char *eMsgStr;

    e = (*env)->ExceptionOccurred(env);
    assert(e); /* Should only be called with a pending exception */ \

    /* Catch actual exception */
    (*env)->ExceptionClear(env);

    eCls = (*env)->GetObjectClass(env, e);
    assert(eCls);
    eToStr = (*env)->GetMethodID(env, eCls, "toString", "()Ljava/lang/String;");
    assert(eToStr);
    (*env)->DeleteLocalRef(env, eCls);

    eMsg = (jstring)(*env)->CallObjectMethod(env, e, eToStr);
    eMsgStr = (*env)->GetStringUTFChars (env, eMsg, NULL);

    OS_REPORT_1(OS_API_INFO, location, 0, "Exception occurred:\n%s", eMsgStr);

    (*env)->ReleaseStringUTFChars (env, eMsg, eMsgStr);
    (*env)->DeleteLocalRef(env, eMsg);
}

C_CLASS(saj_qosProvider);
C_STRUCT(saj_qosProvider){
    qp_qosProvider qp;
    saj_copyCache dCache;
    saj_copyCache tCache;
    saj_copyCache pCache;
    saj_copyCache wCache;
    saj_copyCache sCache;
    saj_copyCache rCache;
};

static saj_qosProvider
saj_qosProviderNew (
        JNIEnv *env,
        qp_qosProvider qp)
{
    saj_qosProvider _this;
    c_type meta;

    assert(env);
    assert(qp);

    if((_this = os_malloc(sizeof(*_this))) == NULL){
        /* TODO: Report error */
        goto err_malloc;
    }

    _this->qp = qp;

    if(qp_qosProviderGetParticipantQosType(_this->qp, &meta) != QP_RESULT_OK){
        goto err_dQos;
    }
    _this->dCache = saj_copyCacheNew(env, c_metaObject(meta), NULL, NULL);
    c_free(meta);
    if(_this->dCache == NULL){
        goto err_dQos;
    }

    if(qp_qosProviderGetTopicQosType(_this->qp, &meta) != QP_RESULT_OK){
        goto err_tQos;
    }
    _this->tCache = saj_copyCacheNew(env, c_metaObject(meta), NULL, NULL);
    c_free(meta);
    if(_this->tCache == NULL){
        goto err_tQos;
    }

    if(qp_qosProviderGetPublisherQosType(_this->qp, &meta) != QP_RESULT_OK){
        goto err_pQos;
    }
    _this->pCache = saj_copyCacheNew(env, c_metaObject(meta), NULL, NULL);
    c_free(meta);
    if(_this->pCache == NULL){
        goto err_pQos;
    }

    if(qp_qosProviderGetDataWriterQosType(_this->qp, &meta) != QP_RESULT_OK){
        goto err_wQos;
    }
    _this->wCache = saj_copyCacheNew(env, c_metaObject(meta), NULL, NULL);
    c_free(meta);
    if(_this->wCache == NULL){
        goto err_wQos;
    }

    if(qp_qosProviderGetSubscriberQosType(_this->qp, &meta) != QP_RESULT_OK){
        goto err_sQos;
    }
    _this->sCache = saj_copyCacheNew(env, c_metaObject(meta), NULL, NULL);
    c_free(meta);
    if(_this->sCache == NULL){
        goto err_sQos;
    }

    if(qp_qosProviderGetDataReaderQosType(_this->qp, &meta) != QP_RESULT_OK){
        goto err_rQos;
    }
    _this->rCache = saj_copyCacheNew(env, c_metaObject(meta), NULL, NULL);
    c_free(meta);
    if(_this->rCache == NULL){
        goto err_rQos;
    }

    return _this;

/* Error handling */
err_rQos:
    saj_copyCacheFree(_this->sCache);
err_sQos:
    saj_copyCacheFree(_this->wCache);
err_wQos:
    saj_copyCacheFree(_this->pCache);
err_pQos:
    saj_copyCacheFree(_this->tCache);
err_tQos:
    saj_copyCacheFree(_this->dCache);
err_dQos:
    os_free(_this);
err_malloc:
    return NULL;

}

static void
saj_qosProviderDeinitCopyCaches (
        saj_qosProvider _this)
{
    saj_copyCacheFree(_this->rCache);
    saj_copyCacheFree(_this->sCache);
    saj_copyCacheFree(_this->wCache);
    saj_copyCacheFree(_this->pCache);
    saj_copyCacheFree(_this->tCache);
    saj_copyCacheFree(_this->dCache);
}

/*
 * Class:     DDS_QosProvider
 * Method:    jniQosProviderNew
 * Signature: (Ljava/lang/String;Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL
SAJ_FUNCTION(jniQosProviderNew)(
        JNIEnv *env,
        jobject jqosProvider,
        jstring juri,
        jstring jprofile)
{
    saj_qosProvider _this;
    qp_qosProvider qp;
    const char *strUri;
    const char *strProfile;
    C_STRUCT(qp_qosProviderInputAttr) qpAttr;

    strUri = juri ? (*env)->GetStringUTFChars (env, juri, NULL) : NULL;
    jniQosProviderCheckError(env, err_strUri);

    strProfile = jprofile ? (*env)->GetStringUTFChars (env, jprofile, NULL) : NULL;
    jniQosProviderCheckError(env, err_strProfile);

    qpAttr.participantQos.copyOut = saj_copyOutStruct;
    qpAttr.topicQos.copyOut = saj_copyOutStruct;
    qpAttr.subscriberQos.copyOut = saj_copyOutStruct;
    qpAttr.dataReaderQos.copyOut = saj_copyOutStruct;
    qpAttr.publisherQos.copyOut = saj_copyOutStruct;
    qpAttr.dataWriterQos.copyOut = saj_copyOutStruct;

    if((qp = qp_qosProviderNew(strUri, strProfile, &qpAttr)) == NULL){
        /* Error reported by qp_qosProviderNew(...) */
        goto err_qosProviderNew;
    }

    if((_this = saj_qosProviderNew(env, qp)) == NULL){
        goto err_sajQosProviderNew;
    }

    saj_write_gapi_address(env, jqosProvider, (PA_ADDRCAST)_this);

    if(strProfile){
        (*env)->ReleaseStringUTFChars (env, jprofile, strProfile);
    }

    if(strUri){
        (*env)->ReleaseStringUTFChars (env, juri, strUri);
    }

    return JNI_TRUE;

/* Error handling */
err_sajQosProviderNew:
    qp_qosProviderFree(qp);
err_qosProviderNew:
    if(strProfile){
        (*env)->ReleaseStringUTFChars (env, jprofile, strProfile);
    }
err_strProfile:
    if(strUri){
        (*env)->ReleaseStringUTFChars (env, juri, strUri);
    }
err_strUri:
    return JNI_FALSE;
}

/*
 * Class:     DDS_QosProvider
 * Method:    jniQosProviderFree
 * Signature: ()V
 */
JNIEXPORT void JNICALL
SAJ_FUNCTION(jniQosProviderFree)(
        JNIEnv *env,
        jobject jqosProvider)
{
    saj_qosProvider qosProvider;

    qosProvider = (saj_qosProvider)saj_read_gapi_address(env, jqosProvider);
    if(qosProvider){
        qp_qosProviderFree(qosProvider->qp);
        saj_qosProviderDeinitCopyCaches(qosProvider);
        os_free(qosProvider);
    }
}

/*
 * Class:     DDS_QosProvider
 * Method:    jniGetParticipantQos
 * Signature: (LDDS/NamedDomainParticipantQosHolder;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetParticipantQos)(
        JNIEnv *env,
        jobject jqosProvider,
        jobject jqosHolder,
        jstring jid)
{
    saj_qosProvider qosProvider;
    gapi_returnCode_t result;
    qp_result qpr;
    const char * strId;
    C_STRUCT(saj_dstInfo) dst;

    /* Parameter checking */
    if(jqosHolder == NULL){
        result = GAPI_RETCODE_BAD_PARAMETER;
        OS_REPORT(OS_API_INFO, SAJ_FUNCTION_STR(jniGetParticipantQos), result, "Bad parameter. NamedDomainParticipantQosHolder may not be null.");
        goto bad_param;
    }
    if((qosProvider = (saj_qosProvider)saj_read_gapi_address(env, jqosProvider)) == NULL){
        /* Don't set API_INFO; this is collateral damage detected, not a new error. */
        result = GAPI_RETCODE_PRECONDITION_NOT_MET;
        goto bad_param;
    }

    strId = jid ? (*env)->GetStringUTFChars (env, NULL, NULL) : NULL;
    jniQosProviderCheckError(env, exception);

    dst.copyProgram = qosProvider->dCache;
    dst.javaObject = NULL;
    dst.javaEnv = env;

    qpr = qp_qosProviderGetParticipantQos(qosProvider->qp, strId, &dst);

    if(qpr == QP_RESULT_OK){
        /* store the NamedDomainParticipantQos object in the Holder object */
        (*env)->SetObjectField(env, jqosHolder, GET_CACHED(namedDomainParticipantQosHolder_value_fid), dst.javaObject);
        jniQosProviderCheckError(env, exception);

        /* delete the local reference to the NamedDomainParticipantQos object */
        (*env)->DeleteLocalRef(env, dst.javaObject);
        jniQosProviderCheckError(env, exception);

        result = GAPI_RETCODE_OK;
    } else if (qpr == QP_RESULT_NO_DATA){
        result = GAPI_RETCODE_NO_DATA;
    } else {
        result = GAPI_RETCODE_ERROR;
    }

    if(strId){
        (*env)->ReleaseStringUTFChars(env, jid, strId);
        jniQosProviderCheckError(env, exception);
    }
    return (jint)result;

/* Error handling */
exception:
    jniQosProviderHandleException(env, SAJ_FUNCTION_STR(jniGetParticipantQos));
    result = GAPI_RETCODE_ERROR;
bad_param:
    return (jint)result;
}

/*
 * Class:     DDS_QosProvider
 * Method:    jniGetTopicQos
 * Signature: (LDDS/NamedTopicQosHolder;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetTopicQos)(
        JNIEnv *env,
        jobject jqosProvider,
        jobject jqosHolder,
        jstring jid)
{
    saj_qosProvider qosProvider;
    gapi_returnCode_t result;
    qp_result qpr;
    const char * strId;
    C_STRUCT(saj_dstInfo) dst;

    /* Parameter checking */
    if(jqosHolder == NULL){
        result = GAPI_RETCODE_BAD_PARAMETER;
        OS_REPORT(OS_API_INFO, SAJ_FUNCTION_STR(jniGetTopicQos), result, "Bad parameter. NamedTopicQosHolder may not be null.");
        goto bad_param;
    }
    if((qosProvider = (saj_qosProvider)saj_read_gapi_address(env, jqosProvider)) == NULL){
        /* Don't set API_INFO; this is collateral damage detected, not a new error. */
        result = GAPI_RETCODE_PRECONDITION_NOT_MET;
        goto bad_param;
    }

    strId = jid ? (*env)->GetStringUTFChars (env, jid, NULL) : NULL;
    jniQosProviderCheckError(env, exception);

    dst.copyProgram = qosProvider->tCache;
    dst.javaObject = NULL;
    dst.javaEnv = env;

    qpr = qp_qosProviderGetTopicQos(qosProvider->qp, strId, &dst);

    if(qpr == QP_RESULT_OK){
        /* store the NamedTopicQos object in the Holder object */
        (*env)->SetObjectField(env, jqosHolder, GET_CACHED(namedTopicQosHolder_value_fid), dst.javaObject);
        jniQosProviderCheckError(env, exception);

        /* delete the local reference to the NamedTopicQos object */
        (*env)->DeleteLocalRef(env, dst.javaObject);
        jniQosProviderCheckError(env, exception);

        result = GAPI_RETCODE_OK;
    } else if (qpr == QP_RESULT_NO_DATA){
        result = GAPI_RETCODE_NO_DATA;
    } else {
        result = GAPI_RETCODE_ERROR;
    }

    if(strId){
        (*env)->ReleaseStringUTFChars(env, jid, strId);
        jniQosProviderCheckError(env, exception);
    }
    return (jint)result;

/* Error handling */
exception:
    jniQosProviderHandleException(env, SAJ_FUNCTION_STR(jniGetTopicQos));
    result = GAPI_RETCODE_ERROR;
bad_param:
    return (jint)result;
}

/*
 * Class:     DDS_QosProvider
 * Method:    jniGetSubscriberQos
 * Signature: (LDDS/NamedSubscriberQosHolder;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetSubscriberQos)(
        JNIEnv *env,
        jobject jqosProvider,
        jobject jqosHolder,
        jstring jid)
{
    saj_qosProvider qosProvider;
    gapi_returnCode_t result;
    qp_result qpr;
    const char * strId;
    C_STRUCT(saj_dstInfo) dst;

    /* Parameter checking */
    if(jqosHolder == NULL){
        result = GAPI_RETCODE_BAD_PARAMETER;
        OS_REPORT(OS_API_INFO, SAJ_FUNCTION_STR(jniGetSubscriberQos), result, "Bad parameter. NamedSubscriberQosHolder may not be null.");
        goto bad_param;
    }
    if((qosProvider = (saj_qosProvider)saj_read_gapi_address(env, jqosProvider)) == NULL){
        /* Don't set API_INFO; this is collateral damage detected, not a new error. */
        result = GAPI_RETCODE_PRECONDITION_NOT_MET;
        goto bad_param;
    }

    strId = jid ? (*env)->GetStringUTFChars (env, jid, NULL) : NULL;
    jniQosProviderCheckError(env, exception);

    dst.copyProgram = qosProvider->sCache;
    dst.javaObject = NULL;
    dst.javaEnv = env;

    qpr = qp_qosProviderGetSubscriberQos(qosProvider->qp, strId, &dst);

    if(qpr == QP_RESULT_OK){
        /* store the Named-Qos object in the Holder object */
        (*env)->SetObjectField(env, jqosHolder, GET_CACHED(namedSubscriberQosHolder_value_fid), dst.javaObject);
        jniQosProviderCheckError(env, exception);

        /* delete the local reference to the Named-Qos object */
        (*env)->DeleteLocalRef(env, dst.javaObject);
        jniQosProviderCheckError(env, exception);

        result = GAPI_RETCODE_OK;
    } else if (qpr == QP_RESULT_NO_DATA){
        result = GAPI_RETCODE_NO_DATA;
    } else {
        result = GAPI_RETCODE_ERROR;
    }

    if(strId){
        (*env)->ReleaseStringUTFChars (env, jid, strId);
        jniQosProviderCheckError(env, exception);
    }
    return (jint)result;

/* Error handling */
exception:
    jniQosProviderHandleException(env, SAJ_FUNCTION_STR(jniGetSubscriberQos));
    result = GAPI_RETCODE_ERROR;
bad_param:
    return (jint)result;
}

/*
 * Class:     DDS_QosProvider
 * Method:    jniGetDatareaderQos
 * Signature: (LDDS/NamedDataReaderQosHolder;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetDatareaderQos)(
        JNIEnv *env,
        jobject jqosProvider,
        jobject jqosHolder,
        jstring jid)
{
    saj_qosProvider qosProvider;
    gapi_returnCode_t result;
    qp_result qpr;
    const char * strId;
    C_STRUCT(saj_dstInfo) dst;

    /* Parameter checking */
    if(jqosHolder == NULL){
        result = GAPI_RETCODE_BAD_PARAMETER;
        OS_REPORT(OS_API_INFO, SAJ_FUNCTION_STR(jniGetDatareaderQos), result, "Bad parameter. NamedDataReaderQosHolder may not be null.");
        goto bad_param;
    }
    if((qosProvider = (saj_qosProvider)saj_read_gapi_address(env, jqosProvider)) == NULL){
        /* Don't set API_INFO; this is collateral damage detected, not a new error. */
        result = GAPI_RETCODE_PRECONDITION_NOT_MET;
        goto bad_param;
    }

    strId = jid ? (*env)->GetStringUTFChars (env, jid, NULL) : NULL;
    jniQosProviderCheckError(env, exception);

    dst.copyProgram = qosProvider->rCache;
    dst.javaObject = NULL;
    dst.javaEnv = env;

    qpr = qp_qosProviderGetDataReaderQos(qosProvider->qp, strId, &dst);

    if(qpr == QP_RESULT_OK){
        /* store the Named-Qos object in the Holder object */
        (*env)->SetObjectField(env, jqosHolder, GET_CACHED(namedDataReaderQosHolder_value_fid), dst.javaObject);
        jniQosProviderCheckError(env, exception);

        /* delete the local reference to the Named-Qos object */
        (*env)->DeleteLocalRef(env, dst.javaObject);
        jniQosProviderCheckError(env, exception);

        result = GAPI_RETCODE_OK;
    } else if (qpr == QP_RESULT_NO_DATA){
        result = GAPI_RETCODE_NO_DATA;
    } else {
        result = GAPI_RETCODE_ERROR;
    }

    if(strId){
        (*env)->ReleaseStringUTFChars (env, jid, strId);
        jniQosProviderCheckError(env, exception);
    }
    return (jint)result;

/* Error handling */
exception:
    jniQosProviderHandleException(env, SAJ_FUNCTION_STR(jniGetDataReaderQos));
    result = GAPI_RETCODE_ERROR;
bad_param:
    return (jint)result;
}

/*
 * Class:     DDS_QosProvider
 * Method:    jniGetPublisherQos
 * Signature: (LDDS/NamedPublisherQosHolder;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetPublisherQos)(
        JNIEnv *env,
        jobject jqosProvider,
        jobject jqosHolder,
        jstring jid)
{
    saj_qosProvider qosProvider;
    gapi_returnCode_t result;
    qp_result qpr;
    const char * strId;
    C_STRUCT(saj_dstInfo) dst;

    /* Parameter checking */
    if(jqosHolder == NULL){
        result = GAPI_RETCODE_BAD_PARAMETER;
        OS_REPORT(OS_API_INFO, SAJ_FUNCTION_STR(jniGetPublisherQos), result, "Bad parameter. NamedPublisherQosHolder may not be null.");
        goto bad_param;
    }
    if((qosProvider = (saj_qosProvider)saj_read_gapi_address(env, jqosProvider)) == NULL){
        /* Don't set API_INFO; this is collateral damage detected, not a new error. */
        result = GAPI_RETCODE_PRECONDITION_NOT_MET;
        goto bad_param;
    }

    strId = jid ? (*env)->GetStringUTFChars (env, jid, NULL) : NULL;
    jniQosProviderCheckError(env, exception);

    dst.copyProgram = qosProvider->pCache;
    dst.javaObject = NULL;
    dst.javaEnv = env;

    qpr = qp_qosProviderGetPublisherQos(qosProvider->qp, strId, &dst);

    if(qpr == QP_RESULT_OK){
        /* store the Named-Qos object in the Holder object */
        (*env)->SetObjectField(env, jqosHolder, GET_CACHED(namedPublisherQosHolder_value_fid), dst.javaObject);
        jniQosProviderCheckError(env, exception);

        /* delete the local reference to the Named-Qos object */
        (*env)->DeleteLocalRef(env, dst.javaObject);
        jniQosProviderCheckError(env, exception);

        result = GAPI_RETCODE_OK;
    } else if (qpr == QP_RESULT_NO_DATA){
        result = GAPI_RETCODE_NO_DATA;
    } else {
        result = GAPI_RETCODE_ERROR;
    }

    if(strId){
        (*env)->ReleaseStringUTFChars (env, jid, strId);
        jniQosProviderCheckError(env, exception);
    }
    return (jint)result;

/* Error handling */
exception:
    jniQosProviderHandleException(env, SAJ_FUNCTION_STR(jniGetPublisherQos));
    result = GAPI_RETCODE_ERROR;
bad_param:
    return (jint)result;
}

/*
 * Class:     DDS_QosProvider
 * Method:    jniGetDatawriterQos
 * Signature: (LDDS/NamedDataWriterQosHolder;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL
SAJ_FUNCTION(jniGetDatawriterQos)(
        JNIEnv *env,
        jobject jqosProvider,
        jobject jqosHolder,
        jstring jid)
{
    saj_qosProvider qosProvider;
    gapi_returnCode_t result;
    qp_result qpr;
    const char * strId;
    C_STRUCT(saj_dstInfo) dst;

    /* Parameter checking */
    if(jqosHolder == NULL){
        result = GAPI_RETCODE_BAD_PARAMETER;
        OS_REPORT(OS_API_INFO, SAJ_FUNCTION_STR(jniGetDatawriterQos), result, "Bad parameter. NamedDataWriterQosHolder may not be null.");
        goto bad_param;
    }
    if((qosProvider = (saj_qosProvider)saj_read_gapi_address(env, jqosProvider)) == NULL){
        /* Don't set API_INFO; this is collateral damage detected, not a new error. */
        result = GAPI_RETCODE_PRECONDITION_NOT_MET;
        goto bad_param;
    }

    strId = jid ? (*env)->GetStringUTFChars (env, jid, NULL) : NULL;
    jniQosProviderCheckError(env, exception);

    dst.copyProgram = qosProvider->wCache;
    dst.javaObject = NULL;
    dst.javaEnv = env;

    qpr = qp_qosProviderGetDataWriterQos(qosProvider->qp, strId, &dst);

    if(qpr == QP_RESULT_OK){
        /* store the Named-Qos object in the Holder object */
        (*env)->SetObjectField(env, jqosHolder, GET_CACHED(namedDataWriterQosHolder_value_fid), dst.javaObject);
        jniQosProviderCheckError(env, exception);

        /* delete the local reference to the Named-Qos object */
        (*env)->DeleteLocalRef(env, dst.javaObject);
        jniQosProviderCheckError(env, exception);

        result = GAPI_RETCODE_OK;
    } else if (qpr == QP_RESULT_NO_DATA){
        result = GAPI_RETCODE_NO_DATA;
    } else {
        result = GAPI_RETCODE_ERROR;
    }

    if(strId){
        (*env)->ReleaseStringUTFChars (env, jid, strId);
        jniQosProviderCheckError(env, exception);
    }
    return (jint)result;

/* Error handling */
exception:
    jniQosProviderHandleException(env, SAJ_FUNCTION_STR(jniGetDataWriterQos));
    result = GAPI_RETCODE_ERROR;
bad_param:
    return (jint)result;
}

#undef SAJ_FUNCTION_STR
#undef SAJ_FUNCTION
