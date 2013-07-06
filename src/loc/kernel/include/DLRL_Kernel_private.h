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
/* \file kernel/include/DLRL_Kernel_private.h
 * \brief This file contains the complete DLRL protected API.
 *
 * All operations listed here should be viewed as 'protected' and used with caution. Unlike the operations listed in the
 * DLRL_Kernel.h file the operations listed here have be following characteristics:<ul>
 * <li> Operations are NOT threadsafe (I.E. the entity the operation is executed upon is not locked. Instead lock and
 * unlock operations are provided for each entity to achieve threadsafe access.</li>
 * <li> Operations MAY claim locks on other entities when and if needed. This will be mentioned in the documentation of
 * that operation </li>
 * <li> Operations will NOT check if the entity they are being executed upon is still alive, instead operations are
 * provided for each entity to check this. This should obviously be done after locking and before calling an operation.
 * Implicit access to other entities that may be needed will be checked ofcourse when and if applicable.
 * <li> Operations returning an object which is a subclass of <code>DK_Entity</code> do NOT have to be explicitly
 * released </li></ul>
 */
#ifndef DLRL_KERNEL_PRIVATE_H
#define DLRL_KERNEL_PRIVATE_H

#include "DLRL_Kernel.h"
#include "Coll_List.h"
#include "Coll_Set.h"

/* Following includes are not needed here due to most DK_MMFacade functions
 * being temporarily moved to the DLRL_Kernel.h file...
#include "DMM_KeyType.h"
#include "DMM_Basis.h"
#include "DMM_AttributeType.h"
#include "DMM_Mapping.h"
*/
#include "DK_CacheBridge.h"
#include "DK_CacheAccessBridge.h"
#include "DK_CollectionBridge.h"
#include "DK_DCPSUtilityBridge.h"
#include "DK_ObjectBridge.h"
#include "DK_ObjectHomeBridge.h"
#include "DK_ObjectReaderBridge.h"
#include "DK_ObjectRelationReaderBridge.h"
#include "DK_ObjectWriterBridge.h"
#include "DK_SelectionBridge.h"
#include "DK_UtilityBridge.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"
#include "u_instanceHandle.h"

#ifdef OSPL_BUILD_LOC_KERNEL
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

struct DK_ReadInfo_s {
    DLRL_Exception* exception;
    void* userData;
    void* dstInfo;
    void (*copyOut)(void* src, void* dest); /* NOT IN DESIGN  */
    Coll_List* keyFields;
    Coll_List* foreignKeyFields;
    Coll_List* validityFields;
    Coll_List dataSamples;
    c_long offset;
 /* NOT IN DESIGN   DK_ReadData* dataArray; */
/* NOT IN DESIGN LOC_unsigned_long dataArraySize; */
};



/**********************************************************************************************************************
************************************************** CacheFactoryAdmin **************************************************
**********************************************************************************************************************/

/* \brief Retrieves the language specific CacheFactory that is currently registered with the
 * <code>DK_CacheFactoryAdmin</code>
 *
 * NOTE: This function may ONLY be called after the <code>DK_CacheFactoryAdmin_ts_getInstance(...)</code> has been
 * called at least once. As that operation initializes the <code>DK_CacheFactoryAdmin</code> singleton.
 *
 * \return the associated language specific CacheFactory pointer.
 */
OS_API DLRL_LS_object
DK_CacheFactoryAdmin_us_getLSCacheFactory();


/* NOT IN DESIGN - all method cache get operations */
OS_API DK_CacheBridge*
DK_CacheFactoryAdmin_us_getCacheBridge();

OS_API DK_CacheAccessBridge*
DK_CacheFactoryAdmin_us_getCacheAccessBridge();

OS_API DK_CollectionBridge*
DK_CacheFactoryAdmin_us_getCollectionBridge();

OS_API DK_DCPSUtilityBridge*
DK_CacheFactoryAdmin_us_getDcpsUtilityBridge();

OS_API DK_ObjectBridge*
DK_CacheFactoryAdmin_us_getObjectBridge();

OS_API DK_ObjectHomeBridge*
DK_CacheFactoryAdmin_us_getObjectHomeBridge();

OS_API DK_ObjectReaderBridge*
DK_CacheFactoryAdmin_us_getObjectReaderBridge();

OS_API DK_ObjectRelationReaderBridge*
DK_CacheFactoryAdmin_us_getObjectRelationReaderBridge();

OS_API DK_ObjectWriterBridge*
DK_CacheFactoryAdmin_us_getObjectWriterBridge();

OS_API DK_SelectionBridge*
DK_CacheFactoryAdmin_us_getSelectionBridge();

OS_API DK_UtilityBridge*
DK_CacheFactoryAdmin_us_getUtilityBridge();

/**********************************************************************************************************************
****************************************************** CacheBase ******************************************************
**********************************************************************************************************************/
/* \brief Returns the language specific representative of this <code>DK_CacheBase</code> object.
 *
 * Preconditions:<ul>
 * <li>Must ensure thread safety</li>
 * <li>Must verify the <code>DK_CacheBase</code> is still alive.</li></ul>
 *
 * \param _this The <code>DK_CacheBase</code> entity that is the target of this operation
 *
 * \return The language specific representative of this <code>DK_CacheBase</code> object.
 */
OS_API DLRL_LS_object
DK_CacheBase_us_getLSCache(
    DK_CacheBase* _this);
/**********************************************************************************************************************
****************************************************** CacheAdmin *****************************************************
**********************************************************************************************************************/

/* \brief Claims the administrative lock of the <code>DK_CacheAdmin</code>.
 *
 * This lock is 'weaker' then the updates lock of the <code>DK_CacheAdmin</code>. I.E. You may not claim the updates
 * lock AFTER claiming the administrative lock. Vice versa is allowed.
 *
 * \param _this The <code>DK_CacheAdmin</code> entity that is the target of this operation.
 */
OS_API void
DK_CacheAdmin_lockAdministrative(
    DK_CacheAdmin* _this);

/* \brief Releases the administrative lock of the <code>DK_CacheAdmin</code>.
 *
 * \param _this The <code>DK_CacheAdmin</code> entity that is the target of this operation/
 */
OS_API void
DK_CacheAdmin_unlockAdministrative(
    DK_CacheAdmin* _this);

/* \brief Claims the updates lock of the <code>DK_CacheAdmin</code>.
 *
 * This lock is 'stronger' then the administrative lock of the <code>DK_CacheAdmin</code>. I.E. You may not claim the
 * administrative lock BEFORE claiming the updates lock. Vice versa is allowed.
 *
 * \param _this The <code>DK_CacheAdmin</code> entity that is the target of this operation.
 */
OS_API void
DK_CacheAdmin_lockUpdates(
    DK_CacheAdmin* _this);

/* \brief Releases the updates lock of the <code>DK_CacheAdmin</code>.
 *
 * \param _this The <code>DK_CacheAdmin</code> entity that is the target of this operation.
 */
OS_API void
DK_CacheAdmin_unlockUpdates(
    DK_CacheAdmin* _this);

