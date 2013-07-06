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
package DDS;

/**
 * <P>A Cache gathers a set of DLRL objects that are managed, published and/or
 * subscribed consistently. Only Objects stored in the same Cache may have
 * navigatable relationships between them. Each DLRL Object type is managed
 * by its own typed {@link DDS.ObjectHome}. </P>
 * <P>Caches may have different usage modes, identified by their cache_usage
 * parameter that specifies the future usage of the Cache, namely WRITE_ONLY
 *  - no subscription, READ_ONLY - no publication, or READ_WRITE - both modes.
 * Depending on this cache_usage a {@link DDS.Subscriber}, a {@link DDS.Publisher},
 * or both will be created, respectively, for the unique usage of the Cache.</P>
 * <P>A Cache can be operated in two update modes: a fully automatic mode in
 * which incoming DCPS events will be automatically applied to the DLRL Cache,
 * triggering the relevant Listeners in the process (updates_enabled =
 * <code>true</code>), and a manual mode in which the state of the Cache is
 * updated on application request by means of the refresh operation
 * (updates_enabled = <code>false</code>).</P>
 * <P>Several {@link DDS.CacheAccess} objects may be created from a Cache, which
 * can be used to store consistent snapshots of (part of) the Cache, or to create
 * new Objects in isolation. The state of these CacheAccess objects can be
 * updated separately from the state of the main Cache.</P>
 * <P>For a Cache to be able to read/write information from/to the DCPS,
 * it must first register and enable the appropriate DCPS entities. This is
 * done in two separate steps (register_all_for_pub_sub and enable_all_for_pub_sub).
 * In the first step, the Cache will automatically create the required DCPS
 * Entities (based upon the registered ObjectHomes) using default QoS settings.
 * A user can then tailor the DLRL behavior by changing the appropriate QoS
 * values on the appropriate DCPS Entities.
 * In the second step, he can enable these DCPS Entities thus finalizing their
 * immutable QoS settings. Only after this second step has been performed will
 * the Cache be able to exchange DLRL Object information with the DCPS.</P>
 *
 * <P> Each Cache <i>must</i> be deleted using the delete_cache(...) operation
 * on the CacheFactory.</P>
 */
public interface Cache extends CacheBase{

	/**
	 * Returns the state of the Cache with respect to the underlying
	 * Pub/Sub infrastructure, which can be INITIAL (no DCPS entities created yet),
	 * REGISTERED (DCPS entities are created but not yet enabled), and ENABLED
	 * (DCPS entities are enabled).
	 *
	 * @return the state of the underlying Pub/Sub infrastructure.
	 * @throws DDS.AlreadyDeleted if the Cache is already deleted.
	 */
	DDS.DCPSState pubsub_state () throws DDS.AlreadyDeleted;

	/**
	 * Returns the DCPS Publisher object attached to this Cache. This Publisher
	 * is always available if the cache usage is READ_WRITE or WRITE_ONLY,
     * regardless of the pubsub_state of the Cache.
	 *
	 * @return the Publisher attached to this Cache.
	 * @throws DDS.AlreadyDeleted if the Cache is already deleted.
	 */
	DDS.Publisher the_publisher () throws DDS.AlreadyDeleted;

	/**
	 * Returns the DCPS Subscriber object attached to this Cache. This Subscriber
	 * is always available if the cache usage is READ_WRITE or READ_ONLY,
     * regardless of the pubsub_state of the Cache.
	 *
	 * @return the Subscriber attached to this Cache.
	 * @throws DDS.AlreadyDeleted if the Cache is already deleted.
	 */
	DDS.Subscriber the_subscriber () throws DDS.AlreadyDeleted;

	/**
	 * Returns the DCPS DomainParticipant object attached to this Cache. This
	 * participant is always available.
	 *
	 * @return the DomainParticipant attached to this Cache.
	 * @throws DDS.AlreadyDeleted if the Cache is already deleted.
	 */
	DDS.DomainParticipant the_participant () throws DDS.AlreadyDeleted;

