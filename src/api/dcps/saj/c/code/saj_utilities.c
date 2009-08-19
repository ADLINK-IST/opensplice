/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */

#include <jni.h>
#include "saj_utilities.h"
#include "saj_listener.h"
#include "os_stdlib.h"
#include "os_heap.h"

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
    if ((*javaEnv)->ExceptionOccurred(javaEnv))
        assert(0);
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
                SET_CACHED(sajSuperClassGapiPeer_fid, (*env)->GetFieldID(env, GET_CACHED(gapiSuperClass_class), "gapiObject", "J"));
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
                SET_CACHED(sajSuperClassGapiPeer_fid, (*env)->GetFieldID(env, GET_CACHED(gapiSuperClass_class),
    				       "gapiPeer",
    				       "J"));
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
    gapi_object gapi_obj)
{
    saj_userData ud;
    jobject result;

    result = NULL;

    if(gapi_obj != GAPI_OBJECT_NIL){
        ud = saj_userData(gapi_object_get_user_data(gapi_obj));

        if(ud != NULL){
            if(ud->listenerData != NULL){
                result = ud->listenerData->jlistener;
            }
        }
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
    gapi_object_set_user_data(gapi_obj, (void*)ud);
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
    gapi_object_set_user_data(gapi_obj, (void*)ud);
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
            gapi_object_set_user_data(gapi_obj, (void*)ud);
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
    assert(java_object != NULL);

    if(gapi_obj != GAPI_OBJECT_NIL){
        ud = saj_userData(gapi_object_get_user_data(gapi_obj));

        if(ud != NULL){
            if(ud->statusConditionUserData != NULL){
                saj_destroy_user_data(env, ud->statusConditionUserData);
            }
            ud->statusConditionUserData = gapi_object_get_user_data(
                                                        (gapi_object)condition);
            gapi_object_set_user_data(gapi_obj, (void*)ud);
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

    ud = saj_userData(entityUserData);
    env = (JNIEnv*)userData;
    saj_destroy_user_data(env, ud);
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
        (*env)->DeleteGlobalRef(env, ud->saj_object);
        saj_exceptionCheck(env);
        os_free(ud);
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

    return SAJ_RETCODE_OK;;
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
            GET_CACHED(time_t_class),
            GET_CACHED(time_t_constructor_mid),
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
        out->_release = FALSE;

        /* fill the string buffer with strings */
        for (i = 0; i < arrayLength && rc == SAJ_RETCODE_OK; i++)
        {
            /* get the java String from the array */
            javaString = (*env)->GetObjectArrayElement(env, stringArray, i);
            saj_exceptionCheck(env);

            /* translate the java string to a c string */
            vm_managed_c_string = (*env)->GetStringUTFChars(env, javaString, NULL);
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
        out->_release = FALSE;

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

saj_returnCode
saj_instanceHandleSequenceCopyOut(
    JNIEnv* env,
    gapi_instanceHandleSeq *src,
    jintArray *dst)
{
    saj_returnCode rc;

    assert(dst != NULL);

    rc = SAJ_RETCODE_ERROR;

    *dst = (*env)->NewIntArray(env, src->_length);
    saj_exceptionCheck(env);

    if (*dst != NULL){
        (*env)->SetIntArrayRegion(env, *dst, 0, src->_length, (jint *)src->_buffer);

        if ((*env)->ExceptionCheck(env) == JNI_FALSE){
            rc = SAJ_RETCODE_OK;
        }
    }
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
    gapi_sampleInfo	*src,
    jobject             *dst)
{
    jobject source_timestamp;

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

    return SAJ_RETCODE_OK;
}

saj_returnCode
saj_sampleInfoHolderCopyOut(
    JNIEnv              *env,
    gapi_sampleInfo	*src,
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
        javaString = (*env)->NewStringUTF(env, src._buffer[i]);
        saj_exceptionCheck(env);

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
               data = (*env)->GetStringUTFChars(env, jresult, NULL);
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
                data = (*env)->GetStringUTFChars(env, jresult, NULL);
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
                data = (*env)->GetStringUTFChars(env, jresult, NULL);
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
