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
#ifndef CCPP_CACHEFACTORY_H
#define CCPP_CACHEFACTORY_H

#include "ccpp_dlrl.h"
#include "os_mutex.h"
#include "ccpp_dlrl_if.h"
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace DDS
{
    typedef CacheFactoryInterface_ptr CacheFactory_ptr;
    typedef CacheFactoryInterface_var CacheFactory_var;

    /**
     * <P>This class represents a singleton factory that allows the creation and
     * deletion of DLRL Cache objects.</P>
     */
    class OS_DLRL_API CacheFactory:
        public virtual DDS::CacheFactoryInterface,
        public LOCAL_REFCOUNTED_OBJECT
    {
        private:
            static os_mutex factoryMutex;
            CacheFactory();

        public:
            virtual ~CacheFactory( );

            /**
             * Static method to get a reference to the singleton CacheFactory
             * instance.
             *
             * @return a reference to the one and only instance of the
             * CacheFactory.
             */
            static DDS::CacheFactory_ptr get_instance() THROW_ORB_EXCEPTIONS;

            /**
             * Returns a newly created DLRL Cache object. A cache_usage
             * parameter specifies the future usage of the Cache, namely
             * WRITE_ONLY - no subscription, READ_ONLY - no publication, or
             * READ_WRITE - both modes.
             * Depending on this cache_usage a {@link DDS::Publisher}, a
             * {@link DDS::Subscriber}, or both will be created, respectively,
             * for the unique usage of the Cache. These two objects will be
             * attached to the passed DomainParticipant. A Cache will be
             * created with updates_enabled() returning false.
             *
             * <P>It is the responsibility of the caller to ensure the
             * entity_factory.autoenable_created_entities QoS setting of the
             * participant is set to the correct value. If set to
             * <code>true</code> then the publisher and subscriber used by this
             * cache will be created in an enabled state. If set to
             * <code>false</code> then the subscriber and publisher will be
             * created in a disabled state.</P>
             *
             * <P>Take note that regardless of this setting the
             * entity_factory.autoenable_created_entities QoS setting of the
             * publisher and subscriber will be set to false, overriding any
             * defaults that may exist for the publisher and subscriber
             * QoS settings.
             * This ensures that any readers/writers are created in a disabled
             * state when performing the register_all_for_pubsub operation on
             * the created cache and only enabled after the explicit call to
             * enable_all_for_pubsub. This explicit DLRL QoS setting on the
             * subscriber/publisher may be overridden and changed back to
             * <code>true</code> at the application's discretion. However if
             * this is done it is important to realize that the readers created
             * by the DLRL will participant in data distribution directly after
             * they are created in the register_all_for_pubsub. The DLRL itself
             * wont show the received data until it is enabled for pubsub and a
             * refresh is done or in case of automated updates directly after
             * the enable_all_for_pubsub call, but data may be contained that
             * was actually written between the register_all_for_pubsub and
             * enable_all_for_pubsub. </P>
             *
             * <P> Each created Cache <i>must</i> be deleted using the
             * delete_cache(...) operation on the CacheFactory. This ensures
             * all resources are cleaned, which is always a good practise</P>
             *
             * @param name the name that will be used to identify the Cache.
             * @param cache_usage indicates the future usage of the Cache.
             * @param domain specifies the DCPS Domainparticipant that is to be
             * used.
             * @return the newly created Cache object.
             * @throws DDS::DCPSError if an unexpected error occured in the DCPS
             * @throws DDS::AlreadyExisting if the specified name has already
             * been used by another Cache object.
             */
            virtual DDS::Cache_ptr
            create_cache(
                const char * name,
                DDS::CacheUsage cache_usage,
                DDS::DomainParticipant_ptr domain)
                    THROW_ORB_AND_USER_EXCEPTIONS(DDS::DCPSError, DDS::AlreadyExisting);

            /**
             * Retrieves a Cache object based by name. If no Cache object is
             * identified by the specified name, a <code>null</code> pointer
             * will be returned.
             *
             * @param name the name that identifies the Cache object that is to
             * be retrieved.
             * @return the Cache object that corresponds to the specified name.
             */
            virtual DDS::Cache_ptr
            find_cache_by_name(
                const char * name) THROW_ORB_EXCEPTIONS;

            /**
             * Delete the specified DLRL Cache and release all resources it has
             * claimed. This operation may not be called during any listener
             * callback initiated by the to-be-deleted Cache. This operation
             * may also not be called during a check_object callback of a typed
             * Filter object that belongs to the to-be-deleted cache.
             * Doing so would result in a deadlock.
             *
             * @param a_cache the Cache object that is to be deleted.
             */
            virtual void
            delete_cache(
                DDS::Cache_ptr a_cache) THROW_ORB_EXCEPTIONS;

            static CacheFactory_ptr
            _nil (
                void)
            {
                return (CacheFactory_ptr)0;
            }
    };
};

#endif /* CCPP_CACHEFACTORY_H */
