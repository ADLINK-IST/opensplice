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
#include "DLRL_Report.h"
#include "DJA_ObjectHome.h"

/* DLRL Metamodel includes */
#include "DMM_AttributeType.h"
#include "DMM_KeyType.h"
#include "DMM_Basis.h"
#include "DLRL_Kernel.h"
#include "DLRL_Kernel_private.h"

/* include DLRL SAJ code */
#include "DJA_ExceptionHandler.h"
#include "DJA_Initialisation.h"

#include "DLRL_Types.h"

/* os abstraction layer includes */
#include "os_heap.h"

/* C includes */
#include <assert.h>

/* NOT IN DESIGN */
static void
DJA_ObjectHome_us_visitObjectAction(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectAdmin* object,
    LOC_unsigned_long totalNrOfObjects,
    LOC_unsigned_long index,
    void** arg);

JNIEXPORT void JNICALL
Java_DDS_ObjectHome_jniConstructObjectHome(
    JNIEnv * env,
    jobject ls_home,
    jstring jname,
    jstring jpathPrefixed,
    jstring jpathPrefixedExceptLast,
    jstring jtopicTypePrefixed,
    jstring jtargetImplClassName)
{
    DK_ObjectHomeAdmin* home = NULL;
    jobject weakGlobalizedHome = NULL;
    DJA_CachedJNITypedObject* typedObjectCachedData = NULL;
    DLRL_LS_object oldVal = NULL;
    DLRL_Exception exception;
    LOC_string name = NULL;
    LOC_string pathPrefixed = NULL;
    LOC_string pathPrefixedExceptLast = NULL;
    LOC_string topicTypePrefixed = NULL;
    LOC_string targetImplClassName = NULL;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "ObjectHome->new (default constructor)");
    assert (jname);
    assert (jpathPrefixed);
    assert (jpathPrefixedExceptLast);
    assert (jtopicTypePrefixed);

    DLRL_Exception_init(&exception);

    name = (LOC_string)(*env)->GetStringUTFChars(
        env,
        jname,
        0);
    DLRL_JavaException_PROPAGATE(env, &exception);

    pathPrefixed = (LOC_string)(*env)->GetStringUTFChars(
        env,
        jpathPrefixed,
        0);
    DLRL_JavaException_PROPAGATE(env, &exception);

    pathPrefixedExceptLast = (LOC_string)(*env)->GetStringUTFChars(
        env,
        jpathPrefixedExceptLast,
        0);
    DLRL_JavaException_PROPAGATE(env, &exception);

    topicTypePrefixed = (LOC_string)(*env)->GetStringUTFChars(
        env,
        jtopicTypePrefixed,
        0) ;
    DLRL_JavaException_PROPAGATE(env, &exception);

    targetImplClassName = (LOC_string)(*env)->GetStringUTFChars(
        env,
        jtargetImplClassName,
        0) ;
    DLRL_JavaException_PROPAGATE(env, &exception);


    /* create it as a weak global ref so that if the home gets discarded by the
     *  application before it was registered to the application that it will be
     * garbage collected normally. If we were to make a normal  global ref it
     * would prevent the discarded object home from being garbage collected as
     * the kernel object home  would still have a pointer to the java object
     * home. A weak global ref prevents this problem
     */
    weakGlobalizedHome = (*env)->NewWeakGlobalRef(env, ls_home);
    DLRL_JavaException_PROPAGATE(env, &exception);
    if(!weakGlobalizedHome){
        DLRL_Exception_THROW(&exception, DLRL_OUT_OF_MEMORY,
            "Unable to create a weak global ref of the provided object home");
    }
    typedObjectCachedData = DJA_Initialisation_loadTypedObjectCache(
        &exception,
        env,
        ls_home,
        pathPrefixed,
        pathPrefixedExceptLast,
        topicTypePrefixed,
        targetImplClassName);
    DLRL_Exception_PROPAGATE(&exception);

    home = DK_ObjectHomeAdmin_new(&exception, name);
    DLRL_Exception_PROPAGATE(&exception);
    DK_ObjectHomeAdmin_ts_setUserData(
        home,
        &exception,
        (void*)typedObjectCachedData);
    DLRL_Exception_PROPAGATE(&exception);

    oldVal = DK_ObjectHomeAdmin_ts_registerLSObjectHome(
        home,
        &exception,
        (DLRL_LS_object)weakGlobalizedHome);
    DLRL_Exception_PROPAGATE(&exception);
    assert(!oldVal);
    (*env)->SetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid,
        (jlong)home);


    DLRL_Exception_EXIT(&exception);
    /* clean up */
    if(exception.exceptionID != DLRL_NO_EXCEPTION){
        if(home){
            /* if home is not null, then home owns the typedObjectCachedData
             * and dont have to delete it explictly
             */
            DK_ObjectHomeAdmin_ts_delete(home, (void*)env);
            DK_Entity_ts_release((DK_Entity*)home);
            home = NULL;
        } else if(typedObjectCachedData){
            /* typedObjectCachedData is already deleted by the object home
             * delete! important to realize! unless the home creation failed
             * ofcourse, hence this path
             */
            DJA_Initialisation_us_destroyTypedObjectCache(
                env,
                typedObjectCachedData);
        }
        if(weakGlobalizedHome){
            (*env)->DeleteWeakGlobalRef(env, weakGlobalizedHome);
            weakGlobalizedHome= NULL;
        }
    }
    if(name){
        (*env)->ReleaseStringUTFChars(
            env,
            jname,
            name);
    }
    if(pathPrefixed){
        (*env)->ReleaseStringUTFChars(
            env,
            jpathPrefixed,
            pathPrefixed);
    }
    if(pathPrefixedExceptLast){
        (*env)->ReleaseStringUTFChars(
            env,
            jpathPrefixedExceptLast,
            pathPrefixedExceptLast);
    }
    if(topicTypePrefixed){
        (*env)->ReleaseStringUTFChars(
            env,
            jtopicTypePrefixed,
            topicTypePrefixed);
    }

    if(targetImplClassName){
        (*env)->ReleaseStringUTFChars(
            env,
            jtargetImplClassName,
            targetImplClassName);
    }


    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT void JNICALL
Java_DDS_ObjectHome_jniDeleteObjectHome(
    JNIEnv * env,
    jobject ls_home)
{
    DK_Entity* homeEntity;

    DLRL_INFO(INF_ENTER);

    homeEntity = (DK_Entity*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    if(homeEntity){
        /* needed if we never got past the weak global ref stage */
        DK_ObjectHomeAdmin_ts_delete(
            (DK_ObjectHomeAdmin*) homeEntity,
            (void *)env);
        DK_Entity_ts_release(homeEntity);
    }

    DLRL_INFO(INF_EXIT);
}

JNIEXPORT jstring JNICALL
Java_DDS_ObjectHome_jniName(
    JNIEnv * env,
    jobject ls_home)
{
    jstring jname = NULL;
    LOC_string name = NULL;
    DK_ObjectHomeAdmin* home = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "ObjectHome->name()");

    DLRL_Exception_init(&exception);
    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);

    DK_ObjectHomeAdmin_lockAdmin(home);
    DK_ObjectHomeAdmin_us_checkAlive(home, &exception);
    DLRL_Exception_PROPAGATE(&exception);

    name = DK_ObjectHomeAdmin_us_getName(home);
    jname = name ? (*env)->NewStringUTF(env, name) : NULL;
    DLRL_JavaException_PROPAGATE(env, &exception);

    DLRL_Exception_EXIT(&exception);
    DK_ObjectHomeAdmin_unlockAdmin(home);

    /* exception check is done outside the lock/unlock. Note that an exception
     * check must always be done after doing a  JNI call on the env.
     */
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return jname;
}

JNIEXPORT jstring JNICALL
Java_DDS_ObjectHome_jniFilter(
    JNIEnv * env,
    jobject ls_home)
{
    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "ObjectHome->filter()");
    DLRL_INFO(INF_EXIT);
    return (*env)->NewStringUTF(env, "");
}

JNIEXPORT jobject JNICALL
Java_DDS_ObjectHome_jniFindObject(
    JNIEnv * env,
    jobject ls_home,
    jobject joid,
    jobject jsource)
{
    /*TODO ID:? not implemented*/
    return NULL;
}

JNIEXPORT jobject JNICALL
Java_DDS_ObjectHome_jniParent(
    JNIEnv * env,
    jobject ls_home)
{
    jobject jparent = NULL;
    DK_ObjectHomeAdmin* parent = NULL;
    DK_ObjectHomeAdmin* home = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "ObjectHome->parent()");

    DLRL_Exception_init(&exception);
    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);

    parent = DK_ObjectHomeAdmin_ts_getParent(home, &exception);
    DLRL_Exception_PROPAGATE(&exception);
    if(parent){
        DK_ObjectHomeAdmin_lockAdmin(parent);
        DK_ObjectHomeAdmin_us_checkAlive(parent, &exception);
        DLRL_Exception_PROPAGATE(&exception);
        assert(DK_ObjectHomeAdmin_us_getLSHome(parent));
        jparent = (*env)->NewLocalRef(
            env,
            (jobject)DK_ObjectHomeAdmin_us_getLSHome(parent));
        if(!jparent){
            DLRL_Exception_THROW(&exception, DLRL_OUT_OF_MEMORY,
                "Not enough memory to complete operation.");
        }
    }

    DLRL_Exception_EXIT(&exception);
    if(parent){
        DK_ObjectHomeAdmin_unlockAdmin(parent);
        DK_Entity_ts_release((DK_Entity*)parent);
    }
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return jparent;
}