/* \brief Retrieves all registered <code>DK_ObjectHomeAdmin</code> objects.
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_CacheAdmin</code>.</li>
 * <li>Must verify the <code>DK_CacheAdmin</code> is still alive.</li></ul>
 *
 * Postconditions:<ul>
 * <li>Do NOT have to free the returned list (or release its elements), as its a direct pointer to the list used by the
 * DLRL kernel.</li></ul>
 *
 * \param _this The <code>DK_CacheAdmin</code> entity that is the target of this operation.
 *
 * \return The sorted list of all registered <code>DK_ObjectHomeAdmin</code> objects. A home at index 0 in the list
 * will also have registration index 0.
 */
OS_API Coll_List*
DK_CacheAdmin_us_getHomes(
    DK_CacheAdmin* _this);

/* \brief Utility operation to verify if the <code>DK_CacheAdmin</code> entity is still alive.
 *
 * If the entity has been deleted already a <code>DLRL_ALREADY_DELETED</code> exception will be raised, indicating the
 * entity is no longer alive.
 * Preconditions:<ul>
 * <li>Must claim the administrative OR updates (either is fine) lock on the <code>DK_CacheAdmin</code>.</li></ul>
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_CacheAdmin</code> has already been deleted.</li>
 * </ul>
 *
 * \param _this The <code>DK_CacheAdmin</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 */
OS_API void
DK_CacheAdmin_us_checkAlive(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception);

/* \brief Returns the language specific participant of this <code>DK_CacheAdmin</code> object.
 *
 * The returned participant is a direct pointer to the participant maintained by the kernel. It should not be
 * altered.
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative OR update lock on the <code>DK_CacheAdmin</code> (either is fine, as this
 * returned attribute is only cleared during deletion of the <code>DK_CacheAdmin</code> object, for which both
 * mutexes are locked.</li>
 * <li>Must verify the <code>DK_CacheAdmin</code> is still alive.</li></ul>
 *
 * \param _this The <code>DK_CacheAdmin</code> entity that is the target of this operation
 *
 * \return The language specific participant of this <code>DK_CacheAdmin</code> object.
 */
OS_API DLRL_LS_object
DK_CacheAdmin_us_getLSParticipant(
    DK_CacheAdmin* _this);

/* \brief Retrieves the language specific DCPS publisher pointer.
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_CacheAdmin</code>.</li>
 * <li>Must verify the <code>DK_CacheAdmin</code> is still alive.</li></ul>
 *
 * \param _this The <code>DK_CacheAdmin</code> entity that is the target of this operation.
 *
 * \return The language specific DCPS publisher pointer.
 */
OS_API DLRL_LS_object
DK_CacheAdmin_us_getLSPublisher(
    DK_CacheAdmin* _this);

/* \brief Retrieves the language specific DCPS subscriber pointer.
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_CacheAdmin</code>.</li>
 * <li>Must verify the <code>DK_CacheAdmin</code> is still alive.</li></ul>
 *
 * \param _this The <code>DK_CacheAdmin</code> entity that is the target of this operation.
 *
 * \return The language specific DCPS subscriber pointer.
 */
OS_API DLRL_LS_object
DK_CacheAdmin_us_getLSSubscriber(
    DK_CacheAdmin* _this);

/* \brief Retrieves the set of attached listeners
 *
 * Preconditions:<ul>
 * <li>Must claim the update lock on the <code>DK_CacheAdmin</code>.</li>
 * <li>Must verify the <code>DK_CacheAdmin</code> is still alive.</li></ul>
 *
 * Postconditions:<ul>
 * <li>Do NOT have to free the returned set (or release its elements), as its a direct pointer to the list used by the
 * DLRL kernel.</li></ul>
 *
 * \return the set of attached listeners.
 */
OS_API Coll_Set*
DK_CacheAdmin_us_getListeners(
    DK_CacheAdmin* _this);

/* \brief Retrieves a registered <code>DK_ObjectHomeAdmin</code> based upon it's name.
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_CacheAdmin</code>.</li>
 * <li>Must verify the <code>DK_CacheAdmin</code> is still alive.</li></ul>
 *
 * Postconditions:<ul>
 * <li>Must release the returned <code>DK_ObjectHomeAdmin</code> pointer.</li></ul>
 *
 * \param _this The <code>DK_CacheAdmin</code> entity that is the target of this operation.
 *
 * \return The language specific DCPS subscriber pointer.
 */
OS_API DK_ObjectHomeAdmin*
DK_CacheAdmin_us_findHomeByName(
    DK_CacheAdmin* _this,
    LOC_const_string name);

OS_API Coll_Set*
DK_CacheAdmin_us_getAccesses(
    DK_CacheAdmin* _this);
/**********************************************************************************************************************
*************************************************** CacheAccessAdmin **************************************************
**********************************************************************************************************************/

/* \brief Claims the lock of the <code>DK_CacheAccessAdmin</code>.
 *
 * \param _this The <code>DK_CacheAccessAdmin</code> entity that is the target of this operation.
 */
OS_API void
DK_CacheAccessAdmin_lock(
    DK_CacheAccessAdmin* _this);
/* \brief Releases the lock of the <code>DK_CacheAccessAdmin</code>.
 *
 * \param _this The <code>DK_CacheAccessAdmin</code> entity that is the target of this operation.
 */
OS_API void
DK_CacheAccessAdmin_unlock(
    DK_CacheAccessAdmin* _this);

/* NOT IN DESIGN */
OS_API void
DK_CacheAccessAdmin_lockAll(
    DK_CacheAccessAdmin* _this);

/* NOT IN DESIGN */
OS_API void
DK_CacheAccessAdmin_unlockAll(
    DK_CacheAccessAdmin* _this);

/* \brief Utility operation to verify if the <code>DK_CacheAccessAdmin</code> entity is still alive.
 *
 * If the entity has been deleted already a <code>DLRL_ALREADY_DELETED</code> exception will be raised, indicating the
 * entity is no longer alive.
 * Preconditions:<ul>
 * <li>Must claim thelock on the <code>DK_CacheAccessAdmin</code>.</li></ul>
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_CacheAccessAdmin</code> has already been deleted.
 * </li></ul>
 *
 * \param _this The <code>DK_CacheAdmin</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 */
OS_API void
DK_CacheAccessAdmin_us_checkAlive(
    DK_CacheAccessAdmin* _this,
    DLRL_Exception* exception);

/* NOT IN DESIGN */
OS_API LOC_long*
DK_CacheAccessAdmin_us_getTypes(
    DK_CacheAccessAdmin* _this);

/* NOT IN DESIGN */
OS_API LOC_unsigned_long
DK_CacheAccessAdmin_us_getMaxTypes(
    DK_CacheAccessAdmin* _this);
/**********************************************************************************************************************
*************************************************** ObjectHomeAdmin ***************************************************
**********************************************************************************************************************/

/* \brief Claims the administrative lock of the <code>DK_ObjectHomeAdmin</code>.
 *
 * This lock is 'weaker' then the update lock of the <code>DK_ObjectHomeAdmin</code>. I.E. You may not claim the update
 * lock AFTER claiming the administrative lock. Vice versa is allowed.
 *
 * \param _this The <code>DK_ObjectHomeAdmin</code> entity that is the target of this operation.
 */
OS_API void
DK_ObjectHomeAdmin_lockAdmin(
    DK_ObjectHomeAdmin* _this);

