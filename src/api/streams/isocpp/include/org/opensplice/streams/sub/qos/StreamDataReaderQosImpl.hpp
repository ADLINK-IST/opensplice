#ifndef ORG_OPENSPLICE_STREAMS_SUB_QOS_STREAM_DATA_READER_QOS_HPP_
#define ORG_OPENSPLICE_STREAMS_SUB_QOS_STREAM_DATA_READER_QOS_HPP_
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

/**
 * @file
 */

#include <dds/streams/core/policy/CorePolicy.hpp>

namespace org
{
namespace opensplice
{
namespace streams
{
namespace sub
{
namespace qos
{

class OSPL_ISOCPP_IMPL_API StreamDataReaderQosImpl;

}
}
}
}
}

class org::opensplice::streams::sub::qos::StreamDataReaderQosImpl
{
public:
    StreamDataReaderQosImpl();

    StreamDataReaderQosImpl(const StreamDataReaderQosImpl& qos);

    ~StreamDataReaderQosImpl();

    template <typename POLICY> const POLICY& policy() const;

    template <typename POLICY> POLICY& policy();

    bool operator ==(const StreamDataReaderQosImpl& other) const
    {
        return true;
    }

private:
};

#endif /* ORG_OPENSPLICE_STREAMS_SUB_QOS_STREAM_DATA_READER_QOS_HPP_ */
