/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
#ifndef CPP_DDS_OPENSPLICE_ERRORINFO_H
#define CPP_DDS_OPENSPLICE_ERRORINFO_H

#include "CppSuperClass.h"
#include "cpp_dcps_if.h"

/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace DDS
{

  typedef ErrorInfoInterface_ptr ErrorInfo_ptr;
  typedef ErrorInfoInterface_var ErrorInfo_var;

  class OS_API ErrorInfo
    : public virtual ::DDS::ErrorInfoInterface,
      public ::DDS::OpenSplice::CppSuperClass
    {
    public:

        ErrorInfo( void );
       ~ErrorInfo( void );

        virtual ::DDS::ReturnCode_t
        update(
            void
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t
        get_code(
            ::DDS::ReturnCode_t & code
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t
        get_message(
           char *& message
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t
        get_location(
            char * & location
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t
        get_source_line(
            char * & source_line
        ) THROW_ORB_EXCEPTIONS;

        virtual ::DDS::ReturnCode_t
        get_stack_trace(
            char * & stack_trace
        ) THROW_ORB_EXCEPTIONS;

        virtual DDS::ReturnCode_t
        wlReq_deinit();

    private:
        ::DDS::Boolean      valid;
        ::DDS::ReturnCode_t code;
        ::DDS::String_var   location;
        ::DDS::String_var   source_line;
        ::DDS::String_var   stack_trace;
        ::DDS::String_var   message;
    };
} /* end namespace DDS */
#undef OS_API
#endif /* CPP_DDS_OPENSPLICE_ERRORINFO_H */
