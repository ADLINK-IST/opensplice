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
#include "ccpp_List_impl.h"

DDS::List_impl::List_impl()
{

}

DDS::List_impl::~List_impl()
{

}


CORBA::Long
DDS::List_impl::length(
    ) THROW_VALUETYPE_ORB_EXCEPTIONS
{
    return 0;
}

void
DDS::List_impl::clear(
    ) THROW_VALUETYPE_ORB_EXCEPTIONS
{

}

void
DDS::List_impl::remove(
    ) THROW_VALUETYPE_ORB_EXCEPTIONS
{

}

DDS::LongSeq *
DDS::List_impl::added_elements(
    ) THROW_VALUETYPE_ORB_EXCEPTIONS
{
    return NULL;
}

DDS::LongSeq *
DDS::List_impl::removed_elements(
    ) THROW_VALUETYPE_ORB_EXCEPTIONS
{
    return NULL;
}

DDS::LongSeq *
DDS::List_impl::modified_elements(
    ) THROW_VALUETYPE_ORB_EXCEPTIONS
{
    return NULL;
}
