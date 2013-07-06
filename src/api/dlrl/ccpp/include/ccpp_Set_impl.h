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
#ifndef CCPP_SET_H
#define CCPP_SET_H

#include "ccpp_dlrl.h"
#include "DLRL_Kernel.h"
#include "ccpp_dlrl_if.h"
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace DDS
{
    class ObjectRoot_impl;

    /**
     * <P>The Set is an abstract class that represents a Set that can store any
     * number of elements in an un-ordered way. The operations to store and
     * retrieve elements in the set are implemented in a specialized
     * sub-class.</P>
     */
    class OS_DLRL_API Set_impl :
        public virtual DDS::Set,
        public LOCAL_REFCOUNTED_VALUEBASE
    {
        friend class DDS::ObjectRoot_impl;

        private:
            virtual ValueBase* _copy_value();

        protected:
            DK_SetAdmin* set;
            Set_impl(DK_SetAdmin* collection);

            virtual ~Set_impl();
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
             * of the added/removed values methods.
             *
             * A PreconditionNotMet is raised if any of the following
             * preconditions is violated:
             * <ul><li>The Set is not located in a (writeable) CacheAccess;</li>
             * <li>The Set belongs to an ObjectRoot which is not yet
             * registered.</li></ul>
             *
             * @throws DDS::AlreadyDeleted if the map is already deleted.
             * @throws DDS::PreconditionNotMet If one of the preconditions was
             * not met.
             */
            virtual void
            clear(
                ) THROW_VALUETYPE_ORB_EXCEPTIONS;
    };
};

#endif /* CCPP_SET_H */
