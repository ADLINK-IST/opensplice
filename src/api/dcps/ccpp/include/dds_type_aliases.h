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
 /* Prevent accidental inclusion into SACPP */
#ifndef SACPP_CORBA_H
#ifndef SACPP_DDS_DCPS_H
#ifndef CCPP_DDS_TYPE_ALIASES_H
#define CCPP_DDS_TYPE_ALIASES_H

namespace DDS
{
    using CORBA::Double;
    using CORBA::Float;
    using CORBA::Short;
    using CORBA::UShort;
    using CORBA::LongLong;
    using CORBA::ULongLong;
    using CORBA::Boolean;
    using CORBA::Octet;
    using CORBA::Char;
    using CORBA::Long;
    using CORBA::ULong;
    using CORBA::Object;
    using CORBA::Object_ptr;
    using CORBA::ValueBase;
    using CORBA::LocalObject;
    using CORBA::LocalObject_ptr;
    using CORBA::String_var;
    using CORBA::Exception;
    using CORBA::UserException;
    using CORBA::SystemException;


    using CORBA::string_free;
    using CORBA::string_dup;
    using CORBA::release;
    using CORBA::add_ref;
    using CORBA::remove_ref;
    using CORBA::is_nil;
}

#endif
#endif
#endif