JNIEXPORT jobjectArray JNICALL
Java_DDS_ObjectHome_jniChildren(
    JNIEnv * env,
    jobject ls_home)
{
    jobjectArray jchildren = NULL;
    DK_ObjectHomeAdmin* home = NULL;
    DK_CacheAdmin* cache = NULL;
    Coll_Iter* iterator = NULL;
    Coll_Iter * childIterator = NULL;
    DLRL_Exception exception;
    Coll_Set* children;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "ObjectRoot->children()");

    DLRL_Exception_init(&exception);
    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);

    /* get and lock the cache, so we can retrieve the ordered homes list for
     * locking purposes.
     */
    cache = DK_ObjectHomeAdmin_ts_getCache(home, &exception);
    DLRL_Exception_PROPAGATE(&exception);
    if(cache){
        DK_CacheAdmin_lockAdministrative(cache);
        DK_CacheAdmin_us_checkAlive(cache, &exception);
        DLRL_Exception_PROPAGATE(&exception);
        DK_ObjectHomeAdmin_lockAdmin(home);
        assert(DK_ObjectHomeAdmin_us_isAlive(home));

        children =  DK_ObjectHomeAdmin_us_getChildren(home);
        assert(children);
        jchildren = (*env)->NewObjectArray(
            env,
            Coll_Set_getNrOfElements(children),
            cachedJNI.objectHome_class,
            NULL);
        if(jchildren){
            LOC_unsigned_long count = 0;
            childIterator = Coll_Set_getFirstElement(children);
            while(childIterator &&
                !DJA_ExceptionHandler_hasJavaExceptionOccurred(env))
            {
                DK_ObjectHomeAdmin* aChild =
                    (DK_ObjectHomeAdmin*)Coll_Iter_getObject(iterator);
                assert(DK_ObjectHomeAdmin_us_getRegistrationIndex(aChild) >
                       DK_ObjectHomeAdmin_us_getRegistrationIndex(home));
                DK_ObjectHomeAdmin_lockAdmin(aChild);
                assert(DK_ObjectHomeAdmin_us_isAlive(aChild));
                (*env)->SetObjectArrayElement(
                    env,
                    jchildren,
                    count,
                    (jobject)DK_ObjectHomeAdmin_us_getLSHome(aChild));
                DK_ObjectHomeAdmin_unlockAdmin(aChild);
                childIterator = Coll_Iter_getNext(childIterator);
                count++;
            }
        }
        DK_ObjectHomeAdmin_unlockAdmin(home);
        DLRL_JavaException_PROPAGATE(env, &exception);
    } else {
        /* make an empty array to return */
        jchildren = (*env)->NewObjectArray(
            env,
            0,
            cachedJNI.objectHome_class,
            NULL);
        DLRL_JavaException_PROPAGATE(env, &exception);
    }
    DLRL_Exception_EXIT(&exception);
    if(cache){
        DK_CacheAdmin_unlockAdministrative(cache);
        DK_Entity_ts_release((DK_Entity*)cache);
    }

    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return jchildren;
}

JNIEXPORT jint JNICALL
Java_DDS_ObjectHome_jniRegistrationIndex(
    JNIEnv * env,
    jobject ls_home)
{
    DK_ObjectHomeAdmin* home = NULL;
    jint regindex;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "ObjectRoot->registration_index()");

    DLRL_Exception_init(&exception);
    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);

    regindex = DK_ObjectHomeAdmin_ts_getRegistrationIndex(home, &exception);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return regindex;
}

JNIEXPORT jboolean JNICALL
Java_DDS_ObjectHome_jniAutoDeref(
    JNIEnv * env,
    jobject ls_home)
{
    DK_ObjectHomeAdmin* home = NULL;
    jboolean autoDeref;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "ObjectHome->auto_deref()");

    DLRL_Exception_init(&exception);
    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);

    autoDeref = (jboolean)DK_ObjectHomeAdmin_ts_getAutoDeref(home, &exception);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return autoDeref;
}

JNIEXPORT void JNICALL
Java_DDS_ObjectHome_jniSetFilter(
    JNIEnv * env,
    jobject ls_home,
    jstring jfilter)
{
    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "ObjectHome->set_filter()");
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT void JNICALL
Java_DDS_ObjectHome_jniSetAutoDeref(
    JNIEnv * env,
    jobject ls_home,
    jboolean autoDeref)
{
    DK_ObjectHomeAdmin* home = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "ObjectHome->set_auto_deref(value = %s)", autoDeref ?
        "TRUE" : "FALSE");

    DLRL_Exception_init(&exception);
    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);


    /* TODO ID: 134 DK_ObjectHomeAdmin_ts_setAutoDeref(
        home,
        &exception,
        (LOC_boolean)autoDeref);
     */

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT void JNICALL
Java_DDS_ObjectHome_jniDerefAll(
    JNIEnv * env,
    jobject ls_home)
{
    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "ObjectHome->deref_all()");
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT void JNICALL
Java_DDS_ObjectHome_jniUnderefAll(
    JNIEnv * env,
    jobject ls_home)
{
    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "ObjectHome->underef_all()");
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT jobject JNICALL
Java_DDS_ObjectHome_jniGetDataReader(
    JNIEnv * env,
    jobject ls_home,
    jstring jtopicName)
{
    jobject retVal = NULL;
    DK_ObjectHomeAdmin* home = NULL;
    DLRL_Exception exception;
    LOC_string topicName = NULL;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "ObjectRoot->get_datareader()");

    DLRL_Exception_init(&exception);

    DLRL_VERIFY_NOT_NULL(&exception, jtopicName, "topic_name");

    topicName = jtopicName ? (LOC_string)(*env)->GetStringUTFChars(
        env,
        jtopicName,
        0) : NULL;
    DLRL_JavaException_PROPAGATE(env, &exception);

    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);

    retVal = (jobject)DK_ObjectHomeAdmin_ts_getLSDataReader(
        home,
        &exception,
        (void*)env,
        topicName);

    DLRL_Exception_EXIT(&exception);
    if (topicName) {
        (*env)->ReleaseStringUTFChars(env, jtopicName, topicName);
    }

    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return retVal;
}

JNIEXPORT jobject JNICALL
Java_DDS_ObjectHome_jniGetDataWriter(
    JNIEnv * env,
    jobject ls_home,
    jstring jtopicName)
{
    jobject retVal = NULL;
    DK_ObjectHomeAdmin* home = NULL;
    DLRL_Exception exception;
    LOC_string topicName = NULL;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "ObjectRoot->get_datawriter()");

    DLRL_Exception_init(&exception);

    DLRL_VERIFY_NOT_NULL(&exception, jtopicName, "topic_name");

    topicName = jtopicName ? (LOC_string)(*env)->GetStringUTFChars(
        env,
        jtopicName,
        0) : NULL;
    DLRL_JavaException_PROPAGATE(env, &exception);

    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);

    retVal = (jobject)DK_ObjectHomeAdmin_ts_getLSDataWriter(
        home,
        &exception,
        (void*)env,
        topicName);

    DLRL_Exception_EXIT(&exception);
    if (topicName) {
        (*env)->ReleaseStringUTFChars(env, jtopicName, topicName);
    }

    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return retVal;
}

JNIEXPORT jobject JNICALL
Java_DDS_ObjectHome_jniGetTopic(
    JNIEnv * env,
    jobject ls_home,
    jstring jtopicName)
{
    jobject retVal = NULL;
    DK_ObjectHomeAdmin* home = NULL;
    DLRL_Exception exception;
    LOC_string topicName = NULL;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "ObjectRoot->get_topic()");

    DLRL_Exception_init(&exception);

    DLRL_VERIFY_NOT_NULL(&exception, jtopicName, "topic_name");

    topicName = jtopicName ? (LOC_string)(*env)->GetStringUTFChars(
        env,
        jtopicName,
        0) : NULL;
    DLRL_JavaException_PROPAGATE(env, &exception);

    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);

    retVal = (jobject)DK_ObjectHomeAdmin_ts_getLSTopic(
        home,
        &exception,
        (void*)env,
        topicName);

    DLRL_Exception_EXIT(&exception);
    if (topicName) {
        (*env)->ReleaseStringUTFChars(env, jtopicName, topicName);
    }

    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return retVal;
}


JNIEXPORT jstring JNICALL
Java_DDS_ObjectHome_jniGetTopicName(
    JNIEnv * env,
    jobject ls_home,
    jstring jattributeName)
{
    jstring jtopicName = NULL;
    LOC_string topicName = NULL;
    DK_ObjectHomeAdmin* home = NULL;
    DLRL_Exception exception;
    LOC_string attributeName = NULL;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "ObjectRoot->get_topic_name()");

    DLRL_Exception_init(&exception);

    attributeName = jattributeName ? (LOC_string)(*env)->GetStringUTFChars(
        env,
        jattributeName,
        0) : NULL;
    DLRL_JavaException_PROPAGATE(env, &exception);

    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);

    DK_ObjectHomeAdmin_lockAdmin(home);
    DK_ObjectHomeAdmin_us_checkAlive(home, &exception);
    DLRL_Exception_PROPAGATE(&exception);

    /*  Assert & verify parameters. */
    DLRL_VERIFY_NOT_NULL(&exception, jattributeName, "attribute_name");

    topicName = DK_ObjectHomeAdmin_us_getTopicName(
        home,
        &exception,
        attributeName);
    jtopicName = topicName ? (*env)->NewStringUTF(env, topicName) : NULL;
    DLRL_JavaException_PROPAGATE(env, &exception);

    DLRL_Exception_EXIT(&exception);
    if (attributeName) {
        (*env)->ReleaseStringUTFChars(env, jattributeName, attributeName);
    }
    if(home){
        DK_ObjectHomeAdmin_unlockAdmin(home);
    }

    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return jtopicName;
}

