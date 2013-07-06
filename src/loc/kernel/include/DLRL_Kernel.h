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
#ifndef DLRL_KERNEL_H
#define DLRL_KERNEL_H

/* Include the DLRL Exception header file definitions */
#include "DLRL_Exception.h"
/* following includes are temporarily listed here because the DK_MMFacade
 * functions were moved to this file
 */
#include "DMM_KeyType.h"
#include "DMM_Basis.h"
#include "DMM_AttributeType.h"
#include "DMM_Mapping.h"

/* For C++ compatibility */
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

/* forward declaration of objects */
struct DK_CacheBase_s;
typedef struct DK_CacheBase_s DK_CacheBase;

struct DK_CacheAdmin_s;
typedef struct DK_CacheAdmin_s DK_CacheAdmin;

struct DK_ObjectHomeAdmin_s;
typedef struct DK_ObjectHomeAdmin_s DK_ObjectHomeAdmin;

struct DK_CacheFactoryAdmin_s;
typedef struct DK_CacheFactoryAdmin_s DK_CacheFactoryAdmin;

struct DK_SelectionAdmin_s;
typedef struct DK_SelectionAdmin_s DK_SelectionAdmin;

struct DK_ObjectAdmin_s;
typedef struct DK_ObjectAdmin_s DK_ObjectAdmin;

struct DK_SecondaryObjectAdmin_s;
typedef struct DK_SecondaryObjectAdmin_s DK_SecondaryObjectAdmin;

struct DK_CacheAccessAdmin_s;
typedef struct DK_CacheAccessAdmin_s DK_CacheAccessAdmin;

struct DK_Entity_s;
typedef struct DK_Entity_s DK_Entity;

struct DK_Collection_s;
typedef struct DK_Collection_s DK_Collection;

struct DK_MapAdmin_s;
typedef struct DK_MapAdmin_s DK_MapAdmin;

struct DK_SetAdmin_s;
typedef struct DK_SetAdmin_s DK_SetAdmin;

struct DK_ObjectHolder_s;
typedef struct DK_ObjectHolder_s DK_ObjectHolder;

struct DK_QueryCriterion_s;
typedef struct DK_QueryCriterion_s DK_QueryCriterion;

struct DK_ObjectID_s;
typedef struct DK_ObjectID_s DK_ObjectID;

struct DK_ReadInfo_s;
typedef struct DK_ReadInfo_s DK_ReadInfo;

struct DK_ObjectWriter_s;
typedef struct DK_ObjectWriter_s DK_ObjectWriter;

struct DK_ObjectReader_s;
typedef struct DK_ObjectReader_s DK_ObjectReader;

struct DK_TopicInfo_s;
typedef struct DK_TopicInfo_s DK_TopicInfo;

struct DK_ObjectID_s{/* NOT IN DESIGN */
    LOC_long oid[3];
};

/* \brief This struct is a used when retrieving all objects from the DLRL, its essentially a wrapper for an array of
 * ObjectAdmins.
 */
typedef struct DK_ObjectArrayHolder_s{
    DK_ObjectAdmin** objectArray;   /* Array of <code>DK_ObjectAdmin</code> pointers. The allocated size can be found
                                     * in the max size variable of this struct, and the size until where valid values
                                     * are contained is determined by the size attribute.
                                     */
    LOC_unsigned_long maxSize;      /* The maximum number of elements that can be contained within the the objectArray.
                                     * I.E. The memory size used during allocation of the array.
                                     */
    LOC_unsigned_long size;         /* The number of elements contained within the array. Size will always be smaller
                                     * or equal to the maxSize. The last index containing a valid value will always be
                                     * size - 1 or 0 if the size is 0.
                                     */
} DK_ObjectArrayHolder;

/* (forward) declaration of enumerations, each enumeration contains a recommended (by the coding standards of the BU-CS) */
/* enumeration sentinel as final element of the enumeration */
/* \brief Used by the <code>DK_CacheBase</code> to indicate the read/write configuration */
typedef enum DK_Usage {
    DK_USAGE_READ_ONLY, /* Indicates the <code>DK_CacheBase</code> is a read only <code>DK_CacheBase</code> */
    DK_USAGE_WRITE_ONLY, /* Indicates the <code>DK_CacheBase</code> is a write only <code>DK_CacheBase</code> */
    DK_USAGE_READ_WRITE, /* Indicates the <code>DK_CacheBase</code> is a read and write <code>DK_CacheBase</code> */
    DK_Usage_elements /* Sentinel */
} DK_Usage;

/* \brief Used by the <code>DK_CacheAdmin</code> to indicate the state of the underlying DCPS infrastructure */
typedef enum DK_PubSubState {
    DK_PUB_SUB_STATE_INITIAL, /* Indicates the <code>DK_CacheAdmin</code> is still in initial publication and
                               * subscription mode
                               */
    DK_PUB_SUB_STATE_REGISTERED, /* Indicates all required DCPS entities for the <code>DK_CacheAdmin</code>
                                  * have been created, but not yet enabled
                                  */
    DK_PUB_SUB_STATE_ENABLED, /* Indicates all required DCPS entities for the <code>DK_CacheAdmin</code> have been
                               * enabled and that the DLRL is currently actively participating in Data Distribution
                               */
    DK_PubSubState_elements /* Sentinel */
} DK_PubSubState;

/* \brief Used by the <code>DK_ObjectAdmin</code> to indicate what it's read or write state currently is */
typedef enum DK_ObjectState {
    DK_OBJECT_STATE_OBJECT_VOID, /* Indicates the state is not valid/used for this <code>DK_ObjectAdmin</code> */
    DK_OBJECT_STATE_OBJECT_NEW, /* Indicates the <code>DK_ObjectAdmin</code> is newly created */
    DK_OBJECT_STATE_OBJECT_NOT_MODIFIED, /* Indicates the <code>DK_ObjectAdmin</code> has not been modified */
    DK_OBJECT_STATE_OBJECT_MODIFIED,/* Indicates the <code>DK_ObjectAdmin</code> has been modified */
    DK_OBJECT_STATE_OBJECT_DELETED,/* Indicates the <code>DK_ObjectAdmin</code> has been deleted */
    DK_ObjectState_elements /* Sentinel */
} DK_ObjectState;

/* \brief Used in various operations to indicate the scope of interest */
typedef enum DK_ObjectScope {
    DK_OBJECT_SCOPE_SIMPLE_OBJECT_SCOPE, /* Indicates only the object is of interest. I.E. Dont take any relations
                                          *into account.
                                          */
    DK_OBJECT_SCOPE_CONTAINED_OBJECTS_SCOPE, /* Indicates the object AND any composite relations the object might have
                                              * are of interest. Any non-composite relations are ignored. */
    DK_OBJECT_SCOPE_RELATED_OBJECTS_SCOPE, /* Indicates the object AND any composite relations AND any non-composite
                                            * relations it might have are of interest */
    DK_ObjectScope_elements /* Sentinel */
} DK_ObjectScope;

/* \brief Used by the <code>DK_CacheBase</code> to indicate the correct subclass. */
typedef enum DK_CacheKind {
    DK_CACHE_KIND_CACHE, /* Indicates this <code>DK_CacheBase</code> is subclassed as a <code>DK_CacheAdmin</code> */
    DK_CACHE_KIND_CACHE_ACCESS, /* Indicates this <code>DK_CacheBase</code> is subclassed as a
                                 *<code>DK_CacheAccessAdmin</code>
                                 */
    DK_CacheKind_elements /* Sentinel */
} DK_CacheKind;

/* \brief Used to indicate the relation type of an relation between DLRL objects. */
typedef enum DK_RelationType {
    DK_RELATION_TYPE_STR_MAP, /* Indicates this relation is a String Map type */
    DK_RELATION_TYPE_INT_MAP, /* Indicates this relation is an Integer Map type */
    DK_RELATION_TYPE_SET, /* Indicates this relation is a Set type */
    DK_RELATION_TYPE_REF,  /* Indicates this relation is a single relation type. The <code>DK_RELATION_TYPE_REF</code>
                            * element must always be the last element in this enumeration, as it is not a known type
                            * within the DLRL specification and therefore any API language binding ontop of this kernel.
                            */
    DK_RelationType_elements /* Sentinel */
} DK_RelationType;

/* \brief Used to indicate the correct case for the <code>DK_SelectionCriterion</code> union */
typedef enum DK_CriterionKind_e{
    DK_CRITERION_KIND_QUERY, /* Indicates this criterion is actually a query */
    DK_CRITERION_KIND_FILTER, /* Indicates this criterion is actually an user defined filter */
    DK_CriterionKind_elements /* Sentinel */
} DK_CriterionKind;

typedef enum DK_MembershipState_e {
    DK_MEMBERSHIPSTATE_UNDEFINED_MEMBERSHIP,
    DK_MEMBERSHIPSTATE_ALREADY_MEMBER,
    DK_MEMBERSHIPSTATE_NOT_MEMBER
}DK_MembershipState;

/* (forward) declaration of unions used by the DLRL */
/* \brief This union represents a criterion as used by <code>DK_SelectionAdmin</code> */
typedef union DK_SelectionCriterion_u{
    DLRL_LS_object filterCriterion; /* The user defined filter case */
    DK_QueryCriterion* queryCriterion; /* The query case */
} DK_SelectionCriterion;

typedef enum DK_MapAdmin_ElementType_e{
    DK_MAPADMIN_ELEMENTTYPE_ADDED,
    DK_MAPADMIN_ELEMENTTYPE_MODIFIED,
    DK_MAPADMIN_ELEMENTTYPE_REMOVED,
    DK_MAPADMIN_ELEMENTTYPE_KEYS
} DK_MapAdmin_ElementType;

/**********************************************************************************************************************
*********************************** DLRL Kernel API calls of the Entity ***********************************************
**********************************************************************************************************************/

/* \brief This operation increases the reference count of the provided entity with one.
 *
 * For every call to this operation the <code>DK_Entity_ts_release(...)</code> operation must be called once.
 *
 * \param _this The entity for which the reference count has to be increased.
 *
 * \return The same entity as provided as parameter with the reference count increased. There is no difference
 * between using the return value entity pointer or the param entity pointer, its exactly the same.
 */
OS_API DK_Entity*
DK_Entity_ts_duplicate(
    DK_Entity* _this);

/* \brief This operation decreases the reference count of the provided entity with one.
 *
 * \param _this The entity for which the reference count has to be decreased.
 */
OS_API void
DK_Entity_ts_release(
    DK_Entity* _this);

/**********************************************************************************************************************
************************************** CacheFactoryAdmin (does not extend entity) *************************************
**********************************************************************************************************************/
/* \brief Returns a newly created <code>DK_CacheAdmin</code>.
 *
 * A purpose parameter specifies the future usage of the <code>DK_CacheAdmin</code>, namely
 * <code>DK_USAGE_WRITE_ONLY</code> - no subscription, <code>DK_USAGE_READ_ONLY</code> -  no publication, or
 * <code>DK_USAGE_READ_WRITE</code> - both modes. Depending on this purpose a DCPS publisher, a DCPS Subscriber, or
 * both will be created, respectively, for the unique usage of the <code>DK_CacheAdmin</code>. These two objects will be
 * attached to the passed DomainParticipant.
 * The newly created <code>DK_CacheAdmin</code> will be inserted under the specified name in the map of caches of the
 * <code>DK_CacheFactoryAdmin</code>
 * NOTE: This function may ONLY be called after the <code>DK_CacheFactoryAdmin_ts_getInstance(...)</code> has been
 * called at least once. As that operation initializes the <code>DK_CacheFactoryAdmin</code> singleton.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_DCPS_ERROR</code> - If an unexpected error occured in the DCPS.</li>
 * <li><code>DLRL_ALREADY_EXISTING</code> - If the specified name has already been used by another Cache object.</li>
 * </ul>
 *
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param ls_cache The language specific representative of the newly to be created <code>DK_CacheAdmin</code>.
 * \param userData Optional user data.
 * \param purpose Indicates the future usage of the Cache.
 * \param cacheName The name under which the to-be-created cache should be inserted within the
 *                  <code>DK_CacheFactoryAdmin</code>
 * \param participant Specifies the DCPS Domainparticipant that is to be used.
 * \param ls_participant The language specific participant representative of the <code>participant</code> parameter.
 *
 * \return <code>NULL</code> if and and only if an exception occured. Otherwise it returns the newly created
 *         <code>DK_CacheAdmin</code>. If not <code>NULL</code> then the caller must release the returned pointer.
 */
OS_API DK_CacheAdmin*
DK_CacheFactoryAdmin_ts_createCache(
    DLRL_Exception* exception,
    DLRL_LS_object ls_cache,
    void* userData,
    DK_Usage purpose,
    LOC_const_string cacheName,
    /* operation becomes owner of the participant, and must clean up if an exception occurs*/
    u_participant participant,
    DLRL_LS_object ls_participant);

