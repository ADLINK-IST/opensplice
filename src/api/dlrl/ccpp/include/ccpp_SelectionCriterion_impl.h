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
#ifndef CCPP_FILTERCRITERION_H
#define CCPP_FILTERCRITERION_H

#include "ccpp_dlrl.h"
#include "ccpp_dlrl_if.h"
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace DDS
{

    /**
     * Generic base class for typed all criterion-like classes.
     */
    class OS_DLRL_API SelectionCriterion_impl :
        public virtual DDS::SelectionCriterion,
        public LOCAL_REFCOUNTED_OBJECT
    {
    private:
        CriterionKind myKind;

    protected:
        SelectionCriterion_impl(CriterionKind myKind);
        virtual ~SelectionCriterion_impl();

    public:
        /**
         * Returns the final type of this selection criterion
         */
        virtual CriterionKind kind() THROW_ORB_EXCEPTIONS;

    };
};

#endif /* CCPP_FILTERCRITERION_H */