	/**
	 * Returns the update mode of the Cache. The Cache can either be in
	 * the automatic update mode <code>(true)</code>, or in the manual
	 * update mode <code>(false)</code>.
	 *
	 * @return the update mode of the Cache.
	 * @throws DDS.AlreadyDeleted if the Cache is already deleted.
	 */
	boolean updates_enabled () throws DDS.AlreadyDeleted;

	/**
	 * Returns the (untyped) list of all ObjectHomes that are attached
	 * to this Cache.
	 *
	 * @return the (untyped) list of all ObjectHomes attached to this Cache.
	 * @throws DDS.AlreadyDeleted if the Cache is already deleted.
	 */
	DDS.ObjectHome[] homes () throws DDS.AlreadyDeleted;

	/**
	 * <P>Returns the list of all CacheAccess objects that are created by
	 * this Cache.</P>
	 *
	 * @return the list of all CacheAccess objects created by this Cache.
	 * @throws DDS.AlreadyDeleted if the Cache is already deleted.
	 */
	DDS.CacheAccess[] sub_accesses () throws DDS.AlreadyDeleted;

	/**
	 * Returns the list of all CacheListener objects that are attached to
	 * this Cache. This operation may not be called during any listener
     * callback, as it would result in a deadlock.
	 *
	 * @return the list of all CacheListener objects attached to this Cache.
	 * @throws DDS.AlreadyDeleted if the Cache is already deleted.
	 */
	DDS.CacheListener[] listeners () throws DDS.AlreadyDeleted;

	/**
	 * <P>Creates and registers all the required DCPS Entities (except for
	 * Publisher and Subscriber, which are always available).  Registration
	 * is performed for publication, for subscription or for both, according
	 * to the cache_usage. The task of creating and registering the typed
	 * DCPS entities is delegated to their corresponding ObjectHomes, but is
	 * performed only for ObjectHomes that have been registered to this Cache.</P>
	 * <P>An ObjectHome may only have dependencies to other ObjectHomes when
	 * these other ObjectHomes have also been registered to this Cache. A
	 * BadHomeDefinition is thrown otherwise. Also a number of preconditions
	 * must be satisfied before invoking the register_all_for_pubsub method:<ul>
	 * <li>at least one ObjectHome needs to have been registered</li>
	 * <li>the pubsub_state may not yet be ENABLED.</li></ul>
	 * If these preconditions are not satisfied, a PreconditionNotMet Exception
	 * will be thrown. Invoking the register_all_for_pub_sub on a REGISTERED
	 * pubsub_state will be considered a no-op.</P>
	 *
	 * @throws DDS.BadHomeDefinition if a registered ObjectHome has dependencies
	 * to other, unregistered ObjectHomes.
	 * @throws DDS.DCPSError if an unexpected error occured in the DCPS.
	 * @throws DDS.AlreadyDeleted if the Cache is already deleted.
	 * @throws DDS.PreconditionNotMet if one of the pre-conditions has not been met.
	 */
	void register_all_for_pubsub ()throws DDS.BadHomeDefinition,DDS.DCPSError,DDS.AlreadyDeleted,DDS.PreconditionNotMet;

	/**
	 * <P>Enables all the attached DCPS Entities. Immutable QoS Settings for the
	 * created DCPS Entities can only be changed before these Entities have
	 * been enabled. This operation may not be called during any listener callback,
     * as it would result in a deadlock.</P>
	 * <P>One precondition must be satisfied before invoking the
	 * enable_all_for_pub_sub method: the pubsub_state must already have
	 * been set to REGISTERED before. A PreconditionNotMet Exception is thrown
	 * otherwise. Invoking the enable_all_for_pub_sub method on an ENABLED
	 * pubsub_state will be considered a no-op. </P>
	 *
	 * @throws DDS.DCPSError if an unexpected error occured in the DCPS.
	 * @throws DDS.AlreadyDeleted if the Cache is already deleted.
	 * @throws DDS.PreconditionNotMet if one of the pre-conditions has not been met.
	 */
	void enable_all_for_pubsub () throws DDS.DCPSError, DDS.AlreadyDeleted, DDS.PreconditionNotMet;

