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
#ifndef CCPP_LIST_H
#define CCPP_LIST_H

#include "ccpp_dlrl.h"
#include "ccpp_dlrl_if.h"
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace DDS
{

    /**
     * The List class is not supported.
     *
     * <P>The List is an abstract class that represents a linked list that can
     * store elements identified by an Long index type. The operations to store
     * and retrieve elements in the list are implemented in a specialized
     * sub-class.</P>
     */
    class OS_DLRL_API List_impl :
        public virtual DDS::List,
        public LOCAL_REFCOUNTED_VALUEBASE
    {

        protected:
            List_impl();

            virtual ~List_impl();

        public:

            /**
             * Not supported
             */
            virtual CORBA::Long
            length(
                ) THROW_VALUETYPE_ORB_EXCEPTIONS;

            /**
             * Not supported
             */
            virtual void
            clear(
                ) THROW_VALUETYPE_ORB_EXCEPTIONS;

            /**
             * Not supported
             */
            virtual void
            remove(
                ) THROW_VALUETYPE_ORB_EXCEPTIONS;

            /**
             * Not supported
             */
            virtual DDS::LongSeq *
            added_elements(
                ) THROW_VALUETYPE_ORB_EXCEPTIONS;

            /**
             * Not supported
             */
            virtual DDS::LongSeq *
            removed_elements(
                ) THROW_VALUETYPE_ORB_EXCEPTIONS;

            /**
             * Not supported
             */
            virtual DDS::LongSeq *
            modified_elements(
                ) THROW_VALUETYPE_ORB_EXCEPTIONS;

    };
};

#endif /* CCPP_LIST_H */
