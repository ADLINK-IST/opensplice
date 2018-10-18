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
#ifndef CPP_DDS_OPENSPLICE_TYPESUPPORTFACTORY_H
#define CPP_DDS_OPENSPLICE_TYPESUPPORTFACTORY_H

#include "u_writer.h"
#include "CppSuperClass.h"
#include "cpp_dcps_if.h"

/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

namespace DDS
{
    namespace OpenSplice
    {
        /* Forward declaration. */
        class DomainParticipant;
        class Topic;
        class Publisher;
        class Subscriber;
        class DataWriter;
        class DataReader;
        class DataReaderView;
        class TypeSupport;

        typedef v_copyin_result (*cxxCopyIn)(void *, const void *, void *);
        typedef void (*cxxCopyOut)(const void *, void *);
        typedef void (*cxxReaderCopy)(void *sample, void *target, void *copy_arg);

        /* TODO: This is just a mock class; implement properly. */
        class OS_API TypeSupportMetaHolder :
                public ::DDS::OpenSplice::CppSuperClass
        {
            friend class DDS::OpenSplice::TypeSupport;
            friend class DDS::OpenSplice::Topic;
            friend class DDS::OpenSplice::DomainParticipant;
            friend class DDS::OpenSplice::Subscriber;
            friend class DDS::OpenSplice::Publisher;
            friend class DDS::OpenSplice::DataReader;

        protected:
            const char **metaDescriptor;
            ::DDS::ULong metaDescriptorArrLength;
            ::DDS::ULong metaDescriptorLength;
            const char *keyList;
            const char *typeName;
            const char *internalTypeName;
            cxxCopyIn copyIn;
            cxxCopyOut copyOut;
            u_writerCopy writerCopy;
            cxxReaderCopy readerCopy;
            void *cdrMarshaler;

            TypeSupportMetaHolder(const char *typeName, const char *internalTypeName, const char *keyList);
            virtual ~TypeSupportMetaHolder();

            virtual TypeSupportMetaHolder *
            clone() = 0;

            virtual TypeSupportMetaHolder *
            createProxyCDRMetaHolder(TypeSupportMetaHolder *originalTSMH, c_type topicType);

            virtual DDS::ReturnCode_t
            wlReq_deinit();

            virtual const char *
            get_type_name();

            virtual const char *
            get_internal_type_name();

            virtual const char *
            get_key_list();

            virtual char *
            get_meta_descriptor();

            virtual cxxCopyIn
            get_copy_in();

            virtual cxxCopyOut
            get_copy_out();

            virtual u_writerCopy
            get_writerCopy();

            virtual cxxReaderCopy
            get_readerCopy();

            virtual void *
            get_cdrMarshaler();

            virtual DDS::OpenSplice::DataWriter *
            create_datawriter() = 0;

            virtual DDS::OpenSplice::DataReader *
            create_datareader() = 0;

            virtual DDS::OpenSplice::DataReaderView *
            create_view() = 0;

        private:
            void initialize();

        }; /* class TypeSupportMetaHolder */
    } /* namespace OpenSplice */
} /* namespace DDS */

#undef OS_API
#endif /* CPP_DDS_OPENSPLICE_TYPESUPPORTFACTORY_H */