/* \brief Releases the administrative lock of the <code>DK_ObjectHomeAdmin</code>.
 *
 * \param _this The <code>DK_ObjectHomeAdmin</code> entity that is the target of this operation.
 */
OS_API void
DK_ObjectHomeAdmin_unlockAdmin(
    DK_ObjectHomeAdmin* _this);
/* \brief Claims the updates lock of the <code>DK_ObjectHomeAdmin</code>.
 *
 * This lock is 'stronger' then the administrative lock of the <code>DK_ObjectHomeAdmin</code>. I.E. You may not claim
 * the administrative lock BEFORE claiming the update lock. Vice versa is allowed.
 *
 * \param _this The <code>DK_ObjectHomeAdmin</code> entity that is the target of this operation.
 */
OS_API void
DK_ObjectHomeAdmin_lockUpdate(
    DK_ObjectHomeAdmin* _this);
/* \brief Releases the update lock of the <code>DK_ObjectHomeAdmin</code>.
 *
 * \param _this The <code>DK_ObjectHomeAdmin</code> entity that is the target of this operation.
 */
OS_API void
DK_ObjectHomeAdmin_unlockUpdate(
    DK_ObjectHomeAdmin* _this);

/* \brief Utility operation to verify if the <code>DK_ObjectHomeAdmin</code> entity is still alive.
 *
 * If the entity has been deleted already a <code>DLRL_ALREADY_DELETED</code> exception will be raised, indicating the
 * entity is no longer alive.
 * Preconditions:<ul>
 * <li>Must claim the administrative OR update (either is fine) lock on the <code>DK_ObjectHomeAdmin</code>.</li></ul>
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_ObjectHomeAdmin</code> has already been deleted.
 * </li></ul>
 *
 * \param _this The <code>DK_ObjectHomeAdmin</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 */
OS_API void
DK_ObjectHomeAdmin_us_checkAlive(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception);

/* \brief Utility operation to verify if the <code>DK_ObjectHomeAdmin</code> entity is still alive.
 *
 * Returns a boolean stating whether the <code>DK_ObjectHomeAdmin</code> entity is still alive.
 * Preconditions:<ul>
 * <li>Must claim the administrative OR update (either is fine) lock on the <code>DK_ObjectHomeAdmin</code>.</li></ul>
 *
 * \param _this The <code>DK_ObjectHomeAdmin</code> entity that is the target of this operation.
 *
 * \return TRUE if the <code>DK_ObjectHomeAdmin</code> is still alive, FALSE otherwise.
 */
OS_API LOC_boolean
DK_ObjectHomeAdmin_us_isAlive(
    DK_ObjectHomeAdmin* _this);

/* \brief Retrieves the registration index of this <code>DK_ObjectHomeAdmin</code> under which it is registered
 * to the <code>CacheAdmin</code>.
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_ObjectHomeAdmin</code>.</li>
 * <li>Must verify the <code>DK_ObjectHomeAdmin</code> is still alive.</li></ul>
 *
 *  \param _this The <code>DK_ObjectHomeAdmin</code> entity that is the target of this operation.
 *
 * \return the index under which the <code>DK_ObjectHomeAdmin</code> is registered with the <code>CacheAdmin</code>
 * or -1 if it isnt registered to any <code>CacheAdmin</code>.
 */
OS_API LOC_long
DK_ObjectHomeAdmin_us_getRegistrationIndex(
    DK_ObjectHomeAdmin* _this);

/* \brief Retrieves the fully qualified name of the <code>DK_ObjectHomeAdmin</code>.
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_ObjectHomeAdmin</code>.</li>
 * <li>Must verify the <code>DK_ObjectHomeAdmin</code> is still alive.</li></ul>
 *
 * Postconditions:<ul>
 * <li>Must NOT free the returned string. Its a direct reference to the string used by the DLRL kernel.</li></ul>
 *
 * \param _this The <code>DK_ObjectHomeAdmin</code> entity that is the target of this operation.
 *
 * \return the fully qualified name of the <code>DK_ObjectHomeAdmin</code>.
 */
OS_API LOC_string
DK_ObjectHomeAdmin_us_getName(
    DK_ObjectHomeAdmin* _this);

/* \brief Retrieves the registered language specific representative of the <code>DK_ObjectHomeAdmin</code>.
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_ObjectHomeAdmin</code>.</li>
 * <li>Must verify the <code>DK_ObjectHomeAdmin</code> is still alive.</li></ul>
 *
 * Postconditions:<ul>
 * <li>Must NOT free/release the returned pointer. Its a direct reference to the pointer used by the DLRL kernel.</li>
 * </ul>
 *
 * \param _this The <code>DK_ObjectHomeAdmin</code> entity that is the target of this operation.
 *
 * \return the registered language specific representative of the <code>DK_ObjectHomeAdmin</code> or <code>NULL</code>
 * if none present .
 */
OS_API DLRL_LS_object
DK_ObjectHomeAdmin_us_getLSHome(
    DK_ObjectHomeAdmin* _this);

/* \brief Retrieves the registered user data of the <code>DK_ObjectHomeAdmin</code>.
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_ObjectHomeAdmin</code>.</li>
 * <li>Must verify the <code>DK_ObjectHomeAdmin</code> is still alive.</li></ul>
 *
 * Postconditions:<ul>
 * <li>Must NOT free/release the returned pointer. Its a direct reference to the pointer used by the DLRL kernel.</li>
 * </ul>
 *
 * \param _this The <code>DK_ObjectHomeAdmin</code> entity that is the target of this operation.
 *
 * \return the registered user dara of the <code>DK_ObjectHomeAdmin</code> or <code>NULL</code> if none present.
 */
OS_API void*
DK_ObjectHomeAdmin_us_getUserData(
    DK_ObjectHomeAdmin* _this);

/* \brief Retrieves the set of attached listeners of the <code>DK_ObjectHomeAdmin</code>.
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_ObjectHomeAdmin</code>.</li>
 * <li>Must verify the <code>DK_ObjectHomeAdmin</code> is still alive.</li></ul>
 *
 * Postconditions:<ul>
 * <li>Must NOT free/release the returned set or contained elements. Its a direct reference to the set used by the
 * DLRL kernel.</li></ul>
 *
 * \param _this The <code>DK_ObjectHomeAdmin</code> entity that is the target of this operation.
 *
 * \return the set of attached listeners of the <code>DK_ObjectHomeAdmin</code>.
 */
OS_API Coll_Set*
DK_ObjectHomeAdmin_us_getListeners(
    DK_ObjectHomeAdmin* _this);

/* \brief Retrieves the set of created selections of the <code>DK_ObjectHomeAdmin</code>.
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_ObjectHomeAdmin</code>.</li>
 * <li>Must verify the <code>DK_ObjectHomeAdmin</code> is still alive.</li></ul>
 *
 * Postconditions:<ul>
 * <li>Must NOT free/release the returned set or contained elements. Its a direct reference to the set used by the
 * DLRL kernel.</li></ul>
 *
 * \param _this The <code>DK_ObjectHomeAdmin</code> entity that is the target of this operation.
 *
 * \return the set of created selections of the <code>DK_ObjectHomeAdmin</code>.
 */