JNIEXPORT jobjectArray JNICALL
Java_DDS_ObjectHome_jniGetAllTopicNames(
    JNIEnv * env,
    jobject ls_home)
{
    jobjectArray jtopicNames = NULL;
    Coll_Set* topicNames;
    DLRL_Exception exception;
    DK_ObjectHomeAdmin* home = NULL;
    LOC_unsigned_long count = 0;
    LOC_string aTopicName = NULL;
    Coll_Iter* iterator;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "ObjectRoot->get_all_topic_names()");

    DLRL_Exception_init(&exception);
    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);

    DK_ObjectHomeAdmin_lockAdmin(home);
    DK_ObjectHomeAdmin_us_checkAlive(home, &exception);
    DLRL_Exception_PROPAGATE(&exception);


    topicNames = DK_ObjectHomeAdmin_us_getAllTopicNames(home, &exception);
    DLRL_Exception_PROPAGATE(&exception);
    assert(topicNames);

    jtopicNames =(*env)->NewObjectArray(
        env,
        Coll_Set_getNrOfElements(topicNames),
        cachedJNI.string_class,
        NULL);
    DLRL_JavaException_PROPAGATE(env, &exception);

    iterator = Coll_Set_getFirstElement(topicNames);
    while(iterator){
        /*  Even when an exception occured, the loop must be completed to empty
         * the returned collection.
         */
        if(!DJA_ExceptionHandler_hasJavaExceptionOccurred(env)){
            jstring aName;
            aTopicName = (LOC_string)Coll_Iter_getObject(iterator);
            /* delay propagating expection */
            aName = (*env)->NewStringUTF(env, aTopicName);
            if(aName){
                if(!DJA_ExceptionHandler_hasJavaExceptionOccurred(env)){
                    /* delay propagating expection */
                    (*env)->SetObjectArrayElement(
                        env,
                        jtopicNames,
                        count++,
                        aName);
                }
                (*env)->DeleteLocalRef(env, aName);
            }
        }
        iterator = Coll_Iter_getNext(iterator);
        Coll_Set_remove(topicNames, (void*)aTopicName);
    }
    Coll_Set_delete(topicNames);
    DLRL_JavaException_PROPAGATE(env, &exception);

    DLRL_Exception_EXIT(&exception);
    DK_ObjectHomeAdmin_unlockAdmin(home);

    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return jtopicNames;
}

JNIEXPORT jboolean JNICALL
Java_DDS_ObjectHome_jniAttachListener(
    JNIEnv * env,
    jobject ls_home,
    jobject jlistener,
    jboolean concernsContained)
{
    jobject jlistenerGlobalRef = NULL;
    jboolean succeeded = FALSE;
    DK_ObjectHomeAdmin* home = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "ObjectHome->attach_listener(listener)");

    DLRL_Exception_init(&exception);

    /*  Assert & verify parameters. */
    DLRL_VERIFY_NOT_NULL(&exception, jlistener, "listener");

    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);

    jlistenerGlobalRef = (*env)->NewGlobalRef (env, jlistener);
    if (!jlistenerGlobalRef) {
        DLRL_Exception_THROW(&exception, DLRL_OUT_OF_MEMORY,
            "Not enough memory to complete operation.");
    }

    succeeded = (jboolean)DK_ObjectHomeAdmin_ts_attachListener(
        home,
        &exception,
        (void*)env,
        (DLRL_LS_object)jlistenerGlobalRef,
        (LOC_boolean)concernsContained);
    if(!succeeded){
        (*env)->DeleteGlobalRef(env, jlistenerGlobalRef);
    }
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return succeeded;
}

JNIEXPORT jboolean JNICALL
Java_DDS_ObjectHome_jniDetachListener(
    JNIEnv * env,
    jobject ls_home,
    jobject jlistener)
{
    DK_ObjectHomeAdmin* home = NULL;
    jboolean succeeded = FALSE;
    DLRL_Exception exception;
    jobject globalRefListener;

    DLRL_INFO(INF_ENTER);
    DLRL_INFO(INF_API, "ObjectHome->detach_listener(listener)");

    DLRL_Exception_init(&exception);

    /*  Assert & verify parameters. */
    DLRL_VERIFY_NOT_NULL(&exception, jlistener, "listener");

    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);

    globalRefListener = (jobject)DK_ObjectHomeAdmin_ts_detachListener(
        home,
        &exception,
        (void*)env,
        (DLRL_LS_object)jlistener);
    if(globalRefListener){
        (*env)->DeleteGlobalRef(env, globalRefListener);
        succeeded = TRUE;
    }
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return succeeded;
}

JNIEXPORT jobjectArray JNICALL
Java_DDS_ObjectHome_jniListeners(
    JNIEnv * env,
    jobject ls_home)
{
    jobjectArray array = NULL;
    Coll_Set* listeners = NULL;
    Coll_Iter* iterator = NULL;
    DK_ObjectHomeAdmin* home = NULL;
    DJA_CachedJNITypedObject* typedObjectCachedData = NULL;
    LOC_unsigned_long count = 0;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);
    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);

    DK_ObjectHomeAdmin_lockUpdate(home);
    DK_ObjectHomeAdmin_us_checkAlive(home, &exception);
    DLRL_Exception_PROPAGATE(&exception);

    typedObjectCachedData =
        (DJA_CachedJNITypedObject*)DK_ObjectHomeAdmin_us_getUserData(home);
    assert(typedObjectCachedData);
    listeners = DK_ObjectHomeAdmin_us_getListeners(home);
    array = (*env)->NewObjectArray(
        env,
        Coll_Set_getNrOfElements(listeners),
        typedObjectCachedData->typedListener_class,
        NULL);
    DLRL_JavaException_PROPAGATE(env, &exception);
    /* start copying the data */
    iterator = Coll_Set_getFirstElement(listeners);
    while(iterator && !DJA_ExceptionHandler_hasJavaExceptionOccurred(env)){
        jobject aListener = (jobject)Coll_Iter_getObject(iterator);
        (*env)->SetObjectArrayElement(env, array, count, aListener);
        iterator = Coll_Iter_getNext(iterator);
        count++;
    }
    DLRL_JavaException_PROPAGATE(env, &exception);

    DLRL_Exception_EXIT(&exception);
    DK_ObjectHomeAdmin_unlockUpdate(home);

    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return array;
}

JNIEXPORT jobjectArray JNICALL
Java_DDS_ObjectHome_jniSelections(
    JNIEnv * env,
    jobject ls_home)
{
    jobjectArray array = NULL;
    Coll_Set* selections = NULL;
    Coll_Iter* iterator = NULL;
    DK_ObjectHomeAdmin* home = NULL;
    DJA_CachedJNITypedObject* typedObjectCachedData = NULL;
    LOC_unsigned_long count = 0;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);
    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);

    DK_ObjectHomeAdmin_lockAdmin(home);
    DK_ObjectHomeAdmin_us_checkAlive(home, &exception);
    DLRL_Exception_PROPAGATE((&exception));
    typedObjectCachedData =
        (DJA_CachedJNITypedObject*)DK_ObjectHomeAdmin_us_getUserData(home);
    assert(typedObjectCachedData);

    selections = DK_ObjectHomeAdmin_us_getSelections(home);
    array = (*env)->NewObjectArray(
        env,
        Coll_Set_getNrOfElements(selections),
        typedObjectCachedData->typedSelection_class,
        NULL);
    DLRL_JavaException_PROPAGATE(env, &exception);

    /* start copying the data */
    iterator = Coll_Set_getFirstElement(selections);
    while(iterator && !DJA_ExceptionHandler_hasJavaExceptionOccurred(env)){
        DK_SelectionAdmin* aSelection;
        aSelection = (DK_SelectionAdmin*)Coll_Iter_getObject(iterator);
        (*env)->SetObjectArrayElement(
            env,
            array,
            count,
            (jobject)DK_SelectionAdmin_us_getLSSelection(aSelection));
        iterator = Coll_Iter_getNext(iterator);
        count++;
    }
    DLRL_JavaException_PROPAGATE(env, &exception);

    DLRL_Exception_EXIT(&exception);
    DK_ObjectHomeAdmin_unlockAdmin(home);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return array;
}