/* \brief Retrieves a <code>DK_CacheAdmin</code> object based by name.
 *
 * If no <code>DK_CacheAdmin</code> object is identified by the specified name, <code>NULL</code> will be returned.
 * NOTE: This function may ONLY be called after the <code>DK_CacheFactoryAdmin_ts_getInstance(...)</code> has been
 * called at least once. As that operation initializes the <code>DK_CacheFactoryAdmin</code> singleton.
 *
 * \param cacheName the name that identifies the <code>DK_CacheAdmin</code> object that is to be retrieved.
 *
 * \return The <code>DK_CacheAdmin</code> object that corresponds to the specified name or <code>NULL</code>if no
 * <code>DK_CacheAdmin</code> corresponds to the specified name. If not <code>NULL</code> then the caller must release
 * the returned pointer.
 */
OS_API DK_CacheAdmin*
DK_CacheFactoryAdmin_ts_findCachebyName(
    LOC_const_string cacheName);

/* \brief Deletes the specified <code>DK_CacheAdmin</code> and releases all resources it has claimed.
 *
 * NOTE: This function may ONLY be called after the <code>DK_CacheFactoryAdmin_ts_getInstance(...)</code> has been
 * called at least once. As that operation initializes the <code>DK_CacheFactoryAdmin</code> singleton.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_CacheAdmin</code> has already been deleted.</li>
 * </ul>
 *
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param userData Optional user data.
 * \param cache The <code>DK_CacheAdmin</code> object that is to be deleted.
 */
OS_API void
DK_CacheFactoryAdmin_ts_deleteCache(
    DLRL_Exception* exception,
    void* userData,
    DK_CacheAdmin* cache);

/* \brief Static method to get a reference to the singleton <code>DK_CacheFactoryAdmin</code> instance.
 *
 * \return a reference to the one and only instance of the <code>DK_CacheFactoryAdmin</code>.
 */
OS_API DK_CacheFactoryAdmin*
DK_CacheFactoryAdmin_ts_getInstance(
    DLRL_Exception* exception);

/* \brief Registers a language specific representative of the singleton <code>DK_CacheFactory</code> object
 *
 * NOTE: This function may ONLY be called after the <code>DK_CacheFactoryAdmin_ts_getInstance(...)</code> has been
 * called at least once. As that operation initializes the <code>DK_CacheFactoryAdmin</code> singleton.
 *
 * \param ls_cacheFactory The language specific representative CacheFactory
 */
OS_API void
DK_CacheFactoryAdmin_ts_registerLSCacheFactory(
    DLRL_LS_object ls_cacheFactory);

/**********************************************************************************************************************
********************************************* CacheBase (extends entity) *********************************************
**********************************************************************************************************************/

/* \brief Returns the kind which indicates the eventual subclass of this <code>DK_CacheBase</code>.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_CacheBase</code> has already been deleted.</li>
 * </ul>
 *
 * \param _this The <code>DK_CacheBase</code> entity that is the target of this operation
 * \param exception If an exception occurs the values in this struct will be modified.
 *
 * \return the kind of this <code>DK_CacheBase</code>
 */
OS_API DK_CacheKind
DK_CacheBase_ts_getKind(
    DK_CacheBase* _this,
    DLRL_Exception* exception);

/**********************************************************************************************************************
********************************************* CacheAdmin (extends CacheBase) ******************************************
**********************************************************************************************************************/

OS_API LOC_boolean
DK_CacheAdmin_ts_isAlive(
    DK_CacheAdmin* _this);

/* \brief Registers an <code>DK_ObjectHomeAdmin</code> to this <code>DK_CacheAdmin</code>.
 *
 * The <code>DK_CacheAdmin</code> keeps a list of registered ObjectHomes, and returns the  index number for the
 * specified <code>DK_ObjectHomeAdmin</code> in that list. A number of preconditions must be satisfied when invoking
 * the register_home method:<ul>
 * <li>The <code>DK_CacheAdmin</code> must have a pubsub_state set to <code>DK_PUB_SUB_STATE_INITIAL</code>.</li>
 * <li>The specified <code>DK_ObjectHomeAdmin</code> instance may not yet be registered before (either to this
 * <code>DK_CacheAdmin</code> or to another <code>DK_CacheAdmin</code>)</li>
 * <li>Another instance of the same class as the specified <code>DK_ObjectHomeAdmin</code> may not already have been
 * registered to this <code>DK_CacheAdmin</code>.</li></ul>
 * If these preconditions are not satisfied, a DLRL_PRECONDITION_NOT_MET Exception  will be set.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_CacheAdmin</code> has already been deleted.</li>
 * <li><code>DLRL_PRECONDITION_NOT_MET</code> - If one of the pre-conditions has not been met.</li></ul>
 *
 * \param _this The <code>DK_CacheAdmin</code> entity that is the target of this operation
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param home the <code>DK_ObjectHomeAdmin</code> that is to be registered to this Cache.
 *
 * \return the index number for the specified <code>DK_ObjectHomeAdmin</code> which is only valid if no exception
 * occured.
 */
OS_API LOC_long
DK_CacheAdmin_ts_registerHome(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    DK_ObjectHomeAdmin* home);

/* \brief Retrieves an already registered <code>DK_ObjectHomeAdmin</code> based on its type-name.
 *
 * This type-name must be the fully-qualified name (including its scope), using the IDL notation (with the double colon
 * '::' as the scoping  operator).
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_CacheAdmin</code> has already been deleted.</li>
 * </ul>
 *
 * \param _this The <code>DK_CacheAdmin</code> entity that is the target of this operation
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param name the fully qualified type name of the <code>DK_ObjectHomeAdmin</code> to retrieve.
 *
 * \return <code>NULL</code> if no home could be found for the specified name or if an exception occured. Otherwise
 * it returns the <code>DK_ObjectHomeAdmin</code> instance specified by the type-name. If not <code>NULL</code> then
 * the caller must release the returned pointer.
 */
OS_API DK_ObjectHomeAdmin*
DK_CacheAdmin_ts_findHomeByName(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    LOC_const_string name);

/* not in design */
OS_API DLRL_LS_object
DK_CacheAdmin_ts_findLSHomeByName(DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    LOC_const_string name);

/* not in design */
OS_API DLRL_LS_object
DK_CacheAdmin_ts_findLSHomeByIndex(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    const LOC_long index);

/* \brief Retrieves an already registered ObjectHome based on its registration index.
 *
 * This registration index is the index in the list of registered ObjectHomes of the <code>DK_CacheAdmin</code>, and is
 * returned when registering an <code>DK_ObjectHomeAdmin</code>.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_CacheAdmin</code> has already been deleted.</li>
 * </ul>
 *
 * \param _this The <code>DK_CacheAdmin</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param index the registration index of the <code>DK_ObjectHomeAdmin</code> to retrieve.
 *
 * \return <code>NULL</code> if no home could be found for the specified index or if an exception occured. Otherwise
 * it returns the <code>DK_ObjectHomeAdmin</code> instance specified by the index. If not <code>NULL</code> then
 * the caller must release the returned pointer.
 */
OS_API DK_ObjectHomeAdmin*
DK_CacheAdmin_ts_findHomeByIndex(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    const LOC_long index);

/* \brief Creates and registers all the required DCPS Entities (except for Publisher and Subscriber, which are always
 * available).
 *
 * Registration is performed for publication, for subscription or for both, according to the cache usage. The task of
 * creating and registering the typed DCPS entities is delegated to their corresponding ObjectHomes, but is
 * performed only for ObjectHomes that have been registered to this <code>DK_CacheAdmin</code>. A
 * <code>DK_ObjectHomeAdmin</code> may only have dependencies to other ObjectHomes when  these other ObjectHomes have
 * also been registered to this <code>DK_CacheAdmin</code>. A <code>DLRL_BAD_HOME_DEFINITION</code> is set otherwise.
 * Also a number of preconditions must be satisfied before invoking the
 * <code>DK_CacheAdmin_ts_registerAllForPubSub</code> method:<ul>
 * <li>at least one <code>DK_ObjectHomeAdmin</code> needs to have been registered</li>
 * <li>the pubsub_state may not yet be <code>DK_PUB_SUB_STATE_ENABLED</code>.</li></ul>
 * If these preconditions are not satisfied, a DLRL_PRECONDITION_NOT_MET Exception  will be thrown. Invoking the
 * DK_CacheAdmin_ts_registerAllForPubSub on a <code>DK_PUB_SUB_STATE_REGISTERED</code> pubSubState will be considered a
 * no-op.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_CacheAdmin</code> has already been deleted.</li>
 * <li><code>DLRL_BAD_HOME_DEFINITION</code> - If a registered <code>DK_ObjectHomeAdmin</code> has dependencies to other
 * , unregistered, ObjectHomes.</li>
 * <li><code>DLRL_PRECONDITION_NOT_MET</code> - If one of the pre-conditions has not been met.</li>
 * <li><code>DLRL_DCPS_ERROR</code> - If an unexpected error occured in the DCPS</li></ul>
 *
 * \param _this The <code>DK_CacheAdmin</code> entity that is the target of this operation
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param userData Optional user data.
 */
OS_API void
DK_CacheAdmin_ts_registerAllForPubSub(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData);

/* \brief Enables all the attached DCPS Entities.
 *
 * Immutable QoS Settings for the created DCPS Entities can only be changed before these Entities have
 * been enabled. One precondition must be satisfied before invoking the <code>DK_CacheAdmin_ts_enableAllForPubSub</code>
 * method:<ul>
 * <il> The pubsubState must already have been set to <code>DK_PUB_SUB_STATE_REGISTERED</code> before. A
 * <code>DLRL_PRECONDITION_NOT_MET</code> Exception is thrown otherwise. Invoking the
 * <code>DK_CacheAdmin_ts_enableAllForPubSub</code> method on a <code>DK_PUB_SUB_STATE_ENABLED</code> pubSubState will
 * be considered a no-op.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_CacheAdmin</code> has already been deleted.</li>
 * <li><code>DLRL_PRECONDITION_NOT_MET</code> - If one of the pre-conditions has not been met.</li>
 * <li><code>DLRL_DCPS_ERROR</code> - If an unexpected error occured in the DCPS</li></ul>
 *
 * \param _this The <code>DK_CacheAdmin</code> entity that is the target of this operation
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param userData Optional user data.
 */
OS_API void
DK_CacheAdmin_ts_enableAllForPubSub(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData);

/* \brief Returns the usage mode of the <code>DK_CacheAdmin</code>.
 *
 * The usage mode can be <code>DK_USAGE_READ_ONLY</code>, <code>DK_USAGE_WRITE_ONLY</code> or
 * <code>DK_USAGE_READ_WRITE</code>.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_CacheAdmin</code> has already been deleted.</li>
 * </ul>
 *
 * \param _this The <code>DK_CacheAdmin</code> entity that is the target of this operation
 * \param exception If an exception occurs the values in this struct will be modified.
 *
 * \return the usage mode of the <code>DK_CacheAdmin</code>.
 */
OS_API DK_Usage
DK_CacheAdmin_ts_getCacheUsage(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception);

/* \brief Returns the state of the Cache with respect to the underlying Pub/Sub infrastructure.
 *
 * The underlying Pub/Sub infrastructure can be <code>DK_PUB_SUB_STATE_INITIAL</code> (no DCPS entities created yet),
 * <code>DK_PUB_SUB_STATE_REGISTERED</code> (DCPS entities are created but not yet enabled), and
 * <code>DK_PUB_SUB_STATE_ENABLED</code> (DCPS entities are enabled).
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_CacheAdmin</code> has already been deleted.</li>
 * </ul>
 *
 * \param _this The <code>DK_CacheAdmin</code> entity that is the target of this operation
 * \param exception If an exception occurs the values in this struct will be modified.
 *
 * \return the state of the underlying Pub/Sub infrastructure.
 */
OS_API DK_PubSubState
DK_CacheAdmin_ts_getPubSubState(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception);

/* not in design */
OS_API DK_Usage
DK_CacheAdmin_ts_getUsage(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception);

/* not in design */
OS_API void
DK_CacheAdmin_ts_getLSHomes(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg);

/* not in design */
OS_API void
DK_CacheAdmin_ts_getLSAccesses(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg);

/* not in design */
OS_API void
DK_CacheAdmin_ts_getListeners(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg);

/* \brief Registers a language specific representative of the <code>DK_CacheAdmin</code> object
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_CacheAdmin</code> has already been deleted.</li>
 * </ul>
 *
 * \param _this The <code>DK_CacheAdmin</code> entity that is the target of this operation
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param ls_cache The language specific representative Cache
 */
OS_API void
DK_CacheAdmin_ts_registerLSCache(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    const DLRL_LS_object ls_cache);

