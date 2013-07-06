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
#ifndef DLRL_KERNEL_OBJECT_HOME_ADMIN_H
#define DLRL_KERNEL_OBJECT_HOME_ADMIN_H

/* OS Abstraction layer includes */
#include "os_mutex.h"

/* data base includes */
#include "c_typebase.h"

/* DLRL util includes */
#include "DLRL_Exception.h"
#include "DLRL_Types.h"

/* Collection includes */
#include "Coll_List.h"
#include "Coll_Set.h"

/* DLRL MetaModel includes */
#include "DMM_DLRLClass.h"

/* DLRL Kernel includes */
#include "DK_CacheAdmin.h"
#include "DK_Entity.h"
#include "DK_ObjectHomeBridge.h"
#include "DK_Types.h"
#include "DLRL_Kernel.h"
#include "DLRL_Kernel_private.h"

#if defined (__cplusplus)
extern "C" {
#endif

struct DK_ObjectHomeAdmin_s
{
    DK_Entity entity;
    LOC_boolean alive;
    /* locks listener operations and new/modified/deleted/get objects operations */
    os_mutex updateMutex;
    /* locks everything in the home that the update mutex doesnt take care off + all home sub entities  */
    /* (such as ObjectAdmin/Collection) */
    os_mutex adminMutex;
    DLRL_LS_object ls_home;/* may be NULL until this home is registered with a cache */
    void* userData;

    LOC_string name;
    /* index may not be cleared once set (I.E. another value then -1)!  */
    /* (see memo regarding locking strategy for explanation) */
    LOC_long index;
    LOC_boolean autoDeref;
    LOC_string filter;

    Coll_Set listeners;
    Coll_Set selections;
    Coll_Set children;

    DK_ObjectHomeAdmin* parent;
    DK_CacheAdmin* cache;

    DMM_DLRLClass* meta_representative;
    DK_ObjectReader* objectReader;
    DK_ObjectWriter* objectWriter;
    Coll_List topicInfos;

    /* An array of all related object homes. This array may be set to NULL. It will be created and filled once the owning */
    /* CacheAdmin has been registered for pub/sub. This array will contain reference counted ObjectHomes which can be */
    /* used when its needed to lock other homes. This array may contain multiple references to itself, that just depends */
    /* what kind of relations are defined on the type that this home represents. The homes in this array will be cleared */
    /* once this home is being deleted. So if the home is not alive, then this array will be freed and thus not contain  */
    /* anything */
    DK_ObjectHomeAdmin** relatedHomes;/* NOT IN DESIGN */
};

void
DK_ObjectHomeAdmin_us_setCacheAdmin(
    DK_ObjectHomeAdmin* _this,
    DK_CacheAdmin* cache);

/* maybe only be called once to set the registeration index from -1 to another value (no limit when setting from -1 to -1) */
void
DK_ObjectHomeAdmin_us_setRegistrationIndex(
    DK_ObjectHomeAdmin* _this,
    LOC_long registrationIndex);

LOC_boolean
DK_ObjectHomeAdmin_us_hasCacheAdmin(
    DK_ObjectHomeAdmin* _this);

/* wont lock other homes, only require lock on the home for which the operation is called */
void
DK_ObjectHomeAdmin_us_loadMetamodel(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData);

/* assumes locks on all homes */
void
DK_ObjectHomeAdmin_us_setRelations(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception);

/* wont lock other homes, only require lock on the home for which the operation is called */
void
DK_ObjectHomeAdmin_us_createDCPSEntities(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData);

/* requires a lock of the update mutex of the home */
void
DK_ObjectHomeAdmin_us_triggerListeners(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData);

/* requires a lock of the update mutex of the home */
void
DK_ObjectHomeAdmin_us_triggerSelectionListeners(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData);

/* NOT IN DESIGN - param added */
void
DK_ObjectHomeAdmin_ts_enableDCPSEntities(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData);

void
DK_ObjectHomeAdmin_us_resolveMetaModel(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData);

Coll_List*
DK_ObjectHomeAdmin_us_getTopicInfos(
    DK_ObjectHomeAdmin* _this);

LOC_boolean
DK_ObjectHomeAdmin_us_getAutoDeref(
    DK_ObjectHomeAdmin* _this);

/* may return null */
DMM_DLRLClass*
DK_ObjectHomeAdmin_us_getMetaRepresentative(
    DK_ObjectHomeAdmin* _this);


/* needs locks on its owning home + related homes, meta information of all homes must be in tact! */
void
DK_ObjectHomeAdmin_us_deleteAllObjectAdmins(
    DK_ObjectHomeAdmin* _this,
    void* userData);

DK_ObjectReader*
DK_ObjectHomeAdmin_us_getObjectReader(
    DK_ObjectHomeAdmin* _this);

void
DK_ObjectHomeAdmin_us_registerUnresolvedElement(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHolder* holder,
    LOC_unsigned_long relationIndex);

void
DK_ObjectHomeAdmin_us_unregisterUnresolvedElement(
    DK_ObjectHomeAdmin* _this,
    void* userData,
    DK_ObjectHolder* holder);

/* asumes the home is locked */
DK_Collection*
DK_ObjectHomeAdmin_us_registerUnresolvedCollection(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void* ownerValues,
    LOC_unsigned_long valuesSize,
    DMM_DLRLMultiRelation* metaRelation,
    LOC_unsigned_long collectionIndex);

DK_Collection*
DK_ObjectHomeAdmin_us_unregisterUnresolvedCollection(
    DK_ObjectHomeAdmin* _this,
    void* userData,
    void* ownerValues,
    LOC_unsigned_long valuesSize,
    LOC_unsigned_long collectionIndex);

/* requires update & admin locks on owner home and related homes! */
void
DK_ObjectHomeAdmin_us_resetObjectModificationInformation(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData);

void
DK_ObjectHomeAdmin_us_markObjectAsModified(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    DK_ObjectAdmin* modifiedObject);

void
DK_ObjectHomeAdmin_us_unregisterAllUnresolvedElementsForEntity(
    DK_ObjectHomeAdmin* _this,
    void* userData,
    DK_Entity* entity);

void
DK_ObjectHomeAdmin_us_collectObjectUpdates(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ReadInfo* info);

void
DK_ObjectHomeAdmin_us_processObjectUpdates(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ReadInfo* info);

void
DK_ObjectHomeAdmin_us_processAllObjectRelationUpdates(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ReadInfo* info);

void
DK_ObjectHomeAdmin_us_clearAllRelationsToDeletedObjects(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ReadInfo* info);

/* NOT IN DESIGN */
DK_ObjectHomeAdmin*
DK_ObjectHomeAdmin_us_getRelatedHome(
    DK_ObjectHomeAdmin* _this,
    LOC_unsigned_long index);

/* NOT IN DESIGN */
DK_ObjectWriter*
DK_ObjectHomeAdmin_us_getObjectWriter(
    DK_ObjectHomeAdmin* _this);

void
DK_ObjectHomeAdmin_us_updateSelections(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData);


#if defined (__cplusplus)
}
#endif

#endif /* DLRL_KERNEL_OBJECT_HOME_ADMIN_H */
