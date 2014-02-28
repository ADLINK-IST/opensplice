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

#include "saj_utilities.h"
#include "saj_listener.h"
#include "os_stdlib.h"
#include "os_heap.h"
#include "os_report.h"
#include "saj_qosUtils.h"
#include "saj__fooDataReader.h"

/* Enabling CDR copy out requires compiling the type, for which it
   needs the copy cache - and so we need these two include files. */
#include "saj_copyCache.h"
#include "sd_cdr.h"

/* jni_cache jniCache; */
jni_cache jniCache;

/**
 * @brief private function to check the java_object.
 * If it is an instance of org.opensplice.dds.dcps.SajSuperClass
 * SAJ_RETCODE_OK will be returned otherwise SAJ_RETCODE_ERROR is returned.
 * @param env The JNI environment.
 * @param java_object The object that will be checked.
 * @return SAJ_RETCODE_OK if java_object is an instance of SajSuperClass.
 */
saj_returnCode  checkJavaObject(JNIEnv *env, jobject java_object);


/**
 * @brief private function to copy a gapi_object buffer to a typed java array.
 * @param env The JNI environment.
 * @param classId Identifier of the class of which an array with length
 * seqLength will be created.
 * @param seqLength Number of gapi objects stored in the buffer.
 * @param src A pointer to a gapi_object buffer.
 * @param dst Output parameter for the newly allocated typed java array.
 * @return return code.
 */
saj_returnCode
saj_gapiObjectBufferCopyOut(
    JNIEnv *env,
    jclass classId,
    gapi_unsigned_long seqLength,
    gapi_object *src,
    jobjectArray *dst);

void
saj_exceptionCheck (
    JNIEnv *javaEnv)
{
    if ((*javaEnv)->ExceptionCheck(javaEnv)){
        (*javaEnv)->ExceptionDescribe(javaEnv); /* Will output exception info to stderr */
        assert(0);
    }
}

PA_ADDRCAST
saj_read_gapi_address(
    JNIEnv *env,
    jobject java_object)
{
    PA_ADDRCAST returnValue;
    saj_returnCode rc;

    returnValue = 0;

    if(java_object != NULL){
        rc = checkJavaObject(env, java_object);

        if(rc == SAJ_RETCODE_OK){
            if(GET_CACHED(sajSuperClassGapiPeer_fid) == NULL){
                SET_CACHED(sajSuperClassGapiPeer_fid, (*env)->GetFieldID(env, GET_CACHED(gapiSuperClass_class), "gapiPeer", "J"));
                saj_exceptionCheck(env);
            }
            /* read field gapiObject from the object */
            returnValue = (PA_ADDRCAST)(*env)->GetLongField(env, java_object, GET_CACHED(sajSuperClassGapiPeer_fid));
            saj_exceptionCheck(env);
        }
    }
    return returnValue;
}

void
saj_write_gapi_address(
    JNIEnv *env,
    jobject java_object,
    PA_ADDRCAST address)
{
    saj_returnCode rc;

    if(java_object != NULL){
        rc = checkJavaObject(env, java_object);

        if( rc == SAJ_RETCODE_OK){
            if (GET_CACHED(sajSuperClassGapiPeer_fid) == NULL) {
                SET_CACHED(sajSuperClassGapiPeer_fid, (*env)->GetFieldID(env, GET_CACHED(gapiSuperClass_class), "gapiPeer", "J"));
                saj_exceptionCheck(env);
            }
            /* write value to java object */
            (*env)->SetLongField(env, java_object, GET_CACHED(sajSuperClassGapiPeer_fid), address);
            saj_exceptionCheck(env);
        }
    }
}

jobject
saj_read_java_address(
    gapi_object gapi_obj)
{
    saj_userData ud;
    jobject result;

    result = NULL;

    if(gapi_obj != GAPI_OBJECT_NIL){
        ud = saj_userData(gapi_object_get_user_data(gapi_obj));

        if(ud != NULL){
            result = ud->saj_object;
        }
    }
    return result;
}

jobject
saj_read_java_listener_address(
    void *listenerData)
{
    saj_listenerData ld;
    jobject result;


    result = NULL;

    if(listenerData != GAPI_OBJECT_NIL){
        ld = (saj_listenerData) listenerData;
        result = ld->jlistener;
    }
    return result;
}

jobject
saj_read_java_statusCondition_address(
    gapi_object gapi_obj)
{
    saj_userData ud;
    jobject result;

    result = NULL;

    if(gapi_obj != GAPI_OBJECT_NIL){
        ud = saj_userData(gapi_object_get_user_data(gapi_obj));

        if(ud != NULL){
            ud = ud->statusConditionUserData;

            if(ud != NULL){
                result = ud->saj_object;
            }
        }
    }
    return result;
}

void
saj_write_java_address(
    JNIEnv *env,
    gapi_object gapi_obj,
    jobject java_object)
{
    saj_userData ud;

    assert(env != NULL);
    assert(gapi_obj != NULL);
    assert(java_object != NULL);

    ud = os_malloc(C_SIZEOF(saj_userData));
    ud->saj_object = (*env)->NewGlobalRef(env, java_object);
    saj_exceptionCheck(env);
    ud->listenerData = NULL;
    ud->statusConditionUserData = NULL;
    gapi_object_set_user_data(gapi_obj,
                              (void*)ud,
                              saj_destroy_user_data_callback,
                              (void*)env);
}

void
saj_write_weak_java_address(
    JNIEnv *env,
    gapi_object gapi_obj,
    jobject java_object)
{
    saj_userData ud;

    assert(env != NULL);
    assert(gapi_obj != NULL);
    assert(java_object != NULL);

    ud = os_malloc(C_SIZEOF(saj_userData));
    ud->saj_object = (*env)->NewWeakGlobalRef(env, java_object);
    saj_exceptionCheck(env);
    ud->listenerData = NULL;
    ud->statusConditionUserData = NULL;
    gapi_object_set_user_data(gapi_obj, (void*)ud,NULL,NULL);
}

void
saj_write_java_listener_address(
    JNIEnv *env,
    gapi_object gapi_obj,
    saj_listenerData listenerData)
{
    saj_userData ud;

    assert(env != NULL);
    assert(listenerData != NULL);

    if(gapi_obj != GAPI_OBJECT_NIL){
        ud = saj_userData(gapi_object_get_user_data(gapi_obj));

        if(ud != NULL){
            if(ud->listenerData != NULL){
                saj_listenerDataFree(env, ud->listenerData);
                ud->listenerData = NULL;
            }
            ud->listenerData = listenerData;
            gapi_object_set_user_data(gapi_obj, (void*)ud,NULL,NULL);
        }
    }
}

void
saj_write_java_statusCondition_address(
    JNIEnv *env,
    gapi_object gapi_obj,
    gapi_statusCondition condition,
    jobject java_object)
{
    saj_userData ud;

    assert(env != NULL);

    OS_UNUSED_ARG(java_object);

    if(gapi_obj != GAPI_OBJECT_NIL){
        ud = saj_userData(gapi_object_get_user_data(gapi_obj));

        if(ud != NULL){
            if(ud->statusConditionUserData != NULL){
                saj_destroy_user_data(env, ud->statusConditionUserData);
            }
            ud->statusConditionUserData = gapi_object_get_user_data(
                                                        (gapi_object)condition);
            gapi_object_set_user_data(gapi_obj, (void*)ud,NULL,NULL);
        }
    }
}