JNIEXPORT void JNICALL
Java_DDS_ObjectHome_jniCreateSelection(
    JNIEnv * env,
    jobject ls_home,
    jobject ls_selection,
    jobject ls_criterion,
    jint jkind,
    jboolean jautoRefresh,
    jboolean jconcernsContainedObjects)
{
    DK_ObjectHomeAdmin* home = NULL;
    DLRL_Exception exception;
    DK_CriterionKind kind = (DK_CriterionKind)jkind;
    DK_SelectionCriterion criterion;
    DK_SelectionAdmin* selection = NULL;
    DK_QueryCriterion* queryCriterion;
    jobject global_selection = NULL;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);
    criterion.filterCriterion = NULL;
    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);

    if(kind == DK_CRITERION_KIND_FILTER){
        criterion.filterCriterion = (DLRL_LS_object)(*env)->NewGlobalRef (
            env,
            ls_criterion);
        /*  Ensure GlobalRef to ls_criterion is created (NULL pointer if not
         * enough memory).
         */
        if (!criterion.filterCriterion) {
            DLRL_Exception_THROW(&exception, DLRL_OUT_OF_MEMORY,
                "Not enough memory to complete operation.");
        }
    } else {
        assert(kind == DK_CRITERION_KIND_QUERY);
        queryCriterion = (DK_QueryCriterion*)(*env)->GetLongField(
            env,
            ls_criterion,
            cachedJNI.queryCriterion_admin_fid);
        assert(queryCriterion);
        criterion.queryCriterion = queryCriterion;
    }
    global_selection = (*env)->NewGlobalRef (env, ls_selection);
    /*  Ensure global_selection is created (NULL pointer if not enough memory)*/
    if (!global_selection) {
        DLRL_Exception_THROW(&exception, DLRL_OUT_OF_MEMORY,
            "Not enough memory to complete operation.");
    }

    selection = DK_ObjectHomeAdmin_ts_createSelection(
        home,
        &exception,
        (void*)env,
        global_selection,
        &criterion,
        kind,
        (LOC_boolean)jautoRefresh,
        (LOC_boolean)jconcernsContainedObjects);
    DLRL_Exception_PROPAGATE(&exception);

    /* selection ref count is already increased so we dont have to duplicate
     * when setting the selection to the java object
     */
    (*env)->SetLongField(
        env,
        ls_selection,
        cachedJNI.selection_admin_fid,
        (jlong)selection);

    DLRL_Exception_EXIT(&exception);
    if(exception.exceptionID != DLRL_NO_EXCEPTION){
        /* lets clear-a-roo the global refs */
        if(kind == DK_CRITERION_KIND_FILTER && criterion.filterCriterion){
            (*env)->DeleteGlobalRef (env, criterion.filterCriterion);
        }
        if (global_selection) {
            (*env)->DeleteGlobalRef (env, global_selection);
        }
    }
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT void JNICALL
Java_DDS_ObjectHome_jniDeleteSelection(
    JNIEnv * env,
    jobject ls_home,
    jobject ls_selection)
{
    DK_ObjectHomeAdmin* home = NULL;
    DK_SelectionAdmin* selection = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    /*  Assert & verify parameters. */
    DLRL_VERIFY_NOT_NULL(&exception, ls_selection, "a_selection");

    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);

    selection = (DK_SelectionAdmin*)(*env)->GetLongField(
        env,
        ls_selection,
        cachedJNI.selection_admin_fid);
    if (!selection) {
        DLRL_Exception_THROW(&exception, DLRL_BAD_PARAMETER,
            "a_selection parameter is corrupted.");
    }

    DK_ObjectHomeAdmin_ts_deleteSelection(
        home,
        &exception,
        (void*)env,
        selection);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT jobjectArray JNICALL
Java_DDS_ObjectHome_jniDeletedObjects(
    JNIEnv * env,
    jobject ls_home,
    jobject ls_cacheBase,
    jint jkind)
{
    jobjectArray array = NULL;
    Coll_List* deletedObjects = NULL;
    Coll_Iter* iterator = NULL;
    DK_ObjectHomeAdmin* home = NULL;
    DJA_CachedJNITypedObject* typedObjectCachedData = NULL;
    LOC_unsigned_long count = 0;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    DLRL_VERIFY_NOT_NULL(&exception, ls_cacheBase, "source");
    assert(jkind >= 0);
    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);

    if(((DK_CacheKind)jkind) == DK_CACHE_KIND_CACHE){

        DK_ObjectHomeAdmin_lockUpdate(home);
        DK_ObjectHomeAdmin_us_checkAlive(home, &exception);
        DLRL_Exception_PROPAGATE(&exception);

        typedObjectCachedData =
            (DJA_CachedJNITypedObject*)DK_ObjectHomeAdmin_us_getUserData(home);
        assert(typedObjectCachedData);

        /* may return null */
        deletedObjects = DK_ObjectHomeAdmin_us_getDeletedObjects(home);
        if(deletedObjects){
            array = (*env)->NewObjectArray(
                env,
                Coll_List_getNrOfElements(deletedObjects),
                typedObjectCachedData->typedRoot_class,
                NULL);
            DLRL_JavaException_PROPAGATE(env, &exception);
            /* start copying the data */
            iterator = Coll_List_getFirstElement(deletedObjects);
            while(iterator &&
                !DJA_ExceptionHandler_hasJavaExceptionOccurred(env))
            {
                jobject anObject = (jobject)DK_ObjectAdmin_us_getLSObject(
                    (DK_ObjectAdmin*)Coll_Iter_getObject(iterator));
                (*env)->SetObjectArrayElement(env, array, count, anObject);
                DLRL_JavaException_PROPAGATE(env, &exception);
                iterator = Coll_Iter_getNext(iterator);
                count++;
            }
        } else {/* create empty array */
            array = (*env)->NewObjectArray(
                env,
                0,
                typedObjectCachedData->typedRoot_class,
                NULL);
            DLRL_JavaException_PROPAGATE(env, &exception);
        }
    } else {
        assert(((DK_CacheKind)jkind) == DK_CACHE_KIND_CACHE_ACCESS);
        DK_ObjectHomeAdmin_lockAdmin(home);
        DK_ObjectHomeAdmin_us_checkAlive(home, &exception);
        DLRL_Exception_PROPAGATE(&exception);

        typedObjectCachedData =
            (DJA_CachedJNITypedObject*)DK_ObjectHomeAdmin_us_getUserData(home);
        assert(typedObjectCachedData);

        array = (*env)->NewObjectArray(
            env,
            0,
            typedObjectCachedData->typedRoot_class,
            NULL);
        DLRL_JavaException_PROPAGATE(env, &exception);
    }

    DLRL_Exception_EXIT(&exception);
    if(home){
        if(((DK_CacheKind)jkind) == DK_CACHE_KIND_CACHE){
            DK_ObjectHomeAdmin_unlockUpdate(home);
        } else {
            DK_ObjectHomeAdmin_unlockAdmin(home);
        }
    }

    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return array;
}

JNIEXPORT jobjectArray JNICALL
Java_DDS_ObjectHome_jniModifiedObjects(
    JNIEnv * env,
    jobject ls_home,
    jobject ls_cacheBase,
    jint jkind)
{
    jobjectArray array = NULL;
    DK_ObjectHomeAdmin* home = NULL;
    Coll_List* modifiedObjects = NULL;
    Coll_Iter* iterator = NULL;
    DJA_CachedJNITypedObject* typedObjectCachedData = NULL;
    LOC_unsigned_long count = 0;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    DLRL_VERIFY_NOT_NULL(&exception, ls_cacheBase, "source");
    assert(jkind >= 0);
    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);

    if(((DK_CacheKind)jkind) == DK_CACHE_KIND_CACHE){
        DK_ObjectHomeAdmin_lockUpdate(home);
        DK_ObjectHomeAdmin_us_checkAlive(home, &exception);
        DLRL_Exception_PROPAGATE(&exception);

        typedObjectCachedData =
            (DJA_CachedJNITypedObject*)DK_ObjectHomeAdmin_us_getUserData(home);
        assert(typedObjectCachedData);

        /* may return null */
        modifiedObjects = DK_ObjectHomeAdmin_us_getModifiedObjects(home);
        if(modifiedObjects){
            array = (*env)->NewObjectArray(
                env,
                Coll_List_getNrOfElements(modifiedObjects),
                typedObjectCachedData->typedRoot_class,
                NULL);
            DLRL_JavaException_PROPAGATE(env, &exception);
            /* start copying the data */
            iterator = Coll_List_getFirstElement(modifiedObjects);
            while(iterator &&
                !DJA_ExceptionHandler_hasJavaExceptionOccurred(env))
            {
                jobject anObject = (jobject)DK_ObjectAdmin_us_getLSObject(
                    (DK_ObjectAdmin*)Coll_Iter_getObject(iterator));
                (*env)->SetObjectArrayElement(env, array, count, anObject);
                DLRL_JavaException_PROPAGATE(env, &exception);
                iterator = Coll_Iter_getNext(iterator);
                count++;
            }
        } else {/* create empty array */
            array = (*env)->NewObjectArray(
                env,
                0, typedObjectCachedData->typedRoot_class,
                NULL);
            DLRL_JavaException_PROPAGATE(env, &exception);
        }
    } else {
        assert(((DK_CacheKind)jkind) == DK_CACHE_KIND_CACHE_ACCESS);
        DK_ObjectHomeAdmin_lockAdmin(home);
        DK_ObjectHomeAdmin_us_checkAlive(home, &exception);
        DLRL_Exception_PROPAGATE(&exception);

        typedObjectCachedData =
            (DJA_CachedJNITypedObject*)DK_ObjectHomeAdmin_us_getUserData(home);
        assert(typedObjectCachedData);

        array = (*env)->NewObjectArray(
            env,
            0,
            typedObjectCachedData->typedRoot_class,
            NULL);
        DLRL_JavaException_PROPAGATE(env, &exception);
    }
    DLRL_Exception_EXIT(&exception);
    if(home){
        if(((DK_CacheKind)jkind) == DK_CACHE_KIND_CACHE){
            DK_ObjectHomeAdmin_unlockUpdate(home);
        } else {
            DK_ObjectHomeAdmin_unlockAdmin(home);
        }
    }

    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return array;
}

