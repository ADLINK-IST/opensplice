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
#ifndef CCPP_INTMAP_H
#define CCPP_INTMAP_H

#include "ccpp_dlrl.h"
#include "DLRL_Kernel.h"
#include "ccpp_dlrl_if.h"
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace DDS
{
    class ObjectRoot_impl;

    /**
     * <P>The IntMap is an abstract class that represents a Map that can store
     * elements identified by an Long key type. The operations to store and
     * retrieve elements in the map are implemented in a specialized
     * sub-class.</P>
     * <P>The Map offers operations that can produce the keys of all elements
     * contained in the Map, the keys of all elements that have been added in
     * the last update round, the keys of all elements that were modified during
     * the last update round, and the keys of all elements that were removed
     * during the last update round. Changes to the map as a result of
     * manipulation by a local application cannot be retrieved this way.</P>
     * <P>It is possible for an application to remove an element from the map
     * by means of the key that identifies it. This is only possible for
     * Collections that are located in a writeable CacheAccess.</P>
     */
    class OS_DLRL_API IntMap_impl :
        public virtual DDS::IntMap,
        public LOCAL_REFCOUNTED_VALUEBASE
    {
        friend class ObjectRoot_impl;

        private:
            virtual CORBA::ValueBase* _copy_value();

        protected:
            DK_MapAdmin* map;
            IntMap_impl(DK_MapAdmin* collection);

            virtual ~IntMap_impl();

            DK_ObjectAdmin*
            getObjectRootAdmin(
                DLRL_Exception* exception,
                DDS::ObjectRoot* objectRoot,
                const char* name);

        public:

            /**
             * Returns the number of elements contained in the collection.
             *
             * @return the number of contained elements.
             * @throws DDS::AlreadyDeleted if the collection is already deleted.
             */
            virtual CORBA::Long
            length(
                ) THROW_VALUETYPE_ORB_EXCEPTIONS;

            /**
             * Clears the contents of the collection, does not affect results
             * of the added/modified/removed values methods.
             *
             * A PreconditionNotMet is raised if any of the following
             * preconditions is violated:
             * <ul><li>The Map is not located in a (writeable) CacheAccess;</li>
             * <li>The Map belongs to an ObjectRoot which is not yet
             * registered.</li></ul>
             *
             * @throws DDS::AlreadyDeleted if the map is already deleted.
             * @throws DDS::PreconditionNotMet If one of the preconditions was
             * not met.
             */
            virtual void
            clear(
                ) THROW_VALUETYPE_ORB_EXCEPTIONS;

            /**
             * Returns the keys of all elements that are contained in the map.
             *
             * @return the keys of all elements that are contained in the map.
             * @throws DDS::AlreadyDeleted if the map is already deleted.
             */
            virtual DDS::LongSeq *
            keys(
                ) THROW_VALUETYPE_ORB_EXCEPTIONS;

            /**
             * Removes the element that is identified by the specified key.
             *
             * A PreconditionNotMet is raised if any of the following
             * preconditions is violated:
             * <ul><li>The Map is not located in a (writeable) CacheAccess;</li>
             * <li>The Map belongs to an ObjectRoot which is not yet
             * registered.</li></ul>
             *
             * @param key the key that identifies the element that is to be
             * removed.
             * @throws DDS::AlreadyDeleted if the map is already deleted.
             * @throws PreconditionNotMet if any of the preconditions was
             * not met.
             */
            virtual void
            remove(
                ::CORBA::Long key) THROW_VALUETYPE_ORB_EXCEPTIONS;

            /**
             * Returns the keys of all elements that were added during the last
             * update round.
             * Changes to the map as a result of manipulation by a local
             * application are not taken into account. It is recommended to use
             * the keys() operation instead of this operation when dealing with
             * a collection belonging to an ObjectRoot with read_state
             * OBJECT_NEW. In this case both lists will be equal and the keys()
             * operation will give better performance. But only in the
             * described case, in other situations it's recommended to use this
             * operation. When this collection belongs to an ObjectRoot with
             * read_state VOID then this operation will always return a zero
             * length array.
             *
             * @return the keys of all elements that were added during the last
             * update round.
             * @throws DDS::AlreadyDeleted if the map is already deleted.
             */
            virtual DDS::LongSeq *
            added_elements(
                ) THROW_VALUETYPE_ORB_EXCEPTIONS;

            /**
             * Returns the keys of all elements that were removed during the
             * last update round.
             * Changes to the map as a result of manipulation by a local
             * application are not taken into account. When this collection
             * belongs to an ObjectRoot with read_state VOID or OBJECT_NEW then
             * this operation will always return a zero length array.
             *
             * @return the keys of all elements that were removed during the
             * last update round.
             * @throws DDS::AlreadyDeleted if the map is already deleted.
             */
            virtual DDS::LongSeq *
            removed_elements(
                ) THROW_VALUETYPE_ORB_EXCEPTIONS;

            /**
             * Returns the keys of all elements that were modified during the
             * last update round. Changes to the map as a result of manipulation
             * by a local application are not taken into account. When this
             * collection belongs to an ObjectRoot with read_state VOID or
             * OBJECT_NEW then this operation will always return a zero length
             * array.
             *
             * @return the keys of all elements that were modified during the
             * last update round.
             * @throws DDS::AlreadyDeleted if the map is already deleted.
             */
            virtual DDS::LongSeq *
            modified_elements(
                ) THROW_VALUETYPE_ORB_EXCEPTIONS;

    };
};

#endif /* CCPP_INTMAP_H */
