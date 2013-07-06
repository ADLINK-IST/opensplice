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
#ifndef CCPP_CONTRACT_H
#define CCPP_CONTRACT_H

#include "ccpp_dlrl.h"
#include "ccpp_CacheAccess_impl.h"
#include "ccpp_dlrl_if.h"
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace DDS
{

    /**
     * The Contract class is not supported.
     *
     * <P>The current implementation does not yet support the notion of a
     * Contract, so no operations in this interface are supported.</P>
     */
    class OS_DLRL_API Contract_impl :
        public virtual DDS::Contract,
        public LOCAL_REFCOUNTED_OBJECT
    {

        friend class ccpp_CacheAccess_impl;

        private:
            Contract_impl();
            virtual ~Contract_impl();

        public:

            /**
             * Not supported
             */
            virtual CORBA::Long
            depth(
                ) THROW_ORB_EXCEPTIONS;

            /**
             * Not supported
             */
            virtual DDS::ObjectScope
            scope(
                ) THROW_ORB_EXCEPTIONS;

            /**
             * Not supported
             */
            virtual DDS::ObjectRoot *
            contracted_object(
                ) THROW_ORB_EXCEPTIONS;

            /**
             * Not supported
             */
            virtual void
            set_depth(
                ::CORBA::Long depth) THROW_ORB_EXCEPTIONS;

            /**
             * Not supported
             */
            virtual void
            set_scope(
                DDS::ObjectScope scope) THROW_ORB_EXCEPTIONS;

    };
};

#endif /* CCPP_CONTRACT_H */