JNIEXPORT jobjectArray JNICALL
Java_DDS_ObjectHome_jniNewObjects(
    JNIEnv * env,
    jobject ls_home,
    jobject ls_cacheBase,
    jint jkind)
{
    jobjectArray array = NULL;
    DK_ObjectHomeAdmin* home = NULL;
    Coll_List* newObjects = NULL;
    Coll_Iter* iterator = NULL;
    DJA_CachedJNITypedObject* typedObjectCachedData = NULL;
    LOC_unsigned_long count = 0;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);
    DLRL_VERIFY_NOT_NULL(&exception, ls_cacheBase, "source");
    assert(jkind >= 0);
    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);
    if(((DK_CacheKind)jkind) == DK_CACHE_KIND_CACHE){
        DK_ObjectHomeAdmin_lockUpdate(home);
        DK_ObjectHomeAdmin_us_checkAlive(home, &exception);
        DLRL_Exception_PROPAGATE(&exception);

        typedObjectCachedData =
            (DJA_CachedJNITypedObject*)DK_ObjectHomeAdmin_us_getUserData(home);
        assert(typedObjectCachedData);

        /* may return null */
        newObjects = DK_ObjectHomeAdmin_us_getNewObjects(home);
        if(newObjects){
            array = (*env)->NewObjectArray(
                env,
                Coll_List_getNrOfElements(newObjects),
                typedObjectCachedData->typedRoot_class,
                NULL);
            DLRL_JavaException_PROPAGATE(env, &exception);
            /* start copying the data */
            iterator = Coll_List_getFirstElement(newObjects);
            while(iterator &&
                !DJA_ExceptionHandler_hasJavaExceptionOccurred(env))
            {
                jobject anObject = (jobject)DK_ObjectAdmin_us_getLSObject(
                    (DK_ObjectAdmin*)Coll_Iter_getObject(iterator));
                (*env)->SetObjectArrayElement(env, array, count, anObject);
                DLRL_JavaException_PROPAGATE(env, &exception);
                iterator = Coll_Iter_getNext(iterator);
                count++;
            }
        } else {/* create empty array */
            array = (*env)->NewObjectArray(
                env,
                0,
                typedObjectCachedData->typedRoot_class,
                NULL);
            DLRL_JavaException_PROPAGATE(env, &exception);
        }
    } else {
        assert(((DK_CacheKind)jkind) == DK_CACHE_KIND_CACHE_ACCESS);
        DK_ObjectHomeAdmin_lockAdmin(home);
        DK_ObjectHomeAdmin_us_checkAlive(home, &exception);
        DLRL_Exception_PROPAGATE(&exception);

        typedObjectCachedData =
            (DJA_CachedJNITypedObject*)DK_ObjectHomeAdmin_us_getUserData(home);
        assert(typedObjectCachedData);

        array = (*env)->NewObjectArray(
            env,
            0,
            typedObjectCachedData->typedRoot_class,
            NULL);
        DLRL_JavaException_PROPAGATE(env, &exception);
    }
    DLRL_Exception_EXIT(&exception);
    if(home){
        if(((DK_CacheKind)jkind) == DK_CACHE_KIND_CACHE){
            DK_ObjectHomeAdmin_unlockUpdate(home);
        } else {
            DK_ObjectHomeAdmin_unlockAdmin(home);
        }
    }

    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return array;
}

JNIEXPORT jobjectArray JNICALL
Java_DDS_ObjectHome_jniObjects(
    JNIEnv * env,
    jobject ls_home,
    jobject ls_cacheBase,
    jint jkind)
{
    jobjectArray jobjects = NULL;
    DK_ObjectHomeAdmin* home = NULL;
    DK_ObjectArrayHolder holder;
    DK_ObjectAdmin* anObjectAdmin = NULL;
    jobject janObjectAdmin=NULL;
    DJA_CachedJNITypedObject* typedObjectCachedData = NULL;
    DLRL_Exception exception;
    LOC_unsigned_long count = 0;
    DK_CacheAccessAdmin* access = NULL;

    holder.objectArray = NULL;
    holder.size = 0;
    holder.maxSize = 0;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    DLRL_VERIFY_NOT_NULL(&exception, ls_cacheBase, "source");
    assert(jkind >= 0);
    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);
    if(((DK_CacheKind)jkind) == DK_CACHE_KIND_CACHE)
    {
        DK_ObjectHomeAdmin_lockUpdate(home);
        DK_ObjectHomeAdmin_us_checkAlive(home, &exception);
        DLRL_Exception_PROPAGATE(&exception);

        DK_ObjectHomeAdmin_us_getAllObjects(home, &exception, &holder);
        DLRL_Exception_PROPAGATE(&exception);

        typedObjectCachedData =
            (DJA_CachedJNITypedObject*)DK_ObjectHomeAdmin_us_getUserData(home);
        assert(typedObjectCachedData);
        jobjects = (*env)->NewObjectArray(
            env,
            holder.size,
            typedObjectCachedData->typedRoot_class,
            NULL);
        DLRL_JavaException_PROPAGATE(env, &exception);
        for(count = 0; count < holder.size; count++)
        {
            anObjectAdmin = (DK_ObjectAdmin*)holder.objectArray[count];

            janObjectAdmin =
                (jobject)DK_ObjectAdmin_us_getLSObject(anObjectAdmin);
            assert(janObjectAdmin);
            (*env)->SetObjectArrayElement(env, jobjects, count, janObjectAdmin);
            DLRL_JavaException_PROPAGATE(env, &exception);
        }
    } else {
        assert(((DK_CacheKind)jkind) == DK_CACHE_KIND_CACHE_ACCESS);
        access = (DK_CacheAccessAdmin*)(*env)->GetLongField(
            env,
            ls_cacheBase,
            cachedJNI.cacheAccess_admin_fid);
        assert(access);
        DK_CacheAccessAdmin_ts_visitAllObjectsForHome(
            access,
            &exception,
            (void*)env,
            home,
            DJA_ObjectHome_us_visitObjectAction,
            (void**)&jobjects);
        DLRL_Exception_PROPAGATE(&exception);
        if(!jobjects)
        {
            typedObjectCachedData =
                (DJA_CachedJNITypedObject*)DK_ObjectHomeAdmin_us_getUserData(
                    home);
            assert(typedObjectCachedData);
            jobjects = (*env)->NewObjectArray(
                env,
                0,
                typedObjectCachedData->typedRoot_class,
                NULL);
            DLRL_JavaException_PROPAGATE(env, &exception);
        }

    }
    DLRL_Exception_EXIT(&exception);

    if(holder.objectArray){
        os_free(holder.objectArray);
    }
    if(home){
        if(((DK_CacheKind)jkind) == DK_CACHE_KIND_CACHE){
            DK_ObjectHomeAdmin_unlockUpdate(home);
        }
    }
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return jobjects;
}

void
DJA_ObjectHome_us_visitObjectAction(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectAdmin* object,
    LOC_unsigned_long totalNrOfObjects,
    LOC_unsigned_long index,
    void** arg)
{
    JNIEnv* env = (JNIEnv*)userData;
    DJA_CachedJNITypedObject* typedObjectCachedData = NULL;
    DK_ObjectHomeAdmin* home = NULL;

    DLRL_INFO(INF_ENTER);
    assert(env);
    assert(exception);
    assert(object);
    /* arg may be NULL */

    /* if the arg is still null we have to allocate a new object array for each
     * element.
     */
    if(!(*arg)){
        home = DK_ObjectAdmin_us_getHome(object);
        typedObjectCachedData =
            (DJA_CachedJNITypedObject*)DK_ObjectHomeAdmin_us_getUserData(home);
        assert(typedObjectCachedData);
        *arg = (void*)(*env)->NewObjectArray(
            env,
            totalNrOfObjects,
            typedObjectCachedData->typedRoot_class,
            NULL);
        DLRL_JavaException_PROPAGATE(env, exception);
    }
    assert(DK_ObjectAdmin_us_getLSObject(object));
    (*env)->SetObjectArrayElement(
        env,
        (jobjectArray)*arg,
        index,
        (jobject)DK_ObjectAdmin_us_getLSObject(object));
    DLRL_JavaException_PROPAGATE(env, exception);

    DLRL_Exception_EXIT(exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT void JNICALL
Java_DDS_ObjectHome_jniRegisterObject(
    JNIEnv * env,
    jobject ls_home,
    jobject ls_objectAdmin)
{
    DK_ObjectHomeAdmin* home = NULL;
    DK_ObjectAdmin* objectAdmin = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    /*  Assert & verify parameters. */
    DLRL_VERIFY_NOT_NULL(&exception, ls_objectAdmin, "unregistered_object");

    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);
    objectAdmin = (DK_ObjectAdmin*)(*env)->GetLongField(
        env,
        ls_objectAdmin,
        cachedJNI.objectRoot_admin_fid);
    if (!objectAdmin) {
        DLRL_Exception_THROW(&exception, DLRL_BAD_PARAMETER,
            "unregistered_object parameter is corrupted.");
    }

    DK_ObjectHomeAdmin_ts_registerObject(
        home,
        &exception,
        (void*)env,
        objectAdmin);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT jobject JNICALL
Java_DDS_ObjectHome_jniCreateUnregisteredObject(
    JNIEnv * env,
    jobject ls_home,
    jobject ls_access)
{
    DK_ObjectHomeAdmin* home = NULL;
    DK_CacheAccessAdmin* access = NULL;
    DLRL_Exception exception;
    jobject jobjectAdmin = NULL;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    /*  Assert & verify parameters. */
    DLRL_VERIFY_NOT_NULL(&exception, ls_access, "access");

    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);
    access = (DK_CacheAccessAdmin*)(*env)->GetLongField(
        env,
        ls_access,
        cachedJNI.cacheAccess_admin_fid);
    if (!access) {
        DLRL_Exception_THROW(&exception, DLRL_BAD_PARAMETER,
            "access parameter is corrupted.");
    }

    jobjectAdmin = DK_ObjectHomeAdmin_ts_createUnregisteredObject(
        home,
        &exception,
        (void*)env,
        access);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return jobjectAdmin;
}

JNIEXPORT jobject JNICALL
Java_DDS_ObjectHome_jniCreateObject(
    JNIEnv * env,
    jobject ls_home,
    jobject ls_access)
{
    DK_ObjectHomeAdmin* home = NULL;
    DK_CacheAccessAdmin* access = NULL;
    DLRL_Exception exception;
    jobject jobjectAdmin = NULL;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    /*  Assert & verify parameters. */
    DLRL_VERIFY_NOT_NULL(&exception, ls_access, "access");

    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);
    access = (DK_CacheAccessAdmin*)(*env)->GetLongField(
        env,
        ls_access,
        cachedJNI.cacheAccess_admin_fid);
    if (!access) {
        DLRL_Exception_THROW(&exception, DLRL_BAD_PARAMETER,
            "access parameter is corrupted.");
    }

    jobjectAdmin = DK_ObjectHomeAdmin_ts_createLSObject(
        home,
        &exception,
        (void*)env,
        access);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
    return jobjectAdmin;
}


