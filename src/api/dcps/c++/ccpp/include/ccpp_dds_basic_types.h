/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#ifndef CPP_DDS_BASIC_TYPES_H_
#define CPP_DDS_BASIC_TYPES_H_

#include "ccpp.h"
#include "cpp_dcps_if.h"

namespace DDS {
    /* 
     * By using 'using' instead of actual functions and typedefs, 
     * we prevent these compiler errors: 
     *      - call of overloaded ‘function’ is ambiguous
     *      - reference to ‘type’ is ambiguous 
     */

    /* Types */
    using CORBA::Boolean;
    using CORBA::Char;
    using CORBA::Octet;
    using CORBA::Short;
    using CORBA::UShort;
    using CORBA::Long;
    using CORBA::ULong;
    using CORBA::Float;
    using CORBA::Double;
    using CORBA::LongLong;
    using CORBA::ULongLong;

    using CORBA::String_var;

    using CORBA::Object;
    using CORBA::Object_ptr;
    using CORBA::LocalObject;
    using CORBA::LocalObject_ptr;
    using CORBA::Exception;
    using CORBA::UserException;
    using CORBA::SystemException;
    using CORBA::ValueBase;

    /* Functions */
    using CORBA::release;
    using CORBA::string_alloc;
    using CORBA::string_free;
    using CORBA::string_dup;
    using CORBA::is_nil;
}

#undef OS_API
#endif /* CPP_DDS_BASIC_TYPES_H_ */