/* \brief Returns the update mode of the <code>DK_CacheAdmin</code>.
 *
 * The <code>DK_CacheAdmin</code> can either be in the automatic update mode <code>(true)</code>, or in the manual
 * update mode (<code>false</code>).
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_CacheAdmin</code> has already been deleted.</li>
 * </ul>
 *
 * \param _this The <code>DK_CacheAdmin</code> entity that is the target of this operation
 * \param exception If an exception occurs the values in this struct will be modified.
 *
 * \return the update mode of the Cache (invalid if an exception is raised)
 */
OS_API LOC_boolean
DK_CacheAdmin_ts_getUpdatesEnabled(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception);

/* \brief Enables automatic update mode.
 *
 * Available modifications in DCPS are  automatically applied to the DLRL objects and the corresponding Listeners
 * will be triggered.
 * One precondition must be satisfied before invoking the enable_updates method:<ul>
 * <il>the cache_usage must either be <code>DK_USAGE_READ_ONLY</code> or <code>DK_USAGE_READ_WRITE</code>.</il></ul>
 * A <code>DLRL_PRECONDITION_NOT_MET</code> Exception is thrown otherwise. Invoking the
 * <code>DK_CacheAdmin_ts_enableUpdates</code> method when updatesEnabled is already <code>true</code> will be
 * considered a no-op.
 * Take note that this call will result in DLRL listeners being triggered on the account of the thread that called
 * this operation. If cache listeners are present then the on_updates_enabled() operation will be triggered to indicate
 * a change in the way DLRL processes updates to interested parties. However this operation also forces a refresh of the
 * cache, resulting in the cache listeners operation on_begin_updates() and on_end_updates() from being called if
 * updates are available. Any registered object listeners at object homes that have updates will also be triggered, any
 * of the listed operations of the object listener may be triggered. Finally also take note that this operation clears
 * any modification info (object root is_modified operation or object home get_created_objects for example) that was
 * stored since the last time a refresh was done.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_CacheAdmin</code> has already been deleted.</li>
 * <li><code>DLRL_PRECONDITION_NOT_MET</code> - If one of the pre-conditions has not been met.</li></ul>
 *
 * \param _this The <code>DK_CacheAdmin</code> entity that is the target of this operation
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param userData Optional user data.
 */
OS_API void
DK_CacheAdmin_ts_enableUpdates(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData);

/* \brief Disables automatic update mode.
 *
 * Available modifications in DCPS must  manually be applied to the DLRL objects by using the
 * <code>DK_CacheAdmin_ts_refresh(...)</code> method.
 * One precondition must be satisfied when invoking the disable_updates method:<ul>
 * <il>The cache_usage must either be <code>DK_USAGE_READ_ONLY</code> or <code>DK_USAGE_READ_WRITE</code>.</il></ul>
 * A <code>DLRL_PRECONDITION_NOT_MET</code> Exception is thrown otherwise. Invoking the
 * <code>DK_CacheAdmin_ts_disableUpdates</code> method when updatesEnabled is already <code>false</code>
 * will be considered a no-op.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_CacheAdmin</code> has already been deleted.</li>
 * <li><code>DLRL_PRECONDITION_NOT_MET</code> - If one of the pre-conditions has not been met.</li></ul>
 *
 * \param _this The <code>DK_CacheAdmin</code> entity that is the target of this operation
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param userData Optional user data.
 */
OS_API void
DK_CacheAdmin_ts_disableUpdates(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData);

/* \brief Applies waiting modifications in the DCPS onto the DLRL objects, when the updatesEnabled is <code>false</code>.
 *
 * No Listeners are being triggered in this case. When the automatic_updates mode is true, this  operation is considered
 * a no-op.
 * One precondition must be satisfied when invoking the disable_updates method:<ul>
 * <il>The cache_usage must either be <code>DK_USAGE_READ_ONLY</code> or <code>DK_USAGE_READ_WRITE</code>.</il></ul>
 * A <code>DLRL_PRECONDITION_NOT_MET</code> Exception is thrown otherwise.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_CacheAdmin</code> has already been deleted.</li>
 * <li><code>DLRL_PRECONDITION_NOT_MET</code> - If one of the pre-conditions has not been met.</li>
 * <li><code>DLRL_DCPS_ERROR</code> - If an unexpected error occured in the DCPS</li></ul>
 *
 * \param _this The <code>DK_CacheAdmin</code> entity that is the target of this operation
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param userData Optional user data.
 */
OS_API void
DK_CacheAdmin_ts_refresh(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData);

/* \brief Attaches a CacheListener to this <code>DK_CacheAdmin</code>.
 *
 * When successful, it returns <code>true</code>. If the same CacheListener instance was already
 * attached before, the operation is ignored and returns <code>false</code>.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_CacheAdmin</code> has already been deleted.</li>
 * </ul>
 *
 * \param _this The <code>DK_CacheAdmin</code> entity that is the target of this operation
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param userData Optional user data.
 * \param listener the CacheListener that needs to be attached.
 *
 * \return whether the specified CacheListener is successfuly attached.
 */
OS_API LOC_boolean
DK_CacheAdmin_ts_attachListener(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    const DLRL_LS_object listener);

/* \brief Detaches a CacheListener from this <code>DK_CacheAdmin</code>.
 *
 * Returns <code>true</code> when  successfuly detached and <code>false</code> when the specified CacheListener
 * instance was not attached to this Cache.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_CacheAdmin</code> has already been deleted.</li>
 * </ul>
 *
 * \param _this The <code>DK_CacheAdmin</code> entity that is the target of this operation
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param userData Optional user data.
 * \param listener the CacheListener that needs to be detached.
 *
 * \return whether the specified CacheListener is successfuly detached.
 */
OS_API DLRL_LS_object
DK_CacheAdmin_ts_detachListener(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    const DLRL_LS_object listener);

/* \brief Returns the list of all objects that are available in this <code>DK_CacheAdmin</code>.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_CacheAdmin</code> has already been deleted.</li>
 * <li><code>DLRL_DCPS_ERROR</code> - If an unexpected error occured in the DCPS</li></ul>
 *
 * \param _this The <code>DK_CacheAdmin</code> entity that is the target of this operation
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param userData Optional user data.
 * \param arg The argument for the provided action routine
 * \param action This function pointer will be called for each element contained within the cache.
 *
 * \return the list of available objects.
 */
OS_API void
DK_CacheAdmin_ts_getObjects(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg);

/* NOT IN DESIGN */
/* no homes may be locked! */
OS_API DK_CacheAccessAdmin*
DK_CacheAdmin_ts_createAccess(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DLRL_LS_object ls_access,
    DK_Usage usage);

/* NOT IN DESIGN */
/* no homes may be locked! */
OS_API void
DK_CacheAdmin_ts_deleteAccess(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_CacheAccessAdmin* access);

/* not in design */
OS_API DLRL_LS_object
DK_CacheAdmin_ts_getLSCache(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData);

/* not in design */
OS_API DLRL_LS_object
    DK_CacheAdmin_ts_getLSPublisher(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData);

/* not in design */
OS_API DLRL_LS_object
    DK_CacheAdmin_ts_getLSPParticipant(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData);

/* not in design */
OS_API DLRL_LS_object
DK_CacheAdmin_ts_getLSSubscriber(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData);

OS_API DLRL_LS_object
DK_CacheAdmin_ts_getLSParticipant(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData);

OS_API DK_Usage
DK_CacheAccessAdmin_ts_getCacheUsage(
    DK_CacheAccessAdmin* _this,
    DLRL_Exception* exception);

/* NOT IN DESIGN */
OS_API DLRL_LS_object
DK_CacheAccessAdmin_ts_getLSAccess(
    DK_CacheAccessAdmin* _this,
    DLRL_Exception* exception,
    void* userData);

/* NOT IN DESIGN */
OS_API void
DK_CacheAccessAdmin_ts_write(
    DK_CacheAccessAdmin* _this,
    DLRL_Exception* exception,
    void* userData);

OS_API DLRL_LS_object
DK_CacheAccessAdmin_ts_getLSOwner(
    DK_CacheAccessAdmin* _this,
    DLRL_Exception* exception,
    void* userData);

/* NOT IN DESIGN */
OS_API void
DK_CacheAccessAdmin_ts_purge(
    DK_CacheAccessAdmin* _this,
    DLRL_Exception* exception,
    void* userData);

/* NOT IN DESIGN */
/* calls the action routine for each object in the cache access. During the action routine the access   */
/* to which the element belongs to will be locked */
OS_API void
DK_CacheAccessAdmin_ts_getObjects(
    DK_CacheAccessAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
                                                                                                            void** arg);
/* NOT IN DESIGN */
/* calls the action routine for each object in the cache access. During the action routine the access and the home  */
/* (admin lock) to which the element belongs to will be locked */
OS_API void
DK_CacheAccessAdmin_ts_visitAllObjectsForHome(
    DK_CacheAccessAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectHomeAdmin* home,
    void (*action)( DLRL_Exception*,
                    void*,
                    DK_ObjectAdmin*,
                    LOC_unsigned_long,
                    LOC_unsigned_long,
                    void**),
    void** arg);

/* NOT IN DESIGN */
OS_API void
DK_CacheAccessAdmin_ts_getContainedTypes(
    DK_CacheAccessAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg);

/* NOT IN DESIGN */
OS_API void
DK_CacheAccessAdmin_ts_getContainedTypeNames(
    DK_CacheAccessAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg);

OS_API void
DK_CacheAccessAdmin_ts_getInvalidObjects(
    DK_CacheAccessAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg);

/**********************************************************************************************************************
****************************************** ObjectHomeAdmin (extends entity) *******************************************
**********************************************************************************************************************/
/* \brief This is the default constructor for the <code>DK_ObjectHomeAdmin</code>.
 *
 * It initializes the CityHome in the default configuration (i.e. auto deref will be set to <code>true</code> and
 * content_filter will be set to <code>NULL</code>).
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_ObjectHomeAdmin</code> has already been deleted.
 * </li></ul>
 *
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param name The fully qualified name to identify this <code>DK_ObjectHomeAdmin</code> with.
 *
 * \return <code>NULL</code> if and only if an exception occured. Otherwise it returns a pointer to a newly created
 * <code>DK_ObjectHomeAdmin</code> object. If not <code>NULL</code> then the caller must release the returned pointer.
 */
OS_API DK_ObjectHomeAdmin*
DK_ObjectHomeAdmin_new(
    DLRL_Exception* exception,
    LOC_string name);

/* \brief This is the default deletion routine for a <code>DK_ObjectHomeAdmin</code>.
 *
 * This operation will clean all resources used by the <code>DK_ObjectHomeAdmin</code>, release any pointer and
 * memory used by the <code>DK_ObjectHomeAdmin</code>. It will not however free the memory of the
 * <code>DK_ObjectHomeAdmin</code> itself, this is done automatically when the reference count of the
 * <code>DK_ObjectHomeAdmin</code> reaches zero. This operation does not perform a release of the
 * <code>DK_ObjectHomeAdmin</code>, that remains a resposibility of the caller of the constructor. If an error occurs
 * during deletion it will be written to an error info log and then the deletion process will try and continue.
 *
 * \param _this The <code>DK_ObjectHomeAdmin</code> entity that is the target of this operation.
 * \param userData Optional user data.
 */
OS_API void
DK_ObjectHomeAdmin_ts_delete(
    DK_ObjectHomeAdmin* _this,
    void* userData);

/* \brief Returns the current setting of the autoDeref attribute.
 * When set to <code>true</code>, the state of each DLRL object is always copied into it. When set to
 * <code>false</code>, the state is only copied into DLRL objects when they are explicitly accessed by the
 * application.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_ObjectHomeAdmin</code> has already been deleted.
 * </li></ul>
 *
 * \param _this The <code>DK_ObjectHomeAdmin</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 *
 * \return the current setting of the autoDeref attribute.
 */
OS_API LOC_boolean
DK_ObjectHomeAdmin_ts_getAutoDeref(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception);

/* \brief Returns the index under which the <code>DK_ObjectHomeAdmin</code> is registered by the
 * <code>DK_CacheAdmin</code>.
 *
 * If the <code>DK_ObjectHomeAdmin</code> has not yet been registered to the <code>DK_CacheAdmin</code>, it returns -1.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_ObjectHomeAdmin</code> has already been deleted.
 * </li></ul>
 *
 * \param _this The <code>DK_ObjectHomeAdmin</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 *
 * \return the index under which the ObjectHome is registered by the Cache.
 */
OS_API LOC_long
DK_ObjectHomeAdmin_ts_getRegistrationIndex(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception);

