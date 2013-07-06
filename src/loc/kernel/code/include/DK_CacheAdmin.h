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
#ifndef DLRL_KERNEL_CACHE_ADMIN_H
#define DLRL_KERNEL_CACHE_ADMIN_H

/* OS abstraction layer includes */
#include "os_mutex.h"

/* user layer include */
#include "u_waitset.h"

/* DLRL util includes */
#include "DLRL_Exception.h"
#include "DLRL_Types.h"

/* Collection includes */
#include "Coll_Set.h"
#include "Coll_List.h"

/* DLRL kernel includes */
#include "DK_CacheAccessAdmin.h"
#include "DK_CacheBase.h"
#include "DK_Entity.h"
#include "DK_ObjectHomeAdmin.h"
#include "DK_Types.h"
#include "DLRL_Kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif

struct DK_CacheAdmin_s
{
    /* A <code>DK_CacheAdmin</code> extends the <code>DK_CacheBase</code> class. This parameter must always be the
     * first parameter of this struct so that it may be cast back to the <code>DK_CacheBase</code> struct.
     */
    DK_CacheBase base;
    /* The (composite) set of <code>DK_CacheAccessAdmin</code> objects created at this <code>DK_CacheAdmin</code> object.
     */
    Coll_Set cache_accesses;
    /* The (composite) ordered list of <code>DK_ObjectHomeAdmin</code> objects registered to this
     * <code>DK_CacheAdmin</code> object.
     */
    Coll_List homes;
    /* The (composite) set of cache listener objects currently attached to this <code>DK_CacheAdmin</code> object.
     */
    Coll_Set listeners;
    /* The DCPS domain participant which was provided upon creation of this <code>DK_CacheAdmin</code> object. All
     * other DCPS entities within the scope of this <code>DK_CacheAdmin</code> object will be attached within the scope
     * of this participant.
     */
    u_participant participant;
    /* The language specific representative of the DCPS domain participant with which this <code>DK_CacheAdmin</code>
     * object was created.
     */
    DLRL_LS_object ls_participant;
    /* The DCPS publisher that is created for use by this DLRL <code>DK_CacheAdmin</code> if the usage (see parent
     * class) of this cache is <code>DK_USAGE_WRITE_ONLY</code> or <code>DK_USAGE_READ_WRITE</code>. Otherwise
     * this attribute will be <code>NULL</code>. All writers created for each type (home) will be attached to this
     * publisher.
     */
    u_publisher publisher;
    /* The language specific representative of the DCPS publisher created for use by this DLRL
     * <code>DK_CacheAdmin</code> if the usage (see parent class) of this cache is <code>DK_USAGE_WRITE_ONLY</code> or
     * <code>DK_USAGE_READ_WRITE</code>. Otherwise this attribute will be <code>NULL</code>.
     */
    DLRL_LS_object ls_publisher;
    /* The DCPS publisher that is created for use by this DLRL <code>DK_CacheAdmin</code> if the usage (see parent
     * class) of this cache is <code>DK_USAGE_READ_ONLY</code> or <code>DK_USAGE_READ_WRITE</code>. Otherwise
     * this attribute will be <code>NULL</code>. All readers created for each type (home) will be attached to this
     * subscriber.
     */
    u_subscriber subscriber;
    /* The language specific representative of the DCPS subscriber created for use by this DLRL
     * <code>DK_CacheAdmin</code> if the usage (see parent class) of this cache is <code>DK_USAGE_READ_ONLY</code> or
     * <code>DK_USAGE_READ_WRITE</code>. Otherwise this attribute will be <code>NULL</code>.
     */
    DLRL_LS_object ls_subscriber;
    /* Enumeration to indicate the current pub/sub state of this <code>DK_CacheAdmin</code> object.
     */
    DK_PubSubState pub_sub_state;
    /* Boolean to indicate whether this <code>DK_CacheAdmin</code> object is operating in manual update mode (
     * <code>FALSE</code>) or in automated update mode (<code>TRUE</code>).
     */
    LOC_boolean updatesEnabled;
    /* This mutex is locked during all update related actions upon this <code>DK_CacheAdmin</code> object. Each
     * operation defined for this class will note whether or not this mutex is locked.
     */
    os_mutex updateMutex;
    /* This mutex is locked during all non-update related actions upon this <code>DK_CacheAdmin</code> object. Each
     * operation defined for this class will note whether or not this mutex is locked.
     */
    os_mutex adminMutex;
    /* The name of this cache. Note that this name is 'owned' by the <code>DK_CacheFactoryAdmin</code> singleton, so
     * this name may not be freed by the cache itself, but always from the context of the singleton.
     */
    LOC_string name;
    /* The dispatcher used by this <code>DK_CacheAdmin</code> object when the 'updatesEnabled' attribute is
     * <code>TRUE</code> and the usage (see parent class) of this cache is <code>DK_USAGE_READ_ONLY</code> or
     * <code>DK_USAGE_READ_WRITE</code>. Otherwise this attribute will be <code>NULL</code>.
     */
    DK_EventDispatcher* dispatcher;
};