JNIEXPORT void JNICALL
Java_DDS_ObjectHome_jniCreateDLRLClass(
    JNIEnv * env,
    jobject ls_home,
    jstring jparentName,
    jint jmapping)
{
    DK_ObjectHomeAdmin* home = NULL;
    DLRL_Exception exception;
    LOC_string parentName = NULL;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    parentName = jparentName ? (LOC_string)(*env)->GetStringUTFChars(
        env,
        jparentName,
        0) : NULL;
    DLRL_JavaException_PROPAGATE(env, &exception);

    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);

    DK_MMFacade_us_createDLRLClass(
        home,
        &exception,
        parentName,
        (DMM_Mapping)jmapping);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);

    if(parentName){
        (*env)->ReleaseStringUTFChars(env, jparentName, parentName);
    }
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT void JNICALL
Java_DDS_ObjectHome_jniCreateMainTopic(
    JNIEnv * env,
    jobject ls_home,
    jstring jname,
    jstring jtypeName)
{
    DK_ObjectHomeAdmin* home = NULL;
    DLRL_Exception exception;
    LOC_string name= NULL;
    LOC_string typeName= NULL;

    DLRL_INFO(INF_ENTER);
    /*  Assert & verify parameters. */
    assert(jname);
    assert(jtypeName);

    DLRL_Exception_init(&exception);

    name = (LOC_string)(*env)->GetStringUTFChars(env, jname, 0);
    DLRL_JavaException_PROPAGATE(env, &exception);
    typeName = (LOC_string)(*env)->GetStringUTFChars(env, jtypeName, 0);
    DLRL_JavaException_PROPAGATE(env, &exception);

    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);

    DK_MMFacade_us_createMainTopic(home, &exception, name, typeName);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);

    if(name){
        (*env)->ReleaseStringUTFChars(env, jname, name);
    }
    if(typeName){
        (*env)->ReleaseStringUTFChars(env, jtypeName, typeName);
    }
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT void JNICALL
Java_DDS_ObjectHome_jniCreateExtensionTopic(
    JNIEnv * env,
    jobject ls_home,
    jstring jname,
    jstring jtypeName)
{
    DK_ObjectHomeAdmin* home = NULL;
    DLRL_Exception exception;
    LOC_string name= NULL;
    LOC_string typeName= NULL;

    DLRL_INFO(INF_ENTER);
    /*  Assert & verify parameters. */
    assert(jname);
    assert(jtypeName);

    DLRL_Exception_init(&exception);

    name = (LOC_string)(*env)->GetStringUTFChars(env, jname, 0);
    DLRL_JavaException_PROPAGATE(env, &exception);\
    typeName = (LOC_string)(*env)->GetStringUTFChars(env, jtypeName, 0);
    DLRL_JavaException_PROPAGATE(env, &exception);

    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);

    DK_MMFacade_us_createExtensionTopic(home, &exception, name, typeName);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    if(name){
        (*env)->ReleaseStringUTFChars(env, jname, name);
    }
    if(typeName){
        (*env)->ReleaseStringUTFChars(env, jtypeName, typeName);
    }
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT void JNICALL
Java_DDS_ObjectHome_jniCreatePlaceTopic(
    JNIEnv * env,
    jobject ls_home,
    jstring jname,
    jstring jtypeName)
{
    DK_ObjectHomeAdmin* home = NULL;
    DLRL_Exception exception;
    LOC_string typeName  = NULL;
    LOC_string name = NULL;

    DLRL_INFO(INF_ENTER);
    /*  Assert & verify parameters. */
    assert(jname);
    assert(jtypeName);

    DLRL_Exception_init(&exception);

    name = (LOC_string)(*env)->GetStringUTFChars(env, jname, 0);
    DLRL_JavaException_PROPAGATE(env, &exception);
    typeName = (LOC_string)(*env)->GetStringUTFChars(env, jtypeName, 0);
    DLRL_JavaException_PROPAGATE(env, &exception);

    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);

    DK_MMFacade_us_createPlaceTopic(home, &exception, name, typeName);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    if(name){
        (*env)->ReleaseStringUTFChars(env, jname, name);
    }
    if(typeName){
        (*env)->ReleaseStringUTFChars(env, jtypeName, typeName);
    }
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT void JNICALL
Java_DDS_ObjectHome_jniCreateMultiPlaceTopic(
    JNIEnv * env,
    jobject ls_home,
    jstring jname,
    jstring jtypeName)
{
    DK_ObjectHomeAdmin* home = NULL;
    DLRL_Exception exception;
    LOC_string typeName = NULL;
    LOC_string name = NULL;

    DLRL_INFO(INF_ENTER);
    /*  Assert & verify parameters. */
    assert(jname);
    assert(jtypeName);

    DLRL_Exception_init(&exception);

    name = (LOC_string)(*env)->GetStringUTFChars(env, jname, 0);
    DLRL_JavaException_PROPAGATE(env, &exception);
    typeName = (LOC_string)(*env)->GetStringUTFChars(env, jtypeName, 0);
    DLRL_JavaException_PROPAGATE(env, &exception);

    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);

    DK_MMFacade_us_createMultiPlaceTopic(home, &exception, name, typeName);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    if(name){
        (*env)->ReleaseStringUTFChars(env, jname, name);
    }
    if(typeName){
        (*env)->ReleaseStringUTFChars(env, jtypeName, typeName);
    }
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT void JNICALL
Java_DDS_ObjectHome_jniCreateDCPSField(
    JNIEnv * env,
    jobject ls_home,
    jstring jname,
    jint jfieldType,
    jint jtype,
    jstring jowningTopic)
{
    DK_ObjectHomeAdmin* home = NULL;
    DLRL_Exception exception;
    LOC_string name= NULL;
    LOC_string owningTopicName= NULL;

    DLRL_INFO(INF_ENTER);
    /*  Assert & verify parameters. */
    assert(jname);
    assert(jowningTopic);

    DLRL_Exception_init(&exception);

    name = (LOC_string)(*env)->GetStringUTFChars(env, jname, 0);
    DLRL_JavaException_PROPAGATE(env, &exception);
    owningTopicName = (LOC_string)(*env)->GetStringUTFChars(
        env,
        jowningTopic,
        0);
    DLRL_JavaException_PROPAGATE(env, &exception);

    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);

    DK_MMFacade_us_createDCPSField(
        home,
        &exception,
        name,
        (DMM_KeyType)jfieldType,
        (DMM_AttributeType)jtype,
        owningTopicName);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    if(owningTopicName){
        (*env)->ReleaseStringUTFChars(env, jowningTopic, owningTopicName);
    }
    if(name){
        (*env)->ReleaseStringUTFChars(env, jname, name);
    }
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT void JNICALL
Java_DDS_ObjectHome_jniCreateRelation(
    JNIEnv * env,
    jobject ls_home,
    jboolean jisComposition,
    jstring jname,
    jstring jtypeName,
    jstring jassociatedRelationName,
    jboolean jisOptional)
{
    DK_ObjectHomeAdmin* home = NULL;
    DLRL_Exception exception;
    LOC_string name= NULL;
    LOC_string typeName= NULL;
    LOC_string associatedRelationName= NULL;

    DLRL_INFO(INF_ENTER);
    /*  Assert & verify parameters. */
    assert(jname);
    assert(jtypeName);

    DLRL_Exception_init(&exception);

    name = (LOC_string)(*env)->GetStringUTFChars(env, jname, 0);
    DLRL_JavaException_PROPAGATE(env, &exception);
    typeName = (LOC_string)(*env)->GetStringUTFChars(env, jtypeName, 0);
    DLRL_JavaException_PROPAGATE(env, &exception);
    associatedRelationName = jassociatedRelationName ?
        (LOC_string)(*env)->GetStringUTFChars(
            env,
            jassociatedRelationName,
            0) : NULL;
    DLRL_JavaException_PROPAGATE(env, &exception);

    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);

    DK_MMFacade_us_createRelation(
        home,
        &exception,
        (LOC_boolean)jisComposition,
        name,
        typeName,
        associatedRelationName,
        (LOC_boolean)jisOptional);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    if(associatedRelationName){
        (*env)->ReleaseStringUTFChars(
            env,
            jassociatedRelationName,
            associatedRelationName);
    }
    if(typeName){
        (*env)->ReleaseStringUTFChars(
            env,
            jtypeName,
            typeName);
    }
    if(name){
        (*env)->ReleaseStringUTFChars(
            env,
            jname,
            name);
    }
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT void JNICALL
Java_DDS_ObjectHome_jniCreateMultiRelation(
    JNIEnv * env,
    jobject ls_home,
    jboolean jisComposition,
    jstring jname,
    jstring jtypeName,
    jstring jassociatedRelationName,
    jint jbasis)
{
    DK_ObjectHomeAdmin* home = NULL;
    DLRL_Exception exception;
    LOC_string name = NULL;
    LOC_string typeName = NULL;
    LOC_string associatedRelationName = NULL;

    DLRL_INFO(INF_ENTER);
    /*  Assert & verify parameters. */
    assert(jname);
    assert(jtypeName);

    DLRL_Exception_init(&exception);

    name = (LOC_string)(*env)->GetStringUTFChars(env, jname, 0);
    DLRL_JavaException_PROPAGATE(env, &exception);
    typeName = (LOC_string)(*env)->GetStringUTFChars(env, jtypeName, 0);
    DLRL_JavaException_PROPAGATE(env, &exception);
    associatedRelationName = jassociatedRelationName ?
        (LOC_string)(*env)->GetStringUTFChars(
            env,
            jassociatedRelationName,
            0) : NULL;
    DLRL_JavaException_PROPAGATE(env, &exception);

    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);

    DK_MMFacade_us_createMultiRelation(
        home,
        &exception,
        (LOC_boolean)jisComposition,
        name,
        typeName,
        associatedRelationName,
        (DMM_Basis)jbasis);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    if(associatedRelationName){
        (*env)->ReleaseStringUTFChars(
            env,
            jassociatedRelationName,
            associatedRelationName);
    }
    if(typeName){
        (*env)->ReleaseStringUTFChars(
            env,
            jtypeName,
            typeName);
    }
    if(name){
        (*env)->ReleaseStringUTFChars(
            env,
            jname,
            name);
    }

    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT void JNICALL