OS_API Coll_Set*
DK_ObjectHomeAdmin_us_getSelections(
    DK_ObjectHomeAdmin* _this);

/* \brief Retrieves the DCPS topic name to which the specified attribute is mapped.
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_ObjectHomeAdmin</code>.</li>
 * <li>Must verify the <code>DK_ObjectHomeAdmin</code> is still alive.</li></ul>
 *
 * Postconditions:<ul>
 * <li>Must NOT free the returned String. Its a direct reference to the String used by the DLRL kernel.</li></ul>
 *
 * \param _this The <code>DK_ObjectHomeAdmin</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param attributeName The name of the attribute for which the mapped topic name is wanted.
 *
 * \return <code>NULL</code> if an exception occured or  no topic was found for the specified attribute name. Otherwise
 * returns the DCPS topic name to which the specified attribute is mapped.
 */
OS_API LOC_string
DK_ObjectHomeAdmin_us_getTopicName(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    const char* attributeName);

/* \brief Retrieves the set of <code>DK_ObjectHomeAdmin</code> objects that manage child classes of the class managed
 * by this <code>DK_ObjectHomeAdmin</code>.
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_ObjectHomeAdmin</code>.</li>
 * <li>Must verify the <code>DK_ObjectHomeAdmin</code> is still alive.</li></ul>
 *
 * Postconditions:<ul>
 * <li>Must NOT free/release the returned set or contained elements. Its a direct reference to the set used by the
 * DLRL kernel.</li></ul>
 *
 * \param _this The <code>DK_ObjectHomeAdmin</code> entity that is the target of this operation.
 *
 * \return the set of <code>DK_ObjectHomeAdmin</code> objects that manage child classes of the class managed
 * by this <code>DK_ObjectHomeAdmin</code>.
 */
OS_API Coll_Set*
DK_ObjectHomeAdmin_us_getChildren(
    DK_ObjectHomeAdmin* _this);

/* \brief Retrieves the set of topic names that represent the topics onto which the class that is managed
 * by this <code>DK_ObjectHomeAdmin</code> is mapped.
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_ObjectHomeAdmin</code>.</li>
 * <li>Must verify the <code>DK_ObjectHomeAdmin</code> is still alive.</li></ul>
 *
 * Postconditions:<ul>
 * <li>Must NOT free the strings contained within the set. Each string is a direct reference to a string used by the
 * DLRL kernel.</li>
 * <li>Must free the returned set.</ul>
 *
 * \param _this The <code>DK_ObjectHomeAdmin</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 *
 * \return <code>NULL</code> if and only if an exception occured. Otherwise returns the set of topic names that
 * represent the topics onto which the class that is managed by this <code>DK_ObjectHomeAdmin</code> is mapped.
 */
OS_API Coll_Set*
DK_ObjectHomeAdmin_us_getAllTopicNames(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception);

/* \brief Retrieves the <code>DK_CacheAdmin</code> to which this <code>DK_ObjectHomeAdmin</code> is registered, if any.
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_ObjectHomeAdmin</code>.</li>
 * <li>Must verify the <code>DK_ObjectHomeAdmin</code> is still alive.</li></ul>
 *
 * Postconditions:<ul>
 * <li>Must NOT release the returned <code>DK_CacheAdmin</code>. No explicit duplicate is done.</li></ul>
 *
 * \param _this The <code>DK_ObjectHomeAdmin</code> entity that is the target of this operation.
 *
 * \return <code>NULL</code> if no <code>DK_CacheAdmin</code> is known. Otherwise returns the <code>DK_CacheAdmin</code>
 * to which this <code>DK_ObjectHomeAdmin</code> is registered.
 */
OS_API DK_CacheAdmin*
DK_ObjectHomeAdmin_us_getCache(
    DK_ObjectHomeAdmin* _this);

/* \brief the list of objects that were newly created during the last update round.
 *
 * The list is cleared each time an update round ends.
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_ObjectHomeAdmin</code>.</li>
 * <li>Must verify the <code>DK_ObjectHomeAdmin</code> is still alive.</li></ul>
 *
 * Postconditions:<ul>
 * <li>Must NOT free/release the returned list or contained elements. Its a direct reference to the list used by the
 * DLRL kernel.</li></ul>
 *
 * \param _this The <code>DK_ObjectHomeAdmin</code> entity that is the target of this operation.
 *
 * \return <code>NULL</code> if the <code>DK_ObjectHomeAdmin</code> is not attached to a <code>DK_CacheAdmin</code> or
 * is attached to a <code>DK_CacheAdmin</code> which still is in pub/sub state with value
 * <code>DK_PUB_SUB_STATE_INITIAL</code>. Otherwise returns the list of objects that were newly created during the last
 * update round.
 */
OS_API Coll_List*
DK_ObjectHomeAdmin_us_getNewObjects(
    DK_ObjectHomeAdmin* _this);

/* \brief the list of objects that were modified during the last update round.
 *
 * The list is cleared each time an update round ends.
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_ObjectHomeAdmin</code>.</li>
 * <li>Must verify the <code>DK_ObjectHomeAdmin</code> is still alive.</li></ul>
 *
 * Postconditions:<ul>
 * <li>Must NOT free/release the returned list or contained elements. Its a direct reference to the list used by the
 * DLRL kernel.</li></ul>
 *
 * \param _this The <code>DK_ObjectHomeAdmin</code> entity that is the target of this operation.
 *
 * \return <code>NULL</code> if the <code>DK_ObjectHomeAdmin</code> is not attached to a <code>DK_CacheAdmin</code> or
 * is attached to a <code>DK_CacheAdmin</code> which still is in pub/sub state with value
 * <code>DK_PUB_SUB_STATE_INITIAL</code>. Otherwise returns the list of objects that were modified during the last
 * update round.
 */
OS_API Coll_List*
DK_ObjectHomeAdmin_us_getModifiedObjects(
    DK_ObjectHomeAdmin* _this);

/* \brief the list of objects that were deleted during the last update round.
 *
 * The list is cleared each time an update round ends. Objects contained within this list will then be deleted and are
 * thus no longer alive once they are deleted.
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_ObjectHomeAdmin</code>.</li>
 * <li>Must verify the <code>DK_ObjectHomeAdmin</code> is still alive.</li></ul>
 *
 * Postconditions:<ul>
 * <li>Must NOT free/release the returned list or contained elements. Its a direct reference to the list used by the
 * DLRL kernel.</li></ul>
 *
 * \param _this The <code>DK_ObjectHomeAdmin</code> entity that is the target of this operation.
 *
 * \return <code>NULL</code> if the <code>DK_ObjectHomeAdmin</code> is not attached to a <code>DK_CacheAdmin</code> or
 * is attached to a <code>DK_CacheAdmin</code> which still is in pub/sub state with value
 * <code>DK_PUB_SUB_STATE_INITIAL</code>. Otherwise returns the list of objects that were deleted during the last
 * update round.
 */
OS_API Coll_List*
DK_ObjectHomeAdmin_us_getDeletedObjects(
    DK_ObjectHomeAdmin* _this);

