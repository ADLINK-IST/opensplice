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
#ifndef CCPP_DLRL_UTILS_H
#define CCPP_DLRL_UTILS_H

#include "ccpp_dlrl.h"
#include "DLRL_Types.h"
#include "DLRL_Kernel.h"
#include "gapi.h"
#include "ccpp_dlrl_if.h"
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace DDS
{
    class ObjectRoot_impl;
};

struct ccpp_TypedTopicCache_s;
typedef struct ccpp_TypedTopicCache_s ccpp_TypedTopicCache;

struct ccpp_TypedObjectCache_s;
typedef struct ccpp_TypedObjectCache_s ccpp_TypedObjectCache;

typedef DLRL_LS_object (*ccpp_DlrlUtils_us_getCurrentTopic) (
    DLRL_LS_object lsObject);

typedef void (*ccpp_DlrlUtils_us_setCurrentTopic) (
    DLRL_LS_object lsObject,
    DLRL_LS_object lsTopic);

typedef DLRL_LS_object (*ccpp_DlrlUtils_us_getPreviousTopic) (
    DLRL_LS_object lsObject);

typedef void (*ccpp_DlrlUtils_us_setPreviousTopic) (
    DLRL_LS_object lsObject,
    DLRL_LS_object lsTopic);

typedef DDS::ObjectRoot_impl* (*ccpp_DlrlUtils_us_createTypedObject) (
    DLRL_Exception* exception);

typedef CORBA::Boolean (*ccpp_DlrlUtils_us_invokeXXXObjectCallback)(
    DLRL_LS_object listener,
    DLRL_LS_object xxxObject);

typedef CORBA::Boolean (*ccpp_DlrlUtils_us_invokeNewObjectCallback)(
    DLRL_LS_object listener,
    DLRL_LS_object newObject);

typedef CORBA::Boolean (*ccpp_DlrlUtils_us_invokeModifiedObjectCallback)(
    DLRL_LS_object listener,
    DLRL_LS_object modifiedObject);

typedef CORBA::Boolean (*ccpp_DlrlUtils_us_invokeDeletedObjectCallback)(
    DLRL_LS_object listener,
    DLRL_LS_object deletedObject);

typedef DLRL_LS_object (*ccpp_DlrlUtils_us_createTypedStrMap)(
    DLRL_Exception* exception,
    DK_Collection* collection);

typedef DLRL_LS_object (*ccpp_DlrlUtils_us_createTypedIntMap)(
    DLRL_Exception* exception,
    DK_Collection* collection);

typedef DLRL_LS_object (*ccpp_DlrlUtils_us_createTypedSet)(
    DLRL_Exception* exception,
    DK_Collection* collection);

typedef void (*ccpp_DlrlUtils_us_initializeTopicCache)(
    ccpp_TypedTopicCache* topicCache);

typedef DLRL_LS_object (*ccpp_DlrlUtils_us_createTypedTopic)(
    DLRL_Exception* exception);

typedef void (*ccpp_DlrlUtils_us_setTopicClassName)(
    DLRL_LS_object lsTopic,
    const char *className);

typedef void (*ccpp_DlrlUtils_us_setTopicOidField)(
    DLRL_LS_object lsTopic,
    const DDS::DLRLOid& oid);

typedef void (*ccpp_DlrlUtils_us_createTypedObjectSeq)(
    DLRL_Exception* exception,
    void** arg,
    LOC_unsigned_long size);

typedef void (*ccpp_DlrlUtils_us_addElementToTypedObjectSeq)(
    DLRL_Exception* exception,
    void* arg,
    DLRL_LS_object lsObject,
    LOC_unsigned_long count);

typedef void (*ccpp_DlrlUtils_us_createTypedSelectionSeq)(
    DLRL_Exception* exception,
    void** arg,
    LOC_unsigned_long size);

typedef void (*ccpp_DlrlUtils_us_addElementToTypedSelectionSeq)(
    DLRL_Exception* exception,
    void* arg,
    DLRL_LS_object lsObject,
    LOC_unsigned_long count);

