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
#ifndef CCPP_CACHE_H
#define CCPP_CACHE_H

#include "ccpp_dlrl.h"
#include "DLRL_Kernel.h"
#include "ccpp_CacheFactory_impl.h"

#include "ccpp_dlrl_if.h"

/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace DDS
{
    /**
     * <P>A Cache gathers a set of DLRL objects that are managed, published
     * and/or subscribed consistently. Only Objects stored in the same Cache may
     * have navigatable relationships between them. Each DLRL Object type is
     * managed by its own typed {@link DDS::ObjectHome_impl}. </P>
     * <P>Caches may have different usage modes, identified by their cache_usage
     * parameter that specifies the future usage of the Cache, namely WRITE_ONLY
     *  - no subscription, READ_ONLY - no publication, or READ_WRITE - both
     * modes. Depending on this cache_usage a DDS::Subscriber, a DDS::Publisher,
     * or both will be created, respectively, for the unique usage of the
     * Cache.</P>
     * <P>A Cache can be operated in two update modes: a fully automatic mode in
     * which incoming DCPS events will be automatically applied to the DLRL
     * Cache, triggering the relevant Listeners in the process (updates_enabled
     * = <code>true</code>), and a manual mode in which the state of the Cache
     * is updated on application request by means of the refresh operation
     * (updates_enabled = <code>false</code>).</P>
     * <P>Several {@link DDS::CacheAccess_impl} objects may be created from a
     * Cache, which can be used to store consistent snapshots of (part of) the
     * Cache, or to create new Objects in isolation. The state of these
     * CacheAccess objects can be updated separately from the state of the
     * main Cache.</P>
     * <P>For a Cache to be able to read/write information from/to the DCPS,
     * it must first register and enable the appropriate DCPS entities. This is
     * done in two separate steps (register_all_for_pub_sub and
     * enable_all_for_pub_sub).
     * In the first step, the Cache will automatically create the required DCPS
     * Entities (based upon the registered ObjectHomes) using default QoS
     * settings. A user can then tailor the DLRL behavior by changing the
     * appropriate QoS values on the appropriate DCPS Entities.
     * In the second step, he can enable these DCPS Entities thus finalizing
     * their immutable QoS settings. Only after this second step has been
     * performed will the Cache be able to exchange DLRL Object information with
     * the DCPS.</P>
     *
     * <P> Each Cache <i>must</i> be deleted using the delete_cache(...)
     * operation on the CacheFactory.</P>
     */
    class OS_DLRL_API Cache_impl :
        public virtual DDS::Cache,
        public LOCAL_REFCOUNTED_OBJECT
    {

    friend class DDS::CacheFactory;

    private:
        DK_CacheAdmin* cache;
        Cache_impl();
        virtual ~Cache_impl();

    public:

        /**
         * Returns the state of the Cache with respect to the underlying Pub/Sub
         * infrastructure, which can be INITIAL (no DCPS entities created yet),
         * REGISTERED (DCPS entities are created but not yet enabled), and
         * ENABLED (DCPS entities are enabled).
         *
         * @return the state of the underlying Pub/Sub infrastructure.
         * @throws DDS::AlreadyDeleted if the Cache is already deleted.
         */
        virtual DDS::DCPSState
        pubsub_state(
            ) THROW_ORB_EXCEPTIONS;

        /**
	     * Returns the DCPS DomainParticipant object attached to this Cache.
	     * This participant is always available.
         *
         * @return the DomainParticipant attached to this Cache.
         * @throws DDS::AlreadyDeleted if the Cache is already deleted.
         */
        virtual DDS::DomainParticipant_ptr
        the_participant(
            ) THROW_ORB_EXCEPTIONS;

        /**
         * Returns the DCPS Publisher object attached to this Cache. This
         * Publisher is always available if the cache usage is READ_WRITE or
         * WRITE_ONLY, regardless of the pubsub_state of the Cache.
         *
         * @return the Publisher attached to this Cache.
         * @throws DDS::AlreadyDeleted if the Cache is already deleted.
         */
        virtual DDS::Publisher_ptr
        the_publisher(
            ) THROW_ORB_EXCEPTIONS;

        /**
         * Returns the DCPS Subscriber object attached to this Cache. This
         * Subscriber is always available if the cache usage is READ_WRITE or
         * READ_ONLY, regardless of the pubsub_state of the Cache.
         *
         * @return the Subscriber attached to this Cache.
         * @throws DDS::AlreadyDeleted if the Cache is already deleted.
         */
        virtual DDS::Subscriber_ptr
        the_subscriber(
            ) THROW_ORB_EXCEPTIONS;

        /**
         * Returns the update mode of the Cache. The Cache can either be in
         * the automatic update mode <code>(true)</code>, or in the manual
         * update mode <code>(false)</code>.
         *
         * @return the update mode of the Cache.
         * @throws DDS::AlreadyDeleted if the Cache is already deleted.
         */
        virtual ::CORBA::Boolean
        updates_enabled(
            ) THROW_ORB_EXCEPTIONS;

        /**
         * Returns the (untyped) list of all ObjectHomes that are attached
         * to this Cache.
         *
         * @return the (untyped) list of all ObjectHomes attached to this Cache.
         * @throws DDS::AlreadyDeleted if the Cache is already deleted.
         */
        virtual DDS::ObjectHomeSeq *
        homes(
            ) THROW_ORB_EXCEPTIONS;

        /**
         * Returns the list of all CacheAccess objects that are created by
         * this Cache.
         *
         * @return the list of all CacheAccess objects created by this Cache.
         * @throws DDS::AlreadyDeleted if the Cache is already deleted.
         */
        virtual DDS::CacheAccessSeq *
        sub_accesses(
            ) THROW_ORB_EXCEPTIONS;

        /**
         * Returns the list of all CacheListener objects that are attached to
         * this Cache. This operation may not be called during any listener
         * callback, as it would result in a deadlock.
         *
         * @return the list of all CacheListener objects attached to this Cache.
         * @throws DDS::AlreadyDeleted if the Cache is already deleted.
         */
        virtual DDS::CacheListenerSeq *
        listeners(
            ) THROW_ORB_EXCEPTIONS;

        /**
         * Creates and registers all the required DCPS Entities (except for
         * Publisher and Subscriber, which are always available).
         *
         * <P>Registration
         * is performed for publication, for subscription or for both, according
         * to the cache_usage. The task of creating and registering the typed
         * DCPS entities is delegated to their corresponding ObjectHomes, but is
         * performed only for ObjectHomes that have been registered to this
         * Cache.</P>
         * <P>An ObjectHome may only have dependencies to other ObjectHomes when
         * these other ObjectHomes have also been registered to this Cache. A
         * BadHomeDefinition is thrown otherwise. Also a number of preconditions
         * must be satisfied before invoking the register_all_for_pubsub method:
         * <ul>
         * <li>at least one ObjectHome needs to have been registered</li>
         * <li>the pubsub_state may not yet be ENABLED.</li></ul>
         * If these preconditions are not satisfied, a PreconditionNotMet
         * Exception will be thrown. Invoking the register_all_for_pub_sub on a
         * REGISTERED pubsub_state will be considered a no-op.</P>
         *
         * @throws DDS::BadHomeDefinition if a registered ObjectHome has
         * dependencies to other, unregistered ObjectHomes.
         * @throws DDS::DCPSError if an unexpected error occured in the DCPS.
         * @throws DDS::AlreadyDeleted if the Cache is already deleted.
         * @throws DDS::PreconditionNotMet if one of the pre-conditions has not
         * been met.
         */
        virtual void
        register_all_for_pubsub(
            ) THROW_ORB_AND_USER_EXCEPTIONS(
                DDS::BadHomeDefinition,
                DDS::DCPSError,
                DDS::PreconditionNotMet);

        /**
         * Enables all the attached DCPS Entities.
         *
         * <P>Immutable QoS Settings for
         * the created DCPS Entities can only be changed before these Entities
         * have been enabled. This operation may not be called during any
         * listener callback, as it would result in a deadlock.</P>
         * <P>One precondition must be satisfied before invoking the
         * enable_all_for_pub_sub method: the pubsub_state must already have
         * been set to REGISTERED before. A PreconditionNotMet Exception is
         * thrown otherwise. Invoking the enable_all_for_pub_sub method on an
         * ENABLED pubsub_state will be considered a no-op. </P>
         *
         * @throws DDS::DCPSError if an unexpected error occured in the DCPS.
         * @throws DDS::AlreadyDeleted if the Cache is already deleted.
         * @throws DDS::PreconditionNotMet if one of the pre-conditions has not
         * been met.
         */
        virtual void
        enable_all_for_pubsub(
            ) THROW_ORB_AND_USER_EXCEPTIONS(
                DDS::DCPSError,
                DDS::PreconditionNotMet);

        /**
         * Registers an ObjectHome to this Cache. <P>The Cache keeps a list of
         * registered ObjectHomes, and returns the index number for the
         * specified ObjectHome in that list.</P>
         * <P>A number of preconditions must be satisfied when invoking the
         * register_home method:<ul>
         * <li>the Cache must have a pubsub_state set to INITIAL.</li>
         * <li>the specified ObjectHome instance may not yet be registered
         * before (either to this Cache or to another Cache)</li>
         * <li>Another instance of the same class as the specified ObjectHome
         * may not already have been registered to this Cache.</li></ul>
         * If these preconditions are not satisfied, a PreconditionNotMet
         * Exception will be thrown.</P>
         *
         * @param a_home the ObjectHome that is to be registered to this Cache.
         * @return the index number for the specified ObjectHome.
         * @throws DDS::AlreadyDeleted if the Cache is already deleted.
         * @throws DDS::PreconditionNotMet if one of the pre-conditions has not
         * been met.
         */
        virtual CORBA::Long
        register_home(
            DDS::ObjectHome_ptr a_home) THROW_ORB_AND_USER_EXCEPTIONS(
                DDS::PreconditionNotMet);

        /**
         * Retrieves an already registered ObjectHome based on its type-name.
         * <P>This type-name must be the fully-qualified name(including its scope),
         * using the IDL notation (with the double colon '::' as the scoping
         * operator).</P>
         *
         * @param class_name the fully qualified type name of the ObjectHome
         * to retrieve.
         * @return the ObjectHome instance specified by the type-name.
         * @throws DDS::AlreadyDeleted if the Cache is already deleted.
         */
        virtual DDS::ObjectHome_ptr
        find_home_by_name(
            const char * class_name) THROW_ORB_EXCEPTIONS;

        /**
         * Retrieves an already registered ObjectHome based on its registration
         * index. This registration index is the index in the list of registered
         * ObjectHomes of the Cache, and is returned when registering an
         * ObjectHome.
         *
         * @param index the registration index of the ObjectHome to retrieve.
         * @return the ObjectHome instance specified by the registration index.
         * @throws DDS::AlreadyDeleted if the Cache is already deleted.
         */
        virtual DDS::ObjectHome_ptr
        find_home_by_index(
            ::CORBA::Long index) THROW_ORB_EXCEPTIONS;

        /**
         * Attaches a CacheListener to this Cache. This operation may not be
         * called during any listener callback, as it would result in a
         * deadlock. When successful, it returns <code>true</code>. If the same
         * CacheListener instance was already attached before, the operation is
         * ignored and returns <code>false</code>.
         *
         * @param listener the CacheListener that needs to be attached.
         * @return whether the specified CacheListener is successfuly attached.
         * @throws DDS::AlreadyDeleted if the Cache is already deleted.
         */
        virtual CORBA::Boolean
        attach_listener(
            DDS::CacheListener_ptr listener) THROW_ORB_EXCEPTIONS;

        /**
         * Detaches a CacheListener from this Cache. This operation may not be
         * called during any listener callback, as it would result in a
         * deadlock. Returns <code>true</code> when successfuly detached and
         * <code>false</code> when the specified  CacheListener instance was not
         * attached to this Cache.
         *
         * @param listener the CacheListener that needs to be detached.
         * @return whether the specified CacheListener is successfuly detached.
         * @throws DDS::AlreadyDeleted if the Cache is already deleted.
         */
        virtual CORBA::Boolean
        detach_listener(
            DDS::CacheListener_ptr listener) THROW_ORB_EXCEPTIONS;

        /**
         * Enables automatic update mode.
         *
         * <P>Available modifications in DCPS are
         * automatically applied to the DLRL objects and the corresponding
         * Listeners will be triggered. This operation may not be called during
         * any listener callback, as it would result in a deadlock.</P>
         * <P>One precondition must be satisfied before invoking the
         * enable_updates method: the cache_usage must either be READ_ONLY or
         * READ_WRITE. A PreconditionNotMet exception is thrown otherwise.
         * Invoking the enable_updates method when updates_enabled is already
         * <code>true</code> will be considered a no-op.</P>
         * <P>This operation clears any modification info (ObjectRoot
         * is_modified operation or object home get_created_objects for example)
         * that was stored since the last time a refresh was done.</P>
         * <P>Take note that this call will result in CacheListeners being
         * triggered on the account of the thread that called this operation.
         * The on_updates_enabled() operation will be triggered to indicate a
         * change in the way DLRL processes updates to interested parties.</P>
         * <P>Any already pending updates will automatically be processed by the
         * DLRL once automatic updates has been enabled and all CacheListener
         * have been made aware that this is the case.</P>
         *
         * @throws DDS::AlreadyDeleted if the Cache is already deleted.
         * @throws DDS::PreconditionNotMet if one of the pre-conditions has not
         * been met.
         */
        virtual void
        enable_updates(
            ) THROW_ORB_AND_USER_EXCEPTIONS(
                DDS::PreconditionNotMet);

        /**
         * Disables automatic update mode.
         *
         * <P>Available modifications in DCPS
         * must manually be applied to the DLRL objects by using the
         * {@link #refresh} method. This operation may not be called during any
         * listener callback, as it would result in a deadlock.</P>
         * <P>One precondition must be satisfied when invoking the
         * disable_updates method: the cache_usage must either be READ_ONLY or
         * READ_WRITE. A PreconditionNotMet exception is thrown otherwise.
         * Invoking the disable_updates method when updates_enabled is already
         * <code>false</code> will be considered a no-op.</P>
         * <P>Take note that this call will result in CacheListeners being
         * triggered on the account of the thread that called this operation.
         * The on_updates_disabled() operation will be triggered to indicate a
         * change in the way DLRL processes updates to interested parties.</P>
         *
         * @throws DDS::AlreadyDeleted if the Cache is already deleted.
         * @throws DDS::PreconditionNotMet if one of the pre-conditions has not
         * been met.
         */
        virtual void
        disable_updates(
            ) THROW_ORB_AND_USER_EXCEPTIONS(
                DDS::PreconditionNotMet);

        /**
         * Creates a CacheAccess object that can be used to store consistent
         * snapshots of (part of) the Cache, or to create new Objects in
         * isolation.
         *
         * <P>The state of these CacheAccess objects can be updated
         * separately from the state of the main Cache.</P>
         * <P>Three preconditions must be satisfied when invoking the
         * create_access method: the specified cache_usage must be compatible
         * with the cache_usage of the main Cache: i.e. the CacheAccess can
         * only support READ mode if the primary Cache supports this as well.
         * The same goes for for the WRITE mode. Example: A READ_ONLY
         * CacheAccess may be created by a READ_WRITE Cache, but a READ_WRITE
         * CacheAccess may not be created by a READ_ONLY Cache.
         * Secondly the cache must be enabled for pubsub.
         * And finally one may not create a READ_ONLY CacheAccess within a
         * WRITE_ONLY cache, as it would be impossible to insert objects into
         * that CacheAccess and thus makes no sense to do.
         * If one of these preconditions is not met, a PreconditionNotMet
         * Exception is thrown.</P>
         *
         * @param purpose the usage mode for the CacheAccess.
         * @return the created CacheAccess.
         * @throws DDS::PreconditionNotMet if one of the pre-conditions has not
         * been met.
         * @throws DDS::AlreadyDeleted if the Cache is already deleted.
         */
        virtual DDS::CacheAccess_ptr
        create_access(
            DDS::CacheUsage purpose) THROW_ORB_AND_USER_EXCEPTIONS(
                DDS::PreconditionNotMet);

        /**
         * Deletes an existing CacheAccess.
         *
         * <P>The contents of this CacheAccess
         * will be purged. One precondition must be satisfied when invoking the
         * delete_access method: the specified CacheAccess must be created by
         * this Cache object, otherwise a PreconditionNotMet Exception is
         * thrown.</P>
         *
         * @param access the CacheAccess object that is to be deleted.
         * @throws DDS::AlreadyDeleted if the Cache is already deleted.
         * @throws DDS::PreconditionNotMet if one of the pre-conditions has
         * not been met.
         */
        virtual void
        delete_access(
            DDS::CacheAccess_ptr access) THROW_ORB_AND_USER_EXCEPTIONS(
                DDS::PreconditionNotMet);

        /**
         * Returns the list of all objects that are available in this Cache.
         *
         * @return the list of available objects.
         * @throws DDS::AlreadyDeleted if the Cache is already deleted.
         * @throws DDS::DCPSError if an unexpected error occured in the DCPS.
         */
        virtual DDS::ObjectRootSeq *
        objects(
            ) THROW_ORB_EXCEPTIONS;

        /**
         * Applies waiting modifications in the DCPS onto the DLRL objects,
         * when updates_enabled is <code>false</code>.
         *
         * <P>No Listeners are
         * triggered in this case. When updates_enabled is <code>true</code>,
         * this operation is considered a no-op. This operation may not be
         * called during any listener callback, as it would result in a
         * deadlock.</P>
         * <P>One precondition must be satisfied when invoking the refresh
         * method: the Cache must be in a readable mode (READ_ONLY or
         * READ_WRITE, but not WRITE_ONLY). If this precondition is not met,
         * a PreconditionNotMet Exception is thrown.</P>
         *
         * @throws DDS::DCPSError if an unexpected error occured in the DCPS.
         * @throws DDS::AlreadyDeleted if the CacheBase is already deleted.
         * @throws DDS::PreconditionNotMet if one of the pre-conditions has
         * not been met.
         */
        virtual void
        refresh(
            ) THROW_ORB_AND_USER_EXCEPTIONS(
                DDS::DCPSError,
                DDS::PreconditionNotMet);

        /**
         * Returns the kind which in this case is CACHE_KIND.
         *
         * @return the kind of this Cache
         * @throws DDS::AlreadyDeleted if the Cache is already deleted.
         */
        virtual DDS::CacheKind
        kind(
            ) THROW_ORB_EXCEPTIONS;

        /**
         * Returns the usage mode of the Cache, which can be READ_ONLY,
         * WRITE_ONLY or READ_WRITE.
         *
         * @return the usage mode of the Cache.
         * @throws DDS::AlreadyDeleted if the Cache is already deleted.
         */
        virtual DDS::CacheUsage
        cache_usage(
            ) THROW_ORB_EXCEPTIONS;
    };
};

#endif /* CCPP_CACHE_H */