	/**
	 * <P>Registers an ObjectHome to this Cache. The Cache keeps a list of
	 * registered ObjectHomes, and returns the index number for the specified
	 * ObjectHome in that list.</P>
	 * <P>A number of preconditions must be satisfied when invoking the
	 * register_home method:<ul>
	 * <li>the Cache must have a pubsub_state set to INITIAL.</li>
	 * <li>the specified ObjectHome instance may not yet be registered before (either
	 * to this Cache or to another Cache)</li>
	 * <li>Another instance of the same class as the specified ObjectHome
	 * may not already have been registered to this Cache.</li></ul>
	 * If these preconditions are not satisfied, a PreconditionNotMet Exception
	 * will be thrown.</P>
	 *
	 * @param a_home the ObjectHome that is to be registered to this Cache.
	 * @return the index number for the specified ObjectHome.
	 * @throws DDS.AlreadyDeleted if the Cache is already deleted.
	 * @throws DDS.PreconditionNotMet if one of the pre-conditions has not been met.
	 */
	int register_home (DDS.ObjectHome a_home) throws DDS.AlreadyDeleted, DDS.PreconditionNotMet;

	/**
	 * <P>Retrieves an already registered ObjectHome based on its type-name.
	 * This type-name must be the fully-qualified name (including its scope),
	 * using the IDL notation (with the double colon '::' as the scoping
	 * operator).</P>
	 *
	 * @param class_name the fully qualified type name of the ObjectHome to retrieve.
	 * @return the ObjectHome instance specified by the type-name.
	 * @throws DDS.AlreadyDeleted if the Cache is already deleted.
	 */
	DDS.ObjectHome find_home_by_name (String class_name) throws DDS.AlreadyDeleted;

	/**
	 * Retrieves an already registered ObjectHome based on its registration
	 * index. This registration index is the index in the list of registered
	 * ObjectHomes of the Cache, and is returned when registering an ObjectHome.
	 *
	 * @param index the registration index of the ObjectHome to retrieve.
	 * @return the ObjectHome instance specified by the registration index.
	 * @throws DDS.AlreadyDeleted if the Cache is already deleted.
	 */
	DDS.ObjectHome find_home_by_index (int index) throws DDS.AlreadyDeleted;

	/**
	 * Attaches a CacheListener to this Cache. This operation may not be called
     * during any listener callback, as it would result in a deadlock. When successful,
     * it returns <code>true</code>. If the same CacheListener instance was already
	 * attached before, the operation is ignored and returns <code>false</code>.
	 *
	 * @param listener the CacheListener that needs to be attached.
	 * @return whether the specified CacheListener is successfuly attached.
	 * @throws DDS.AlreadyDeleted if the Cache is already deleted.
	 */
	boolean attach_listener (DDS.CacheListener listener) throws DDS.AlreadyDeleted;

	/**
	 * Detaches a CacheListener from this Cache. This operation may not be called
     * during any listener callback, as it would result in a deadlock.
     * Returns <code>true</code> when  successfuly detached and
     * <code>false</code> when the specified  CacheListener instance was not
     * attached to this Cache.
	 *
	 * @param listener the CacheListener that needs to be detached.
	 * @return whether the specified CacheListener is successfuly detached.
	 * @throws DDS.AlreadyDeleted if the Cache is already deleted.
	 */
	boolean detach_listener (DDS.CacheListener listener) throws DDS.AlreadyDeleted;

