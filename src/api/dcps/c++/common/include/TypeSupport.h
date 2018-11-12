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
#ifndef CPP_DDS_OPENSPLICE_TYPESUPPORT_H
#define CPP_DDS_OPENSPLICE_TYPESUPPORT_H

#include "ccpp.h"
#include "CppSuperClass.h"
#include "TypeSupportMetaHolder.h"
#include "os_atomics.h"
#include "cpp_dcps_if.h"

/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace DDS
{
    namespace OpenSplice
    {
        class OS_API TypeSupport :
              public virtual ::DDS::TypeSupport,
              public ::DDS::OpenSplice::CppSuperClass
        {
            friend class ::DDS::OpenSplice::DomainParticipant;
            friend class ::DDS::OpenSplice::CdrTypeSupport;

        protected:
            DDS::OpenSplice::TypeSupportMetaHolder *tsMetaHolder;

            TypeSupport();

            virtual ~TypeSupport();

            DDS::ReturnCode_t
            wlReq_deinit();

            DDS::OpenSplice::TypeSupportMetaHolder *
            get_metaHolder();

        public:
            virtual ::DDS::ReturnCode_t
            register_type(
                ::DDS::DomainParticipant_ptr participant,
                const char * type_name) THROW_ORB_EXCEPTIONS;

            virtual char *
            get_type_name() THROW_ORB_EXCEPTIONS;
        }; /* class TypeSupport */
    } /* namespace OpenSplice */
} /* namespace DDS */

#undef OS_API
#endif /* CPP_DDS_OPENSPLICE_TYPESUPPORT_H */
