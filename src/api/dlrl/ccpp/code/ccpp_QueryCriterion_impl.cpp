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
#include "ccpp_QueryCriterion_impl.h"
#include "ccpp_DlrlUtils.h"
#include "DLRL_Report.h"
#include "DLRL_Types.h"
#include "DLRL_Exception.h"

DDS::QueryCriterion_impl::QueryCriterion_impl() : SelectionCriterion_impl(DDS::QUERY)
{

}

DDS::QueryCriterion_impl::~QueryCriterion_impl()
{

}

char *
DDS::QueryCriterion_impl::expression(
    ) THROW_ORB_EXCEPTIONS
{
    return NULL;
}

DDS::StringSeq *
DDS::QueryCriterion_impl::parameters(
    ) THROW_ORB_EXCEPTIONS
{
    return NULL;
}

void
DDS::QueryCriterion_impl::set_query (
    const char * expression,
    const DDS::StringSeq & parameters) THROW_ORB_AND_USER_EXCEPTIONS(
        DDS::SQLError)
{
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    DLRL_VERIFY_NOT_NULL(&exception, expression, "expression");


    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
}

void
DDS::QueryCriterion_impl::set_parameters (
    const DDS::StringSeq & parameters) THROW_ORB_AND_USER_EXCEPTIONS(
        DDS::SQLError)
{
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);



    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
}
