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
#include "ccpp_SelectionCriterion_impl.h"
#include "DLRL_Report.h"

DDS::SelectionCriterion_impl::SelectionCriterion_impl(CriterionKind _myKind) : myKind(_myKind) 
{ 
};

DDS::SelectionCriterion_impl::~SelectionCriterion_impl() 
{ 
};

DDS::CriterionKind
DDS::SelectionCriterion_impl::kind(
    ) THROW_ORB_EXCEPTIONS
{
    DLRL_INFO(INF_ENTER);

    DLRL_INFO(INF_EXIT);
    return myKind;
}
