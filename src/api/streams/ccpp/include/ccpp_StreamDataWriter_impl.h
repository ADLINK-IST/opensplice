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
#ifndef CCPP_STREAMDATAWRITER_H
#define CCPP_STREAMDATAWRITER_H

#include "ccpp_dds_dcps.h"
#include "streams_ccpp.h"
#include "vortex_os.h"
#include "ccpp_StreamsUtils.h"
#include "ccpp_streams_if.h"

namespace DDS {
    namespace Streams {
        extern OS_STREAMS_API StreamDataWriterQos DefaultStreamDataWriterQos;

        class OS_STREAMS_API StreamDataWriter_impl :
            public virtual StreamDataWriter,
            public LOCAL_REFCOUNTED_OBJECT
        {
            private:
                static DDS::DomainParticipant_var participant;
                static os_uint32 nrParticipantUsers;

            protected:
                DDS::Streams::StreamDataWriterQos qos;
                DDS::Publisher_var publisher;
                DDS::Topic_var topic;

                StreamDataWriter_impl(
                    DDS::Publisher_ptr publisher,
                    DDS::DomainId_t domainId,
                    DDS::Streams::StreamDataWriterQos &qos,
                    DDS::TypeSupport_ptr typeSupport,
                    const char *streamName);

                ~StreamDataWriter_impl();

            public:
                static ::DDS::ReturnCode_t get_default_qos(
                    ::DDS::Streams::StreamDataWriterQos & qos) THROW_ORB_EXCEPTIONS;

                virtual ::DDS::ReturnCode_t set_qos(
                    const ::DDS::Streams::StreamDataWriterQos & qos) THROW_ORB_EXCEPTIONS = 0;

                ::DDS::ReturnCode_t get_qos(
                    ::DDS::Streams::StreamDataWriterQos & qos) THROW_ORB_EXCEPTIONS;
			private:
				StreamDataWriter_impl (const StreamDataWriter_impl &);
				StreamDataWriter_impl & operator = (const StreamDataWriter_impl &);
        };
    }
}

#undef OS_STREAMS_API

#endif /* CCPP_STREAMDATAWRITER_H */
