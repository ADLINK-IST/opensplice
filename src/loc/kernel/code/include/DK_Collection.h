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
#ifndef DLRL_KERNEL_COLLECTION_H
#define DLRL_KERNEL_COLLECTION_H

/* DLRL util includes */
#include "DLRL_Exception.h"
#include "DLRL_Types.h"

/* collection includes */
#include "Coll_Set.h"

/* DLRL MetaModel includes */
#include "DMM_DLRLMultiRelation.h"

/* DLRL Kernel includes */
#include "DK_Entity.h"
#include "DK_ObjectAdmin.h"

#if defined (__cplusplus)
extern "C" {
#endif

/* The <code>DK_Collection</code> is the base class for all types of collections. It should be treated as an abstract
 * class, meaning it should never be instantiated.
 */
struct DK_Collection_s
{
    /* The base class of the <code>DK_Collection</code> class which manages the reference count.
     */
    DK_Entity entity;
    /* Each collection must always belong to an owner object. Unless this collection is a so-called unresolved
     * collection. In that case the owner pointer will be <code>NULL</code>. For each resolved collection it should
     * point to the corresponding owning <code>DK_ObjectAdmin</code> object. This owner is valid until the collection
     * is destroyed (IE ref count reaches zero). Some operations are dependant on this variable and require this
     * variable to have a value.
     */
    DK_ObjectAdmin* owner;
    /* The targetHome attribute points to the <code>DK_ObjectHomeAdmin</code> object that manages the target type
     * of this collection. This pointer should never be set to <code>NULL</code>, it may only become <code>NULL</code>
     * when all references to this collection are released and its about to be destroyed (memory freed). If it is
     * <code>NULL</code> in any other situation it will result in an error.
     * Take note that this pointer may point to the same home as the targetHome pointer.
     */
    DK_ObjectHomeAdmin* targetHome;
    /* The ownerHome attribute points to the <code>DK_ObjectHomeAdmin</code> object that manages the owner type
     * of this collection. This pointer should never be set to <code>NULL</code>, it may only become <code>NULL</code>
     * when all references to this collection are released and its about to be destroyed (memory freed). If it is
     * <code>NULL</code> in any other situation it will result in an error.
     * Take note that this pointer may point to the same home as the targetHome pointer.
     */
    DK_ObjectHomeAdmin* ownerHome;
    /* Each collection is based upon MetaModel information, this is represented by a <code>DMM_DLRLMultiRelation</code>
     * object. This object is always present, take note that MetaModel objects are not reference counted. They are owned
     * by the owner home, and will become invalid when the owning object home is deleted (not the same as destroyed)
     */
    DMM_DLRLMultiRelation* metaRelation;
    /* Each collection maintains a list of added elements during the current update round, this list can be accessed
     * at any time and is cleared each time an update round ends.
     */
    Coll_List addedElements;
    /* Each collection maintains a list of removed elements during the current update round, this list can be accessed
     * at any time and is cleared each time an update round ends.
     */
    Coll_List removedElements;
    /* The language specific representative of this <code>DK_Collection</code> object.
     */
    DLRL_LS_object ls_collection;
    /* Indicates whether this <code>DK_Collection</code> object is 'alive' (<code>TRUE</code>) or not
     * (<code>FALSE</code>). Objects that are no longer alive have been properly deleted and accessing operations
     * on these objects will raise <code>DLRL_ALREADY_DELETED</code> exceptions.
     */
    LOC_boolean alive;
    /* During update rounds it may happen that a target element of a collection cannot be resolved. In this case
     * the unresolved element becomes a part of an unresolved list within an <code>DK_ObjectReader</code> object.
     * A long value will help us to know if there are any unresolved elements waiting, and if so we can unregister
     * them first before deleting the collection. This prevents unneccesary iteration of unresolved list. It is also
     * used when we are managing elements for a collection which is in itself unresolved.
     */
    LOC_long nrOfUnresolvedElements;
    Coll_Set changedElements;/* NOT IN DESIGN */
    Coll_Set deletedElements;/* NOT IN DESIGN */
};

/* \brief Returns the owner <code>DK_ObjectAdmin</code> object of this collection.
 *
 * Preconditions:<ul>
 * <li>Must claim the (administrative) home lock on the <code>DK_Collection</code>'s owner home.</li>
 * <li>Must verify the <code>DK_Collection</code> is still alive as well as the owner home</li> </ul>
 *
 * \param _this The <code>DK_Collection</code> entity that is the target of this operation.
 *
 * \return The owner <code>DK_ObjectAdmin</code> object of this collection or <code>NULL</code> if this collection is
 * unresolved.
 */
DK_ObjectAdmin*
DK_Collection_us_getOwner(
    DK_Collection* _this);

/* \brief Returns the class ID of this collection as defined within the base entity class.
 *
 * Preconditions:<ul>
 * <li>Must claim the (administrative) home lock on the <code>DK_Collection</code>'s owner home.</li>
 * <li>Must verify the <code>DK_Collection</code> is still alive as well as the owner home</li></ul>
 *
 * \param _this The <code>DK_Collection</code> entity that is the target of this operation.
 *
 * \return The class ID of this collection as defined within the base entity class.
 */
DK_Class
DK_Collection_us_getClassID(
    DK_Collection* _this);

/* \brief Returns the DLRL MetaModel object that describes this collection within the MetaModel maintained by the
 * owner object home object.
 *
 * Preconditions:<ul>
 * <li>Must claim the (administrative) home lock on the <code>DK_Collection</code>'s owner home.</li>
 * <li>Must verify the <code>DK_Collection</code> is still alive as well as the owner home</li></ul>
 *
 * \param _this The <code>DK_Collection</code> entity that is the target of this operation.
 *
 * \return The DLRL MetaModel object that describes this collection within the MetaModel maintained by the owner object
 * home object.
 */
DMM_DLRLMultiRelation*
DK_Collection_us_getMetaRepresentative(
    DK_Collection* _this);

/* \brief Returns the value of the number of unresolved elements registered for this collection to the target object
 * home
 *
 * Preconditions:<ul>
 * <li>Must claim the (administrative) home lock on the <code>DK_Collection</code>'s owner home.</li>
 * <li>Must verify the <code>DK_Collection</code> is still alive as well as the owner home</li></ul>
 *
 * \param _this The <code>DK_Collection</code> entity that is the target of this operation.
 *
 * \return The value of the number of unresolved elements registered for this collection to the target object home
 */
LOC_long
DK_Collection_us_getNrOfUnresolvedElements(
    DK_Collection* _this);

/* \brief Registers a language specific object to this collection.
 *
 * Preconditions:<ul>
 * <li>Must claim the (administrative) home lock on the <code>DK_Collection</code>'s owner home.</li>
 * <li>Must verify the <code>DK_Collection</code> is still alive as well as the owner home</li></ul>
 *
 * \param _this The <code>DK_Collection</code> entity that is the target of this operation.
 * \param ls_collection The language specific collection pointer. (may be <code>NULL</code>
 */
void
DK_Collection_us_setLSObject(
    DK_Collection* _this,
    DLRL_LS_object ls_collection);

/* \Brief Increases the number of unresolved elements attribute by one.
 *
 * Preconditions:<ul>
 * <li>Must claim the (administrative) home lock on the <code>DK_Collection</code>'s owner home.</li>
 * <li>Must verify the <code>DK_Collection</code> is still alive as well as the owner home</li></ul>
 *
 * \param _this The <code>DK_Collection</code> entity that is the target of this operation.
 */
void
DK_Collection_us_increaseNrOfUnresolvedElements(
    DK_Collection* _this);

/* \Brief Decreases the number of unresolved elements attribute by one.
 *
 * Preconditions:<ul>
 * <li>Must claim the (administrative) home lock on the <code>DK_Collection</code>'s owner home.</li>
 * <li>Must verify the <code>DK_Collection</code> is still alive as well as the owner home</li></ul>
 *
 * \param _this The <code>DK_Collection</code> entity that is the target of this operation.
 */
void
DK_Collection_us_decreaseNrOfUnresolvedElements(
    DK_Collection* _this);

/* \Brief Registers the owner <code>DK_ObjectAdmin</code> object to this  <code>DK_Collection</code> object.
 *
 * No owner object may be known for this collection when setting it.
 *
 * Preconditions:<ul>
 * <li>Must claim the (administrative) home lock on the <code>DK_Collection</code>'s owner home.</li>
 * <li>Must verify the <code>DK_Collection</code> is still alive as well as the owner home</li></ul>
 *
 * \param _this The <code>DK_Collection</code> entity that is the target of this operation. May NOT be NIL
 */
void
DK_Collection_us_setOwner(
    DK_Collection* _this,
    DK_ObjectAdmin* owner);

/* \brief Locks the administrative mutex of the owner home
 *
 * \param _this The <code>DK_Collection</code> entity that is the target of this operation.
 */
void
DK_Collection_lockHome(
    DK_Collection* _this);

/* \brief Unlocks the administrative mutex of the owner home
 *
 * \param _this The <code>DK_Collection</code> entity that is the target of this operation.
 */
void
DK_Collection_unlockHome(
    DK_Collection* _this);

/* \brief An utility function that ensures the <code>DK_Collection</code> object is still alive.
 *
 * Preconditions:<ul>
 * <li>Must claim the (administrative) home lock on the <code>DK_Collection</code>'s owner home.</li></ul>
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_Collection</code> has already been deleted.</li>
 * </ul>
 *
 * \param _this The <code>DK_Collection</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 */
void
DK_Collection_us_checkAlive(
    DK_Collection* _this,
    DLRL_Exception* exception);

/* NOT IN DESIGN */
/* This operation may ONLY be used for a collection which has an owner object admin!! */
void
DK_Collection_us_checkAliveAllForChanges(
    DK_Collection* _this,
    DLRL_Exception* exception);

/* NOT IN DESIGN */
/* This operation may ONLY be used for a collection which has an owner object admin!! */
void
DK_Collection_unlockAllForChanges(
    DK_Collection* _this);

/* NOT IN DESIGN */
/* This operation may ONLY be used for a collection which has an owner object admin!! */
void
DK_Collection_lockAllForChanges(
    DK_Collection* _this,
    DLRL_Exception* exception);

/* NOT IN DESIGN */
void
DK_Collection_us_clear(
    DK_Collection* _this,
    DLRL_Exception* exception,
    void* userData,
    Coll_Set* objectHolders);

/* NOT IN DESIGN */
void
DK_Collection_us_removeElement(
    DK_Collection* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHolder* tmpHolder,
    Coll_Set* objectHolders);

/* NOT IN DESIGN */
void
DK_Collection_us_changeExistingElement(
    DK_Collection* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHolder* existingElement,
    DK_ObjectAdmin* objectAdmin,
    DK_CacheAccessAdmin* access);

/* NOT IN DESIGN */
LOC_boolean
DK_Collection_us_insertElement(
    DK_Collection* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHolder* holder,
    Coll_Set* objectHolders);

/* NOT IN DESIGN */
Coll_Set*
DK_Collection_us_getDeletedElements(
    DK_Collection* _this);

/* NOT IN DESIGN */
Coll_Set*
DK_Collection_us_getChangedElements(
    DK_Collection* _this);

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_KERNEL_COLLECTION_H */