typedef void (*ccpp_DlrlUtils_us_createTypedListenerSeq)(
    DLRL_Exception* exception,
    void** arg,
    LOC_unsigned_long size);

typedef void (*ccpp_DlrlUtils_us_addElementToTypedListenerSeq)(
    DLRL_Exception* exception,
    void* arg,
    DLRL_LS_object lsObject,
    LOC_unsigned_long count);

typedef void (*ccpp_DlrlUtils_us_changeRelations)(
    DLRL_LS_object ownerObject,
    DLRL_LS_object relationObject,
    LOC_unsigned_long index);

typedef void (*ccpp_DlrlUtils_us_setCollections)(
    DLRL_LS_object ownerObject,
    DLRL_LS_object collectionObject,
    LOC_unsigned_long index);

typedef void (*ccpp_DlrlUtils_us_clearLSObjectAdministration)(
    DLRL_LS_object ls_object);

typedef LOC_boolean (*ccpp_DlrlUtils_us_checkObjectForSelection)(
        DLRL_Exception* exception,
        DLRL_LS_object filter,
        DLRL_LS_object objectAdmin);

typedef void (*ccpp_DlrlUtils_us_triggerListenerInsertedObject)(
        DLRL_Exception* exception,
        DLRL_LS_object listener,
        DLRL_LS_object objectAdmin);

typedef void (*ccpp_DlrlUtils_us_triggerListenerModifiedObject)(
        DLRL_Exception* exception,
        DLRL_LS_object listener,
        DLRL_LS_object objectAdmin);

typedef void (*ccpp_DlrlUtils_us_triggerListenerRemovedObject)(
        DLRL_Exception* exception,
        DLRL_LS_object listener,
        DLRL_LS_object objectAdmin);

struct ccpp_TypedTopicCache_s
{
    ccpp_DlrlUtils_us_createTypedTopic createTypedTopic;
    ccpp_DlrlUtils_us_setTopicClassName setTopicClassName;
    ccpp_DlrlUtils_us_setTopicOidField setTopicOidField;
    gapi_copyIn copyIn;
    gapi_copyOut copyOut;
};

struct ccpp_TypedObjectCache_s
{
    /* initializeTopicCache only required for main topic: not for multiplace topics */
    /* since these are directly manipulated by the kernel in the database.          */
    /* In the future initializeTopicCache might also be used for place topics:      */
    /* then the attribute type would need to be changed into an array type.         */
    ccpp_DlrlUtils_us_initializeTopicCache initializeTopicCache;
    ccpp_DlrlUtils_us_getCurrentTopic getCurrentTopic;
    ccpp_DlrlUtils_us_setCurrentTopic setCurrentTopic;
    ccpp_DlrlUtils_us_getPreviousTopic getPreviousTopic;
    ccpp_DlrlUtils_us_setPreviousTopic setPreviousTopic;
    ccpp_DlrlUtils_us_createTypedObject createTypedObject;
    ccpp_DlrlUtils_us_invokeNewObjectCallback invokeNewObjectCallback;
    ccpp_DlrlUtils_us_invokeModifiedObjectCallback invokeModifiedObjectCallback;
    ccpp_DlrlUtils_us_invokeDeletedObjectCallback invokeDeletedObjectCallback;
    ccpp_DlrlUtils_us_createTypedStrMap createTypedStrMap;
    ccpp_DlrlUtils_us_createTypedIntMap createTypedIntMap;
    ccpp_DlrlUtils_us_createTypedSet createTypedSet;
    ccpp_DlrlUtils_us_createTypedObjectSeq createTypedObjectSeq;
    ccpp_DlrlUtils_us_addElementToTypedObjectSeq addElementToTypedObjectSeq;
    ccpp_DlrlUtils_us_createTypedSelectionSeq createTypedSelectionSeq;
    ccpp_DlrlUtils_us_addElementToTypedSelectionSeq addElementToTypedSelectionSeq;
    ccpp_DlrlUtils_us_createTypedListenerSeq createTypedListenerSeq;
    ccpp_DlrlUtils_us_addElementToTypedListenerSeq addElementToTypedListenerSeq;
    ccpp_DlrlUtils_us_changeRelations changeRelations;
    ccpp_DlrlUtils_us_setCollections setCollections;
    ccpp_DlrlUtils_us_clearLSObjectAdministration clearLSObjectAdministration;
    ccpp_DlrlUtils_us_checkObjectForSelection checkObjectForSelection;
    ccpp_DlrlUtils_us_triggerListenerInsertedObject triggerListenerInsertedObject;
    ccpp_DlrlUtils_us_triggerListenerModifiedObject triggerListenerModifiedObject;
    ccpp_DlrlUtils_us_triggerListenerRemovedObject triggerListenerRemovedObject;
};