void
saj_destroy_user_data_callback(
    void* entityUserData,
    void* userData)
{
    saj_userData ud;
    JNIEnv* env;
    void* threadData;

    OS_UNUSED_ARG(userData);

    ud = saj_userData(entityUserData);
    threadData = os_threadMemGet(OS_THREAD_JVM);
    if (threadData) {
        env = *(JNIEnv**)threadData;
        saj_destroy_user_data(env, ud);
    } else {
        OS_REPORT(OS_WARNING,"saj_destroy_user_data_callback",0,
                  "No JNIEnv reference found, "
                  "possible leakage of a Java object");
    }
    return;
}

void
saj_destroy_user_data(
    JNIEnv *env,
    saj_userData ud)
{
    /*
     * Because of multithread issues the supplied data might be NULL.
     */
    if(ud != NULL){
        if(ud->listenerData != NULL){
            saj_listenerDataFree(env, ud->listenerData);
        }
        if(ud->statusConditionUserData != NULL){
            saj_destroy_user_data(env, ud->statusConditionUserData);
        }
        if((*env)->IsInstanceOf(env, ud->saj_object, GET_CACHED(dataReaderImpl_class))){
            sajParDemContext pdc;

            saj_read_parallelDemarshallingContext_address(env, ud->saj_object, &pdc);
            saj_fooDataReaderParallelDemarshallingContextFinalize(env, ud->saj_object, pdc);
        }
        (*env)->DeleteGlobalRef(env, ud->saj_object);
        saj_exceptionCheck(env);
    }
}

void
saj_destroy_weak_user_data(
    JNIEnv *env,
    saj_userData ud)
{
    assert(env != NULL);

    if(ud != NULL){
        (*env)->DeleteWeakGlobalRef(env, ud->saj_object);
        saj_exceptionCheck(env);
        os_free(ud);
    }
}

saj_returnCode
saj_construct_java_object(
    JNIEnv *env,
    const char *classname,
    PA_ADDRCAST gapi_obj_address,
    jobject *new_java_object)
{
    saj_returnCode rc;

    /* construct a new java object */
    rc = saj_create_new_java_object(env, classname, new_java_object);

    if(rc == SAJ_RETCODE_OK){
        /* write the adress of the gapi object to the java object */
        saj_write_gapi_address(env, *new_java_object, gapi_obj_address);
        /* write the adress of the java object to the gapi object */
        saj_write_java_address(env, (gapi_object)gapi_obj_address, *new_java_object);
    }
    return rc;
}

saj_returnCode
saj_construct_typed_java_object(
    JNIEnv *env,
    const char *classname,
    PA_ADDRCAST gapi_obj_address,
    jobject *new_java_object,
    const char *constructorSignature,
    jobject typeSupport)
{
    saj_returnCode rc;

    /* construct a new java object */
    rc = saj_create_new_typed_java_object(env, classname, new_java_object,
                                            constructorSignature, typeSupport);

    if(rc == SAJ_RETCODE_OK){
        /* write the adress of the gapi object to the java object */
        saj_write_gapi_address(env, *new_java_object, gapi_obj_address);
        /* write the adress of the java object to the gapi object */
        saj_write_java_address(env, (gapi_object)gapi_obj_address, *new_java_object);
    }
    return rc;
}

saj_returnCode
saj_register_weak_java_object(
    JNIEnv *env,
    PA_ADDRCAST gapi_obj_address,
    jobject new_java_object)
{
    assert(gapi_obj_address);
    assert(new_java_object);

    /* write the adress of the gapi object to the java object */
    saj_write_gapi_address(env, new_java_object, gapi_obj_address);
    /* write the adress of the java object to the gapi object */
    saj_write_weak_java_address(env, (gapi_object)gapi_obj_address, new_java_object);

    return SAJ_RETCODE_OK;
}

saj_returnCode
saj_create_new_java_object(
    JNIEnv *env,
    const char *classname,
    jobject *new_java_object)
{
    jclass  newClass;
    jmethodID constructorId;
    jobject newObject;
    saj_returnCode rc;

    assert(new_java_object != NULL);

    rc = SAJ_RETCODE_ERROR;
    newClass = (*env)->FindClass(env, classname);
    saj_exceptionCheck(env);

    if (newClass != NULL){
        constructorId = (*env)->GetMethodID(env, newClass, "<init>", "()V");
        saj_exceptionCheck(env);

        if (constructorId != NULL){
            newObject = (*env)->NewObject(env, newClass, constructorId);
            saj_exceptionCheck(env);

            if(newObject != NULL){
                *new_java_object = newObject;
                rc = SAJ_RETCODE_OK;
            }
        }
    }
    return rc;
}

saj_returnCode
saj_create_new_typed_java_object(
    JNIEnv *env,
    const char *classname,
    jobject *new_java_object,
    const char *constructorSignature,
    jobject typeSupport)
{
    jclass  newClass;
    jmethodID constructorId;
    jobject newObject;
    saj_returnCode rc;

    assert(new_java_object != NULL);

    rc = SAJ_RETCODE_ERROR;
    newClass = (*env)->FindClass(env, classname);
    saj_exceptionCheck(env);

    if (newClass != NULL){
        constructorId = (*env)->GetMethodID(env, newClass, "<init>", constructorSignature);
        saj_exceptionCheck(env);

        if (constructorId != NULL){
            newObject = (*env)->NewObject(env, newClass, constructorId, typeSupport);
            saj_exceptionCheck(env);

            if(newObject != NULL){
                *new_java_object = newObject;
                rc = SAJ_RETCODE_OK;
            }
        }
    }
    return rc;
}

saj_returnCode
checkJavaObject(
    JNIEnv *env,
    jobject java_object)
{
    saj_returnCode rc;
    jclass tempClass;

    rc = SAJ_RETCODE_ERROR;

    if(java_object != NULL){
        /* make sure there is a reference to SajSuperClass */
        if (GET_CACHED(gapiSuperClass_class) == NULL){
            tempClass = (*env)->FindClass(env, "org/opensplice/dds/dcps/SajSuperClass");
            saj_exceptionCheck(env);
            SET_CACHED(gapiSuperClass_class, (*env)->NewGlobalRef(env, tempClass));
            saj_exceptionCheck(env);
            (*env)->DeleteLocalRef(env, tempClass);
            saj_exceptionCheck(env);
        }

        /* Verify the java_object is an instance of SajSuperClass */
        if((*env)->IsInstanceOf(env, java_object, GET_CACHED(gapiSuperClass_class))){
            rc = SAJ_RETCODE_OK;
        }
        saj_exceptionCheck(env);
    }
    return rc;
}

saj_returnCode
saj_durationCopyIn(
    JNIEnv *env,
    jobject javaDuration,
    gapi_duration_t *out)
{
    saj_returnCode rc;

    assert(out != NULL);

    rc = SAJ_RETCODE_OK;

    if(javaDuration != NULL){
        out->sec = (*env)->GetIntField(env, javaDuration, GET_CACHED(duration_t_sec_fid));
        saj_exceptionCheck(env);
        out->nanosec = (*env)->GetIntField(env, javaDuration, GET_CACHED(duration_t_nanosec_fid));
        saj_exceptionCheck(env);
    }

    return rc;
}