/* \brief Constructs a new <code>DK_CacheAdmin</code> object.
 *
 * Using the release/duplicate operations defined on the base entity class will ensure that the memory is automatically
 * freed.
 *
 * Possible exceptions:<ul>
 * <li><code>DLRL_OUT_OF_MEMORY</code> - If creation failed due to a lack of resources.</li>
 * <li><code>DLRL_DCPS_ERROR</code> - If creation failed due to an error in DCPS while trying to create the
 * publisher or subscriber or subscriber listener DCPS entities
 * </li></ul>
 *
 * Postconditions:<ul>
 * <li>Must delete the returned <code>DK_CacheAdmin</code> object using the destructor of this class (if not
 * <code>NULL</code>).</li>
 * <li>Must release the returned pointer (if not <code>NULL</code> and after its destroyed).</li></ul>
 *
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param ls_cache The language specific representative of the newly to be created <code>DK_CacheAdmin</code>.
 * \param userData Optional user data.
 * \param usage Indicates the future usage of the Cache.
 * \param participant Specifies the DCPS Domainparticipant that is to be used.
 * \param ls_participant The language specific participant representative of the <code>participant</code> parameter.
 * \param name The name of the <code>DK_CacheAdmin</code> object.
 *
 * \return <code>NULL</code> if and only if an exception occurred during the construction of the
 * <code>DK_CacheAdmin</code> object. Otherwise returns the created <code>DK_CacheAdmin</code> object.
 */
DK_CacheAdmin*
DK_CacheAdmin_new(
    DLRL_Exception* exception,
    DLRL_LS_object ls_cache,
    void* userData,
    const DK_Usage usage,
    u_participant participant,
    DLRL_LS_object ls_participant,
    const LOC_string name);

/* \brief Cleans up all resources managed by this <code>DK_CacheAdmin</code> object.
 *
 * After this operation the <code>DK_CacheAdmin</code> object in question becomes invalid and will throw a
 * <code>DLRL_ALREADY_DELETED</code> exception when trying to execute operations on it. After this operation it is
 * safe to release the pointer to the <code>DK_CacheAdmin</code> object. Memory is freed automatically when the
 * reference count of the <code>DK_CacheAdmin</code> object reaches zero.
 * On a deleted <code>DK_CacheAdmin</code> object one can still use the lock/unlock operations for the mutexes safely.
 * Mutexes are only cleaned when the reference count reaches zero and the memory is about to be freed.
 *
 * \param _this The <code>DK_CacheAdmin</code> entity that is the target of this operation
 * \param userData Optional user data.
 */
void
DK_CacheAdmin_ts_delete(
    DK_CacheAdmin* _this,
    void* userData);

/* \brief Returns the participant of this <code>DK_CacheAdmin</code> object.
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
 * \return The participant of this <code>DK_CacheAdmin</code> object.
 */
u_participant
DK_CacheAdmin_us_getParticipant(
    DK_CacheAdmin* _this);

/* \brief Processes all waiting DCPS updates and takes the appropiate DLRL actions to insert the update.
 *
 * This operation will claim locks on various other entities as well, such as homes.
 *
 * Preconditions:<ul>
 * <li>Must claim the update lock on the <code>DK_CacheAdmin</code>.</li>
 * <li>Must verify the <code>DK_CacheAdmin</code> is still alive.</li>
 * <li>The update and/or admin mutex of ANY home registered to this cache may not be already claimed.</li></ul>
 *
 * \param _this The <code>DK_CacheAdmin</code> entity that is the target of this operation
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param userData Optional user data.
 */
void
DK_CacheAdmin_us_processDCPSUpdates(
    DK_CacheAdmin* _this,
    DLRL_Exception* exception,
    void* userData);

/* NOT IN DESIGN */
DK_PubSubState
DK_CacheAdmin_us_getPubSubState(
    DK_CacheAdmin* _this);

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_KERNEL_CACHE_ADMIN_H */