/* go from any ccpp object to DLRL_LS_object */
#define UPCAST_DLRL_LS_OBJECT(_this) \
    reinterpret_cast<DLRL_LS_object>(dynamic_cast<CORBA::Object_ptr>(_this))

#define VB_UPCAST_DLRL_LS_OBJECT(_this) \
    reinterpret_cast<DLRL_LS_object>(dynamic_cast<CORBA::ValueBase*>(_this))

#define REINTERPRET_TO_DLRL_LS_OBJECT(_this) \
    reinterpret_cast<DLRL_LS_object>(_this)

/* Following macros are the (very specific) opposite macros to the UPCAST_DLRL_LS_OBJECT macro */
#define DOWNCAST_DDS_ENTITY_IMPL(lsObject) \
    dynamic_cast<DDS::Entity_impl*>(reinterpret_cast<CORBA::Object_ptr>(lsObject))

#define DOWNCAST_DDS_ENTITY(lsObject) \
    dynamic_cast<DDS::Entity_ptr>(reinterpret_cast<CORBA::Object_ptr>(lsObject))

#define DOWNCAST_DDS_PARTICIPANT(lsObject) \
    dynamic_cast<DDS::DomainParticipant_ptr>(reinterpret_cast<CORBA::Object_ptr>(lsObject))

#define DOWNCAST_DDS_PUBLISHER(lsObject) \
    dynamic_cast<DDS::Publisher_ptr>(reinterpret_cast<CORBA::Object_ptr>(lsObject))

#define DOWNCAST_DDS_SUBSCRIBER(lsObject) \
    dynamic_cast<DDS::Subscriber_ptr>(reinterpret_cast<CORBA::Object_ptr>(lsObject))

#define DOWNCAST_DDS_READER(lsObject) \
    dynamic_cast<DDS::DataReader_ptr>(reinterpret_cast<CORBA::Object_ptr>(lsObject))

#define DOWNCAST_DDS_WRITER(lsObject) \
    dynamic_cast<DDS::DataWriter_ptr>(reinterpret_cast<CORBA::Object_ptr>(lsObject))

#define DOWNCAST_DDS_TOPIC(lsObject) \
    dynamic_cast<DDS::Topic_ptr>(reinterpret_cast<CORBA::Object_ptr>(lsObject))

#define DDS_GET_GAPI_SELF(ccppObject) \
    (dynamic_cast<DDS::Entity_impl*>(ccppObject))->get_gapi_self()

#define DOWNCAST_DDS_CACHE(lsObject) \
    dynamic_cast<DDS::Cache_ptr>(reinterpret_cast<CORBA::Object_ptr>(lsObject))

#define DOWNCAST_DDS_CACHELISTENER(_this) \
    dynamic_cast<DDS::CacheListener_ptr>(reinterpret_cast<CORBA::Object_ptr>(_this))

#define DOWNCAST_DDS_CACHEBASE(lsObject) \
    dynamic_cast<DDS::CacheBase_ptr>(reinterpret_cast<CORBA::Object_ptr>(lsObject))

#define DOWNCAST_DDS_ACCESS(lsObject) \
    dynamic_cast<DDS::CacheAccess_ptr>(reinterpret_cast<CORBA::Object_ptr>(lsObject))