saj_returnCode
saj_durationCopyOut(
    JNIEnv *env,
    gapi_duration_t *src,
    jobject *dst)
{
    saj_returnCode rc;

    assert(dst);

    rc = SAJ_RETCODE_OK;

    if (*dst == NULL)
    {
        *dst = (*env)->NewObject(
            env,
            GET_CACHED(duration_t_class),
            GET_CACHED(duration_t_constructor_mid),
            src->sec,
            src->nanosec);
        saj_exceptionCheck(env);
        rc = *dst == NULL ? SAJ_RETCODE_ERROR : SAJ_RETCODE_OK;
    }
    else
    {
        /* set the nanosec and sec fields of the java object */
        (*env)->SetIntField(env, *dst, GET_CACHED(duration_t_sec_fid), src->sec);
        saj_exceptionCheck(env);
        (*env)->SetIntField(env, *dst, GET_CACHED(duration_t_nanosec_fid), src->nanosec);
        saj_exceptionCheck(env);
    }

    return rc;
}

saj_returnCode
saj_timeCopyIn(
    JNIEnv *env,
    jobject src,
    gapi_time_t *dst)
{
    assert (src);
    assert (dst);

    dst->sec = (*env)->GetIntField(env, src, GET_CACHED(time_t_sec_fid));
    saj_exceptionCheck(env);
    dst->nanosec = (*env)->GetIntField(env, src, GET_CACHED(time_t_nanosec_fid));
    saj_exceptionCheck(env);

    return SAJ_RETCODE_OK;
}

saj_returnCode
saj_timeCopyOut(
    JNIEnv *env,
    gapi_time_t *src,
    jobject *dst)
{
    assert(src);
    assert(dst);

    if (*dst == NULL) {
       *dst = (*env)->NewObject (env, GET_CACHED(time_t_class),
                                        GET_CACHED(time_t_constructor_mid),
                                        src->sec, src->nanosec);
        saj_exceptionCheck(env);
        if (*dst == NULL) {
            return SAJ_RETCODE_ERROR;
        }
    } else {
        /* set the nanosec and sec fields of the java object */
        (*env)->SetIntField(env, *dst, GET_CACHED(time_t_sec_fid), src->sec);
        saj_exceptionCheck(env);
        (*env)->SetIntField(env, *dst, GET_CACHED(time_t_nanosec_fid), src->nanosec);
        saj_exceptionCheck(env);
    }
    return SAJ_RETCODE_OK;
}

saj_returnCode
saj_stringSequenceCopyIn(
    JNIEnv *env,
    jobjectArray stringArray,
    gapi_stringSeq *out)
{
    jobject             javaString;
    const gapi_char*    vm_managed_c_string;
    jsize               arrayLength;
    int                 i;
    saj_returnCode      rc;

    assert(out != NULL);
    assert(stringArray != NULL);

    javaString = NULL;
    vm_managed_c_string = NULL;
    arrayLength = 0;
    i = 0;
    rc = SAJ_RETCODE_OK;
    arrayLength = (*env)->GetArrayLength(env, stringArray);
    saj_exceptionCheck(env);

    if (stringArray != NULL)
    {
        out->_maximum = arrayLength;
        out->_length = arrayLength;
        /* allocate a buffer of the right size */
        out->_buffer = gapi_stringSeq_allocbuf(arrayLength);
        out->_release = TRUE;

        /* fill the string buffer with strings */
        for (i = 0; i < arrayLength && rc == SAJ_RETCODE_OK; i++)
        {
            /* get the java String from the array */
            javaString = (*env)->GetObjectArrayElement(env, stringArray, i);
            saj_exceptionCheck(env);

            /* translate the java string to a c string */
            vm_managed_c_string = (*env)->GetStringUTFChars(env, javaString, 0);
            saj_exceptionCheck(env);

            if(vm_managed_c_string != NULL)
            {
                /* copy the c sting to the buffer */
                out->_buffer[i] = gapi_string_dup(vm_managed_c_string);
            }
            else
            {
                rc = SAJ_RETCODE_ERROR; /* VM has thrown an OutOfMemoryError */
            }

            /* release local references */
            (*env)->ReleaseStringUTFChars(env, javaString, vm_managed_c_string);
            saj_exceptionCheck(env);
        }
    }

    return rc;
}

saj_returnCode saj_octetSequenceCopyIn(
    JNIEnv *env,
    jbyteArray jArray,
    gapi_octetSeq *out)
{
    jsize arrayLength;
    jbyte *vmManagedByteArray;
    saj_returnCode rc;

    assert(out != NULL);

    rc = SAJ_RETCODE_OK;

    if (jArray != NULL)
    {
        arrayLength = (*env)->GetArrayLength(env, jArray);
        saj_exceptionCheck(env);

        out->_maximum = arrayLength;
        out->_length = arrayLength;

        /* allocate a buffer of the right size */
        out->_buffer = gapi_octetSeq_allocbuf(arrayLength);
        out->_release = TRUE;

        vmManagedByteArray =
            (*env)->GetPrimitiveArrayCritical(env, jArray, NULL);

        if(vmManagedByteArray != NULL){
            memcpy(out->_buffer, vmManagedByteArray, arrayLength);

            /* don't copy the content of vmManagedByteArray to jArray */
            (*env)->ReleasePrimitiveArrayCritical(
                env, jArray, vmManagedByteArray, JNI_ABORT);
            saj_exceptionCheck(env);
        }
        else
        {
            rc = SAJ_RETCODE_ERROR;
        }
    }
    return rc;
}

saj_returnCode saj_octetSequenceCopyOut(
    JNIEnv *env,
    gapi_octetSeq *src,
    jbyteArray *dst)
{
    saj_returnCode rc;

    assert(dst != NULL);

    rc = SAJ_RETCODE_ERROR;

    /* create a new java byte array */
    *dst = (*env)->NewByteArray(env, src->_length);
    saj_exceptionCheck(env);

    if (*dst != NULL)
    {
        (*env)->SetByteArrayRegion(
            env, *dst, 0, src->_length, (jbyte *)src->_buffer);

        if ((*env)->ExceptionCheck(env) == JNI_FALSE)
        {
            rc = SAJ_RETCODE_OK;
        }
    }

    return rc;
}


saj_returnCode saj_builtinTopicKeyCopyOut(
    JNIEnv *env,
    gapi_builtinTopicKey_t *src,
    jintArray *dst)
{
    saj_returnCode rc;

    assert(dst != NULL);

    rc = SAJ_RETCODE_ERROR;

    /* create a new java byte array */
    *dst = (*env)->NewIntArray(env, 3);
    saj_exceptionCheck(env);

    if (*dst != NULL)
    {
        (*env)->SetIntArrayRegion(
            env, *dst, 0, 3, (jint *)src);

        if ((*env)->ExceptionCheck(env) == JNI_FALSE)
        {
            rc = SAJ_RETCODE_OK;
        }
    }

    return rc;
}

saj_returnCode
saj_instanceHandleSequenceCopyOut(
    JNIEnv* env,
    gapi_instanceHandleSeq *src,
    jlongArray *dst)
{
    saj_returnCode rc;

    assert(dst != NULL);

    rc = SAJ_RETCODE_ERROR;

    *dst = (*env)->NewLongArray(env, src->_length);
    saj_exceptionCheck(env);

    if (*dst != NULL){
        (*env)->SetLongArrayRegion(env, *dst, 0, src->_length, (jlong *)src->_buffer);

        if ((*env)->ExceptionCheck(env) == JNI_FALSE){
            rc = SAJ_RETCODE_OK;
        }
    }
    return rc;
}

