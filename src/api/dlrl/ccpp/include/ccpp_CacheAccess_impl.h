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
#ifndef CCPP_CACHEACCESS_H
#define CCPP_CACHEACCESS_H

#include "ccpp_dlrl.h"
#include "DLRL_Kernel.h"
#include "ccpp_Cache_impl.h"
#include "ccpp_dlrl_if.h"

namespace DDS
{

    class ObjectHome_impl;

    /**
     * <P>This class encapsulates the access to a set of objects. It
     * offers methods to refresh and write objects attached to it.
     * CacheAccess objects can be created in read mode, in order
     * to provide a consistent access to a subset of the Cache
     * without blocking the incoming updates or in write mode in
     * order to provide support for concurrent
     * modifications/updates threads.</P>
     *
     * The current implementation only supports writeable cache accesses.
     */
    class OS_DLRL_API CacheAccess_impl :
        public virtual DDS::CacheAccess,
        public LOCAL_REFCOUNTED_OBJECT
    {

        friend class DDS::Cache_impl;
        friend class DDS::ObjectHome_impl;

        private:
            DK_CacheAccessAdmin* access;
            CacheAccess_impl();
            virtual ~CacheAccess_impl();

        public:

            /**
             * Returns the owning Cache object at which this CacheAccess
             * was created.
             *
             * @return the owner Cache object of this CacheAccess object
             * @throws DDS::AlreadyDeleted if the CacheAccess is already deleted.
             */
            virtual DDS::Cache_ptr
            owner(
                ) THROW_ORB_EXCEPTIONS;

            /**
             * The current implementation does not yet support the notion of a
             * Contract, therefore this operation is not yet supported.
             */
            virtual DDS::ContractSeq *
            contracts(
                ) THROW_ORB_EXCEPTIONS;

            /**
             * A list of names that represent the names of the ObjectHomes for
             * which the CacheAccess contains at least one object. Whenever
             * possible use the {@link #contained_types} instead, as it is more
             * performance efficient then using String identifiers.
             *
             * @throws DDS::AlreadyDeleted if the CacheAccess is already deleted
             */
            virtual DDS::StringSeq *
            type_names(
                ) THROW_ORB_EXCEPTIONS;

            /**
             * A list of indexes that represents the indexes of the ObjectHomes
             * for which the CacheAccess contains at least one object.
             *
             * @throws DDS::AlreadyDeleted if the CacheAccess is already deleted
             */
            virtual DDS::LongSeq *
            contained_types(
                ) THROW_ORB_EXCEPTIONS;

            /**
             * Publishes all changes made to ObjectRoots in the scope of this
             * CacheAccess into the DCPS layer.
             *
             * <P>Any object marked for
             * destruction will be actually destroyed after this operation and
             * accessing such ObjectRoots will raise an AlreadyDeleted
             * exception.</P>
             * <P>This operation will raise a PreconditionNotMet exception if
             * the CacheAccess is created with an usage of READ_ONLY. When this
             * exception occurs nothing is done within the CacheAccess.</P>
             * <P>This operation will raise an InvalidObjects exception if one
             * of the objects contained within the CacheAccess has an invalid
             * relation. Relations are invalid when they point to a NULL pointer
             * and are classified as mandatory or when they point to an object
             * which is marked for destruction in the next write operation.
             * Relations in this context also imply collections of an
             * ObjectRoot. Such collections may also contain invalid elements
             * (IE elements which represent ObjectRoots which are marked for
             * destruction in the next write operation). If such elements are
             * contained within a collection, then the collection is seen as
             * invalid. Naturally NULL pointers can not be contained within a
             * collection. When this exception occurs nothing will be done in
             * the CacheAccess, to find out which objects have invalid objects
             * the ({@link #get_invalid_objects})
             * operation can be used. This operation will return all objects
             * which have invalid relations. The ObjectRoot provides utility
             * operations to determine which relations are invalid.</P>
             * <P>This operation will raise a DCPSError exception if an error
             * occurs while trying to write the changes to DCPS. This exception
             * is unrecoverable.</P>
             *
             * @throws DDS::DCPSError If an error occurred while writing the
             * changes
             * @throws DDS::PreconditionNotMet if one of the pre-conditions has
             * not been met.
             * @throws DDS::AlreadyDeleted if the CacheAccess is already
             * deleted.
             * @throws DDS::InvalidObjects If invalid objects are detected
             * which prevent the write operation to continue
             */
            virtual void
            write(
                ) THROW_ORB_AND_USER_EXCEPTIONS(
                    DDS::DCPSError,
                    DDS::PreconditionNotMet,
                    DDS::InvalidObjects);

            /**
             * Detach all ObjectRoots and Contracts (including the
             * contracted DLRL Objects themselves) from the CacheAccess.
             * <P>If the CacheAccess is writeable then the CacheAccess will
             * unregister itself for each purged ObjectRoot at the respective
             * DCPS data writer entity. If the CacheAccess was the last
             * writeable CacheAccess registered for that ObjectRoot instance
             * within the scope of the owning Cache then an explicit unregister
             * is performed for that instance on DCPS topic level. The default
             * QoS settings for data writers (specifically auto dispose
             * unregistered entities) created by DLRL will ensure an explicit
             * dispose is also propagated throughout the system.</P>
             * <P>However if the QoS for auto dispose unregistered entities is
             * set to false, then the unregister will still be performed but
             * dependant on the fact if no other writers of that instance exist
             * the instance reaches an instance state of NOT_ALIVE_NO_WRITERS.
             * If other writers of that instance do exist, then the instance
             * state wont change in this scenario.</P>
             *
             * @throws DDS::AlreadyDeleted if the CacheAccess is already deleted
             */
            virtual void
            purge(
                ) THROW_ORB_AND_USER_EXCEPTIONS(
                    DDS::DCPSError);

            /**
             * The current implementation does not yet support the notion of a
             * Contract, therefore this operation is not yet supported.
             */
            virtual DDS::Contract_ptr
            create_contract(
                DDS::ObjectRoot * an_object,
                DDS::ObjectScope scope,
                ::CORBA::Long depth) THROW_ORB_AND_USER_EXCEPTIONS(
                    DDS::PreconditionNotMet);

            /**
             * The current implementation does not yet support the notion of a
             * Contract, therefore this operation is not yet supported.
             */
            virtual void
            delete_contract(
                DDS::Contract_ptr a_contract) THROW_ORB_AND_USER_EXCEPTIONS(
                    DDS::PreconditionNotMet);

            /**
             * This is a utility function designed to easily retrieve
             * objects within the CacheAccess which can cause a write to fail.
             * <P>It is recommended to only use this operation if a write operation
             * fails due to invalid objects, as the write operation itself
             * performs the check for invalid objects as well, making it
             * unneccesary to perform the check before the write in application
             * context. Invalid objects are defined as ObjectRoots which have
             * relations to other ObjectRoots that are marked for destruction
             * in the next write() operation. Or ObjectRoots which have
             * relations which are null pointers, but which the Object Model
             * defined as mandatory relations I.E. a cardinality of 1 instead
             * of 0..1. </P>
             *
             * @throws DDS::AlreadyDeleted if the CacheAccess is already deleted
             * @return An array with all ObjectRoots which have invalid
             * relations. Or a zero length array if no invalid objects are found
             */
            virtual DDS::ObjectRootSeq *
            get_invalid_objects(
                ) THROW_ORB_EXCEPTIONS;

            /**
             * Returns the list of all objects that are available in this Cache.
             *
             * @return the list of available objects.
             * @throws DDS::AlreadyDeleted if the Cache is already deleted.
             * @throws DDS::DCPSError if an unexpected error occured in the DCPS
             */
            virtual DDS::ObjectRootSeq *
            objects(
                ) THROW_ORB_EXCEPTIONS;

            /**
             * This operation is not supported.
             *
             * @throws DDS::DCPSError if an unexpected error occured in the DCPS
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
             * Returns the kind which in this case is CACHEACCESS_KIND.
             *
             * @return the kind of this CacheBase
             * @throws DDS::AlreadyDeleted if the CacheBase is already deleted.
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

#endif /* CCPP_CACHEACCESS_H */