#define DOWNCAST_DDS_OBJECTHOME(lsObject) \
    dynamic_cast<DDS::ObjectHome_ptr>(reinterpret_cast<CORBA::Object_ptr>(lsObject))

#define DOWNCAST_DDS_OBJECTROOT(lsObject) \
    dynamic_cast<DDS::ObjectRoot*>(reinterpret_cast<CORBA::ValueBase*>(lsObject))

#define DOWNCAST_DDS_OBJECTROOT_IMPL(lsObject) \
    dynamic_cast<DDS::ObjectRoot_impl*>(reinterpret_cast<CORBA::ValueBase*>(lsObject))

#define DOWNCAST_DDS_OBJECTHOME_IMPL(lsObject) \
    dynamic_cast<DDS::ObjectHome_impl*>(reinterpret_cast<CORBA::Object_ptr>(lsObject))

#define DOWNCAST_DDS_SELECTION_IMPL(lsObject) \
    dynamic_cast<DDS::Selection_impl*>(reinterpret_cast<CORBA::Object_ptr>(lsObject))

#define DOWNCAST_DDS_SELECTIONCRITERION(lsObject) \
    dynamic_cast<DDS::SelectionCriterion_ptr>(reinterpret_cast<CORBA::Object_ptr>(lsObject))

#define DLRL_CCPP_EXCEPTION_CATCH_AND_PROPAGATE(loc_exception)                                          \
    catch (const DDS::OutOfMemory& ccpp_exception) {                                                    \
        DLRL_Exception_THROW(loc_exception, DLRL_OUT_OF_MEMORY, ccpp_exception.message.in());           \
    }                                                                                                   \
    catch (const DDS::DLRLError& ccpp_exception) {                                                      \
        DLRL_Exception_THROW(loc_exception, DLRL_ERROR, ccpp_exception.message.in());                   \
    }                                                                                                   \
    catch (const DDS::DCPSError& ccpp_exception) {                                                      \
        DLRL_Exception_THROW(loc_exception, DLRL_DCPS_ERROR, ccpp_exception.message.in());              \
    }                                                                                                   \
    catch (const DDS::BadHomeDefinition& ccpp_exception) {                                              \
        DLRL_Exception_THROW(loc_exception, DLRL_BAD_HOME_DEFINITION, ccpp_exception.message.in());     \
    }                                                                                                   \
    catch (const DDS::NotFound& ccpp_exception) {                                                       \
        DLRL_Exception_THROW(loc_exception, DLRL_NOT_FOUND, ccpp_exception.message.in());               \
    }                                                                                                   \
    catch (const DDS::AlreadyExisting& ccpp_exception) {                                                \
        DLRL_Exception_THROW(loc_exception, DLRL_ALREADY_EXISTING, ccpp_exception.message.in());        \
    }                                                                                                   \
    catch (const DDS::PreconditionNotMet& ccpp_exception) {                                             \
        DLRL_Exception_THROW(loc_exception, DLRL_PRECONDITION_NOT_MET, ccpp_exception.message.in());    \
    }                                                                                                   \
    catch (const DDS::AlreadyDeleted& ccpp_exception) {                                                 \
        DLRL_Exception_THROW(loc_exception, DLRL_ALREADY_DELETED, ccpp_exception.message.in());         \
    }                                                                                                   \
    catch (const DDS::NoSuchElement& ccpp_exception) {                                                  \
        DLRL_Exception_THROW(loc_exception, DLRL_NO_SUCH_ELEMENT, ccpp_exception.message.in());         \
    }                                                                                                   \
    catch (const DDS::SQLError& ccpp_exception) {                                                       \
        DLRL_Exception_THROW(loc_exception, DLRL_SQL_ERROR, ccpp_exception.message.in());               \
    }                                                                                                   \
    catch (const DDS::BadParameter& ccpp_exception) {                                                   \
        DLRL_Exception_THROW(loc_exception, DLRL_BAD_PARAMETER, ccpp_exception.message.in());           \
    }                                                                                                   \
    catch (const DDS::InvalidObjects& ccpp_exception) {                                                 \
        DLRL_Exception_THROW(loc_exception, DLRL_INVALID_OBJECTS, ccpp_exception.message.in());         \
    }                                                                                                   \
    catch (const DDS::TimeOut& ccpp_exception) {                                                        \
        DLRL_Exception_THROW(loc_exception, DLRL_TIMEOUT, ccpp_exception.message.in());                 \
    }                                                                                                   \
    catch (const CORBA::Exception& ccpp_exception) {                                                    \
        DLRL_Exception_THROW(loc_exception, DLRL_TIMEOUT, "Unrecognized C++ exception received...");    \
    }