saj_returnCode
saj_subscriptionBuiltinTopicDataCopyOut(
    JNIEnv* env,
    gapi_subscriptionBuiltinTopicData *src,
    jobject *dst)
{
    jintArray javaKey;
    jintArray javaParticipant_key;
    jobject javaTopic_name;
    jobject javaType_name;
    jobject javaDurability;
    jobject javaDeadline;
    jobject javaLatency_budget;
    jobject javaLiveliness;
    jobject javaReliability;
    jobject javaDestination_order;
    jobject javaUser_data;
    jobject javaOwnership;
    jobject javaTime_based_filter;
    jobject javaPresentation;
    jobject javaPartition;
    jobject javaTopic_data;
    jobject javaGroup_data;
    saj_returnCode rc;

    assert(dst != NULL);

    javaKey = NULL;
    javaParticipant_key = NULL;
    javaTopic_name = NULL;
    javaType_name = NULL;
    javaDurability = NULL;
    javaLatency_budget = NULL;
    javaLiveliness = NULL;
    javaReliability = NULL;
    javaOwnership = NULL;
    javaDestination_order = NULL;
    javaUser_data = NULL;
    javaTime_based_filter = NULL;
    javaDeadline = NULL;
    javaPresentation = NULL;
    javaPartition = NULL;
    javaTopic_data = NULL;
    javaGroup_data = NULL;

    rc = SAJ_RETCODE_OK;

    if (*dst == NULL)
    {
        rc = saj_create_new_java_object(env, "DDS/SubscriptionBuiltinTopicData", dst);
    }

    /* copy the attributes from the gapi object to the java object */

    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_builtinTopicKeyCopyOut(
            env, &src->key, &javaKey);
    }
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_builtinTopicKeyCopyOut(
            env, &src->participant_key, &javaParticipant_key);
    }
    if (rc == SAJ_RETCODE_OK)
    {
        if (src != NULL && src->topic_name != NULL) {
            javaTopic_name = (*env)->NewStringUTF(env, src->topic_name);
            saj_exceptionCheck(env);
        }

        if(javaTopic_name == NULL){ /* src->topic_name was also NULL */
            javaTopic_name = (*env)->NewStringUTF(env, "");
        }
    }
    if (rc == SAJ_RETCODE_OK)
    {
        if (src != NULL && src->type_name != NULL) {
            javaType_name = (*env)->NewStringUTF(env, src->type_name);
            saj_exceptionCheck(env);
        }

        if(javaType_name == NULL){ /* src->type_name was also NULL */
            javaType_name = (*env)->NewStringUTF(env, "");
        }
    }
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_DurabilityQosPolicyCopyOut(
            env, &src->durability, &javaDurability);
    }
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_LatencyBudgetQosPolicyCopyOut(
            env, &src->latency_budget, &javaLatency_budget);
    }
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_LivelinessQosPolicyCopyOut(
            env, &src->liveliness, &javaLiveliness);
    }
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_ReliabilityQosPolicyCopyOut(
            env, &src->reliability, &javaReliability);
    }
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_OwnershipQosPolicyCopyOut(
            env, &src->ownership, &javaOwnership);
    }
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_DestinationOrderQosPolicyCopyOut(
            env, &src->destination_order, &javaDestination_order);
    }
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_UserDataQosPolicyCopyOut(
            env, &src->user_data, &javaUser_data);
    }
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_TimeBasedFilterQosPolicyCopyOut(
            env, &src->time_based_filter, &javaTime_based_filter);
    }
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_DeadlineQosPolicyCopyOut(
            env, &src->deadline, &javaDeadline);
    }
    if (rc == SAJ_RETCODE_OK)
    {
        rc = saj_PresentationQosPolicyCopyOut(
                env, &src->presentation, &javaPresentation);
    }
    if( rc == SAJ_RETCODE_OK)
    {
        rc = saj_PartitionQosPolicyCopyOut(
            env, &src->partition, &javaPartition);
    }
    if (rc == SAJ_RETCODE_OK)
    {
        rc = saj_TopicDataQosPolicyCopyOut(
            env, &src->topic_data, &javaTopic_data);
    }
    if( rc == SAJ_RETCODE_OK)
    {
        rc = saj_GroupDataQosPolicyCopyOut(
            env, &src->group_data, &javaGroup_data);
    }

    (*env)->SetObjectField(env, *dst,
        GET_CACHED(subscriptionBuiltinTopicData_key_fid), javaKey);
    saj_exceptionCheck(env);
    (*env)->SetObjectField(env, *dst,
        GET_CACHED(subscriptionBuiltinTopicData_participantKey_fid), javaParticipant_key);
    saj_exceptionCheck(env);
    (*env)->SetObjectField(env, *dst,
        GET_CACHED(subscriptionBuiltinTopicData_topicName_fid), javaTopic_name);
    saj_exceptionCheck(env);
    (*env)->SetObjectField(env, *dst,
        GET_CACHED(subscriptionBuiltinTopicData_typeName_fid), javaType_name);
    saj_exceptionCheck(env);
    (*env)->SetObjectField(env, *dst,
        GET_CACHED(subscriptionBuiltinTopicData_durability_fid), javaDurability);
    saj_exceptionCheck(env);
    (*env)->SetObjectField(env, *dst,
        GET_CACHED(subscriptionBuiltinTopicData_latencyBudget_fid), javaLatency_budget);
    saj_exceptionCheck(env);
    (*env)->SetObjectField(env, *dst,
        GET_CACHED(subscriptionBuiltinTopicData_liveliness_fid), javaLiveliness);
    saj_exceptionCheck(env);
    (*env)->SetObjectField(env, *dst,
        GET_CACHED(subscriptionBuiltinTopicData_reliability_fid), javaReliability);
    saj_exceptionCheck(env);
    (*env)->SetObjectField(env, *dst,
        GET_CACHED(subscriptionBuiltinTopicData_ownership_fid), javaOwnership);
    saj_exceptionCheck(env);
    (*env)->SetObjectField(env, *dst,
        GET_CACHED(subscriptionBuiltinTopicData_destinationOrder_fid), javaDestination_order);
    saj_exceptionCheck(env);
    (*env)->SetObjectField(env, *dst,
        GET_CACHED(subscriptionBuiltinTopicData_userData_fid), javaUser_data);
    saj_exceptionCheck(env);
    (*env)->SetObjectField(env, *dst,
        GET_CACHED(subscriptionBuiltinTopicData_timeBasedFilter_fid), javaTime_based_filter);
    saj_exceptionCheck(env);
    (*env)->SetObjectField(env, *dst,
        GET_CACHED(subscriptionBuiltinTopicData_deadline_fid), javaDeadline);
    saj_exceptionCheck(env);
    (*env)->SetObjectField(env, *dst,
        GET_CACHED(subscriptionBuiltinTopicData_presentation_fid), javaPresentation);
    saj_exceptionCheck(env);
    (*env)->SetObjectField(env, *dst,
        GET_CACHED(subscriptionBuiltinTopicData_partition_fid), javaPartition);
    saj_exceptionCheck(env);
    (*env)->SetObjectField(env, *dst,
        GET_CACHED(subscriptionBuiltinTopicData_topicData_fid), javaTopic_data);
    saj_exceptionCheck(env);
    (*env)->SetObjectField(env, *dst,
        GET_CACHED(subscriptionBuiltinTopicData_groupData_fid), javaGroup_data);
    saj_exceptionCheck(env);

    (*env)->DeleteLocalRef (env, javaKey);
    (*env)->DeleteLocalRef (env, javaParticipant_key);
    (*env)->DeleteLocalRef (env, javaTopic_name);
    (*env)->DeleteLocalRef (env, javaType_name);
    (*env)->DeleteLocalRef (env, javaDurability);
    (*env)->DeleteLocalRef (env, javaLatency_budget);
    (*env)->DeleteLocalRef (env, javaLiveliness);
    (*env)->DeleteLocalRef (env, javaReliability);
    (*env)->DeleteLocalRef (env, javaOwnership);
    (*env)->DeleteLocalRef (env, javaDestination_order);
    (*env)->DeleteLocalRef (env, javaUser_data);
    (*env)->DeleteLocalRef (env, javaTime_based_filter);
    (*env)->DeleteLocalRef (env, javaDeadline);
    (*env)->DeleteLocalRef (env, javaPresentation);
    (*env)->DeleteLocalRef (env, javaPartition);
    (*env)->DeleteLocalRef (env, javaTopic_data);
    (*env)->DeleteLocalRef (env, javaGroup_data);

    return rc;
}