	/**
     * <P>Enables automatic update mode. Available modifications in DCPS are
	 * automatically applied to the DLRL objects and the corresponding Listeners
	 * will be triggered. This operation may not be called during any listener
     * callback, as it would result in a deadlock.</P>
	 * <P>One precondition must be satisfied before invoking the enable_updates
	 * method: the cache_usage must either be READ_ONLY or READ_WRITE. A
	 * PreconditionNotMet exception is thrown otherwise. Invoking the
	 * enable_updates method when updates_enabled is already <code>true</code>
	 * will be considered a no-op.</P>
     * <P>This operation clears any modification info (ObjectRoot is_modified
     * operation or object home get_created_objects for example) that was
     * stored since the last time a refresh was done.</P>
     * <P>Take note that this call will result in CacheListeners being triggered
     * on the account of the thread that called this operation. The on_updates_enabled()
     * operation will be triggered to indicate a change in the way DLRL processes
     * updates to interested parties.</P>
     * <P>Any already pending updates will automatically be processed by the DLRL once
     * automatic updates has been enabled and all CacheListener have been made aware that
     * this is the case.</P>
	 *
	 * @throws DDS.AlreadyDeleted if the Cache is already deleted.
	 * @throws DDS.PreconditionNotMet if one of the pre-conditions has not been met.
	 */
	void enable_updates () throws DDS.AlreadyDeleted, DDS.PreconditionNotMet;

	/**
	 * <P>Disables automatic update mode. Available modifications in DCPS must
	 * manually be applied to the DLRL objects by using the {@link #refresh}
	 * method. This operation may not be called during any listener
     * callback, as it would result in a deadlock.</P>
	 * <P>One precondition must be satisfied when invoking the disable_updates
	 * method: the cache_usage must either be READ_ONLY or READ_WRITE. A
	 * PreconditionNotMet exception is thrown otherwise. Invoking the
	 * disable_updates method when updates_enabled is already <code>false</code>
	 * will be considered a no-op.</P>
     * <P>Take note that this call will result in CacheListeners being triggered
     * on the account of the thread that called this operation. The on_updates_disabled()
     * operation will be triggered to indicate a change in the way DLRL processes
     * updates to interested parties.</P>
	 *
	 * @throws DDS.AlreadyDeleted if the Cache is already deleted.
	 * @throws DDS.PreconditionNotMet if one of the pre-conditions has not been met.
	 */
	void disable_updates () throws DDS.AlreadyDeleted, DDS.PreconditionNotMet;

	/**
	 * <P>Creates a CacheAccess object that can be used to store consistent
	 * snapshots of (part of) the Cache, or to create new Objects in isolation.
	 * The state of these CacheAccess objects can be updated separately from
	 * the state of the main Cache.</P>
	 * <P>Three preconditions must be satisfied when invoking the create_access
	 * method: the specified cache_usage must be compatible with the cache_usage
	 * of the main Cache: i.e. the CacheAccess can only support READ mode if
	 * the primary Cache supports this as well. The same goes for for the WRITE
	 * mode. Example: A READ_ONLY CacheAccess may be created by a READ_WRITE
	 * Cache, but a READ_WRITE CacheAccess may not be created by a READ_ONLY
	 * Cache.
     * Secondly the cache must be enabled for pubsub.
     * And finally one may not create a READ_ONLY CacheAccess within a WRITE_ONLY
     * cache, as it would be impossible to insert objects into that CacheAccess and thus
     * makes no sense to do.
     * If one of these preconditions is not met, a PreconditionNotMet Exception
	 * is thrown.</P>
	 *
	 * @param purpose the usage mode for the CacheAccess.
	 * @return the created CacheAccess.
	 * @throws DDS.PreconditionNotMet if one of the pre-conditions has not been met.
	 * @throws DDS.AlreadyDeleted if the Cache is already deleted.
	 */
	DDS.CacheAccess create_access (DDS.CacheUsage purpose) throws DDS.PreconditionNotMet, DDS.AlreadyDeleted;

	/**
	 * <P>Deletes an existing CacheAccess. The contents of this CacheAccess will
	 * be purged. One precondition must be satisfied when invoking the
	 * delete_access method: the specified CacheAccess must be created by
	 * this Cache object, otherwise a PreconditionNotMet Exception is thrown.</P>
	 *
	 * @param access the CacheAccess object that is to be deleted.
	 * @throws DDS.AlreadyDeleted if the Cache is already deleted.
	 * @throws DDS.PreconditionNotMet if one of the pre-conditions has not been met.
	 */
	void delete_access (DDS.CacheAccess access) throws DDS.PreconditionNotMet, DDS.AlreadyDeleted;
} // interface Cache
