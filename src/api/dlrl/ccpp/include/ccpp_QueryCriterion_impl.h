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
#ifndef CCPP_QUERYCRITERION_H
#define CCPP_QUERYCRITERION_H

#include "ccpp_dlrl.h"
#include "ccpp_SelectionCriterion_impl.h"
#include "ccpp_dlrl_if.h"
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace DDS
{

    /**
     * The QueryCriterion class is not supported.
     *
     * <P>The current implementation does not yet support the notion of a
     * QueryCriterion, so no operations in this class are supported.</P>
     */
    class OS_DLRL_API QueryCriterion_impl :
        public virtual QueryCriterion,
        public SelectionCriterion_impl
    {

    public:
        /**
         * Not supported
         */
        QueryCriterion_impl();

        /**
         * Not supported
         */
        virtual ~QueryCriterion_impl();

        /**
         * Not supported
         */
        virtual char *
        expression(
            ) THROW_ORB_EXCEPTIONS;

        /**
         * Not supported
         */
        virtual StringSeq *
        parameters(
            ) THROW_ORB_EXCEPTIONS;

        /**
         * Not supported
         */
        virtual void
        set_query (
            const char * expression,
            const StringSeq & parameters) THROW_ORB_AND_USER_EXCEPTIONS(
                SQLError);

        /**
         * Not supported
         */
        virtual void
        set_parameters (
            const DDS::StringSeq & parameters) THROW_ORB_AND_USER_EXCEPTIONS(
                SQLError);

    };
};

#endif /* CCPP_QUERYCRITERION_H */