/* \brief Retrieves all objects attached to the specified <code>DK_ObjectHomeAdmin</code>.
 *
 * Objects that are contained within the list returned by the <code>DK_ObjectHomeAdmin_us_getDeletedObjects(...)</code>
 * operation are NOT contained within the objects collection retrieved by this operation. If an exception occurs then
 * the <code>DK_ObjectArrayHolder</code> parameters's attributes will all be set to NULL. If the maxSize attribute of
 * the <code>DK_ObjectArrayHolder</code> is 0 then the objectArray attribute will be <code>NULL</code>.
----------------TODO ID: 182 -- not yet implemented like this, they are still contained...----------------------
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_ObjectHomeAdmin</code>.</li>
 * <li>Must verify the <code>DK_ObjectHomeAdmin</code> is still alive.</li>
 * <li>The <code>DK_ObjectArrayHolder</code> must be initialized correctly. IE all attributes must be <code>NULL</code>
 * or 0. All values will be reset and overridden!</li></ul>
 *
 * Postconditions:<ul>
 * <li>Must free the <code>DK_ObjectArrayHolder</code> objectArray. But must NOT release the <code>DK_ObjectAdmin</code>
 * objects contained within the array.</li></ul>
 *
 * \param _this The <code>DK_ObjectHomeAdmin</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param holder A holder for the array of ObjectAdmins, this will be filled by the DLRL and should be inited to NIL.
 */
OS_API void
DK_ObjectHomeAdmin_us_getAllObjects(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    DK_ObjectArrayHolder* holder);

/* NOT IN DESIGN */
OS_API DK_ObjectHomeAdmin*
DK_ObjectHomeAdmin_us_getParent(
    DK_ObjectHomeAdmin* _this);

/**********************************************************************************************************************
************************************************* ObjectReader/Writer *************************************************
**********************************************************************************************************************/

/* \brief Performs a read on the DCPS data reader and transforms new, modified and disposed samples into actions for the
 * corresponding objects.
 *
 * Object reconstruction is achieved by this operation. Any relations of objects are also resolved.
 * The new/modified/deleted objects getters of the ObjectHome will reflect the changes made during this operation.
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_ObjectHomeAdmin</code>.</li>
 * <li>Must ensure all required fields of the <code>DK_ReadInfo</code> are set correctly.</li>
 * <li>Must verify the <code>DK_ObjectHomeAdmin</code> is still alive.</li></ul>
 *
 * \param _this The <code>DK_ObjectReader</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param readInfo User data partially filled by the DLRL kernel and other parts by the language binding on
 * top of the kernel, as needed.
 */
OS_API void
DK_ObjectReader_us_doRead(
    DK_ObjectReader* _this,
    DLRL_Exception* exception,
    DK_ReadInfo* readInfo);

/* \brief Retrieves the language specific DCPS data reader pointer associated with this <code>DK_ObjectReader</code>.
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_ObjectHomeAdmin</code>.</li>
 * <li>Must verify the <code>DK_ObjectHomeAdmin</code> is still alive.</li></ul>
 *
 * \param _this The <code>DK_ObjectReader</code> entity that is the target of this operation.
 *
 * \return the language specific DCPS data reader pointer associated with this <code>DK_ObjectReader</code>.
 */
OS_API DLRL_LS_object
DK_ObjectReader_us_getLSReader(
    DK_ObjectReader* _this);

OS_API DK_TopicInfo*
DK_ObjectReader_us_getTopicInfo(
    DK_ObjectReader* _this);

/* \brief Retrieves the language specific DCPS data writer pointer associated with this <code>DK_ObjectWriter</code>.
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_ObjectHomeAdmin</code>.</li>
 * <li>Must verify the <code>DK_ObjectHomeAdmin</code> is still alive.</li></ul>
 *
 * \param _this The <code>DK_ObjectWriter</code> entity that is the target of this operation.
 *
 * \return the language specific DCPS data writer pointer associated with this <code>DK_ObjectWriter</code>.
 */
OS_API DLRL_LS_object
DK_ObjectWriter_us_getLSWriter(
    DK_ObjectWriter* _this);

/* NOT IN DESIGN */
OS_API DK_TopicInfo*
DK_ObjectWriter_us_getTopicInfo(
    DK_ObjectWriter* _this);

/* NOT IN DESIGN */
OS_API u_writer
DK_ObjectWriter_us_getWriter(
    DK_ObjectWriter* _this);

/**********************************************************************************************************************
*************************************************** Meta model facade *************************************************
****************************************** temporarily moved to the DLRL_Kernel.h ************************************/

/* \brief Return the string names of all single (mono) relations known within a specific type.
 *
 * The string elements within the list are only valid while the administrative lock on the
 * <code>DK_ObjectHomeAdmin</code> is claimed.
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_ObjectHomeAdmin</code>. (see description)</li>
 * <li>Must verify the <code>DK_ObjectHomeAdmin</code> is still alive. (see description)</li>
 * <li>Must have created a DLRLClass object first for the specified <code>DK_ObjectHomeAdmin</code> by calling the
 * <code>DK_MMFacade_us_createDLRLClass(...)</code> operation first</li></ul>
 *
 * Postconditions:<ul>
 * <li>Must free the returned list (if not <code>NULL</code>), but not the values of the contained elements.</li></ul>
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_OUT_OF_MEMORY</code> - If the system ran out of resources to when allocating the list or its elements
 * </li></ul>
 *
 * \param home The <code>DK_ObjectHomeAdmin</code> entity that is the root of the MetaModel information.
 * \param exception If an exception occurs the values in this struct will be modified.
 *
 * \return A list containing zero or more Strings or NULL if and only if an exception occured.
 */
OS_API Coll_List*
DK_MMFacade_us_getSingleRelationNames(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception);

/* \brief Return the string names of all multi relations known within a specific type.
 *
 * The string elements within the list are only valid while the administrative lock on the
 * <code>DK_ObjectHomeAdmin</code> is claimed.
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_ObjectHomeAdmin</code>. (see description)</li>
 * <li>Must verify the <code>DK_ObjectHomeAdmin</code> is still alive. (see description)</li>
 * <li>Must have created a DLRLClass object first for the specified <code>DK_ObjectHomeAdmin</code> by calling the
 * <code>DK_MMFacade_us_createDLRLClass(...)</code> operation first</li></ul>
 *
 * Postconditions:<ul>
 * <li>Must free the returned list (if not <code>NULL</code>), but not the values of the contained elements.</li></ul>
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_OUT_OF_MEMORY</code> - If the system ran out of resources to when allocating the list or its elements
 * </li></ul>
 *
 * \param home The <code>DK_ObjectHomeAdmin</code> entity that is the root of the MetaModel information.
 * \param exception If an exception occurs the values in this struct will be modified.
 *
 * \return A list containing zero or more Strings or NULL if and only if an exception occured.
 */
OS_API Coll_List*
DK_MMFacade_us_getMultiRelationNames(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception);

/**********************************************************************************************************************
***************************************************** ObjectAdmin *****************************************************
**********************************************************************************************************************/
/* \brief Claims the administrative lock of the <code>DK_ObjectHomeAdmin</code>.
 *
 * This lock is 'weaker' then the update lock of the <code>DK_ObjectHomeAdmin</code>. I.E. You may not claim the update
 * lock AFTER claiming the administrative lock. Vice versa is allowed.
 *
 * \param _this The <code>DK_ObjectAdmin</code> entity that is the target of this operation.
 */
