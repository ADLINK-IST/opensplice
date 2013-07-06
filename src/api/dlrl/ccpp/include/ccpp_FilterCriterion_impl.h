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
#ifndef CCPP_FILTERCRITERION_IMPL_H_
#define CCPP_FILTERCRITERION_IMPL_H_

#include "ccpp_dlrl.h"
#include "ccpp_SelectionCriterion_impl.h"
#include "ccpp_dlrl_if.h"
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace DDS
{

    /**
     * Generic base class for typed Filter classes.
     */
    class OS_DLRL_API FilterCriterion_impl :
        public virtual FilterCriterion,
        public SelectionCriterion_impl
    {

    public:
        FilterCriterion_impl();
        virtual ~FilterCriterion_impl();
    };
};

#endif /*CCPP_FILTERCRITERION_IMPL_H_*/