saj_returnCode
saj_publicationBuiltinTopicDataCopyOut(
    JNIEnv* env,
    gapi_publicationBuiltinTopicData *src,
    jobject *dst)
{
    jintArray javaKey;
    jintArray javaParticipant_key;
    jobject javaTopic_name;
    jobject javaType_name;
    jobject javaDurability;
    jobject javaDeadline;
    jobject javaLatency_budget;
    jobject javaLiveliness;
    jobject javaReliability;
    jobject javaDestination_order;
    jobject javaUser_data;
    jobject javaOwnership;
    jobject javaOwnership_strength;
    jobject javaLifespan;
    jobject javaPresentation;
    jobject javaPartition;
    jobject javaTopic_data;
    jobject javaGroup_data;
    saj_returnCode rc;

    assert(dst != NULL);

    javaKey = NULL;
    javaParticipant_key = NULL;
    javaTopic_name = NULL;
    javaType_name = NULL;
    javaDurability = NULL;
    javaLatency_budget = NULL;
    javaLiveliness = NULL;
    javaReliability = NULL;
    javaOwnership = NULL;
    javaOwnership_strength = NULL;
    javaLifespan = NULL;
    javaDestination_order = NULL;
    javaUser_data = NULL;
    javaDeadline = NULL;
    javaPresentation = NULL;
    javaPartition = NULL;
    javaTopic_data = NULL;
    javaGroup_data = NULL;

    rc = SAJ_RETCODE_OK;

    if (*dst == NULL)
    {
        rc = saj_create_new_java_object(env, "DDS/PublicationBuiltinTopicData", dst);
    }

    /* copy the attributes from the gapi object to the java object */

    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_builtinTopicKeyCopyOut(
            env, &src->key, &javaKey);
    }
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_builtinTopicKeyCopyOut(
            env, &src->participant_key, &javaParticipant_key);
    }
    if (rc == SAJ_RETCODE_OK)
    {
        if (src != NULL && src->topic_name != NULL) {
            javaTopic_name = (*env)->NewStringUTF(env, src->topic_name);
            saj_exceptionCheck(env);
        }

        if(javaTopic_name == NULL){ /* src->topic_name was also NULL */
            javaTopic_name = (*env)->NewStringUTF(env, "");
        }
    }
    if (rc == SAJ_RETCODE_OK)
    {
        if (src != NULL && src->type_name != NULL) {
            javaType_name = (*env)->NewStringUTF(env, src->type_name);
            saj_exceptionCheck(env);
        }

        if(javaType_name == NULL){ /* src->type_name was also NULL */
            javaType_name = (*env)->NewStringUTF(env, "");
        }
    }
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_DurabilityQosPolicyCopyOut(
            env, &src->durability, &javaDurability);
    }
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_LatencyBudgetQosPolicyCopyOut(
            env, &src->latency_budget, &javaLatency_budget);
    }
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_LivelinessQosPolicyCopyOut(
            env, &src->liveliness, &javaLiveliness);
    }
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_ReliabilityQosPolicyCopyOut(
            env, &src->reliability, &javaReliability);
    }
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_OwnershipQosPolicyCopyOut(
            env, &src->ownership, &javaOwnership);
    }
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_OwnershipStrengthQosPolicyCopyOut(
            env, &src->ownership_strength, &javaOwnership_strength);
    }
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_LifespanQosPolicyCopyOut(
            env, &src->lifespan, &javaLifespan);
    }
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_DestinationOrderQosPolicyCopyOut(
            env, &src->destination_order, &javaDestination_order);
    }
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_UserDataQosPolicyCopyOut(
            env, &src->user_data, &javaUser_data);
    }
    if(rc == SAJ_RETCODE_OK)
    {
        rc = saj_DeadlineQosPolicyCopyOut(
            env, &src->deadline, &javaDeadline);
    }
    if (rc == SAJ_RETCODE_OK)
    {
        rc = saj_PresentationQosPolicyCopyOut(
                env, &src->presentation, &javaPresentation);
    }
    if( rc == SAJ_RETCODE_OK)
    {
        rc = saj_PartitionQosPolicyCopyOut(
            env, &src->partition, &javaPartition);
    }
    if (rc == SAJ_RETCODE_OK)
    {
        rc = saj_TopicDataQosPolicyCopyOut(
            env, &src->topic_data, &javaTopic_data);
    }
    if( rc == SAJ_RETCODE_OK)
    {
        rc = saj_GroupDataQosPolicyCopyOut(
            env, &src->group_data, &javaGroup_data);
    }

    (*env)->SetObjectField(env, *dst,
        GET_CACHED(publicationBuiltinTopicData_key_fid), javaKey);
    saj_exceptionCheck(env);
    (*env)->SetObjectField(env, *dst,
        GET_CACHED(publicationBuiltinTopicData_participantKey_fid), javaParticipant_key);
    saj_exceptionCheck(env);
    (*env)->SetObjectField(env, *dst,
        GET_CACHED(publicationBuiltinTopicData_topicName_fid), javaTopic_name);
    saj_exceptionCheck(env);
    (*env)->SetObjectField(env, *dst,
        GET_CACHED(publicationBuiltinTopicData_typeName_fid), javaType_name);
    saj_exceptionCheck(env);
    (*env)->SetObjectField(env, *dst,
        GET_CACHED(publicationBuiltinTopicData_durability_fid), javaDurability);
    saj_exceptionCheck(env);
    (*env)->SetObjectField(env, *dst,
        GET_CACHED(publicationBuiltinTopicData_latencyBudget_fid), javaLatency_budget);
    saj_exceptionCheck(env);
    (*env)->SetObjectField(env, *dst,
        GET_CACHED(publicationBuiltinTopicData_liveliness_fid), javaLiveliness);
    saj_exceptionCheck(env);
    (*env)->SetObjectField(env, *dst,
        GET_CACHED(publicationBuiltinTopicData_reliability_fid), javaReliability);
    saj_exceptionCheck(env);
    (*env)->SetObjectField(env, *dst,
        GET_CACHED(publicationBuiltinTopicData_ownership_fid), javaOwnership);
    saj_exceptionCheck(env);
    (*env)->SetObjectField(env, *dst,
        GET_CACHED(publicationBuiltinTopicData_ownershipStrength_fid), javaOwnership_strength);
    saj_exceptionCheck(env);
    (*env)->SetObjectField(env, *dst,
        GET_CACHED(publicationBuiltinTopicData_lifespan_fid), javaLifespan);
    saj_exceptionCheck(env);
    (*env)->SetObjectField(env, *dst,
        GET_CACHED(publicationBuiltinTopicData_destinationOrder_fid), javaDestination_order);
    saj_exceptionCheck(env);
    (*env)->SetObjectField(env, *dst,
        GET_CACHED(publicationBuiltinTopicData_userData_fid), javaUser_data);
    saj_exceptionCheck(env);
    (*env)->SetObjectField(env, *dst,
        GET_CACHED(publicationBuiltinTopicData_deadline_fid), javaDeadline);
    saj_exceptionCheck(env);
    (*env)->SetObjectField(env, *dst,
        GET_CACHED(publicationBuiltinTopicData_presentation_fid), javaPresentation);
    saj_exceptionCheck(env);
    (*env)->SetObjectField(env, *dst,
        GET_CACHED(publicationBuiltinTopicData_partition_fid), javaPartition);
    saj_exceptionCheck(env);
    (*env)->SetObjectField(env, *dst,
        GET_CACHED(publicationBuiltinTopicData_topicData_fid), javaTopic_data);
    saj_exceptionCheck(env);
    (*env)->SetObjectField(env, *dst,
        GET_CACHED(publicationBuiltinTopicData_groupData_fid), javaGroup_data);
    saj_exceptionCheck(env);

    (*env)->DeleteLocalRef (env, javaKey);
    (*env)->DeleteLocalRef (env, javaParticipant_key);
    (*env)->DeleteLocalRef (env, javaTopic_name);
    (*env)->DeleteLocalRef (env, javaType_name);
    (*env)->DeleteLocalRef (env, javaDurability);
    (*env)->DeleteLocalRef (env, javaLatency_budget);
    (*env)->DeleteLocalRef (env, javaLiveliness);
    (*env)->DeleteLocalRef (env, javaReliability);
    (*env)->DeleteLocalRef (env, javaOwnership);
    (*env)->DeleteLocalRef (env, javaOwnership_strength);
    (*env)->DeleteLocalRef (env, javaLifespan);
    (*env)->DeleteLocalRef (env, javaDestination_order);
    (*env)->DeleteLocalRef (env, javaUser_data);
    (*env)->DeleteLocalRef (env, javaDeadline);
    (*env)->DeleteLocalRef (env, javaPresentation);
    (*env)->DeleteLocalRef (env, javaPartition);
    (*env)->DeleteLocalRef (env, javaTopic_data);
    (*env)->DeleteLocalRef (env, javaGroup_data);

    return rc;
}

