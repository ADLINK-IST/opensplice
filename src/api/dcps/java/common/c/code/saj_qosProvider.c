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
#include "saj_QosProvider.h" /* JNI generated headers */
#include "saj_utilities.h"
#include "saj_copyOut.h"

#include "cmn_qosProvider.h"

#include "vortex_os.h"
#include "saj__report.h"

#define SAJ_FUNCTION(name) Java_DDS_QosProvider_##name

#define jniQosProviderCheckError(env, jump) do { if((*(env))->ExceptionOccurred(env)) goto jump; } while(0)

#define saj_qosProvider(o) ((saj_qosProvider)SAJ_VOIDP(o))

C_CLASS(saj_qosProvider);
C_STRUCT(saj_qosProvider){
    cmn_qosProvider qp;
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
        cmn_qosProvider qp)
{
    saj_qosProvider _this;
    c_type meta;

    assert(env);
    assert(qp);

    _this = os_malloc(sizeof(*_this));
    _this->qp = qp;

    if(cmn_qosProviderGetParticipantQosType(_this->qp, &meta) != QP_RESULT_OK){
        goto err_dQos;
    }
    _this->dCache = saj_copyCacheNew(env, c_metaObject(meta), NULL);
    c_free(meta);
    if(_this->dCache == NULL){
        goto err_dQos;
    }

    if(cmn_qosProviderGetTopicQosType(_this->qp, &meta) != QP_RESULT_OK){
        goto err_tQos;
    }
    _this->tCache = saj_copyCacheNew(env, c_metaObject(meta), NULL);
    c_free(meta);
    if(_this->tCache == NULL){
        goto err_tQos;
    }

    if(cmn_qosProviderGetPublisherQosType(_this->qp, &meta) != QP_RESULT_OK){
        goto err_pQos;
    }
    _this->pCache = saj_copyCacheNew(env, c_metaObject(meta), NULL);
    c_free(meta);
    if(_this->pCache == NULL){
        goto err_pQos;
    }

    if(cmn_qosProviderGetDataWriterQosType(_this->qp, &meta) != QP_RESULT_OK){
        goto err_wQos;
    }
    _this->wCache = saj_copyCacheNew(env, c_metaObject(meta), NULL);
    c_free(meta);
    if(_this->wCache == NULL){
        goto err_wQos;
    }

    if(cmn_qosProviderGetSubscriberQosType(_this->qp, &meta) != QP_RESULT_OK){
        goto err_sQos;
    }
    _this->sCache = saj_copyCacheNew(env, c_metaObject(meta), NULL);
    c_free(meta);
    if(_this->sCache == NULL){
        goto err_sQos;
    }

    if(cmn_qosProviderGetDataReaderQosType(_this->qp, &meta) != QP_RESULT_OK){
        goto err_rQos;
    }
    _this->rCache = saj_copyCacheNew(env, c_metaObject(meta), NULL);
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
JNIEXPORT jlong JNICALL
SAJ_FUNCTION(jniQosProviderNew)(
        JNIEnv *env,
        jobject jqosProvider,
        jstring juri,
        jstring jprofile)
{
    saj_qosProvider _this;
    cmn_qosProvider qp;
    const char *strUri;
    const char *strProfile;
    C_STRUCT(cmn_qosProviderInputAttr) qpAttr;
    OS_UNUSED_ARG(jqosProvider);

    if(juri){
        strUri = (*env)->GetStringUTFChars (env, juri, NULL);
    } else {
        strUri = NULL;
    }
    jniQosProviderCheckError(env, err_strUri);

    if(jprofile){
        strProfile = (*env)->GetStringUTFChars (env, jprofile, NULL);
    } else {
        strProfile = NULL;
    }
    jniQosProviderCheckError(env, err_strProfile);

    qpAttr.participantQos.copyOut = saj_copyOutStruct;
    qpAttr.topicQos.copyOut = saj_copyOutStruct;
    qpAttr.subscriberQos.copyOut = saj_copyOutStruct;
    qpAttr.dataReaderQos.copyOut = saj_copyOutStruct;
    qpAttr.publisherQos.copyOut = saj_copyOutStruct;
    qpAttr.dataWriterQos.copyOut = saj_copyOutStruct;

    if((qp = cmn_qosProviderNew(strUri, strProfile, &qpAttr)) == NULL){
        /* Error reported by cmn_qosProviderNew(...) */
        goto err_qosProviderNew;
    }

    if((_this = saj_qosProviderNew(env, qp)) == NULL){
        goto err_sajQosProviderNew;
    }

    if(strProfile){
        (*env)->ReleaseStringUTFChars (env, jprofile, strProfile);
    }

    if(strUri){
        (*env)->ReleaseStringUTFChars (env, juri, strUri);
    }

    return (jlong)(PA_ADDRCAST)_this;

/* Error handling */
err_sajQosProviderNew:
    cmn_qosProviderFree(qp);
err_qosProviderNew:
    if(strProfile){
        (*env)->ReleaseStringUTFChars (env, jprofile, strProfile);
    }
err_strProfile:
    if(strUri){
        (*env)->ReleaseStringUTFChars (env, juri, strUri);
    }
err_strUri:
    return 0;
}

/*
 * Class:     DDS_QosProvider
 * Method:    jniQosProviderFree
 * Signature: ()V
 */
JNIEXPORT void JNICALL
SAJ_FUNCTION(jniQosProviderFree)(
        JNIEnv *env,
        jobject jqosProvider,
        jlong qosProvider)
{
    OS_UNUSED_ARG(env);
    OS_UNUSED_ARG(jqosProvider);
    assert(qosProvider);

    saj_qosProviderDeinitCopyCaches(SAJ_VOIDP(qosProvider));
    cmn_qosProviderFree(saj_qosProvider(qosProvider)->qp);
    os_free(SAJ_VOIDP(qosProvider));
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
        jlong qosProvider,
        jobject jqosHolder,
        jstring jid)
{
    saj_returnCode result;
    cmn_qpResult qpr;
    const char * strId;
    C_STRUCT(saj_dstInfo) dst;
    OS_UNUSED_ARG(jqosProvider);

    if(jqosHolder != NULL){
        assert(qosProvider != 0);

        if(jid){
            strId = (*env)->GetStringUTFChars (env, jid, NULL);
        } else {
            strId = NULL;
        }

        dst.copyProgram = saj_qosProvider(qosProvider)->dCache;
        dst.javaObject = NULL;
        dst.javaEnv = env;

        qpr = cmn_qosProviderGetParticipantQos(saj_qosProvider(qosProvider)->qp, strId, &dst);

        if(qpr == QP_RESULT_OK){
            /* store the NamedDomainParticipantQos object in the Holder object */
            (*env)->SetObjectField(env, jqosHolder, GET_CACHED(namedDomainParticipantQosHolder_value_fid), dst.javaObject);

            /* delete the local reference to the NamedDomainParticipantQos object */
            (*env)->DeleteLocalRef(env, dst.javaObject);
            result = SAJ_RETCODE_OK;
        } else if (qpr == QP_RESULT_NO_DATA){
            result = SAJ_RETCODE_NO_DATA;
        } else {
            result = SAJ_RETCODE_ERROR;
        }

        if(strId){
            (*env)->ReleaseStringUTFChars(env, jid, strId);
        }
    } else {
        result = SAJ_RETCODE_BAD_PARAMETER;
    }

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
        jlong qosProvider,
        jobject jqosHolder,
        jstring jid)
{
    saj_returnCode result;
    cmn_qpResult qpr;
    const char * strId;
    C_STRUCT(saj_dstInfo) dst;
    OS_UNUSED_ARG(jqosProvider);

    if(jqosHolder != NULL){
        assert(qosProvider != 0);

        if(jid){
            strId = (*env)->GetStringUTFChars (env, jid, NULL);
        } else {
            strId = NULL;
        }

        dst.copyProgram = saj_qosProvider(qosProvider)->tCache;
        dst.javaObject = NULL;
        dst.javaEnv = env;

        qpr = cmn_qosProviderGetTopicQos(saj_qosProvider(qosProvider)->qp, strId, &dst);

        if(qpr == QP_RESULT_OK){
            /* store the NamedTopicQos object in the Holder object */
            (*env)->SetObjectField(env, jqosHolder, GET_CACHED(namedTopicQosHolder_value_fid), dst.javaObject);

            /* delete the local reference to the NamedTopicQos object */
            (*env)->DeleteLocalRef(env, dst.javaObject);
            result = SAJ_RETCODE_OK;
        } else if (qpr == QP_RESULT_NO_DATA){
            result = SAJ_RETCODE_NO_DATA;
        } else {
            result = SAJ_RETCODE_ERROR;
        }

        if(strId){
            (*env)->ReleaseStringUTFChars(env, jid, strId);
        }
    } else {
        result = SAJ_RETCODE_BAD_PARAMETER;
    }

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
        jlong qosProvider,
        jobject jqosHolder,
        jstring jid)
{
    saj_returnCode result;
    cmn_qpResult qpr;
    const char * strId;
    C_STRUCT(saj_dstInfo) dst;
    OS_UNUSED_ARG(jqosProvider);

    if(jqosHolder != NULL){
        assert(qosProvider != 0);

        if(jid){
            strId = (*env)->GetStringUTFChars (env, jid, NULL);
        } else {
            strId = NULL;
        }

        dst.copyProgram = saj_qosProvider(qosProvider)->sCache;
        dst.javaObject = NULL;
        dst.javaEnv = env;

        qpr = cmn_qosProviderGetSubscriberQos(saj_qosProvider(qosProvider)->qp, strId, &dst);

        if(qpr == QP_RESULT_OK){
            /* store the Named-Qos object in the Holder object */
            (*env)->SetObjectField(env, jqosHolder, GET_CACHED(namedSubscriberQosHolder_value_fid), dst.javaObject);

            /* delete the local reference to the Named-Qos object */
            (*env)->DeleteLocalRef(env, dst.javaObject);
            result = SAJ_RETCODE_OK;
        } else if (qpr == QP_RESULT_NO_DATA){
            result = SAJ_RETCODE_NO_DATA;
        } else {
            result = SAJ_RETCODE_ERROR;
        }

        if(strId){
            (*env)->ReleaseStringUTFChars (env, jid, strId);
        }
    } else {
        result = SAJ_RETCODE_BAD_PARAMETER;
    }

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
        jlong qosProvider,
        jobject jqosHolder,
        jstring jid)
{
    saj_returnCode result;
    cmn_qpResult qpr;
    const char * strId;
    C_STRUCT(saj_dstInfo) dst;
    OS_UNUSED_ARG(jqosProvider);

    if(jqosHolder != NULL){
        assert(qosProvider != 0);

        if(jid){
            strId = (*env)->GetStringUTFChars (env, jid, NULL);
        } else {
            strId = NULL;
        }

        dst.copyProgram = saj_qosProvider(qosProvider)->rCache;
        dst.javaObject = NULL;
        dst.javaEnv = env;

        qpr = cmn_qosProviderGetDataReaderQos(saj_qosProvider(qosProvider)->qp, strId, &dst);

        if(qpr == QP_RESULT_OK){
            /* store the Named-Qos object in the Holder object */
            (*env)->SetObjectField(env, jqosHolder, GET_CACHED(namedDataReaderQosHolder_value_fid), dst.javaObject);

            /* delete the local reference to the Named-Qos object */
            (*env)->DeleteLocalRef(env, dst.javaObject);
            result = SAJ_RETCODE_OK;
        } else if (qpr == QP_RESULT_NO_DATA){
            result = SAJ_RETCODE_NO_DATA;
        } else {
            result = SAJ_RETCODE_ERROR;
        }

        if(strId){
            (*env)->ReleaseStringUTFChars (env, jid, strId);
        }
    } else {
        result = SAJ_RETCODE_BAD_PARAMETER;
    }

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
        jlong qosProvider,
        jobject jqosHolder,
        jstring jid)
{
    saj_returnCode result;
    cmn_qpResult qpr;
    const char * strId;
    C_STRUCT(saj_dstInfo) dst;
    OS_UNUSED_ARG(jqosProvider);

    if(jqosHolder != NULL){
        assert(qosProvider != 0);

        if(jid){
            strId = (*env)->GetStringUTFChars (env, jid, NULL);
        } else {
            strId = NULL;
        }

        dst.copyProgram = saj_qosProvider(qosProvider)->pCache;
        dst.javaObject = NULL;
        dst.javaEnv = env;

        qpr = cmn_qosProviderGetPublisherQos(saj_qosProvider(qosProvider)->qp, strId, &dst);

        if(qpr == QP_RESULT_OK){
            /* store the Named-Qos object in the Holder object */
            (*env)->SetObjectField(env, jqosHolder, GET_CACHED(namedPublisherQosHolder_value_fid), dst.javaObject);
            saj_exceptionCheck(env);

            /* delete the local reference to the Named-Qos object */
            (*env)->DeleteLocalRef(env, dst.javaObject);
            saj_exceptionCheck(env);

            result = SAJ_RETCODE_OK;
        } else if (qpr == QP_RESULT_NO_DATA){
            result = SAJ_RETCODE_NO_DATA;
        } else {
            result = SAJ_RETCODE_ERROR;
        }

        if(strId){
            (*env)->ReleaseStringUTFChars (env, jid, strId);
            saj_exceptionCheck(env);
        }
    } else {
        result = SAJ_RETCODE_BAD_PARAMETER;
    }

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
        jlong qosProvider,
        jobject jqosHolder,
        jstring jid)
{
    saj_returnCode result;
    cmn_qpResult qpr;
    const char * strId;
    C_STRUCT(saj_dstInfo) dst;
    OS_UNUSED_ARG(jqosProvider);

    if(jqosHolder != NULL){
        assert(qosProvider != 0);

        if(jid){
            strId = (*env)->GetStringUTFChars (env, jid, NULL);
        } else {
            strId = NULL;
        }

        dst.copyProgram = saj_qosProvider(qosProvider)->wCache;
        dst.javaObject = NULL;
        dst.javaEnv = env;

        qpr = cmn_qosProviderGetDataWriterQos(saj_qosProvider(qosProvider)->qp, strId, &dst);

        if(qpr == QP_RESULT_OK){
            /* store the Named-Qos object in the Holder object */
            (*env)->SetObjectField(env, jqosHolder, GET_CACHED(namedDataWriterQosHolder_value_fid), dst.javaObject);

            /* delete the local reference to the Named-Qos object */
            (*env)->DeleteLocalRef(env, dst.javaObject);
            result = SAJ_RETCODE_OK;
        } else if (qpr == QP_RESULT_NO_DATA){
            result = SAJ_RETCODE_NO_DATA;
        } else {
            result = SAJ_RETCODE_ERROR;
        }

        if(strId){
            (*env)->ReleaseStringUTFChars (env, jid, strId);
        }
    } else {
        result = SAJ_RETCODE_BAD_PARAMETER;
    }

    return (jint)result;
}

#undef SAJ_FUNCTION