OS_API void
DK_ObjectAdmin_lockHome(
    DK_ObjectAdmin* _this);

/* \brief Releases the administrative lock of the <code>DK_ObjectHomeAdmin</code>.
 *
 * \param _this The <code>DK_ObjectAdmin</code> entity that is the target of this operation.
 */
OS_API void
DK_ObjectAdmin_unlockHome(
    DK_ObjectAdmin* _this);

/* \brief Utility operation to verify if the <code>DK_ObjectAdmin</code> entity is still alive.
 *
 * If the entity has been deleted already a <code>DLRL_ALREADY_DELETED</code> exception will be raised, indicating the
 * entity is no longer alive.
 *
 * Preconditions:<ul>
 * <li>Must claim the (administrative) home lock on the <code>DK_ObjectAdmin</code>'s home.</li></ul>
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_ObjectAdmin</code> has already been deleted.
 * </li></ul>
 *
 * \param _this The <code>DK_ObjectAdmin</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 */
OS_API void
DK_ObjectAdmin_us_checkAlive(
    DK_ObjectAdmin* _this,
    DLRL_Exception* exception);

/* \brief Retrieves the list of names of relations that are invalid. The caller
 * of this operation must delete the returned list (and thus pop back all elements).
 * But the elements itself do not have to be freed, as they are direct references to
 * strings maintained by the kernel.
 *
 * Preconditions:<ul>
 * <li>Must claim the (administrative) home lock on the <code>DK_ObjectAdmin</code>'s home.</li>
 * <li>Must verify the <code>DK_ObjectAdmin</code> is still alive.</li> </ul>
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_OUT_OF_MEMORY</code> - If resources ran out during the execution of this operation.
 * </li></ul>
 *
 * \param _this The <code>DK_ObjectAdmin</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 *
 * \return The list of names of relations that are invalid.
 */
/* NOT IN DESIGN */
OS_API Coll_List*
DK_ObjectAdmin_us_getInvalidRelations(
    DK_ObjectAdmin* _this,
    DLRL_Exception* exception);

/* \brief Retrieves the language specific representative of the <code>DK_ObjectAdmin</code>.
 *
 * Preconditions:<ul>
 * <li>Must claim the (administrative) home lock on the <code>DK_ObjectAdmin</code>'s home.</li>
 * <li>Must verify the <code>DK_ObjectAdmin</code> is still alive.</li> </ul>
 *
 * \param _this The <code>DK_ObjectAdmin</code> entity that is the target of this operation.
 *
 * \return The language specific representative of the <code>DK_ObjectAdmin</code>.
 */
OS_API DLRL_LS_object
DK_ObjectAdmin_us_getLSObject(
    DK_ObjectAdmin* _this);

/* NOT IN DESIGN */
OS_API u_instanceHandle
DK_ObjectAdmin_us_getHandle(
    DK_ObjectAdmin* _this);

/* NOT IN DESIGN */
/* no duplicate done */
OS_API DK_ObjectHomeAdmin*
DK_ObjectAdmin_us_getHome(
    DK_ObjectAdmin* _this);

/* NOT IN DESIGN */
OS_API DK_ObjectState
DK_ObjectAdmin_us_getWriteState(
    DK_ObjectAdmin* _this);

/* NOT IN DESIGN - removed DLRL_LS_object DK_ObjectAdmin_us_getOID(DK_ObjectAdmin* _this); */

/* not in design */
OS_API LOC_string
DK_ObjectAdmin_us_getMainTopicName(
    DK_ObjectAdmin* _this);

/* \brief Returns <code>TRUE</code> if the <code>DK_ObjectAdmin</code> is alive, <code>FALSE</code> otherwise.
 *
 * \param _this The <code>DK_ObjectAdmin</code> entity that is the target of this operation.
 *
 * \return <code>TRUE</code> if the <code>DK_ObjectAdmin</code> is alive, <code>FALSE</code> otherwise.
 */
OS_API LOC_boolean
DK_ObjectAdmin_us_isAlive(
    DK_ObjectAdmin* _this);
/**********************************************************************************************************************
***************************************************** Collection ******************************************************
**********************************************************************************************************************/
/* \brief Claims the admin lock on both the owner and the target <code>DK_ObjectHomeAdmin</code> objects belonging to
 * this <code>DK_Collection</code> object.
 *
 * This operation is a utility function that ensures no deadlock can occur by following the locking strategy
 * required when locking multiple homes.
 *
 * \param _this The <code>DK_Collection</code> entity that is the target of this operation.
 */
OS_API void
DK_Collection_lockAll(
    DK_Collection* _this);

/* \brief Releases the admin lock on both the owner and the target <code>DK_ObjectHomeAdmin</code> objects belonging to
 * this <code>DK_Collection</code> object.
 *
 * This operation is a utility function that ensures no deadlock can occur by following the locking strategy
 * required when locking multiple homes.
 *
 * \param _this The <code>DK_Collection</code> entity that is the target of this operation.
 */
OS_API void
DK_Collection_unlockAll(
    DK_Collection* _this);

/* \brief An utility function that ensures the collection object and its owner and target <code>DK_ObjectHomeAdmin</code>
* objects are still alive.
 *
 * Preconditions:<ul>
 * <li>Must claim the (administrative) home lock on the <code>DK_Collection</code>'s owner and target homes.</li></ul>
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_Collection</code> has already been deleted. Or if
 * its owner or target <code>DK_ObjectHomeAdmin</code> objects have already been deleted.</li></ul>
 *
 * \param _this The <code>DK_Collection</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 */
OS_API void
DK_Collection_us_checkAliveAll(
    DK_Collection* _this,
    DLRL_Exception* exception);

/* \brief Retrieves the list of removed elements from the collection in the last update round
 *
 * The returned list is a direct reference to the list used within the kernel, it therefore does not have to be
 * freed and will only be valid while the collection (thus the owner/target home) is locked. This list contains
 * <code>DK_ObjectHolder</code> objects.
 *
 * Preconditions:<ul>
 * <li>Must claim the (administrative) home lock on the <code>DK_Collection</code>'s owner and target homes.</li>
 * <li>Must verify the <code>DK_Collection</code> is still alive as well as the owner and target homes</li> </ul>
 *
 * \param _this The <code>DK_Collection</code> entity that is the target of this operation.
 *
 * \return The list of removed elements from the collection in the last update round
 */
/* This operation may ONLY be used for a collection which has an owner object admin!! */
OS_API Coll_List*
DK_Collection_us_getRemovedElements(
    DK_Collection* _this);

/* \brief Retrieves the language specific representative of the <code>DK_Collection</code>.
 *
 * Preconditions:<ul>
 * <li>Must claim the (administrative) home lock on the <code>DK_Collection</code>'s owner and target homes.</li>
 * <li>Must verify the <code>DK_Collection</code> is still alive as well as the owner and target homes</li> </ul>
 *
 * \param _this The <code>DK_Collection</code> entity that is the target of this operation.
 *
 * \return The language specific representative of the <code>DK_Collection</code>.
 */