saj_returnCode
saj_EnumCopyIn(
    JNIEnv      *env,
    jobject     src,
    gapi_unsigned_long *dst)
{
    jclass enumClass;
    jfieldID enumValue_fid;
    jmethodID valueMethodId;
    saj_returnCode rc;
    jthrowable jexception = NULL;

    assert(dst != NULL);

    enumClass = NULL;
    enumValue_fid = NULL;
    rc = SAJ_RETCODE_OK;

    if(src != NULL)
    {
        enumClass = (*env)->GetObjectClass(env, src);
        saj_exceptionCheck(env);

        /* get __value fieldid from the enum class */
        enumValue_fid = (*env)->GetFieldID(env, enumClass, "__value", "I");
        jexception = (*env)->ExceptionOccurred(env);

        if (jexception)
        {
            /*  clear the exception */
            (*env)->ExceptionClear(env);
            valueMethodId = (*env)->GetMethodID(env, enumClass, "value", "()I");
            saj_exceptionCheck(env);
            if(valueMethodId != NULL)
            {
                *dst = (*env)->CallIntMethod(env, src, valueMethodId);
            } else
            {
                rc = SAJ_RETCODE_ERROR;
            }
        } else
        {
            *dst = (*env)->GetIntField(env, src, enumValue_fid);
            saj_exceptionCheck(env);
        }
    }

    return rc;

}

saj_returnCode
saj_EnumCopyOut(
    JNIEnv              *env,
    const char          *classname,
    gapi_unsigned_long  src,
    jobject             *dst)
{
    jclass enumClassId;
    jmethodID from_intMethodId;
    saj_returnCode rc;
    char methodString[255];

    assert(dst != NULL && classname != NULL);
    assert(strlen(classname) < 249);

    enumClassId = NULL;
    from_intMethodId = NULL;
    rc = SAJ_RETCODE_ERROR;

    /* construct a method signature */
    snprintf(methodString, 255, "(I)L%s;", classname);

    enumClassId = (*env)->FindClass(env, classname);
    saj_exceptionCheck(env);

    if (enumClassId != NULL){
        from_intMethodId = (*env)->GetStaticMethodID(
            env, enumClassId, "from_int", methodString);
        saj_exceptionCheck(env);

        if (from_intMethodId != NULL){
            *dst = (*env)->CallStaticObjectMethod(
                env, enumClassId, from_intMethodId, src);
            saj_exceptionCheck(env);

            if(*dst != NULL){
                rc = SAJ_RETCODE_OK;
            }
        }
    }

    return rc;
}

saj_returnCode
saj_sampleInfoCopyOut(
    JNIEnv              *env,
    gapi_sampleInfo     *src,
    jobject             *dst)
{
    jobject source_timestamp;
    jobject reception_timestamp;

    assert (src);
    assert (dst);

    if (*dst == NULL) {
       *dst = (*env)->NewObject (env, GET_CACHED(sampleInfo_class), GET_CACHED(sampleInfo_constructor_mid), NULL);
       saj_exceptionCheck(env);

       if (*dst == NULL) {
           return SAJ_RETCODE_ERROR;
       }
    }
    (*env)->SetIntField    (env, *dst, GET_CACHED(sampleInfo_sample_state_fid), src->sample_state);
    saj_exceptionCheck(env);
    (*env)->SetIntField    (env, *dst, GET_CACHED(sampleInfo_view_state_fid), src->view_state);
    saj_exceptionCheck(env);
    (*env)->SetIntField    (env, *dst, GET_CACHED(sampleInfo_instance_state_fid), src->instance_state);
    saj_exceptionCheck(env);
    (*env)->SetBooleanField(env, *dst, GET_CACHED(sampleInfo_valid_data_fid), src->valid_data);
    saj_exceptionCheck(env);
    source_timestamp = (*env)->GetObjectField (env, *dst, GET_CACHED(sampleInfo_source_timestamp_fid));
    saj_exceptionCheck(env);
    saj_timeCopyOut (env,  &src->source_timestamp, &source_timestamp);
    saj_exceptionCheck(env);

    if (source_timestamp == NULL) {
       return SAJ_RETCODE_ERROR;
    }
    (*env)->SetObjectField (env, *dst, GET_CACHED(sampleInfo_source_timestamp_fid), source_timestamp);
    saj_exceptionCheck(env);
    (*env)->SetLongField    (env, *dst, GET_CACHED(sampleInfo_instance_handle_fid), src->instance_handle);
    saj_exceptionCheck(env);

    (*env)->SetLongField    (env, *dst, GET_CACHED(sampleInfo_publication_handle_fid), src->publication_handle);
    saj_exceptionCheck(env);

    (*env)->SetIntField    (env, *dst, GET_CACHED(sampleInfo_disposed_generation_count_fid), src->disposed_generation_count);
    saj_exceptionCheck(env);
    (*env)->SetIntField    (env, *dst, GET_CACHED(sampleInfo_no_writers_generation_count_fid), src->no_writers_generation_count);
    saj_exceptionCheck(env);
    (*env)->SetIntField    (env, *dst, GET_CACHED(sampleInfo_sample_rank_fid), src->sample_rank);
    saj_exceptionCheck(env);
    (*env)->SetIntField    (env, *dst, GET_CACHED(sampleInfo_generation_rank_fid), src->generation_rank);
    saj_exceptionCheck(env);
    (*env)->SetIntField    (env, *dst, GET_CACHED(sampleInfo_absolute_generation_rank_fid), src->absolute_generation_rank);
    saj_exceptionCheck(env);

    reception_timestamp = (*env)->GetObjectField (env, *dst, GET_CACHED(sampleInfo_reception_timestamp_fid));
    saj_exceptionCheck(env);
    saj_timeCopyOut (env,  &src->reception_timestamp, &reception_timestamp);
    saj_exceptionCheck(env);

    if (reception_timestamp == NULL) {
       return SAJ_RETCODE_ERROR;
    }
    (*env)->SetObjectField (env, *dst, GET_CACHED(sampleInfo_reception_timestamp_fid), reception_timestamp);
    saj_exceptionCheck(env);

    return SAJ_RETCODE_OK;
}

