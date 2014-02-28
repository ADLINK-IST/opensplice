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
#ifndef SACPP_CORBA_H
#define SACPP_CORBA_H

#include "sacpp_DDS_DCPS.h"
#include "sacpp_if.h"

#ifndef OSPL_CPP_CORBA_BACWARD_COMPAT
#define OSPL_CPP_CORBA_BACWARD_COMPAT 1
#endif

#if OSPL_CPP_CORBA_BACWARD_COMPAT == 1

namespace CORBA
{
    using DDS::Double;
    using DDS::Float;
    using DDS::Short;
    using DDS::LongLong;
    using DDS::Boolean;
    using DDS::Octet;
    using DDS::Long;
    using DDS::ULong;
    using DDS::Object;
    using DDS::Object_ptr;
    using DDS::ValueBase;
    using DDS::LocalObject;
    using DDS::LocalObject_ptr;
    using DDS::String_var;
    using DDS::Exception;
    using DDS::UserException;
    using DDS::SystemException;


    using DDS::string_free;
    using DDS::string_dup;
    using DDS::release;
    using DDS::add_ref;
    using DDS::remove_ref;
    using DDS::is_nil;
}

#endif

#undef SACPP_API

#endif /* SACPP_CORBA_H */