OS_API DLRL_LS_object
DK_Collection_us_getLSObject(
    DK_Collection* _this);

/* \brief Retrieves the target <code>DK_ObjectHomeAdmin</code> of this <code>DK_Collection</code> object.
 *
 * The pointer returned should not be released, as it isnt duplicate. Its just a direct reference to the
 * pointer used by the kernel and is only valid within the lock on the collections homes.
 *
 * Preconditions:<ul>
 * <li>Must claim the (administrative) home lock on the <code>DK_Collection</code>'s owner and target homes.</li>
 * <li>Must verify the <code>DK_Collection</code> is still alive as well as the owner and target homes</li> </ul>
 *
 * \param _this The <code>DK_Collection</code> entity that is the target of this operation.
 *
 * \return The target <code>DK_ObjectHomeAdmin</code> of this <code>DK_Collection</code> object.
 */
OS_API DK_ObjectHomeAdmin*
DK_Collection_us_getTargetHome(
    DK_Collection* _this);

/**********************************************************************************************************************
********************************************* Set (extends Collection) ************************************************
**********************************************************************************************************************/

/* \brief Returns a set containing all <code>DK_ObjectHolders</code> objects that are a part of this
 * <code>DK_SetAdmin</code>.
 *
 * The returned set does not have to be freed, nor do its elements. Its a direct reference to the set used by the kernel
 * and it is therefore only valid within the context of a lock.
 *
 * Preconditions:<ul>
 * <li>Must claim the (administrative) home lock on the <code>DK_SetAdmin</code>'s owner and target homes.</li>
 * <li>Must verify the <code>DK_SetAdmin</code> is still alive as well as the owner and target homes</li> </ul>
 *
 * \param _this The <code>DK_SetAdmin</code> entity that is the target of this operation.
 *
 * \return A set containing all <code>DK_ObjectHolders</code> objects that are a part of this <code>DK_SetAdmin</code>.
 */
OS_API Coll_Set*
DK_SetAdmin_us_getHolders(
    DK_SetAdmin* _this);

/* \brief Returns a list containing all <code>DK_ObjectHolders</code> objects that have become a part of this
 * <code>DK_SetAdmin</code> in the last update round.
 *
 * The returned list does not have to be freed, nor do its elements. Its a direct reference to the list used by the
 * kernel and it is therefore only valid within the context of a lock.
 * This operation may ONLY be used for a collection which has an owner object admin.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_OUT_OF_MEMORY</code> - If no more memory can be allocated to copy elements into the list (which the
 * caller does not have to free!)</li></ul>
 *
 * Preconditions:<ul>
 * <li>Must claim the (administrative) home lock on the <code>DK_SetAdmin</code>'s owner and target homes.</li>
 * <li>Must verify the <code>DK_SetAdmin</code> is still alive as well as the owner and target homes</li> </ul>
 *
 * \param _this The <code>DK_SetAdmin</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 *
 * \return A list containing all <code>DK_ObjectHolders</code> objects that have become a part of this
 * <code>DK_SetAdmin</code> in the last update round.
 */
OS_API Coll_List*
DK_SetAdmin_us_getAddedElements(
    DK_SetAdmin* _this,
    DLRL_Exception* exception);

/**********************************************************************************************************************
********************************************* Map (extends Collection) ************************************************
**********************************************************************************************************************/

/* \brief Returns a set containing all <code>DK_ObjectHolders</code> objects that are a part of this
 * <code>DK_MapAdmin</code>.
 *
 * The returned set does not have to be freed, nor do its elements. Its a direct reference to the set used by the kernel
 * and it is therefore only valid within the context of a lock.
 *
 * Preconditions:<ul>
 * <li>Must claim the (administrative) home lock on the <code>DK_MapAdmin</code>'s owner and target homes.</li>
 * <li>Must verify the <code>DK_MapAdmin</code> is still alive as well as the owner and target homes</li> </ul>
 *
 * \param _this The <code>DK_MapAdmin</code> entity that is the target of this operation.
 *
 * \return A set containing all <code>DK_ObjectHolders</code> objects that are a part of this <code>DK_MapAdmin</code>.
 */
OS_API Coll_Set*
DK_MapAdmin_us_getObjectHolders(
    DK_MapAdmin* _this);

/* \brief Returns a list containing all <code>DK_ObjectHolders</code> objects that have modified within the
 * <code>DK_MapAdmin</code> in the last update round.
 *
 * The returned list does not have to be freed, nor do its elements. Its a direct reference to the list used by the
 * kernel and it is therefore only valid within the context of a lock.
 * This operation may ONLY be used for a collection which has an owner object admin.
 *
 * Preconditions:<ul>
 * <li>Must claim the (administrative) home lock on the <code>DK_MapAdmin</code>'s owner and target homes.</li>
 * <li>Must verify the <code>DK_MapAdmin</code> is still alive as well as the owner and target homes</li> </ul>
 *
 * \param _this The <code>DK_MapAdmin</code> entity that is the target of this operation.
 *
 * \return A list containing all <code>DK_ObjectHolders</code> objects that have modified within the
 * <code>DK_MapAdmin</code> in the last update round.
 */
OS_API Coll_List*
DK_MapAdmin_us_getModifiedElements(
    DK_MapAdmin* _this);

/* \brief Returns a list containing all <code>DK_ObjectHolders</code> objects that have become a part of this
 * <code>DK_MapAdmin</code> in the last update round.
 *
 * The returned list does not have to be freed, nor do its elements. Its a direct reference to the list used by the
 * kernel and it is therefore only valid within the context of a lock.
 * This operation may ONLY be used for a collection which has an owner object admin.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_OUT_OF_MEMORY</code> - If no more memory can be allocated to copy elements into the list (which the
 * caller does not have to free!)</li></ul>
 *
 * Preconditions:<ul>
 * <li>Must claim the (administrative) home lock on the <code>DK_MapAdmin</code>'s owner and target homes.</li>
 * <li>Must verify the <code>DK_MapAdmin</code> is still alive as well as the owner and target homes</li> </ul>
 *
 * \param _this The <code>DK_MapAdmin</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 *
 * \return A list containing all <code>DK_ObjectHolders</code> objects that have become a part of this
 * <code>DK_MapAdmin</code> in the last update round.
 */
OS_API Coll_List*
DK_MapAdmin_us_getAddedElements(
    DK_MapAdmin* _this,
    DLRL_Exception* exception);

/**********************************************************************************************************************
**************************************************** ObjectHolder *****************************************************
**********************************************************************************************************************/
/* \brief Returns <code>TRUE</code> if the <code>DK_ObjectHolder</code> is succesfully resolved, <code>FALSE</code>
 * otherwise.
 *
 * Preconditions:<ul>
 * <li>Must ensure threadsafe access. The <code>DK_ObjectHolder</code> has no facilities to accomplish this, therefore
 * the 'owner' of the <code>DK_ObjectHolder</code> should be consulted to accomplish threadsafe access.</li>
 * <li>If applicable to the owner, must verify the owner is still alive. The <code>DK_ObjectHolder</code> does not
 * support this facility.</li></ul>
 *
 * \param _this The <code>DK_ObjectHolder</code> struct that is the target of this operation.
 *
 * \return <code>TRUE</code> if the <code>DK_ObjectHolder</code> is succesfully resolved, <code>FALSE</code> otherwise.
 */