/* \brief Returns the <code>DK_ObjectHomeAdmin</code> that manages the parent-class of the class that is managed by this
 * <code>DK_ObjectHomeAdmin</code>.
 *
 * I.E. when class Bar extends from class Foo, then 'Foo' Home is the parent for 'Bar' Home.) When this
 * <code>DK_ObjectHomeAdmin</code> has no parent, it returns <code>NULL</code>. Also when this operation is called
 * before the <code>DK_ObjectHomeAdmin</code> is registered to a Cache and before the
 * <code>DK_CacheAdmin_ts_registerAllForPubSub(...)</code> operation has been successfully called on the cache
 * then this operation will return a <code>NULL</code>.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_ObjectHomeAdmin</code> has already been deleted.
 * </li></ul>
 *
 * \param _this The <code>DK_ObjectHomeAdmin</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 *
 * \return <code>NULL</code if an exception occured, no parent is specified or if the
 * <code>DK_CacheAdmin_ts_registerAllForPubSub(...)</code> operation of the corresponding <code>DK_CacheAdmin</code>
 * has not yet been successfully called. Otherwise returns the <code>DK_ObjectHomeAdmin</code> of the parent-class of
 * the class that is managed by this <code>DK_ObjectHomeAdmin</code>. If not <code>NULL</code> then the caller must
 * release the returned pointer.
 */
OS_API DK_ObjectHomeAdmin*
DK_ObjectHomeAdmin_ts_getParent(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception);

/* \brief Attaches an ObjectListener to this <code>DK_ObjectHomeAdmin</code>.
 *
 * It is possible to specify whether the Listener should also listen for incoming events on the contained objects.
 * Each listener instance can only be attached once.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_ObjectHomeAdmin</code> has already been deleted.
 * </li></ul>
 *
 * \param _this The <code>DK_ObjectHomeAdmin</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param userData Optional user data.
 * \param listener the ObjectListener to be attached.
 * \param concernsContained when set to <code>true</code>, the listener will also listen for incoming events on the
 * contained objects.
 *
 * \return a boolean that specifies whether the listener was successfully  attached (<code>true</code>) or not
 * (<code>false</code>) because it was already attached before.
 */
OS_API LOC_boolean
DK_ObjectHomeAdmin_ts_attachListener(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DLRL_LS_object listener,
    LOC_boolean concernsContained);

/* \brief Detaches an ObjectListener to this <code>DK_ObjectHomeAdmin</code>.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_ObjectHomeAdmin</code> has already been deleted.
 * </li></ul>
 *
 * \param _this The <code>DK_ObjectHomeAdmin</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param userData Optional user data.
 * \param listener the ObjectListener to be attached.
 *
 * \return <code>NULL</code> if the listener was not attached in the first place or if an exception occured. Otherwise
 * return a pointer to the detached listener.
 */
OS_API DLRL_LS_object
DK_ObjectHomeAdmin_ts_detachListener(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DLRL_LS_object listener);

/* \brief Registers a language specific ObjectHome representative to this <code>DK_ObjectHomeAdmin</code>.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_ObjectHomeAdmin</code> has already been deleted.
 * </li></ul>
 *
 * \param _this The <code>DK_ObjectHomeAdmin</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param ls_home The language specific ObjectHome that corresponds to this <code>DK_ObjectHomeAdmin</code>
 *
 * \return A pointer to the previously associated language specific ObjectHome or <code>NULL</code> if there was none.
 */
OS_API DLRL_LS_object
DK_ObjectHomeAdmin_ts_registerLSObjectHome(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    DLRL_LS_object ls_home);

/* \brief Sets <code>DK_ObjectHomeAdmin</code> specific user data.
 *
 * Any previously stored user data is overridden.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_ObjectHomeAdmin</code> has already been deleted.
 * </li></ul>
 *
 * \param _this The <code>DK_ObjectHomeAdmin</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param homeUserData The user data (may be <code>NULL</code>) to be associated with this
 * <code>DK_ObjectHomeAdmin</code>
 */
OS_API void
DK_ObjectHomeAdmin_ts_setUserData(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* homeUserData);

OS_API DLRL_LS_object
DK_ObjectHomeAdmin_ts_getLSDataReader(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    const char* topicName);

OS_API DLRL_LS_object
DK_ObjectHomeAdmin_ts_getLSDataWriter(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    const char* topicName);

OS_API DLRL_LS_object
DK_ObjectHomeAdmin_ts_getLSTopic(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    const char* topicName);

OS_API LOC_string
DK_ObjectHomeAdmin_ts_getTopicName(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    const char* attributeName);

OS_API LOC_string
DK_ObjectHomeAdmin_ts_getName(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception);

OS_API void
DK_ObjectHomeAdmin_ts_getAllTopicNames(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg);

/* \brief Returns the associated <code>DK_CacheAdmin</code> if this <code>DK_ObjectHomeAdmin</code> is registered to one.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_ObjectHomeAdmin</code> has already been deleted.
 * </li></ul>
 *
 * \param _this The <code>DK_ObjectHomeAdmin</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 *
 * \return The <code>DK_CacheAdmin</code> that this <code>DK_ObjectHomeAdmin</code> is registered, or <code>NULL</code>
 * if this home is not yet registered with any <code>DK_CacheAdmin</code> object. If not <code>NULL</code> then the
 * caller must release the returned pointer.
 */
OS_API DK_CacheAdmin*
DK_ObjectHomeAdmin_ts_getCache(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception);

/* \brief Creates a <code>DK_SelectionAdmin</code> within this <code>DK_ObjectHomeAdmin</code> that will work based
 * upon the provided criterion.
 *
 * Upon creation time it must be specified wether the <code>DK_SelectionCriterion</code> will be refreshed using the
 * refresh operation of the <code>DK_SelectionCriterion</code> (autoRefresh is <code>false</code>)or refreshed each
 * time object updates arrive in the <code>DK_CacheAdmin</code> (due to an application triggered or DCPS triggered
 * refresh)(autoRefresh is <code>true</code>). This value can not be changed after the selection has been created.
 * The current implementation will always create selections with autoRefresh set to <code>false</code>.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_ObjectHomeAdmin</code> has already been deleted.
 * </li>
 * <li><code>DLRL_PRECONDITION_NOT_MET</code> - if the <code>DK_CacheAdmin</code> to which the
 * <code>DK_ObjectHomeAdmin</code> belongs is still in <code>DK_PUB_SUB_STATE_INITIAL</code> pubsub mode or if the home
 * does not yet belong to any <code>DK_CacheAdmin</code></li></ul>
 *
 * \param _this The <code>DK_ObjectHomeAdmin</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param userData Optional user data.
 * \param ls_selection The language specific representative to whom the to-be-created <code>DK_SelectionAdmin</code>
 * should be linked.
 * \param criterion the <code>DK_SelectionCriterion</code> determining how the selection determines which DLRL objects
 * become a part of the <code>DK_SelectionAdmin</code>.
 * \param kind Indicates whether the criterion param represents a query or a filter.
 * \param autoRefresh specifies whether the <code>DK_SelectionAdmin</code> will be refreshed using the refresh operation
 * of the <code>DK_SelectionAdmin</code> (autoRefresh is <code>false</code>)or refreshed each time object updates arrive
 * in the <code>DK_CacheAdmin</code> (due to an application triggered or DCPS triggered refresh)(autoRefresh
 * is <code>true</code>)
 * \param concerns_contained_objects Not supported currently
 *
 * \return <code>NULL</code> if and only if an exception occured. Otherwise returns the created
 * <code>DK_SelectionAdmin</code> object. If not <code>NULL</code> then the caller must release the returned pointer.
 */
OS_API DK_SelectionAdmin*
DK_ObjectHomeAdmin_ts_createSelection(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DLRL_LS_object ls_selection,
    DK_SelectionCriterion* criterion,
    DK_CriterionKind kind,
    LOC_boolean autoRefresh,
    LOC_boolean concernsContainedObjects);

/* \brief Deletes a <code>DK_SelectionAdmin</code> of this <code>DK_ObjectHomeAdmin</code>.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_ObjectHomeAdmin</code> has already been deleted.
 * </li>
 * <li><code>DLRL_PRECONDITION_NOT_MET</code> -  if the <code>DK_SelectionAdmin</code> provided was not created by
 * this <code>DK_ObjectHomeAdmin</code></li></ul>
 *
 * \param _this The <code>DK_ObjectHomeAdmin</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param userData Optional user data.
 * \param selection the <code>DK_SelectionAdmin</code> to be deleted.
 */
OS_API void
DK_ObjectHomeAdmin_ts_deleteSelection(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_SelectionAdmin* selection);

/* not in design */
OS_API DLRL_LS_object
DK_ObjectHomeAdmin_ts_getLSHome(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData);
/* not in design */
OS_API void
DK_ObjectHomeAdmin_ts_getModifiedObjects(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg);
/* not in design */
OS_API void
DK_ObjectHomeAdmin_ts_getLSSelections(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg);
/* not in design */
OS_API void
DK_ObjectHomeAdmin_ts_getLSListeners(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg);
/* not in design */
OS_API void
DK_ObjectHomeAdmin_ts_getCreatedObjects(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg);
/* not in design */
OS_API void
DK_ObjectHomeAdmin_ts_getDeletedObjects(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg);

/* \brief Creates a fully registered <code>DK_ObjectAdmin</code> belonging to this <code>DK_ObjectHomeAdmin</code> and
 * inserts it into the specified <code>DK_CacheAccessAdmin</code>.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_ObjectHomeAdmin</code> has already been deleted.
 * </li>
 * <li><code>DLRL_PRECONDITION_NOT_MET</code> -  if the <code>DK_CacheAccessAdmin</code> is not writeable or if the
 * <code>DK_ObjectHomeAdmin</code> follows a predefined mapping</li></ul>
 *
 * \param _this The <code>DK_ObjectHomeAdmin</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param userData Optional user data.
 * \param access the <code>DK_CacheAccessAdmin</code> to insert the newly created object into.
 */
 /* NOT IN DESIGN */
OS_API DLRL_LS_object
DK_ObjectHomeAdmin_ts_createLSObject(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_CacheAccessAdmin* access);

/* \brief Registers an unregistered <code>DK_ObjectAdmin</code> belonging to this <code>DK_ObjectHomeAdmin</code>
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_ObjectHomeAdmin</code> has already been deleted.
 * </li>
 * <li><code>DLRL_PRECONDITION_NOT_MET</code> -  if the <code>DK_ObjectAdmin</code> is already registered</li>
 * <li<code>DLRL_ALREADY_EXISTING</code> - if the object being registered represents an already existing object.</li>
 * </ul>
 *
 * \param _this The <code>DK_ObjectHomeAdmin</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param userData Optional user data.
 * \param objectAdmin the <code>DK_ObjectAdmin</code> to register.
 */
/* NOT IN DESIGN */
OS_API void
DK_ObjectHomeAdmin_ts_registerObject(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectAdmin* objectAdmin);

/* \brief Creates an unregistered <code>DK_ObjectAdmin</code> belonging to this <code>DK_ObjectHomeAdmin</code>
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_ObjectHomeAdmin</code> has already been deleted.
 * </li>
 * <li><code>DLRL_PRECONDITION_NOT_MET</code> -  if the <code>DK_ObjectAdmin</code> is already registered or if the
 * <code>DK_ObjectHomeAdmin</code> follows a default mapping</li></ul>
 *
 * \param _this The <code>DK_ObjectHomeAdmin</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param userData Optional user data.
 * \param objectAdmin the <code>DK_ObjectAdmin</code> to register.
 */
/* NOT IN DESIGN */
OS_API DLRL_LS_object
DK_ObjectHomeAdmin_ts_createUnregisteredObject(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_CacheAccessAdmin* access);

/**********************************************************************************************************************
********************************************* ObjectAdmin (extends entity) ********************************************
**********************************************************************************************************************/

/* \brief Returns the read state of this object.
 *
 * For objects in a <code>DK_USAGE_WRITE_ONLY</code> <code>DK_CacheAccessAdmin</code> this state will be set to
 * <code>DK_OBJECT_STATE_OBJECT_VOID</code>.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_ObjectAdmin</code> has already been deleted.</li>
 * </ul>
 *
 * \param _this The <code>DK_ObjectAdmin</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 *
 * \return the read state of the DLRL object.
 */
OS_API DK_ObjectState
DK_ObjectAdmin_ts_getReadState(
    DK_ObjectAdmin* _this,
    DLRL_Exception* exception);

/* \brief Returns the index of this object's home.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_ObjectAdmin</code> has already been deleted.</li>
 * </ul>
 *
 * \param _this The <code>DK_ObjectAdmin</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 *
 * \return the index of this object's home.
 */
/* NOT IN DESIGN */
OS_API LOC_long
DK_ObjectAdmin_ts_getHomeIndex(
    DK_ObjectAdmin* _this,
    DLRL_Exception* exception);

/* \brief Returns the write state of this object.
 *
 * For objects in a <code>DK_CacheAdmin</code> or <code>DK_USAGE_READ_ONLY</code> <code>DK_CacheAccessAdmin</code> this
 * state will be set to <code>DK_OBJECT_STATE_OBJECT_VOID</code>.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_ObjectAdmin</code> has already been deleted.</li>
 * </ul>
 *
 * \param _this The <code>DK_ObjectAdmin</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 *
 * \return the write state of the DLRL object.
 */