saj_returnCode
saj_sampleInfoHolderCopyOut(
    JNIEnv              *env,
    gapi_sampleInfo     *src,
    jobject             *dst)
{
    jobject sampleInfo;
    jobject si;

    assert (dst);
    assert (*dst);

    sampleInfo = (*env)->GetObjectField (env, *dst, GET_CACHED(sampleInfoHolder_value_fid));
    saj_exceptionCheck(env);
    si = sampleInfo;

    if (saj_sampleInfoCopyOut (env, src, &sampleInfo) == SAJ_RETCODE_ERROR) {
       return SAJ_RETCODE_ERROR;
    }
    if (sampleInfo != si) {
        (*env)->SetObjectField (env, *dst, GET_CACHED(sampleInfoHolder_value_fid), sampleInfo);
        saj_exceptionCheck(env);
    }
    return SAJ_RETCODE_OK;
}

saj_returnCode
saj_stringSequenceCopyOut(
    JNIEnv *env,
    gapi_stringSeq src,
    jobjectArray *dst)
{
    jclass stringArrCls;
    jstring javaString;
    gapi_unsigned_long i;
    saj_returnCode rc;
    gapi_unsigned_long seqLength;

    assert(dst != NULL);

    javaString = NULL;
    rc = SAJ_RETCODE_OK;

    seqLength = src._length;
    stringArrCls = (*env)->FindClass(env, "java/lang/String");
    saj_exceptionCheck(env);

    assert(stringArrCls != NULL);

    *dst = (*env)->NewObjectArray(env, seqLength, stringArrCls, NULL);
    saj_exceptionCheck(env);


    /* get the c strings from the buffer */
    for (i = 0; i < seqLength && rc == SAJ_RETCODE_OK; i++)
    {
        gapi_string gapi_str = src._buffer[i];
        if (gapi_str != NULL) {
            javaString = (*env)->NewStringUTF(env, gapi_str);
            saj_exceptionCheck(env);
        }

        if (javaString != NULL)
        {
            /* store the string object in the string array */
            (*env)->SetObjectArrayElement(env, *dst, i, javaString);

            if ((*env)->ExceptionCheck(env) == JNI_TRUE)
            {
                rc = SAJ_RETCODE_ERROR;
            }
        }
        else
        {
            rc = SAJ_RETCODE_ERROR;
        }

        (*env)->DeleteLocalRef(env, javaString);
        saj_exceptionCheck(env);
    }

    return rc;
}

saj_returnCode
saj_LookupExistingDataReaderSeq(
    JNIEnv *env,
    gapi_dataReaderSeq *src,
    jobjectArray *dst)
{
    jclass classId;
    saj_returnCode rc;

    assert(dst != NULL);

    classId = NULL;
    rc = SAJ_RETCODE_ERROR;

    /* find the class id of the sequence class */
    classId = (*env)->FindClass(env, "org/opensplice/dds/dcps/DataReaderImpl");
    saj_exceptionCheck(env);

    assert(classId != NULL);

    rc = saj_gapiObjectBufferCopyOut(
        env, classId, src->_length, src->_buffer, dst);

    return rc;
}

saj_returnCode
saj_LookupExistingConditionSeq(
    JNIEnv *env,
    gapi_conditionSeq *src,
    jobjectArray *dst)
{
    jclass classId;
    saj_returnCode rc;

    assert(dst != NULL);

    classId = NULL;
    rc = SAJ_RETCODE_ERROR;

    /* find the class id of the sequence class */
    classId = (*env)->FindClass(env, "DDS/Condition");
    saj_exceptionCheck(env);

    assert(classId != NULL);

    rc = saj_gapiObjectBufferCopyOut(
        env, classId, src->_length, src->_buffer, dst);

    return rc;
}

saj_returnCode
saj_gapiObjectBufferCopyOut(
    JNIEnv *env,
    jclass classId,
    gapi_unsigned_long seqLength,
    gapi_object *src,
    jobjectArray *dst)
{
    jobject object;
    gapi_unsigned_long i;
    saj_returnCode rc;

    assert(dst != NULL);
    assert((seqLength == 0) || (src != NULL));

    object = NULL;
    rc = SAJ_RETCODE_ERROR;

    if(classId != NULL)
    {
        /* create a new object array */
        *dst = (*env)->NewObjectArray(env, seqLength, classId, NULL);
        saj_exceptionCheck(env);

        if (*dst != NULL)
        {
            rc = SAJ_RETCODE_OK;
            for (i = 0; i < seqLength && rc == SAJ_RETCODE_OK; i++)
            {
                /* find the reference to the already existing java object */
                object = saj_read_java_address((gapi_object)src[i]);

                assert(object != NULL);

                (*env)->SetObjectArrayElement(env, *dst, i, object);

                if ((*env)->ExceptionCheck(env) == JNI_TRUE)
                {
                    /* ArrayIndexOutOfBoundsException or ArrayStoreException */
                    rc = SAJ_RETCODE_ERROR;
                }
            }
        }
    }

    return rc;
}

saj_returnCode
saj_LookupTypeSupportDataReader(
    JNIEnv* env,
    jobject jtypeSupport,
    gapi_char** result)
{
    jfieldID fid;
    jstring jresult;
    saj_returnCode rc;
    const char* data;

    rc = SAJ_RETCODE_ERROR;

    if(jtypeSupport != NULL)
    {
        fid = GET_CACHED(typeSupportDataReader_fid);

        if(fid != NULL)
        {
            jresult = (jstring)((*env)->GetObjectField(env, jtypeSupport, fid));
            saj_exceptionCheck(env);

            if(jresult != NULL){
               data = (*env)->GetStringUTFChars(env, jresult, 0);
               saj_exceptionCheck(env);
               *result = gapi_string_dup(data);
               (*env)->ReleaseStringUTFChars(env, jresult, data);
               saj_exceptionCheck(env);
            }
            rc = SAJ_RETCODE_OK;
        }
    }
    return rc;
}

saj_returnCode
saj_LookupTypeSupportDataReaderView(
    JNIEnv* env,
    jobject jtypeSupport,
    gapi_char** result)
{
    jfieldID fid;
    jstring jresult;
    saj_returnCode rc;
    const char* data;

    rc = SAJ_RETCODE_ERROR;

    if(jtypeSupport != NULL)
    {
        fid = GET_CACHED(typeSupportDataReaderView_fid);

        if(fid != NULL)
        {
            jresult = (jstring)((*env)->GetObjectField(env, jtypeSupport, fid));
            saj_exceptionCheck(env);

            if(jresult != NULL){
               data = (*env)->GetStringUTFChars(env, jresult, 0);
               saj_exceptionCheck(env);
               *result = gapi_string_dup(data);
               (*env)->ReleaseStringUTFChars(env, jresult, data);
               saj_exceptionCheck(env);
            }
            rc = SAJ_RETCODE_OK;
        }
    }
    return rc;
}