/* functions used by the exception macros */

OS_DLRL_API const char *ccpp_DlrlUtils_returnCodeGAPIToString(gapi_returnCode_t returnCode);

OS_DLRL_API const char *ccpp_DlrlUtils_returnCodeDCPSToString(DDS::ReturnCode_t returnCode);

extern "C" {
    OS_DLRL_API void
    ccpp_DCPSUtilityBridge_us_registerType(
        DLRL_Exception* exception,
        void* userData,
        DK_ObjectHomeAdmin* home,
        DK_CacheAdmin* cache,
        LOC_char* topicName,
        LOC_char* typeName);

    OS_DLRL_API void
    ccpp_ObjectHomeBridge_us_loadMetamodel(
        DLRL_Exception* exception,
        DK_ObjectHomeAdmin* home,
        void* userData);

    OS_DLRL_API DLRL_LS_object
    ccpp_ObjectHomeBridge_us_createTypedObject(
        DLRL_Exception* exception,
        void* userData,
        DK_ObjectHomeAdmin* home,
        DK_TopicInfo* topicInfo,
        DLRL_LS_object ls_topic,
        DK_ObjectAdmin* objectAdmin);

    OS_DLRL_API DK_ObjectAdmin**
    ccpp_SelectionBridge_us_checkObjects(
        DLRL_Exception* exception,
        void* userData,
        DK_SelectionAdmin* selection,
        DLRL_LS_object filterCriterion,
        DK_ObjectAdmin** objectArray,
        LOC_unsigned_long size,
        LOC_unsigned_long* passedAdminsArraySize);
} /* extern "C" */

OS_DLRL_API void
ccpp_DlrlUtils_us_handleException(
    DLRL_Exception* exception);

/* Exception macros */

/* TODO should be in generic part as SAJ and CCPP both use it */
#define DLRL_Exception_PROPAGATE_GAPI_RESULT(_this, returnCode, message) \
    do { \
        if(returnCode != GAPI_RETCODE_OK){ \
            DLRL_Exception_THROW(_this, DLRL_DCPS_ERROR, "%s: %s. Check DCPS error log file for (possibly) more information.", ccpp_DlrlUtils_returnCodeGAPIToString((returnCode)), message); \
        } \
    } while(0)

/* TODO should be in generic part as SAJ and CCPP both use it */
#define DLRL_DcpsException_PROPAGATE(_this, returnCode, ...) \
    do { \
        if(returnCode != DDS::RETCODE_OK){\
            DLRL_Exception_THROW(_this, DLRL_DCPS_ERROR, "%s: %s. Check DCPS error log file for (possibly) more information.", ccpp_DlrlUtils_returnCodeDCPSToString((returnCode)), __VA_ARGS__);\
        } \
    } while(0)


/* Functions that need to be reported as friends for class DDS::ObjectRoot_impl. */

extern "C" {
    OS_DLRL_API void
    ccpp_ObjectReaderBridge_us_updateObject(
        DLRL_Exception* exception,
        void* userData,
        DK_ObjectHomeAdmin* home,
        DK_ObjectAdmin* object,
        DLRL_LS_object ls_topic);

    OS_DLRL_API void ccpp_ObjectReaderBridge_us_resetLSModificationInfo(
        DLRL_Exception* exception,
        void* userData,
        DK_ObjectAdmin* objectAdmin);
} /* extern "C" */

#endif /* CCPP_DLRL_UTILS_H */

/* TODO propagate exceptions from c++ and translate into kernel exception struct */