OS_API DK_ObjectState
DK_ObjectAdmin_ts_getWriteState(
    DK_ObjectAdmin* _this,
    DLRL_Exception* exception);

/* \brief Returns a reference to the corresponding <code>DK_ObjectHomeAdmin</code>.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_ObjectAdmin</code> has already been deleted.</li>
 * </ul>
 *
 * \param _this The <code>DK_ObjectAdmin</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 *
 * \return <code>NULL</code> if an exception occurred. Otherwise it returns the corresponding
 * <code>DK_ObjectHomeAdmin</code>. If not <code>NULL</code> then the caller must release the returned pointer.
 */
OS_API DK_ObjectHomeAdmin*
DK_ObjectAdmin_ts_getHome(
    DK_ObjectAdmin* _this,
    DLRL_Exception* exception);

/* \brief Returns whether the object, or one of the objects in its related ObjectScope, has  been modified in the last
* update round.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_ObjectAdmin</code> has already been deleted.</li>
 * </ul>
 *
 * \param _this The <code>DK_ObjectAdmin</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param scope the scope of related objects that will be checked for applied modifications.
 *
 * \return whether the object (or a related object) has been modified in the last update round.
 */
OS_API LOC_boolean
DK_ObjectAdmin_ts_isModified(
    DK_ObjectAdmin* _this,
    DLRL_Exception* exception,
    DK_ObjectScope scope);

/* \brief Returns the <code>DK_CacheBase</code> in which this DLRL object is contained.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_ObjectAdmin</code> has already been deleted.</li>
 * </ul>
 *
 * \param _this The <code>DK_ObjectAdmin</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 *
 * \return the <code>DK_CacheBase</code> in which the DLRL object is contained.
 */
OS_API DK_CacheBase*
DK_ObjectAdmin_ts_getOwner(
    DK_ObjectAdmin* _this,
    DLRL_Exception* exception);

/* \brief Marks the <code>DK_ObjectAdmin</code> for destruction.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_ObjectAdmin</code> has already been deleted.</li>
 * <li><code>DLRL_PRECONDITION_NOT_MET</code> - If the specified <code>DK_ObjectAdmin</code> is not writeable.</li>
 * </ul>
 *
 * \param _this The <code>DK_ObjectAdmin</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 *
 * \return the <code>DK_CacheBase</code> in which the DLRL object is contained.
 */
/* NOT IN DESIGN */
OS_API void
DK_ObjectAdmin_ts_destroy(
    DK_ObjectAdmin* _this,
    DLRL_Exception* exception,
    void* userData);

/* NOT IN DESIGN */
OS_API void
DK_ObjectAdmin_ts_changeRelation(
    DK_ObjectAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    LOC_unsigned_long index,
    DK_ObjectAdmin* relatedObjectAdmin);

/* NOT IN DESIGN */
OS_API void
DK_ObjectAdmin_ts_changeCollection(
    DK_ObjectAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    LOC_unsigned_long index,
    DK_Collection* targetCollection);

/* NOT IN DESIGN */
OS_API void
DK_ObjectAdmin_ts_stateHasChanged(
    DK_ObjectAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    LOC_boolean isImmutable);

OS_API LOC_string
DK_ObjectAdmin_ts_getMainTopicName(
    DK_ObjectAdmin* _this,
    DLRL_Exception* exception,
    void* userData);

OS_API void
DK_ObjectAdmin_ts_getInvalidRelations(
    DK_ObjectAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg);

OS_API void
DK_ObjectHomeAdmin_ts_getAllObjectsForCache(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg);

OS_API void
DK_ObjectHomeAdmin_ts_getAllObjectsForCacheAccess(
    DK_ObjectHomeAdmin* _this,
    DLRL_Exception* exception,
    DK_CacheAccessAdmin* source,
    void* userData,
    void** arg);

OS_API void
DK_ObjectAdmin_ts_getObjectID(
    DK_ObjectAdmin* _this,
    DLRL_Exception* exception,
    DK_ObjectID* oid);

/* not in design */
OS_API DLRL_LS_object
DK_ObjectAdmin_ts_getLSObject(
    DK_ObjectAdmin* _this,
    DLRL_Exception* exception,
    void* userData);

/* not in design */
OS_API DLRL_LS_object
DK_ObjectAdmin_ts_getLSHome(
    DK_ObjectAdmin* _this,
    DLRL_Exception* exception,
    void* userData);

/* following 3 functions are to be used with caution, placed in this file temporarily */

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

OS_API void
DK_ObjectAdmin_us_checkRelationIsFound(
    DK_ObjectAdmin* _this,
    DLRL_Exception* exception,
    LOC_unsigned_long index);

/**********************************************************************************************************************
********************************** MapAdmin (extends Collection which extends entity) *********************************
**********************************************************************************************************************/

/* \brief Returns the number of elements contained in the <code>DK_MapAdmin</code>.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_MapAdmin</code> has already been deleted.</li>
 * </ul>
 *
 * \param _this The <code>DK_MapAdmin</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 *
 * \return the number of contained elements.
 */
OS_API LOC_unsigned_long
DK_MapAdmin_ts_getLength(
    DK_MapAdmin* _this,
    DLRL_Exception* exception);

OS_API DLRL_LS_object
DK_MapAdmin_ts_get(
    DK_MapAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    const void* key);

/* \brief Removes the element that is identified by the specified key.
 *
 * This operation may only be invoked on a Collection that is located in a writeable <code>DK_CacheAccessAdmin</code>.
 * A DLRL_PRECONDITION_NOT_MET is raised otherwise.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_MapAdmin</code> has already been deleted.</li>
 * <li><code>DLRL_PRECONDITION_NOT_MET</code> - If the specified <code>DK_MapAdmin</code> is not located within a
 * writeable <code>DK_CacheAccessAdmin</code>.</li></ul>
 *
 * \param _this The <code>DK_MapAdmin</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param key the key that identifies the element that is to be removed.
 */
OS_API void
DK_MapAdmin_ts_remove(
    DK_MapAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    const void* key);

/* \brief Stores the specified item into the Map, using the specified key as its identifier.
 *
 * If the key already represented another item in the Map, then that item will be replaced by the currently specified
 * item.
 * This operation may ONLY be used for a collection which has an owner object admin.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_MapAdmin</code> has already been deleted.</li>
 * </ul>
 *
 * \param _this The <code>DK_MapAdmin</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param userData Optional user data.
 * \param key the key that will identify the specified item.
 * \param target the item that needs to be stored in the Map.
 */
OS_API void
DK_MapAdmin_ts_put(
    DK_MapAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    const void* key,
    DK_ObjectAdmin* target);

/* \brief Removes all elements from the Map.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_MapAdmin</code> has already been deleted.</li>
 * </ul>
 *
 * \param _this The <code>DK_MapAdmin</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param userData optional user data
 */
/* NOT IN DESIGN */
OS_API void
DK_MapAdmin_ts_clear(
    DK_MapAdmin* _this,
    DLRL_Exception* exception,
    void* userData);

/* NOT IN DESIGN */
OS_API void
DK_MapAdmin_ts_getLSValues(
    DK_MapAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg);

OS_API void
DK_MapAdmin_ts_getLSElementsGeneric(
    DK_MapAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg,
    DK_MapAdmin_ElementType type);
/**********************************************************************************************************************
********************************** SetAdmin (extends Collection which extends entity) *********************************
**********************************************************************************************************************/

/* \brief Returns the number of elements contained in the <code>DK_SetAdmin</code>.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_SetAdmin</code> has already been deleted.</li>
 * </ul>
 *
 * \param _this The <code>DK_SetAdmin</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 *
 * \return the number of contained elements.
 */
OS_API LOC_unsigned_long
DK_SetAdmin_ts_getLength(
    DK_SetAdmin* _this,
    DLRL_Exception* exception);

/* \brief Stores the specified item into the Set.
 *
 * If the item was already contained in the Set, then this operation will have no effect.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_SetAdmin</code> has already been deleted.</li>
 * </ul>
 *
 * \param _this The <code>DK_SetAdmin</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param userData Optional user data.
 * \param objectAdmin The item that needs to be stored in the Set.
 */
OS_API void
DK_SetAdmin_ts_add(
    DK_SetAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectAdmin* objectAdmin);

/* \brief Returns whether the specified element is already contained in the Set (true) or not (false).
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_SetAdmin</code> has already been deleted.</li>
 * </ul>
 *
 * \param _this The <code>DK_SetAdmin</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param objectAdmin The item that needs to be examined.
 *
 * \return whether the specified element is already contained in the Set.
 */
OS_API LOC_boolean
DK_SetAdmin_ts_contains(
    DK_SetAdmin* _this,
    DLRL_Exception* exception,
    DK_ObjectAdmin* objectAdmin);

/* \brief Removes the specified element from the Set.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_SetAdmin</code> has already been deleted.</li>
 * </ul>
 *
 * \param _this The <code>DK_SetAdmin</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param objectAdmin The item that needs to be removed.
 */
OS_API void
DK_SetAdmin_ts_remove(
    DK_SetAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DK_ObjectAdmin* objectAdmin);

/* \brief Removes all elements from the Set.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_PRECONDITION_NOT_MET</code> - If one of the precondition was not met</li>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_SetAdmin</code> has already been deleted.</li>
 * </ul>
 *
 * \param _this The <code>DK_SetAdmin</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param userData optional user data
 */
/* NOT IN DESIGN */
OS_API void
DK_SetAdmin_ts_clear(
    DK_SetAdmin* _this,
    DLRL_Exception* exception,
    void* userData);
/* NOT IN DESIGN */
OS_API void
DK_SetAdmin_ts_getLSValues(
    DK_SetAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg);

OS_API void
DK_SetAdmin_ts_getLSAddedElements(
    DK_SetAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg);

/* NOT IN DESIGN */
OS_API void
DK_SetAdmin_ts_getLSRemovedElements(
    DK_SetAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg);

/**********************************************************************************************************************
********************************************** Selection (extends entity) *********************************************
**********************************************************************************************************************/

/* \brief This operation returns if the <code>DK_SelectionAdmin</code> can be manually refreshed using the refresh
 * operation of the <code>DK_SelectionAdmin</code> (<code>false</code>) or if the <code>DK_SelectionAdmin</code> is
 * automatically refreshed whenever the related <code>DK_CacheAdmin</code> is refreshed (<code>true</code>).
 *
 * Take note that it does not matter if the related <code>DK_CacheAdmin</code> is in enabled or disabled update mode.
 * The current implementation will always return false.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_SelectionAdmin</code> has already been deleted.</li>
 * </ul>
 *
 * \param _this The <code>DK_SelectionAdmin</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 *
 * \return <code>true</code> if the <code>DK_SelectionAdmin</code> will be automatically refreshed and
 * <code>false</code> if it will not.
 */
OS_API LOC_boolean
DK_SelectionAdmin_ts_getAutoRefresh(
    DK_SelectionAdmin* _this,
    DLRL_Exception* exception);

/* \brief This operation returns true if the <code>DK_SelectionAdmin</code> considers change to the contained relations
 * of it's member objects as a modification to itself and false if it only takes changes to it's member objects into
 * account.
 *
 * This feature is only usefull when using the selection in combination with a selection listener, which is currently
 * unsupported. The current implementation will therefore always return false.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_SelectionAdmin</code> has already been deleted.</li>
 * </ul>
 *
 * \param _this The <code>DK_SelectionAdmin</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 *
 * \return <code>true</code> if the <code>DK_SelectionAdmin</code> sees modifications to its contained members as
 * modifications to itself and  <code>false</code> otherwise.
 */
OS_API LOC_boolean
DK_SelectionAdmin_ts_getConcernsContained(
    DK_SelectionAdmin* _this,
    DLRL_Exception* exception);

/* \brief This operation updates the membership of the <code>DK_SelectionAdmin</code>.
 *
 * Any objects that no longer pass the criterion are removed and objects that now match the criterion are added. If
 * this operation is called the <code>DK_SelectionAdmin_ts_getAutoRefresh(...)</code> operation returns <code>true</code>
 * then this operation is considered a no-op.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_ALREADY_DELETED</code> - If the specified <code>DK_SelectionAdmin</code> has already been deleted.</li>
 * </ul>
 *
 * \param _this The <code>DK_SelectionAdmin</code> entity that is the target of this operation.
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param userData Optional user data.
 */
OS_API void
DK_SelectionAdmin_ts_refresh(
    DK_SelectionAdmin* _this,
    DLRL_Exception* exception,
    void* userData);

OS_API DLRL_LS_object
DK_SelectionAdmin_ts_getCriterion(
    DK_SelectionAdmin* _this,
    DLRL_Exception* exception,
    void* userData);

/* NOT IN DESIGN */
OS_API void
DK_SelectionAdmin_ts_getLSMembers(
    DK_SelectionAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg);

