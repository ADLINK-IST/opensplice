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
#ifndef DLRL_KERNEL_OBJECT_HOME_BRIDGE_H
#define DLRL_KERNEL_OBJECT_HOME_BRIDGE_H

/* collection includes */
#include "Coll_List.h"

#if defined (__cplusplus)
extern "C" {
#endif

/* \brief This operation is responsible for inserting all meta information into the kernel ObjectHome's MetaModel.
 *
 * This operation is called when the cache receives the 'register all for pubsub' call, within that call the meta model
 * information inserted here is automatically resolved with meta information of other object homes, creating an
 * inter linked meta model between the various DLRLClasses. The intention of this operation is to used the operations
 * defined in the DLRL Kernel Meta Model Facade (DK_MMFacade_...) to insert all required meta model information.
 *
 * Mutex claims during this operation:<ul>
 * <li>The admin mutex of the <code>DK_CacheAdmin</code> object to which the objects in question belong.</li>
 * <li>The admin mutex of ALL <code>DK_ObjectHomeAdmin</code> objects which are registered to the
 * <code>DK_CacheAdmin</code> to which the objects in question belong.</li></ul>
 *
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param home The object home for which the meta model should be loaded.
 * \param userData The userData as provided by the language specific binding when entering the kernel
 */
typedef void (*DK_ObjectHomeBridge_us_loadMetamodel)(
    DLRL_Exception* exception,
    DK_ObjectHomeAdmin* home,
    void* userData);

/* \brief Performs any required clean up actions for language specific representatives of the
 * <code>DK_ObjectHomeAdmin</code>.
 *
 * This function is called during the deletion of a <code>DK_ObjectHomeAdmin</code>. After this function the owning
 * <code>DK_ObjectHomeAdmin</code> object will no longer maintain it's reference to the language specific
 * representative (IE the pointer is set to NULL). Therefore this operations represents the last possibility for
 * clean up actions.
 *
 * Mutex claims during this operation:<ul>
 * <li>The mutex of the <code>DK_CacheFactoryAdmin</code> object.</li>
 * <li>The update and admin mutex of the <code>DK_CacheAdmin</code> object to which the objects in question belong.</li>
 * <li>The update and admin mutexes of ALL <code>DK_ObjectHomeAdmin</code> objects which are registered to the
 * <code>DK_CacheAdmin</code> to which the objects in question belong.</li></ul>
 *
 * \param userData The userData as provided by the language specific binding when entering the kernel
 * \param ls_home The language specific representative of a <code>DK_ObjectHomeAdmin</code> object.
 * \param isRegistered To indicate if the home in question belong to a <code>DK_CacheAdmin</code> object or not.
 */
typedef void (*DK_ObjectHomeBridge_us_unregisterAdminWithLSHome)(
    void* userData,
    DLRL_LS_object ls_home,
    LOC_boolean isRegistered);

/* \brief Ensures the home specific user data is deleted and all memory freed.
 *
 * This function is called during the deletion of a <code>DK_ObjectHomeAdmin</code>. After this function the owning
 * <code>DK_ObjectHomeAdmin</code> object will no longer maintain it's reference to the home specific user data
 * object. (IE the pointer is set to NULL). Therefore this operation represents the last possibility for clean up
 * actions.
 *
 * Mutex claims during this operation:<ul>
 * <li>The mutex of the <code>DK_CacheFactoryAdmin</code> object.</li>
 * <li>The update and admin mutex of the <code>DK_CacheAdmin</code> object to which the objects in question belong.</li>
 * <li>The update and admin mutexes of ALL <code>DK_ObjectHomeAdmin</code> objects which are registered to the
 * <code>DK_CacheAdmin</code> to which the objects in question belong.</li></ul>
 *
 * \param userData The userData as provided by the language specific binding when entering the kernel
 * \param homeUserData The home specific userdata that was registered before and needs to be deleted now.
 */
typedef void (*DK_ObjectHomeBridge_us_deleteUserData)(
    void* userData,
    void* homeUserData);

/* \brief Triggers all attached object listeners for a specific object homes and if neccesary any object listeners
 * registered with object homes that managed the parent class of the class managed by the object home in question.
 *
 * Mutex claims during this operation:<ul>
 * <li>The update mutex of the <code>DK_CacheAdmin</code> object to which the objects in question belong.</li>
 * <li>The update mutex of ALL <code>DK_ObjectHomeAdmin</code> objects which are registered to the
 * <code>DK_CacheAdmin</code> to which the objects in question belong.</li></ul>
 *
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param userData The userData as provided by the language specific binding when entering the kernel
 * \param home The object home for which listeners should be triggered.
 * \param newSamples All newly created objects in the last update round for which the
 * <code>on_object_created(...)</code> operation should be called.
 * \param modifiedSamples All modified objects in the last update round for which the
 * <code>on_object_modified(...)</code> operation should be called.
 * \param deletedSamples All deleted objects in the last update round for which the
 * <code>on_object_deleted(...)</code> operation should be called.
 */
typedef void (*DK_ObjectHomeBridge_us_triggerListeners)(
    DLRL_Exception* exception,
    void *userData,
    DK_ObjectHomeAdmin* home,
    Coll_List* newSamples,
    Coll_List* modifiedSamples,
    Coll_List* deletedSamples);

/* \brief Responsible for creating a language specific representative for a kernel object admin.
 *
 * This operation should created the typed langauge specific object and store the topic data into the object.
 * It should return the created object so the kernel can store it, if the language binding requires it, any
 * reference increasing should be done before returning this operation, as the kernel will maintain the returned pointer.
 *
 * Mutex claims during this operation:<ul>
 * <li>The update mutex of the <code>DK_CacheAdmin</code> object to which the objects in question belong.</li>
 * <li>The admin and update mutexes of ALL <code>DK_ObjectHomeAdmin</code> objects which are registered to the
 * <code>DK_CacheAdmin</code> to which the objects in question belong.</li></ul>
 *
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param userData Optional language specific user data
 * \param home The home that is the manager of the type of object that needs to be created.
 * \param data Data regarding the object being processed
 *
 * \return The newly created language specific typed object which belong to the provided kernel object admin or
 * <code>NULL</code> if an exception occured.
 */
typedef DLRL_LS_object (*DK_ObjectHomeBridge_us_createTypedObject)(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DK_TopicInfo* topicInfo,
    DLRL_LS_object ls_topic,
    DK_ObjectAdmin* objectAdmin);

/* NOT IN DESIGN */
typedef void (*DK_ObjectHomeBridge_us_doCopyInForTopicOfObject)(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DK_ObjectWriter* objWriter,
    DK_ObjectAdmin* objectAdmin,
    void* message,
    void* dataSample);

/* NOT IN DESIGN */
/* className may be NIL */
typedef void (*DK_ObjectHomeBridge_us_setDefaultTopicKeys)(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    DK_TopicInfo* topicInfo,
    DLRL_LS_object ls_object,
    DK_ObjectID* oid,
    LOC_string className);

typedef void (*DK_ObjectHomeBridge_us_createTypedObjectSeq)(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* targetLockedHome,
    void** arg,
    LOC_unsigned_long size);

typedef void (*DK_ObjectHomeBridge_us_addElementToTypedObjectSeq)(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* targetLockedHome,
    void* arg,
    DLRL_LS_object lsObject,
    LOC_unsigned_long count);

typedef void (*DK_ObjectHomeBridge_us_createTypedSelectionSeq)(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* targetLockedHome,
    void** arg,
    LOC_unsigned_long size);

typedef void (*DK_ObjectHomeBridge_us_addElementToTypedSelectionSeq)(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* targetLockedHome,
    void* arg,
    DLRL_LS_object lsObject,
    LOC_unsigned_long count);

typedef void (*DK_ObjectHomeBridge_us_createTypedListenerSeq)(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* targetLockedHome,
    void** arg,
    LOC_unsigned_long size);

typedef void (*DK_ObjectHomeBridge_us_addElementToTypedListenerSeq)(
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* targetLockedHome,
    void* arg,
    DLRL_LS_object lsObject,
    LOC_unsigned_long count);

typedef LOC_boolean (*DK_ObjectHomeBridge_us_checkObjectForSelection)(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    void* userData,
    DK_SelectionAdmin* selection,
    DK_ObjectAdmin* objectAdmin);

typedef struct DK_ObjectHomeBridge_s{
    DK_ObjectHomeBridge_us_loadMetamodel loadMetamodel;
    DK_ObjectHomeBridge_us_unregisterAdminWithLSHome unregisterAdminWithLSHome;
    DK_ObjectHomeBridge_us_deleteUserData deleteUserData;
    DK_ObjectHomeBridge_us_triggerListeners triggerListeners;
    DK_ObjectHomeBridge_us_createTypedObject createTypedObject;
    DK_ObjectHomeBridge_us_doCopyInForTopicOfObject doCopyInForTopicOfObject;
    DK_ObjectHomeBridge_us_setDefaultTopicKeys setDefaultTopicKeys;
    DK_ObjectHomeBridge_us_createTypedObjectSeq createTypedObjectSeq;
    DK_ObjectHomeBridge_us_addElementToTypedObjectSeq addElementToTypedObjectSeq;
    DK_ObjectHomeBridge_us_createTypedSelectionSeq createTypedSelectionSeq;
    DK_ObjectHomeBridge_us_addElementToTypedSelectionSeq addElementToTypedSelectionSeq;
    DK_ObjectHomeBridge_us_createTypedListenerSeq createTypedListenerSeq;
    DK_ObjectHomeBridge_us_addElementToTypedListenerSeq addElementToTypedListenerSeq;
    DK_ObjectHomeBridge_us_checkObjectForSelection checkObjectForSelection;
} DK_ObjectHomeBridge;

extern DK_ObjectHomeBridge objectHomeBridge;

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_KERNEL_OBJECT_HOME_BRIDGE_H */