Java_DDS_ObjectHome_jniCreateAttribute(
    JNIEnv * env,
    jobject ls_home,
    jstring jname,
    jboolean jisImmutable,
    jint jtype)
{
    DK_ObjectHomeAdmin* home = NULL;
    DLRL_Exception exception;
    LOC_string name = NULL;

    DLRL_INFO(INF_ENTER);
    /*  Assert & verify parameters. */
    assert(jname);

    DLRL_Exception_init(&exception);

    name = (LOC_string)(*env)->GetStringUTFChars(env, jname, 0);
    DLRL_JavaException_PROPAGATE(env, &exception);

    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);

    DK_MMFacade_us_createAttribute(
        home,
        &exception,
        name,
        (LOC_boolean)jisImmutable,
        (DMM_AttributeType)jtype);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    if(name){
        (*env)->ReleaseStringUTFChars(env, jname, name);
    }
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT void JNICALL
Java_DDS_ObjectHome_jniCreateMultiAttribute(
    JNIEnv * env,
    jobject ls_home,
    jstring jname,
    jboolean jisImmutable,
    jint jtype,
    jint jbasis)
{
    DK_ObjectHomeAdmin* home = NULL;
    DLRL_Exception exception;
    LOC_string name = NULL;

    DLRL_INFO(INF_ENTER);
    /*  Assert & verify parameters. */
    assert(jname);

    DLRL_Exception_init(&exception);

    name = (LOC_string)(*env)->GetStringUTFChars(env, jname, 0);
    DLRL_JavaException_PROPAGATE(env, &exception);

    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);

    DK_MMFacade_us_createMultiAttribute(
        home,
        &exception,
        name,
        (LOC_boolean)jisImmutable,
        (DMM_AttributeType)jtype,
        (DMM_Basis)jbasis);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    if(name){
        (*env)->ReleaseStringUTFChars(env, jname, name);
    }
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT void JNICALL
Java_DDS_ObjectHome_jniMapAttributeToDCPSField(
    JNIEnv * env,
    jobject ls_home,
    jstring jattributeName,
    jstring jdcpsFieldName)
{
    DK_ObjectHomeAdmin* home = NULL;
    DLRL_Exception exception;
    LOC_string dcpsFieldName= NULL;
    LOC_string attributeName= NULL;

    DLRL_INFO(INF_ENTER);
    /*  Assert & verify parameters. */
    assert(jattributeName);
    assert(jdcpsFieldName);

    DLRL_Exception_init(&exception);

    attributeName = (LOC_string)(*env)->GetStringUTFChars(
        env,
        jattributeName,
        0);
    DLRL_JavaException_PROPAGATE(env, &exception);
    dcpsFieldName = (LOC_string)(*env)->GetStringUTFChars(
        env,
        jdcpsFieldName,
        0);
    DLRL_JavaException_PROPAGATE(env, &exception);

    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);

    DK_MMFacade_us_mapDLRLAttributeToDCPSField(
        home,
        &exception,
        attributeName,
        dcpsFieldName);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    if(dcpsFieldName){
        (*env)->ReleaseStringUTFChars(env, jdcpsFieldName, dcpsFieldName);
    }
    if(attributeName){
        (*env)->ReleaseStringUTFChars(env, jattributeName, attributeName);
    }
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT void JNICALL
Java_DDS_ObjectHome_jniMapMultiAttributeToIndexDCPSField(
    JNIEnv * env,
    jobject ls_home,
    jstring jattributeName,
    jstring jindexFieldName)
{
    DK_ObjectHomeAdmin* home = NULL;
    DLRL_Exception exception;
    LOC_string attributeName = NULL;
    LOC_string indexFieldName = NULL;

    DLRL_INFO(INF_ENTER);
    /*  Assert & verify parameters. */
    assert(jattributeName);
    assert(jindexFieldName);

    DLRL_Exception_init(&exception);

    attributeName = (LOC_string)(*env)->GetStringUTFChars(
        env,
        jattributeName,
        0);
    DLRL_JavaException_PROPAGATE(env, &exception);
    indexFieldName =  (LOC_string)(*env)->GetStringUTFChars(
        env,
        jindexFieldName,
        0);
    DLRL_JavaException_PROPAGATE(env, &exception);

    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);

    DK_MMFacade_us_mapDLRLMultiAttributeToIndexDCPSField(
        home,
        &exception,
        attributeName,
        indexFieldName);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    if(indexFieldName){
        (*env)->ReleaseStringUTFChars(
            env,
            jindexFieldName,
            indexFieldName);
    }
    if(attributeName){
        (*env)->ReleaseStringUTFChars(
            env,
            jattributeName,
            attributeName);
    }
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT void JNICALL
Java_DDS_ObjectHome_jniMapAttributeToDCPSTopic(
    JNIEnv * env,
    jobject ls_home,
    jstring jattributeName,
    jstring jtopicName)
{
    DK_ObjectHomeAdmin* home = NULL;
    DLRL_Exception exception;
    LOC_string topicName = NULL;
    LOC_string attributeName = NULL;

    DLRL_INFO(INF_ENTER);
    /*  Assert & verify parameters. */
    assert(jattributeName);
    assert(jtopicName);

    DLRL_Exception_init(&exception);

    attributeName = (LOC_string)(*env)->GetStringUTFChars(
        env,
        jattributeName,
        0);
    DLRL_JavaException_PROPAGATE(env, &exception);
    topicName = (LOC_string)(*env)->GetStringUTFChars(
        env,
        jtopicName,
        0) ;
    DLRL_JavaException_PROPAGATE(env, &exception);

    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);

    DK_MMFacade_us_mapDLRLAttributeToDCPSTopic(
        home,
        &exception,
        attributeName,
        topicName);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    if(topicName){
        (*env)->ReleaseStringUTFChars(
            env,
            jtopicName,
            topicName);
    }
    if(attributeName){
        (*env)->ReleaseStringUTFChars(
            env,
            jattributeName,
            attributeName);
    }
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT void JNICALL
Java_DDS_ObjectHome_jniSetRelationValidityField(
    JNIEnv * env,
    jobject ls_home,
    jstring jrelationName,
    jstring jvalidityFieldName)
{
    DK_ObjectHomeAdmin* home = NULL;
    DLRL_Exception exception;
    LOC_string relationName= NULL;
    LOC_string validityFieldName = NULL;

    DLRL_INFO(INF_ENTER);
    /*  Assert & verify parameters. */
    assert(jrelationName);
    assert(jvalidityFieldName);

    DLRL_Exception_init(&exception);

    relationName = (LOC_string)(*env)->GetStringUTFChars(
        env,
        jrelationName,
        0);
    DLRL_JavaException_PROPAGATE(env, &exception);
    validityFieldName = (LOC_string)(*env)->GetStringUTFChars(
        env,
        jvalidityFieldName,
        0);
    DLRL_JavaException_PROPAGATE(env, &exception);

    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);

    DK_MMFacade_us_setDLRLRelationValidityField(
        home,
        &exception,
        relationName,
        validityFieldName);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    if(validityFieldName){
        (*env)->ReleaseStringUTFChars(
            env,
            jvalidityFieldName,
            validityFieldName);
    }
    if(relationName){
        (*env)->ReleaseStringUTFChars(
            env,
            jrelationName,
            relationName);
    }
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT void JNICALL
Java_DDS_ObjectHome_jniAddRelationKeyFieldPair(
    JNIEnv * env,
    jobject ls_home,
    jstring jrelationName,
    jstring jownerKeyName,
    jstring jtargetKeyName)
{
    DK_ObjectHomeAdmin* home = NULL;
    DLRL_Exception exception;
    LOC_string relationName= NULL;
    LOC_string ownerKeyName = NULL;
    LOC_string targetKeyName = NULL;

    DLRL_INFO(INF_ENTER);
    /*  Assert & verify parameters. */
    assert(jrelationName);
    assert(jownerKeyName);
    assert(jtargetKeyName);

    DLRL_Exception_init(&exception);

    relationName = (LOC_string)(*env)->GetStringUTFChars(
        env,
        jrelationName,
        0);
    DLRL_JavaException_PROPAGATE(env, &exception);
    ownerKeyName = (LOC_string)(*env)->GetStringUTFChars(
        env,
        jownerKeyName,
        0);
    DLRL_JavaException_PROPAGATE(env, &exception);
    targetKeyName = (LOC_string)(*env)->GetStringUTFChars(
        env,
        jtargetKeyName,
        0);
    DLRL_JavaException_PROPAGATE(env, &exception);

    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);

    DK_MMFacade_us_addDLRLRelationKeyFieldPair(
        home,
        &exception,
        relationName,
        ownerKeyName,
        targetKeyName);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    if(targetKeyName){
        (*env)->ReleaseStringUTFChars(env, jtargetKeyName, targetKeyName);
    }
    if(ownerKeyName){
        (*env)->ReleaseStringUTFChars(env, jownerKeyName, ownerKeyName);
    }
    if(relationName){
        (*env)->ReleaseStringUTFChars(env, jrelationName, relationName);
    }
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT void JNICALL
Java_DDS_ObjectHome_jniSetRelationTopicPair(
    JNIEnv * env,
    jobject ls_home,
    jstring jrelationName,
    jstring jownerTopicName,
    jstring jtargetTopicName)
{
    DK_ObjectHomeAdmin* home = NULL;
    DLRL_Exception exception;
    LOC_string relationName = NULL;
    LOC_string ownerTopicName = NULL;
    LOC_string targetTopicName = NULL;

    DLRL_INFO(INF_ENTER);
    /*  Assert & verify parameters. */
    assert(jrelationName);
    assert(jownerTopicName);
    assert(jtargetTopicName);

    DLRL_Exception_init(&exception);

    relationName = (LOC_string)(*env)->GetStringUTFChars(
        env,
        jrelationName,
        0);
    DLRL_JavaException_PROPAGATE(env, &exception);
    ownerTopicName = (LOC_string)(*env)->GetStringUTFChars(
        env,
        jownerTopicName,
        0);
    DLRL_JavaException_PROPAGATE(env, &exception);
    targetTopicName = (LOC_string)(*env)->GetStringUTFChars(
        env,
        jtargetTopicName,
        0);
    DLRL_JavaException_PROPAGATE(env, &exception);

    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);

    DK_MMFacade_us_setDLRLRelationTopicPair(
        home,
        &exception,
        relationName,
        ownerTopicName,
        targetTopicName);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    if(targetTopicName){
        (*env)->ReleaseStringUTFChars(env, jtargetTopicName, targetTopicName);
    }
    if(ownerTopicName){
        (*env)->ReleaseStringUTFChars(env, jownerTopicName, ownerTopicName);
    }
    if(relationName){
        (*env)->ReleaseStringUTFChars(env, jrelationName, relationName);
    }
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT void JNICALL
Java_DDS_ObjectHome_jniAddOwnerField(
    JNIEnv * env,
    jobject ls_home,
    jstring jrelationName,
    jstring jfieldName)
{
    DK_ObjectHomeAdmin* home = NULL;
    DLRL_Exception exception;
    LOC_string relationName = NULL;
    LOC_string fieldName = NULL;

    DLRL_INFO(INF_ENTER);
    /*  Assert & verify parameters. */
    assert(jrelationName);
    assert(jfieldName);

    DLRL_Exception_init(&exception);

    relationName = (LOC_string)(*env)->GetStringUTFChars(
        env,
        jrelationName,
        0);
    DLRL_JavaException_PROPAGATE(env, &exception);
    fieldName = (LOC_string)(*env)->GetStringUTFChars(env, jfieldName, 0);
    DLRL_JavaException_PROPAGATE(env, &exception);

    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);

    DK_MMFacade_us_addOwnerField(home, &exception, relationName, fieldName);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    if(fieldName){
        (*env)->ReleaseStringUTFChars(env, jfieldName, fieldName);
    }
    if(relationName){
        (*env)->ReleaseStringUTFChars(env, jrelationName, relationName);
    }
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT void JNICALL
Java_DDS_ObjectHome_jniAddTargetField(
    JNIEnv * env,
    jobject ls_home,
    jstring jrelationName,
    jstring jfieldName)
{
    DK_ObjectHomeAdmin* home = NULL;
    DLRL_Exception exception;
    LOC_string fieldName = NULL;
    LOC_string relationName= NULL;

    DLRL_INFO(INF_ENTER);
    /*  Assert & verify parameters. */
    assert(jrelationName);
    assert(jfieldName);

    DLRL_Exception_init(&exception);

    relationName = (LOC_string)(*env)->GetStringUTFChars(env, jrelationName, 0);
    DLRL_JavaException_PROPAGATE(env, &exception);
    fieldName = (LOC_string)(*env)->GetStringUTFChars(env, jfieldName, 0);
    DLRL_JavaException_PROPAGATE(env, &exception);

    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);

    DK_MMFacade_us_addTargetField(home, &exception, relationName, fieldName);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    if(fieldName){
        (*env)->ReleaseStringUTFChars(env, jfieldName, fieldName);
    }
    if(relationName){
        (*env)->ReleaseStringUTFChars(env, jrelationName, relationName);
    }
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT void JNICALL
Java_DDS_ObjectHome_jniSetMultiRelationRelationTopic(
    JNIEnv * env,
    jobject ls_home,
    jstring jrelationName,
    jstring jrelationTopicName)
{
    DK_ObjectHomeAdmin* home = NULL;
    DLRL_Exception exception;
    LOC_string relationName = NULL;
    LOC_string relationTopicName = NULL;

    DLRL_INFO(INF_ENTER);
    /*  Assert & verify parameters. */
    assert(jrelationName);
    assert(jrelationTopicName);

    DLRL_Exception_init(&exception);

    relationName = (LOC_string)(*env)->GetStringUTFChars(
        env,
        jrelationName,
        0);
    DLRL_JavaException_PROPAGATE(env, &exception);
    relationTopicName = (LOC_string)(*env)->GetStringUTFChars(
        env,
        jrelationTopicName,
        0);
    DLRL_JavaException_PROPAGATE(env, &exception);

    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);

    DK_MMFacade_us_setMultiRelationRelationTopic(
        home,
        &exception,
        relationName,
        relationTopicName);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    if(relationTopicName){
        (*env)->ReleaseStringUTFChars(
            env,
            jrelationTopicName,
            relationTopicName);
    }
    if(relationName){
        (*env)->ReleaseStringUTFChars(
            env,
            jrelationName,
            relationName);
    }
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT void JNICALL
Java_DDS_ObjectHome_jniAddRelationTopicOwnerField(
    JNIEnv * env,
    jobject ls_home,
    jstring jrelationName,
    jstring jrelationTopicFieldName)
{
    DK_ObjectHomeAdmin* home = NULL;
    DLRL_Exception exception;
    LOC_string relationName = NULL;
    LOC_string relationTopicFieldName = NULL;

    DLRL_INFO(INF_ENTER);
    /*  Assert & verify parameters. */
    assert(jrelationName);
    assert(jrelationTopicFieldName);

    DLRL_Exception_init(&exception);

    relationName = (LOC_string)(*env)->GetStringUTFChars(env, jrelationName, 0);
    DLRL_JavaException_PROPAGATE(env, &exception);
    relationTopicFieldName = (LOC_string)(*env)->GetStringUTFChars(
        env,
        jrelationTopicFieldName,
        0);
    DLRL_JavaException_PROPAGATE(env, &exception);

    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);

    DK_MMFacade_us_addRelationTopicOwnerField(
        home,
        &exception,
        relationName,
        relationTopicFieldName);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    if(relationTopicFieldName){
        (*env)->ReleaseStringUTFChars(
            env,
            jrelationTopicFieldName,
            relationTopicFieldName);
    }
    if(relationName){
        (*env)->ReleaseStringUTFChars(env, jrelationName, relationName);
    }
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT void JNICALL
Java_DDS_ObjectHome_jniAddRelationTopicTargetField(
    JNIEnv * env,
    jobject ls_home,
    jstring jrelationName,
    jstring jrelationTopicFieldName)
{
    DK_ObjectHomeAdmin* home = NULL;
    DLRL_Exception exception;
    LOC_string relationName = NULL;
    LOC_string relationTopicFieldName = NULL;

    DLRL_INFO(INF_ENTER);
    /*  Assert & verify parameters. */
    assert(jrelationName);
    assert(jrelationTopicFieldName);

    DLRL_Exception_init(&exception);

    relationName = (LOC_string)(*env)->GetStringUTFChars(env, jrelationName, 0);
    DLRL_JavaException_PROPAGATE(env, &exception);
    relationTopicFieldName = (LOC_string)(*env)->GetStringUTFChars(
        env,
        jrelationTopicFieldName,
        0);
    DLRL_JavaException_PROPAGATE(env, &exception);

    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);

    DK_MMFacade_us_addRelationTopicTargetField(
        home,
        &exception,
        relationName,
        relationTopicFieldName);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    if(relationTopicFieldName){
        (*env)->ReleaseStringUTFChars(
            env,
            jrelationTopicFieldName,
            relationTopicFieldName);
    }
    if(relationName){
        (*env)->ReleaseStringUTFChars(
            env,
            jrelationName,
            relationName);
    }
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}