OS_API void
DK_SelectionAdmin_ts_getLSInsertedMembers(
    DK_SelectionAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg);

OS_API void
DK_SelectionAdmin_ts_getLSModifiedMembers(
    DK_SelectionAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg);

OS_API void
DK_SelectionAdmin_ts_getLSRemovedMembers(
    DK_SelectionAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    void** arg);

OS_API DLRL_LS_object
DK_SelectionAdmin_ts_getListener(
    DK_SelectionAdmin* _this,
    DLRL_Exception* exception,
    void* userData);

OS_API DLRL_LS_object
DK_SelectionAdmin_ts_setListener(
    DK_SelectionAdmin* _this,
    DLRL_Exception* exception,
    void* userData,
    DLRL_LS_object listener);
/**********************************************************************************************************************
********************************************** DCPSUTILITY ************************************************************
**********************************************************************************************************************/

/* NOT IN DESIGN */
OS_API u_instanceHandle
DK_DCPSUtility_ts_getNilHandle();

/**********************************************************************************************************************
*************************************************** Meta model facade *************************************************
***************************************** temporarily moved to this file *********************************************/

/* \brief Creates a MetaModel DLRLClass object and sets it as the MetaModel representative of the provided home
 *
 * This operation is a part of the MetaModel facade used to create and fill the DLRL meta model with type specific
 * information. This operation is intended to be used only within the context of the callback operation
 * '<code>DK_ObjectHomeBridge_us_loadMetamodel(...)</code>'. Both mentioned preconditions of this operation have been
 * met if used within the context of the mentioned callback operation.
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_ObjectHomeAdmin</code>. (see description)</li>
 * <li>Must verify the <code>DK_ObjectHomeAdmin</code> is still alive. (see description)</li></ul>
 *
 * \param home The <code>DK_ObjectHomeAdmin</code> entity that is the root for the to-be-inserted MetaModel information.
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param parentName The name of the parent object type (fully qualified, IE the same format as used for the
 * <code>DK_ObjectHomeAdmin</code> name)
 * \param mapping To indicate if this object is mapped using default mapping rules of pre-defined mapping rules.
 */
OS_API void
DK_MMFacade_us_createDLRLClass(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    const LOC_string parentName,
    DMM_Mapping mapping);

/* \brief Creates a MetaModel DCPS topic as the main topic for the type represented by the specified object home.
 *
 * This operation is a part of the MetaModel facade used to create and fill the DLRL meta model with type specific
 * information. This operation is intended to be used only within the context of the callback operation
 * '<code>DK_ObjectHomeBridge_us_loadMetamodel(...)</code>'. The first two mentioned preconditions of this operation
 * have been met if used within the context of the mentioned callback operation.
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_ObjectHomeAdmin</code>. (see description)</li>
 * <li>Must verify the <code>DK_ObjectHomeAdmin</code> is still alive. (see description)</li>
 * <li>Must have created a DLRLClass object first for the specified <code>DK_ObjectHomeAdmin</code> by calling the
 * <code>DK_MMFacade_us_createDLRLClass(...)</code> operation first</li></ul>
 *
 * \param home The <code>DK_ObjectHomeAdmin</code> entity that is the root for the to-be-inserted MetaModel information.
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param name The (application specific) name of the topic
 * \param typeName The fully qualified IDL type name used to correctly identify the topic.
 */
OS_API void
DK_MMFacade_us_createMainTopic(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    LOC_string name,
    LOC_string typeName);

/* \brief Creates a MetaModel DCPS topic as the extension topic for the type represented by the specified object home.
 *
 * This operation is a part of the MetaModel facade used to create and fill the DLRL meta model with type specific
 * information. This operation is intended to be used only within the context of the callback operation
 * '<code>DK_ObjectHomeBridge_us_loadMetamodel(...)</code>'. The first two mentioned preconditions of this operation
 * have been met if used within the context of the mentioned callback operation.
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_ObjectHomeAdmin</code>. (see description)</li>
 * <li>Must verify the <code>DK_ObjectHomeAdmin</code> is still alive. (see description)</li>
 * <li>Must have created a DLRLClass object first for the specified <code>DK_ObjectHomeAdmin</code> by calling the
 * <code>DK_MMFacade_us_createDLRLClass(...)</code> operation first</li></ul>
 *
 * \param home The <code>DK_ObjectHomeAdmin</code> entity that is the root for the to-be-inserted MetaModel information.
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param name The (application specific) name of the topic
 * \param typeName The fully qualified IDL type name used to correctly identify the topic.
 */
OS_API void
DK_MMFacade_us_createExtensionTopic(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    LOC_string name,
    LOC_string typeName);

/* \brief Creates a MetaModel DCPS topic as one of the place topics for the type represented by the specified object
 * home.
 *
 * This operation is a part of the MetaModel facade used to create and fill the DLRL meta model with type specific
 * information. This operation is intended to be used only within the context of the callback operation
 * '<code>DK_ObjectHomeBridge_us_loadMetamodel(...)</code>'. The first two mentioned preconditions of this operation
 * have been met if used within the context of the mentioned callback operation.
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_ObjectHomeAdmin</code>. (see description)</li>
 * <li>Must verify the <code>DK_ObjectHomeAdmin</code> is still alive. (see description)</li>
 * <li>Must have created a DLRLClass object first for the specified <code>DK_ObjectHomeAdmin</code> by calling the
 * <code>DK_MMFacade_us_createDLRLClass(...)</code> operation first</li></ul>
 *
 * \param home The <code>DK_ObjectHomeAdmin</code> entity that is the root for the to-be-inserted MetaModel information.
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param name The (application specific) name of the topic
 * \param typeName The fully qualified IDL type name used to correctly identify the topic.
 */
OS_API void
DK_MMFacade_us_createPlaceTopic(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    LOC_string name,
    LOC_string typeName);

/* \brief Creates a MetaModel DCPS topic as one of the multi place topics for the type represented by the specified
 * object home.
 *
 * This operation is a part of the MetaModel facade used to create and fill the DLRL meta model with type specific
 * information. This operation is intended to be used only within the context of the callback operation
 * '<code>DK_ObjectHomeBridge_us_loadMetamodel(...)</code>'. The first two mentioned preconditions of this operation
 * have been met if used within the context of the mentioned callback operation.
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_ObjectHomeAdmin</code>. (see description)</li>
 * <li>Must verify the <code>DK_ObjectHomeAdmin</code> is still alive. (see description)</li>
 * <li>Must have created a DLRLClass object first for the specified <code>DK_ObjectHomeAdmin</code> by calling the
 * <code>DK_MMFacade_us_createDLRLClass(...)</code> operation first</li></ul>
 *
 * \param home The <code>DK_ObjectHomeAdmin</code> entity that is the root for the to-be-inserted MetaModel information.
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param name The (application specific) name of the topic
 * \param typeName The fully qualified IDL type name used to correctly identify the topic.
 */
OS_API void
DK_MMFacade_us_createMultiPlaceTopic(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    LOC_string name,
    LOC_string typeName);

/* \brief Creates a MetaModel DCPS field and attaches it to the topic specified by the provided name.
 *
 * This operation is a part of the MetaModel facade used to create and fill the DLRL meta model with type specific
 * information. This operation is intended to be used only within the context of the callback operation
 * '<code>DK_ObjectHomeBridge_us_loadMetamodel(...)</code>'. The first two mentioned preconditions of this operation
 * have been met if used within the context of the mentioned callback operation.
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_ObjectHomeAdmin</code>. (see description)</li>
 * <li>Must verify the <code>DK_ObjectHomeAdmin</code> is still alive. (see description)</li>
 * <li>Must have created a DCPSTopic object with the name specified by the 'owningTopicName' attribute within the
 * context of the specified <code>DK_ObjectHomeAdmin</code> by calling one of the topic creation operations first</li>
 * </ul>
 *
 * \param home The <code>DK_ObjectHomeAdmin</code> entity that is the root for the to-be-inserted MetaModel information.
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param name The (application specific) name of the DCPSField
 * \param fieldType The key type of the field
 * \param type The attribute type of the DCPSField.
 * \param owningTopicName The (not fully qualified) name of the topic to which this field should be added
 */
OS_API void
DK_MMFacade_us_createDCPSField(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    LOC_string name,
    DMM_KeyType fieldType,
    DMM_AttributeType type,
    LOC_string owningTopicName);

/* \brief Creates a MetaModel DLRL relation for the DLRLClass within the specified object home.
 *
 * This operation is a part of the MetaModel facade used to create and fill the DLRL meta model with type specific
 * information. This operation is intended to be used only within the context of the callback operation
 * '<code>DK_ObjectHomeBridge_us_loadMetamodel(...)</code>'. The first two mentioned preconditions of this operation
 * have been met if used within the context of the mentioned callback operation.
 * Note that the target DLRL object (typeName) and associated relation (associatedRelationName, if any) do not have to
 * exist when calling this operation.
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_ObjectHomeAdmin</code>. (see description)</li>
 * <li>Must verify the <code>DK_ObjectHomeAdmin</code> is still alive. (see description)</li>
 * <li>Must have created a DLRLClass object first for the specified <code>DK_ObjectHomeAdmin</code> by calling the
 * <code>DK_MMFacade_us_createDLRLClass(...)</code> operation first</li></ul>
 *
 * \param home The <code>DK_ObjectHomeAdmin</code> entity that is the root for the to-be-inserted MetaModel information.
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param isComposition Indicates if this relation is a composite relation(<code>true</code>) or not(code>false</code>)
 * \param name The name of the relation
 * \param typeName The fully qualified name of the target DLRL object type of this relation.
 * \param associatedRelationName The name of the associated relation within the target DLRL object. May be NULL if no
 * associated relation exists.
 * \param isOptional Indicates of this relation may a NIL pointer or not
 */
 /* NOT IN DESIGN - isOptional param*/
OS_API void
DK_MMFacade_us_createRelation(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    LOC_boolean isComposition,
    LOC_string name,
    LOC_string typeName,
    LOC_string associatedRelationName,
    LOC_boolean isOptional);

/* \brief Creates a MetaModel DLRL multi relation for the DLRLClass within the specified object home.
 *
 * This operation is a part of the MetaModel facade used to create and fill the DLRL meta model with type specific
 * information. This operation is intended to be used only within the context of the callback operation
 * '<code>DK_ObjectHomeBridge_us_loadMetamodel(...)</code>'. The first two mentioned preconditions of this operation
 * have been met if used within the context of the mentioned callback operation.
 * Note that the target DLRL object (typeName) and associated mulyi relation (associatedRelationName, if any) do not
 * have to exist when calling this operation.
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_ObjectHomeAdmin</code>. (see description)</li>
 * <li>Must verify the <code>DK_ObjectHomeAdmin</code> is still alive. (see description)</li>
 * <li>Must have created a DLRLClass object first for the specified <code>DK_ObjectHomeAdmin</code> by calling the
 * <code>DK_MMFacade_us_createDLRLClass(...)</code> operation first</li></ul>
 *
 * \param home The <code>DK_ObjectHomeAdmin</code> entity that is the root for the to-be-inserted MetaModel information.
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param isComposition Indicates if this multi relation is a composite multi relation(<code>true</code>) or
 * not(code>false</code>)
 * \param name The name of the multi relation
 * \param typeName The fully qualified name of the target DLRL object type of this multi relation.
 * \param associatedRelationName The name of the associated multi relation within the target DLRL object. May be NULL
 * if no associated multi relation exists.
 * \param basis The base type of the multi relation (MAP/SET/etc)
 */
OS_API void
DK_MMFacade_us_createMultiRelation(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    LOC_boolean isComposition,
    LOC_string name,
    LOC_string typeName,
    LOC_string associatedRelationName,
    DMM_Basis basis);

/* \brief Creates a MetaModel DLRL attribute for the DLRLClass within the specified object home.
 *
 * This operation is a part of the MetaModel facade used to create and fill the DLRL meta model with type specific
 * information. This operation is intended to be used only within the context of the callback operation
 * '<code>DK_ObjectHomeBridge_us_loadMetamodel(...)</code>'. The first two mentioned preconditions of this operation
 * have been met if used within the context of the mentioned callback operation.
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_ObjectHomeAdmin</code>. (see description)</li>
 * <li>Must verify the <code>DK_ObjectHomeAdmin</code> is still alive. (see description)</li>
 * <li>Must have created a DLRLClass object first for the specified <code>DK_ObjectHomeAdmin</code> by calling the
 * <code>DK_MMFacade_us_createDLRLClass(...)</code> operation first</li></ul>
 *
 * \param home The <code>DK_ObjectHomeAdmin</code> entity that is the root for the to-be-inserted MetaModel information.
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param name The name of the attribute
 * \param isImmutable Indicates if the attribute can be changed after the DLRL object is registered.
 * \param type The attribute type of the DLRL attribute.
 */
OS_API void
DK_MMFacade_us_createAttribute(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    LOC_string name,
    LOC_boolean isImmutable,
    DMM_AttributeType type);