saj_returnCode
saj_LookupTypeSupportDataWriter(
    JNIEnv* env,
    jobject jtypeSupport,
    gapi_char** result)
{
    jfieldID fid;
    jstring jresult;
    saj_returnCode rc;
    const char* data;

    assert(result != NULL);

    rc = SAJ_RETCODE_ERROR;

    if(jtypeSupport != NULL)
    {
        fid = GET_CACHED(typeSupportDataWriter_fid);

        if(fid != NULL){
            jresult = (jstring)((*env)->GetObjectField(env, jtypeSupport, fid));
            saj_exceptionCheck(env);

            if(jresult != NULL){
                data = (*env)->GetStringUTFChars(env, jresult, 0);
                saj_exceptionCheck(env);
                *result = gapi_string_dup(data);
                (*env)->ReleaseStringUTFChars(env, jresult, data);
                saj_exceptionCheck(env);
            }
            rc = SAJ_RETCODE_OK;
        }
    }
    return rc;
}

saj_returnCode
saj_LookupTypeSupportConstructorSignature(
    JNIEnv* env,
    jobject jtypeSupport,
    gapi_char** result)
{
    jfieldID fid;
    jstring jresult;
    saj_returnCode rc;
    const char* data;

    rc = SAJ_RETCODE_ERROR;

    if(jtypeSupport != NULL)
    {
        fid = GET_CACHED(typeSupportConstructorSignature_fid);

        if(fid != NULL)
        {
            jresult = (jstring)((*env)->GetObjectField(env, jtypeSupport, fid));
            saj_exceptionCheck(env);

            if(jresult != NULL){
                data = (*env)->GetStringUTFChars(env, jresult, 0);
                saj_exceptionCheck(env);
                *result = gapi_string_dup(data);
                (*env)->ReleaseStringUTFChars(env, jresult, data);
                saj_exceptionCheck(env);
            }
            rc = SAJ_RETCODE_OK;
        }
    }
    return rc;
}

saj_returnCode
saj_write_parallelDemarshallingContext_address(
    JNIEnv *env,
    jobject java_object,
    sajParDemContext address)
{
    saj_returnCode rc = SAJ_RETCODE_ERROR;

    if(java_object != NULL){
        rc = checkJavaObject(env, java_object);

        if(rc == SAJ_RETCODE_OK){
            /* Verify the java_object is an instance of DataReaderImpl */
            assert(GET_CACHED(dataReaderImpl_class));
            if( !(*env)->IsInstanceOf(env, java_object, GET_CACHED(dataReaderImpl_class))){
                rc = SAJ_RETCODE_ERROR;
            }

            if( rc == SAJ_RETCODE_OK){
                assert(GET_CACHED(dataReaderImplClassParallelDemarshallingContext_fid));
                /* write value to java object */
                (*env)->SetLongField(env, java_object, GET_CACHED(dataReaderImplClassParallelDemarshallingContext_fid), (PA_ADDRCAST)address);
                saj_exceptionCheck(env);
            }
        }
    }
    return rc;
}

saj_returnCode
saj_read_parallelDemarshallingContext_address(
    JNIEnv *env,
    jobject java_object,
    sajParDemContext *address)
{
    saj_returnCode rc = SAJ_RETCODE_ERROR;

    assert(address);

    if(java_object != NULL){
        rc = checkJavaObject(env, java_object);

        if(rc == SAJ_RETCODE_OK){
            /* Verify the java_object is an instance of DataReaderImpl */
            assert(GET_CACHED(dataReaderImpl_class));
            if( !(*env)->IsInstanceOf(env, java_object, GET_CACHED(dataReaderImpl_class))){
                rc = SAJ_RETCODE_ERROR;
            }

            if( rc == SAJ_RETCODE_OK){
                assert(GET_CACHED(dataReaderImplClassParallelDemarshallingContext_fid));
                /* get value from java object */
                *address = (sajParDemContext)(PA_ADDRCAST) (*env)->GetLongField(env, java_object, GET_CACHED(dataReaderImplClassParallelDemarshallingContext_fid));
                saj_exceptionCheck(env);
            }
        }
    }
    if(rc != SAJ_RETCODE_OK){
        *address = NULL; /* Ensure out-param is always initialized */
    }

    return rc;
}

static saj_returnCode saj_prepare_CDRCopy (JNIEnv *env, jobject obj)
{
  /* obj is an instance of <Type>DataReaderImpl, I'm pretty sure. That
     one has a "private long copyCache" containing the address of the
     relevant copyCache. */
  const jclass objClass = (*env)->GetObjectClass (env, obj);
  const jfieldID fid = (*env)->GetFieldID (env, objClass, "copyCache", "J");
  saj_copyCache copyCache;
  if (fid == NULL)
    return SAJ_RETCODE_ERROR;
  copyCache = (saj_copyCache)(PA_ADDRCAST) (*env)->GetLongField (env, obj, fid);
  if (sd_cdrCompile (saj_copyCacheCdrInfo (copyCache)) < 0) {
    return SAJ_RETCODE_ERROR;
  } else {
    return SAJ_RETCODE_OK;
  }
}

saj_returnCode
saj_write_CDRCopy_value(
    JNIEnv *env,
    jobject java_object,
    long value)
{
    saj_returnCode rc = SAJ_RETCODE_ERROR;

    if(java_object != NULL){
        rc = checkJavaObject(env, java_object);

        if(rc == SAJ_RETCODE_OK){
            /* Verify the java_object is an instance of DataReaderImpl */
            assert(GET_CACHED(dataReaderImpl_class));
            if( !(*env)->IsInstanceOf(env, java_object, GET_CACHED(dataReaderImpl_class))){
                rc = SAJ_RETCODE_ERROR;
            }

            if( rc == SAJ_RETCODE_OK){
                assert(GET_CACHED(dataReaderImplClassCDRCopy_fid));
                /* write value to java object */

                if (value) {
                  rc = saj_prepare_CDRCopy (env, java_object);
                }

                if (rc == SAJ_RETCODE_OK) {
                  (*env)->SetLongField(env, java_object, GET_CACHED(dataReaderImplClassCDRCopy_fid), value);
                  saj_exceptionCheck(env);
                } else {
                  OS_REPORT (OS_ERROR, "dcpssaj", 0, "saj_write_CDRCopy_value: unsupported type");
                }
            }
        }
    }
    return rc;
}

c_bool
saj_setThreadEnv(
    JNIEnv *env)
{
    void *threadData;
    c_bool result;

    threadData = os_threadMemGet(OS_THREAD_JVM);
    if (threadData) {
        *(JNIEnv**)threadData = env;
        result = FALSE;
    } else {
        threadData = os_threadMemMalloc(OS_THREAD_JVM, sizeof(env));
        if (threadData) {
            *(JNIEnv**)threadData = env;
            result = TRUE;
        } else {
            result = FALSE;
            assert(0);
        }
    }
    return result;
}

JNIEnv *
saj_getThreadEnv()
{
    return (JNIEnv *)os_threadMemGet(OS_THREAD_JVM);
}

void
saj_delThreadEnv(
    c_bool value)
{
    if (value) {
        os_threadMemFree(OS_THREAD_JVM);
    }
}