JNIEXPORT void JNICALL
Java_DDS_ObjectHome_jniSetRelationTopicIndexField(
    JNIEnv * env,
    jobject ls_home,
    jstring jrelationName,
    jstring jrelationTopicFieldName)
{
    DK_ObjectHomeAdmin* home = NULL;
    DLRL_Exception exception;
    LOC_string relationName = NULL;
    LOC_string relationTopicFieldName = NULL;

    DLRL_INFO(INF_ENTER);
    /*  Assert & verify parameters. */
    assert(jrelationName);
    assert(jrelationTopicFieldName);

    DLRL_Exception_init(&exception);

    relationName = (LOC_string)(*env)->GetStringUTFChars(env, jrelationName, 0);
    DLRL_JavaException_PROPAGATE(env, &exception);
    relationTopicFieldName =  (LOC_string)(*env)->GetStringUTFChars(
        env,
        jrelationTopicFieldName,
        0);
    DLRL_JavaException_PROPAGATE(env, &exception);

    home = (DK_ObjectHomeAdmin*)(*env)->GetLongField(
        env,
        ls_home,
        cachedJNI.objectHome_admin_fid);
    assert(home);

    DK_MMFacade_us_setRelationTopicIndexField(
        home,
        &exception,
        relationName,
        relationTopicFieldName);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    if(relationTopicFieldName){
        (*env)->ReleaseStringUTFChars(
            env,
            jrelationTopicFieldName,
            relationTopicFieldName);
    }
    if(relationName){
        (*env)->ReleaseStringUTFChars(env, jrelationName, relationName);
    }
    DJA_ExceptionHandler_handleException(env, &exception);
    DLRL_INFO(INF_EXIT);
}