/* \brief Creates a MetaModel DLRL multi attribute for the DLRLClass within the specified object home.
 *
 * This operation is a part of the MetaModel facade used to create and fill the DLRL meta model with type specific
 * information. This operation is intended to be used only within the context of the callback operation
 * '<code>DK_ObjectHomeBridge_us_loadMetamodel(...)</code>'. The first two mentioned preconditions of this operation
 * have been met if used within the context of the mentioned callback operation.
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_ObjectHomeAdmin</code>. (see description)</li>
 * <li>Must verify the <code>DK_ObjectHomeAdmin</code> is still alive. (see description)</li>
 * <li>Must have created a DLRLClass object first for the specified <code>DK_ObjectHomeAdmin</code> by calling the
 * <code>DK_MMFacade_us_createDLRLClass(...)</code> operation first</li></ul>
 *
 * \param home The <code>DK_ObjectHomeAdmin</code> entity that is the root for the to-be-inserted MetaModel information.
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param name The name of the multi attribute
 * \param isImmutable Indicates if the multi attribute can be changed after the DLRL object is registered.
 * \param type The target attribute type of the DLRL multi attribute.
 * \param basis The base type of the multi attribute (MAP/SET/etc)
 */
OS_API void
DK_MMFacade_us_createMultiAttribute(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    LOC_string name,
    LOC_boolean isImmutable,
    DMM_AttributeType type,
    DMM_Basis basis);

/* \brief Maps an already existing DLRLAttribute object to an already existing DCPSField object.
 *
 * This operation is a part of the MetaModel facade used to create and fill the DLRL meta model with type specific
 * information. This operation is intended to be used only within the context of the callback operation
 * '<code>DK_ObjectHomeBridge_us_loadMetamodel(...)</code>'. The first two mentioned preconditions of this operation
 * have been met if used within the context of the mentioned callback operation.
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_ObjectHomeAdmin</code>. (see description)</li>
 * <li>Must verify the <code>DK_ObjectHomeAdmin</code> is still alive. (see description)</li>
 * <li>Must have created a DLRLClass object first for the specified <code>DK_ObjectHomeAdmin</code> by calling the
 * <code>DK_MMFacade_us_createDLRLClass(...)</code> operation first</li>
 * <li>Must have created the DLRLAtttribute specified by the 'attributeName' attribute first  by calling the
 * <code>DK_MMFacade_us_createAttribute(...)</code> operation..</li>
 * <li>Must have mapped the specified DLRLAttribute to a DCPS topic first by calling the
 * <code>DK_MMFacade_us_mapDLRLAttributeToDCPSTopic(...)</code> operation.</li>
 * <li>Must have created the DCPSField specified by the 'fieldName' attribute first within the topic the DLRL
 * attribute is mapped to by calling the <code>DK_MMFacade_us_createDCPSField(...)</code> operation..</li></ul>
 *
 * \param home The <code>DK_ObjectHomeAdmin</code> entity that is the root for the to-be-inserted MetaModel information.
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param attributeName The name of the attribute
 * \param fieldName The name of the DCPSField
 */
OS_API void
DK_MMFacade_us_mapDLRLAttributeToDCPSField(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    LOC_string attributeName,
    LOC_string fieldName);

/* \brief Maps an already existing multi DLRLAttribute object to an already existing (index) DCPSField object.
 *
 * This operation is a part of the MetaModel facade used to create and fill the DLRL meta model with type specific
 * information. This operation is intended to be used only within the context of the callback operation
 * '<code>DK_ObjectHomeBridge_us_loadMetamodel(...)</code>'. The first two mentioned preconditions of this operation
 * have been met if used within the context of the mentioned callback operation.
 * This DCPSField used here should be marked as a key of the topic and will be used as the index field of the multi
 * attribute.
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_ObjectHomeAdmin</code>. (see description)</li>
 * <li>Must verify the <code>DK_ObjectHomeAdmin</code> is still alive. (see description)</li>
 * <li>Must have created a DLRLClass object first for the specified <code>DK_ObjectHomeAdmin</code> by calling the
 * <code>DK_MMFacade_us_createDLRLClass(...)</code> operation first</li>
 * <li>Must have created the DLRLAtttribute specified by the 'attributeName' attribute first  by calling the
 * <code>DK_MMFacade_us_createAttribute(...)</code> operation..</li>
 * <li>Must have mapped the specified DLRLAttribute to a DCPS topic first by calling the
 * <code>DK_MMFacade_us_mapDLRLAttributeToDCPSTopic(...)</code> operation.</li>
 * <li>Must have created the DCPSField specified by the 'fieldName' attribute first within the topic the DLRL
 * attribute is mapped to by calling the <code>DK_MMFacade_us_createDCPSField(...)</code> operation..</li></ul>
 *
 * \param home The <code>DK_ObjectHomeAdmin</code> entity that is the root for the to-be-inserted MetaModel information.
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param attributeName The name of the attribute
 * \param indexFieldName The name of the DCPSField to be used as the index field for this multi attribute.
 */
OS_API void
DK_MMFacade_us_mapDLRLMultiAttributeToIndexDCPSField(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    LOC_string attributeName,
    LOC_string indexFieldName);

/* \brief Maps an already existing (multi) DLRLAttribute object to an already existing DCPSTopic object.
 *
 * This operation is a part of the MetaModel facade used to create and fill the DLRL meta model with type specific
 * information. This operation is intended to be used only within the context of the callback operation
 * '<code>DK_ObjectHomeBridge_us_loadMetamodel(...)</code>'. The first two mentioned preconditions of this operation
 * have been met if used within the context of the mentioned callback operation.
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_ObjectHomeAdmin</code>. (see description)</li>
 * <li>Must verify the <code>DK_ObjectHomeAdmin</code> is still alive. (see description)</li>
 * <li>Must have created a DLRLClass object first for the specified <code>DK_ObjectHomeAdmin</code> by calling the
 * <code>DK_MMFacade_us_createDLRLClass(...)</code> operation first</li>
 * <li>Must have created the DLRLAtttribute specified by the 'attributeName' attribute first  by calling the
 * <code>DK_MMFacade_us_createAttribute(...)</code> operation.</li>
 * <li>Must have created a DCPSTopic object with the name specified by the 'topicName' attribute within the
 * context of the specified <code>DK_ObjectHomeAdmin</code> by calling one of the topic creation operations first</li>
 * </ul>
 *
 * \param home The <code>DK_ObjectHomeAdmin</code> entity that is the root for the to-be-inserted MetaModel information.
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param attributeName The name of the attribute
 * \param topicName The name of the DCPSTopic
 */
OS_API void
DK_MMFacade_us_mapDLRLAttributeToDCPSTopic(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    LOC_string attributeName,
    LOC_string topicName);

/* \brief Adds a key field pair to the relation which describes which field in the owning topic maps to which field in
 * the target topic.
 *
 * This operation is a part of the MetaModel facade used to create and fill the DLRL meta model with type specific
 * information. This operation is intended to be used only within the context of the callback operation
 * '<code>DK_ObjectHomeBridge_us_loadMetamodel(...)</code>'. The first two mentioned preconditions of this operation
 * have been met if used within the context of the mentioned callback operation.
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_ObjectHomeAdmin</code>. (see description)</li>
 * <li>Must verify the <code>DK_ObjectHomeAdmin</code> is still alive. (see description)</li>
 * <li>Must have created a DLRLClass object first for the specified <code>DK_ObjectHomeAdmin</code> by calling the
 * <code>DK_MMFacade_us_createDLRLClass(...)</code> operation first</li>
 * <li>Must have created the DLRLRelation specified by the 'relationName' attribute first by calling the
 * <code>DK_MMFacade_us_createRelation(...)</code> operation.</li>
 * <li>Must have mapped the specified DLRLRelation to a DCPS topic first by calling the
 * <code>DK_MMFacade_us_setDLRLRelationTopicPair(...)</code> operation.</li>
 * <li>Must have created the DCPSField as a specified by the 'ownerKeyName' attribute first within the topic the DLRL
 * attribute is mapped to by calling the <code>DK_MMFacade_us_createDCPSField(...)</code> operation..</li></ul>
 *
 * \param home The <code>DK_ObjectHomeAdmin</code> entity that is the root for the to-be-inserted MetaModel information.
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param relationName The name of the relation
 * \param ownerKeyName The name of the DCPSField which represents the keyfield in the owning topic
 * \param targetKeyName The name of the DCPSField which represents the keyfield in the target topic
 */
OS_API void
DK_MMFacade_us_addDLRLRelationKeyFieldPair(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    LOC_string relationName,
    LOC_string ownerKeyName,
    LOC_string targetKeyName);

/* NOT IN DESIGN */
OS_API void
DK_MMFacade_us_setDLRLRelationValidityField(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    LOC_string relationName,
    LOC_string validityFieldName);

/* \brief Sets the already existing owning topic and related target topic for a specific relation.
 *
 * This operation is a part of the MetaModel facade used to create and fill the DLRL meta model with type specific
 * information. This operation is intended to be used only within the context of the callback operation
 * '<code>DK_ObjectHomeBridge_us_loadMetamodel(...)</code>'. The first two mentioned preconditions of this operation
 * have been met if used within the context of the mentioned callback operation.
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_ObjectHomeAdmin</code>. (see description)</li>
 * <li>Must verify the <code>DK_ObjectHomeAdmin</code> is still alive. (see description)</li>
 * <li>Must have created a DLRLClass object first for the specified <code>DK_ObjectHomeAdmin</code> by calling the
 * <code>DK_MMFacade_us_createDLRLClass(...)</code> operation first</li>
 * <li>Must have created the DLRLRelation specified by the 'relationName' attribute first by calling the
 * <code>DK_MMFacade_us_createRelation(...)</code> operation.</li>
 * <li>Must have created a DCPSTopic objects with the names specified by the 'ownerTopicName' and 'targetTopicName'
 * attributes within the context of the specified <code>DK_ObjectHomeAdmin</code> by calling one of the topic creation
 * operations (one time for each topic) first</li>
 * </ul>
 *
 * \param home The <code>DK_ObjectHomeAdmin</code> entity that is the root for the to-be-inserted MetaModel information.
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param relationName The name of the relation
 * \param ownerTopicName The name of the DCPSTopic which represents the owning topic
 * \param targetTopicName The name of the DCPSTopic which represents the target topic
 */
OS_API void
DK_MMFacade_us_setDLRLRelationTopicPair(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    LOC_string relationName,
    LOC_string ownerTopicName,
    LOC_string targetTopicName);

/* \brief Adds an owner field to a MultiRelation.
 *
 * <p>An owner field represents the DCPSField within the owner topic of the 'owning' DLRLClass. This topic is determined
 * by calling the operation <code>DK_MMFacade_us_setDLRLRelationTopicPair(...)</code>. The owner field is placed in a
 * list, starting at index 0. The sequence of owner fields being added MUST be the same as the sequence in which the
 * owner fields for the topic describing the MultiRelation are added (represented by the calls to the operation
 * <code>DK_MMFacade_us_addRelationTopicOwnerField(...)</code>). This topic describing the MultiRelation is determined by
 * a call to the <code>DK_MMFacade_us_setMultiRelationRelationTopic(...)</code> operation. </p>
 *
 * This operation is a part of the MetaModel facade used to create and fill the DLRL meta model with type specific
 * information. This operation is intended to be used only within the context of the callback operation
 * '<code>DK_ObjectHomeBridge_us_loadMetamodel(...)</code>'. The first two mentioned preconditions of this operation
 * have been met if used within the context of the mentioned callback operation.
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_ObjectHomeAdmin</code>. (see description)</li>
 * <li>Must verify the <code>DK_ObjectHomeAdmin</code> is still alive. (see description)</li>
 * <li>Must have created a DLRLClass object first for the specified <code>DK_ObjectHomeAdmin</code> by calling the
 * <code>DK_MMFacade_us_createDLRLClass(...)</code> operation first</li>
 * <li>Must have created the DLRLRelation specified by the 'relationName' attribute first by calling the
 * <code>DK_MMFacade_us_createRelation(...)</code> operation.</li>
 * <li>Must have set the owner topic of the MultiRelation first by calling the
 * <code>DK_MMFacade_us_setDLRLRelationTopicPair</code> operation.</li>
 * <li>Must have created a DCPSField object with the name specified by the 'fieldName' attribute within the
 * context of the specified DCPSTopic by calling the <code>DK_MMFacade_us_createDCPSField</code> operation first</li>
 * </ul>
 *
 * \param home The <code>DK_ObjectHomeAdmin</code> entity that is the root for the to-be-inserted MetaModel information.
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param relationName The name of the relation
 * \param fieldName The name of the to-be-added owner DCPSField as created within the relation topic.
 */
OS_API void
DK_MMFacade_us_addOwnerField(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    LOC_string relationName,
    LOC_string fieldName);