OS_API LOC_boolean
DK_ObjectHolder_us_isResolved(
    DK_ObjectHolder* _this);

/* \brief Returns the <code>DK_ObjectAdmin</code> object that is the target of this holder, if any.
 *
 * Preconditions:<ul>
 * <li>Must ensure threadsafe access. The <code>DK_ObjectHolder</code> has no facilities to accomplish this, therefore
 * the 'owner' of the <code>DK_ObjectHolder</code> should be consulted to accomplish threadsafe access.</li>
 * <li>If applicable to the owner, must verify the owner is still alive. The <code>DK_ObjectHolder</code> does not
 * support this facility.</li></ul>
 *
 * \param _this The <code>DK_ObjectHolder</code> struct that is the target of this operation.
 *
 * \return The <code>DK_ObjectAdmin</code> object that is the target of this holder or <code>NULL</code> if no target
 * is available for this holder.
 */
OS_API DK_ObjectAdmin*
DK_ObjectHolder_us_getTarget(
    DK_ObjectHolder* _this);

/* \brief Retrieves the userdata that is registered with this holder, if any.
 *
 * Preconditions:<ul>
 * <li>Must ensure threadsafe access. The <code>DK_ObjectHolder</code> has no facilities to accomplish this, therefore
 * the 'owner' of the <code>DK_ObjectHolder</code> should be consulted to accomplish threadsafe access.</li>
 * <li>If applicable to the owner, must verify the owner is still alive. The <code>DK_ObjectHolder</code> does not
 * support this facility.</li></ul>
 *
 * \param _this The <code>DK_ObjectHolder</code> struct that is the target of this operation.
 *
 * \return The userdata that is registered with this holder or <code>NULL</code> if no userdata is available for this
 * holder.
 */
OS_API void*
DK_ObjectHolder_us_getUserData(
    DK_ObjectHolder* _this);
/**********************************************************************************************************************
***************************************************** Selection ******************************************************
**********************************************************************************************************************/
/* \brief Claims the admin lock on the <code>DK_ObjectHomeAdmin</code> object belonging to
* this <code>DK_SelectionAdmin</code> object.
 *
 * \param _this The <code>DK_SelectionAdmin</code> entity that is the target of this operation.
 */
OS_API void
DK_SelectionAdmin_lockHome(
    DK_SelectionAdmin* _this);

/* \brief Releases the admin lock on the <code>DK_ObjectHomeAdmin</code> object belonging to
* this <code>DK_SelectionAdmin</code> object.
 *
 * \param _this The <code>DK_SelectionAdmin</code> entity that is the target of this operation.
 */
OS_API void
DK_SelectionAdmin_unlockHome(
    DK_SelectionAdmin* _this);

/* \brief An utility function that ensures the <code>DK_SelectionAdmin</code> object is still alive.
 *
 * Preconditions:<ul>
 * <li>Must claim the (administrative) home lock on the <code>DK_SelectionAdmin</code>'s owner home.</li></ul>
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_SelectionAdmin</code> has already been deleted.
 * </li></ul>
 *
 * \param _this The <code>DK_SelectionAdmin</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 */
OS_API void
DK_SelectionAdmin_us_checkAlive(
    DK_SelectionAdmin* _this,
    DLRL_Exception* exception);

/* \brief Retrieves the language specific representative of this <code>DK_SelectionAdmin</code>.
 *
 * Preconditions:<ul>
 * <li>Must claim the (administrative) home lock on the <code>DK_SelectionAdmin</code>'s owner home.</li>
 * <li>Must verify the <code>DK_SelectionAdmin</code> is still alive</li></ul>
 *
 * \param _this The <code>DK_SelectionAdmin</code> entity that is the target of this operation.
 *
 * \return The language specific representative of this <code>DK_SelectionAdmin</code>.
 */
OS_API DLRL_LS_object
DK_SelectionAdmin_us_getLSSelection(
    DK_SelectionAdmin* _this);

/* \brief Returns a list containing all <code>DK_ObjectAdmin</code> objects that are a part of the selection.
 *
 * The list is a direct reference to the list maintained by the kernel and is only valid in an encapsulating lock.
 * In that light it should not be freed or released in any way
 *
 * Preconditions:<ul>
 * <li>Must claim the (administrative) home lock on the <code>DK_SelectionAdmin</code>'s owner home.</li>
 * <li>Must verify the <code>DK_SelectionAdmin</code> is still alive</li></ul>
 *
 * \param _this The <code>DK_SelectionAdmin</code> entity that is the target of this operation.
 *
 * \return A list containing all <code>DK_ObjectAdmin</code> objects that are a part of the selection.
 */
OS_API Coll_Set*
DK_SelectionAdmin_us_getMembers(
    DK_SelectionAdmin* _this);

OS_API Coll_List*
DK_SelectionAdmin_us_getInsertedMembers(
    DK_SelectionAdmin* _this);

OS_API Coll_List*
DK_SelectionAdmin_us_getModifiedMembers(
    DK_SelectionAdmin* _this);

OS_API Coll_List*
DK_SelectionAdmin_us_getRemovedMembers(
    DK_SelectionAdmin* _this);

OS_API DK_CriterionKind
DK_SelectionAdmin_us_getCriterionKind(
    DK_SelectionAdmin* _this);

OS_API DLRL_LS_object
DK_SelectionAdmin_us_getLSFilter(
    DK_SelectionAdmin* _this);

/* \brief Returns the <code>DK_ObjectHomeAdmin</code> object hat is the owner of this <code>DK_SelectionAdmin</code>.
 *
 * The returned home does not have to be released and is only valid in an encapsulating lock.
 *
 * Preconditions:<ul>
 * <li>Must claim the (administrative) home lock on the <code>DK_SelectionAdmin</code>'s owner home.</li>
 * <li>Must verify the <code>DK_SelectionAdmin</code> is still alive</li></ul>
 *
 * \param _this The <code>DK_SelectionAdmin</code> entity that is the target of this operation.
 *
 * \return The <code>DK_ObjectHomeAdmin</code> object hat is the owner of this <code>DK_SelectionAdmin</code>.
 */
OS_API DK_ObjectHomeAdmin*
DK_SelectionAdmin_us_getOwnerHome(
    DK_SelectionAdmin* _this);

/**********************************************************************************************************************
**************************************************** TopicInfo *****************************************************
**********************************************************************************************************************/

OS_API DLRL_LS_object
DK_TopicInfo_us_getLSTopic(
    DK_TopicInfo* _this);

OS_API void*
DK_TopicInfo_us_getTopicUserData(
    DK_TopicInfo* _this);

OS_API DK_ObjectHomeAdmin*
DK_TopicInfo_us_getOwner(
    DK_TopicInfo* _this);

OS_API LOC_string
DK_TopicInfo_us_getTopicName(
    DK_TopicInfo* _this);

OS_API u_entity
DK_DCPSUtility_ts_createProxyUserEntity(
    DLRL_Exception* exception,
    u_entity entity);


#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_KERNEL_PRIVATE_H */
