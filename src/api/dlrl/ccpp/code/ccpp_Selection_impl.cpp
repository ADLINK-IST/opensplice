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
#include "ccpp_Selection_impl.h"
#include "DLRL_Report.h"

DDS::Selection_impl::Selection_impl() : selection(NULL){

}

DDS::Selection_impl::~Selection_impl()
{
    DLRL_INFO(INF_ENTER);
    if(selection){
        DK_Entity_ts_release(reinterpret_cast<DK_Entity*>(selection));
        selection = NULL;
    }
    DLRL_INFO(INF_EXIT);
}

CORBA::Boolean
DDS::Selection_impl::auto_refresh(
    ) THROW_ORB_EXCEPTIONS
{
    CORBA::Boolean autoRefresh = FALSE;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    autoRefresh = static_cast<CORBA::Boolean>(
        DK_SelectionAdmin_ts_getAutoRefresh(selection, &exception));
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
    return autoRefresh;
}

CORBA::Boolean
DDS::Selection_impl::concerns_contained(
    ) THROW_ORB_EXCEPTIONS
{
    CORBA::Boolean concernsContained = FALSE;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    concernsContained = static_cast<CORBA::Boolean>(
        DK_SelectionAdmin_ts_getConcernsContained(selection, &exception));
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
    return concernsContained;
}

DDS::SelectionCriterion_ptr
DDS::Selection_impl::criterion(
    ) THROW_ORB_EXCEPTIONS
{
    DDS::SelectionCriterion_ptr criterion = NULL;
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    criterion = DOWNCAST_DDS_SELECTIONCRITERION(
        DK_SelectionAdmin_ts_getCriterion(
            selection,
            &exception,
            NULL));
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
    return criterion;
}

void
DDS::Selection_impl::refresh(
    ) THROW_ORB_EXCEPTIONS
{
    DLRL_Exception exception;

    DLRL_INFO(INF_ENTER);

    DLRL_Exception_init(&exception);

    DK_SelectionAdmin_ts_refresh(selection, &exception, NULL);
    DLRL_Exception_PROPAGATE(&exception);

    DLRL_Exception_EXIT(&exception);
    DLRL_INFO(INF_EXIT);
    ccpp_DlrlUtils_us_handleException(&exception);
}