/* \brief Adds an target field to a MultiRelation.
 *
 * <p>An target field represents the DCPSField within the target topic of the 'target' DLRLClass. This topic is
 * determined by calling the operation <code>DK_MMFacade_us_setDLRLRelationTopicPair(...)</code>. The target field is
 * placed in a list, starting at index 0. The sequence of target fields being added MUST be the same as the sequence in
 * which the target fields for the topic describing the MultiRelation are added (represented by the calls to the
 * operation <code>DK_MMFacade_us_addRelationTopicTargetField(...)</code>). This topic describing the MultiRelation is
 * determined by a call to the <code>DK_MMFacade_us_setMultiRelationRelationTopic(...)</code> operation. </p>
 *
 * This operation is a part of the MetaModel facade used to create and fill the DLRL meta model with type specific
 * information. This operation is intended to be used only within the context of the callback operation
 * '<code>DK_ObjectHomeBridge_us_loadMetamodel(...)</code>'. The first two mentioned preconditions of this operation
 * have been met if used within the context of the mentioned callback operation.
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_ObjectHomeAdmin</code>. (see description)</li>
 * <li>Must verify the <code>DK_ObjectHomeAdmin</code> is still alive. (see description)</li>
 * <li>Must have created a DLRLClass object first for the specified <code>DK_ObjectHomeAdmin</code> by calling the
 * <code>DK_MMFacade_us_createDLRLClass(...)</code> operation first</li>
 * <li>Must have created the DLRLRelation specified by the 'relationName' attribute first by calling the
 * <code>DK_MMFacade_us_createRelation(...)</code> operation.</li>
 * <li>Must have set the target topic of the MultiRelation first by calling the
 * <code>DK_MMFacade_us_setDLRLRelationTopicPair</code> operation.</li>
 * <li>Must have created a DCPSField object with the name specified by the 'fieldName' attribute within the
 * context of the specified DCPSTopic by calling the <code>DK_MMFacade_us_createDCPSField</code> operation first</li>
 * </ul>
 *
 * \param home The <code>DK_ObjectHomeAdmin</code> entity that is the root for the to-be-inserted MetaModel information.
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param relationName The name of the relation
 * \param fieldName The name of the to-be-added target DCPSField as created within the relation topic.
 */
OS_API void
DK_MMFacade_us_addTargetField(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    LOC_string relationName,
    LOC_string fieldName);

/* \brief Sets the topic that describes the MultiRelation elements.
 *
 * This operation is a part of the MetaModel facade used to create and fill the DLRL meta model with type specific
 * information. This operation is intended to be used only within the context of the callback operation
 * '<code>DK_ObjectHomeBridge_us_loadMetamodel(...)</code>'. The first two mentioned preconditions of this operation
 * have been met if used within the context of the mentioned callback operation.
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_ObjectHomeAdmin</code>. (see description)</li>
 * <li>Must verify the <code>DK_ObjectHomeAdmin</code> is still alive. (see description)</li>
 * <li>Must have created a DLRLClass object first for the specified <code>DK_ObjectHomeAdmin</code> by calling the
 * <code>DK_MMFacade_us_createDLRLClass(...)</code> operation first</li>
 * <li>Must have created the DLRLRelation specified by the 'relationName' attribute first by calling the
 * <code>DK_MMFacade_us_createRelation(...)</code> operation.</li>
 * <li>Must have created a DCPSTopic object with the names specified by the 'relationTopicName' attribute within the
 * context of the specified <code>DK_ObjectHomeAdmin</code> by calling one of the topic creation  operations first</li>
 * </ul>
 *
 * \param home The <code>DK_ObjectHomeAdmin</code> entity that is the root for the to-be-inserted MetaModel information.
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param relationName The name of the relation
 * \param relationTopicName The name of the relation topic.
 */
OS_API void
DK_MMFacade_us_setMultiRelationRelationTopic(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    LOC_string relationName,
    LOC_string relationTopicName);

/* \brief Adds an owner field of the relation topic to a MultiRelation.
 *
 * <p>An owner field represents the DCPSField within the relation topic which describes the MultiRelation. This topic is
 * set by calling the operation <code>DK_MMFacade_us_setMultiRelationRelationTopic(...)</code>. The owner field is
 * placed in a list, starting at index 0. The sequence of owner fields being added MUST be the same as the sequence in
 * which the owner fields for the topic of the 'owning' DLRLClass are added (represented by the calls to the operation
 * <code>DK_MMFacade_us_addOwnerField(...)</code>). This topic is determined by a call to the
 * <code>DK_MMFacade_us_setDLRLRelationTopicPair(...)</code> operation. </p>
 *
 * This operation is a part of the MetaModel facade used to create and fill the DLRL meta model with type specific
 * information. This operation is intended to be used only within the context of the callback operation
 * '<code>DK_ObjectHomeBridge_us_loadMetamodel(...)</code>'. The first two mentioned preconditions of this operation
 * have been met if used within the context of the mentioned callback operation.
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_ObjectHomeAdmin</code>. (see description)</li>
 * <li>Must verify the <code>DK_ObjectHomeAdmin</code> is still alive. (see description)</li>
 * <li>Must have created a DLRLClass object first for the specified <code>DK_ObjectHomeAdmin</code> by calling the
 * <code>DK_MMFacade_us_createDLRLClass(...)</code> operation first</li>
 * <li>Must have created the DLRLRelation specified by the 'relationName' attribute first by calling the
 * <code>DK_MMFacade_us_createRelation(...)</code> operation.</li>
 * <li>Must have set the relation topic of the MultiRelation first by calling the
 * <code>DK_MMFacade_us_setMultiRelationRelationTopic</code> operation.</li>
 * <li>Must have created a DCPSField object with the name specified by the 'fieldName' attribute within the
 * context of the specified DCPSTopic by calling the <code>DK_MMFacade_us_createDCPSField</code> operation first</li>
 * </ul>
 *
 * \param home The <code>DK_ObjectHomeAdmin</code> entity that is the root for the to-be-inserted MetaModel information.
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param relationName The name of the relation
 * \param relationTopicFieldName The name of the to-be-added owner DCPSField as created within the relation topic.
 */
OS_API void
DK_MMFacade_us_addRelationTopicOwnerField(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    LOC_string relationName,
    LOC_string relationTopicFieldName);

/* \brief Adds a target field of the relation topic to a MultiRelation.
 *
 * <p>A target field represents the DCPSField within the relation topic which describes the MultiRelation. This topic is
 * set by calling the operation <code>DK_MMFacade_us_setMultiRelationRelationTopic(...)</code>. The target field is
 * placed in a list, starting at index 0. The sequence of target fields being added MUST be the same as the sequence in
 * which the target fields for the topic of the 'target' DLRLClass are added (represented by the calls to the operation
 * <code>DK_MMFacade_us_addTargetField(...)</code>). This topic is determined by a call to the
 * <code>DK_MMFacade_us_setDLRLRelationTopicPair(...)</code> operation. </p>
 *
 * This operation is a part of the MetaModel facade used to create and fill the DLRL meta model with type specific
 * information. This operation is intended to be used only within the context of the callback operation
 * '<code>DK_ObjectHomeBridge_us_loadMetamodel(...)</code>'. The first two mentioned preconditions of this operation
 * have been met if used within the context of the mentioned callback operation.
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_ObjectHomeAdmin</code>. (see description)</li>
 * <li>Must verify the <code>DK_ObjectHomeAdmin</code> is still alive. (see description)</li>
 * <li>Must have created a DLRLClass object first for the specified <code>DK_ObjectHomeAdmin</code> by calling the
 * <code>DK_MMFacade_us_createDLRLClass(...)</code> operation first</li>
 * <li>Must have created the DLRLRelation specified by the 'relationName' attribute first by calling the
 * <code>DK_MMFacade_us_createRelation(...)</code> operation.</li>
 * <li>Must have set the relation topic of the MultiRelation first by calling the
 * <code>DK_MMFacade_us_setMultiRelationRelationTopic</code> operation.</li>
 * <li>Must have created a DCPSField object with the name specified by the 'fieldName' attribute within the
 * context of the specified DCPSTopic by calling the <code>DK_MMFacade_us_createDCPSField</code> operation first</li>
 * </ul>
 *
 * \param home The <code>DK_ObjectHomeAdmin</code> entity that is the root for the to-be-inserted MetaModel information.
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param relationName The name of the relation
 * \param relationTopicFieldName The name of the to-be-added owner DCPSField as created within the relation topic.
 */
OS_API void
DK_MMFacade_us_addRelationTopicTargetField(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    LOC_string relationName,
    LOC_string relationTopicFieldName);

/* \brief Adds an index field of the relation topic to a MultiRelation.
 *
 * <p>An index field represents the DCPSField within the relation topic which describes the MultiRelation. This topic is
 * set by calling the operation <code>DK_MMFacade_us_setMultiRelationRelationTopic(...)</code>.</p>
 *
 * This operation is a part of the MetaModel facade used to create and fill the DLRL meta model with type specific
 * information. This operation is intended to be used only within the context of the callback operation
 * '<code>DK_ObjectHomeBridge_us_loadMetamodel(...)</code>'. The first two mentioned preconditions of this operation
 * have been met if used within the context of the mentioned callback operation.
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_ObjectHomeAdmin</code>. (see description)</li>
 * <li>Must verify the <code>DK_ObjectHomeAdmin</code> is still alive. (see description)</li>
 * <li>Must have created a DLRLClass object first for the specified <code>DK_ObjectHomeAdmin</code> by calling the
 * <code>DK_MMFacade_us_createDLRLClass(...)</code> operation first</li>
 * <li>Must have created the DLRLRelation specified by the 'relationName' attribute first by calling the
 * <code>DK_MMFacade_us_createRelation(...)</code> operation.</li>
 * <li>Must have set the relation topic of the MultiRelation first by calling the
 * <code>DK_MMFacade_us_setMultiRelationRelationTopic</code> operation.</li>
 * <li>Must have created a DCPSField object with the name specified by the 'fieldName' attribute within the
 * context of the specified DCPSTopic by calling the <code>DK_MMFacade_us_createDCPSField</code> operation first</li>
 * </ul>
 *
 * \param home The <code>DK_ObjectHomeAdmin</code> entity that is the root for the to-be-inserted MetaModel information.
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param relationName The name of the relation
 * \param relationTopicFieldName The name of the to-be-added owner DCPSField as created within the relation topic.
 */
OS_API void
DK_MMFacade_us_setRelationTopicIndexField(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    LOC_string relationName,
    LOC_string relationTopicFieldName);

/* \brief Retrieves the name of the target DLRLClass for a specific relation.
 *
 * Return value is only valid while the administrative lock on the <code>DK_ObjectHomeAdmin</code> is maintained.
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_ObjectHomeAdmin</code>.</li>
 * <li>Must verify the <code>DK_ObjectHomeAdmin</code> is still alive.</li>
 * <li>Must have created a DLRLClass object first for the specified <code>DK_ObjectHomeAdmin</code> by calling the
 * <code>DK_MMFacade_us_createDLRLClass(...)</code> operation first</li></ul>
 *
 * \param home The <code>DK_ObjectHomeAdmin</code> entity that is the root of the MetaModel information.
 * \param relationName The name of the relation
 *
 * \return <code>NULL</code> if and only if no target relation can be found, otherwise returns the name of the target
 * type. This returned string should NOT be freed, it is direct reference to the string used within the MetaModel.
 */
OS_API LOC_string
DK_MMFacade_us_getTargetTypeNameForRelation(
    DK_ObjectHomeAdmin* home,
    LOC_string relationName);

/* \brief Determines the type of a specific relation (ref/map/set/etc)
 *
 * Preconditions:<ul>
 * <li>Must claim the administrative lock on the <code>DK_ObjectHomeAdmin</code>. (see description)</li>
 * <li>Must verify the <code>DK_ObjectHomeAdmin</code> is still alive. (see description)</li>
 * <li>Must have created a DLRLClass object first for the specified <code>DK_ObjectHomeAdmin</code> by calling the
 * <code>DK_MMFacade_us_createDLRLClass(...)</code> operation first</li></ul>
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_BAD_HOME_DEFINITION</code> - If no relation with the specified name can be found.
 * </li></ul>
 *
 * \param home The <code>DK_ObjectHomeAdmin</code> entity that is the root of the MetaModel information.
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param relationName The name of the relation for which the type needs to be determined.
 *
 * \return The type of the relation, invalid if an exception occured.
 */
OS_API DK_RelationType
DK_MMFacade_us_getRelationType(
    DK_ObjectHomeAdmin* home,
    DLRL_Exception* exception,
    LOC_string relationName);

/* NOT IN DESIGN */
OS_API void
DK_MMFacade_us_getKeyFieldNamesForDefaultMappedObject(
    DLRL_Exception* exception,
    DK_ObjectHomeAdmin* home,
    LOC_string* oidfieldName,
    LOC_string* nameField);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_KERNEL_H */
