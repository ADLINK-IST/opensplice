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
#include "ccpp_Contract_impl.h"

DDS::Contract_impl::Contract_impl()
{

}

DDS::Contract_impl::~Contract_impl()
{

}

CORBA::Long
DDS::Contract_impl::depth(
    ) THROW_ORB_EXCEPTIONS
{
    return 0;
}

DDS::ObjectScope
DDS::Contract_impl::scope(
    ) THROW_ORB_EXCEPTIONS
{
    return SIMPLE_OBJECT_SCOPE;
}

DDS::ObjectRoot *
DDS::Contract_impl::contracted_object(
    ) THROW_ORB_EXCEPTIONS
{
    return NULL;
}

void
DDS::Contract_impl::set_depth(
    ::CORBA::Long depth) THROW_ORB_EXCEPTIONS
{

}

void
DDS::Contract_impl::set_scope(
    DDS::ObjectScope scope) THROW_ORB_EXCEPTIONS
{

}
