#ifndef ORG_OPENSPLICE_STREAMS_PUB_QOS_STREAM_DATA_WRITER_QOS_HPP_
#define ORG_OPENSPLICE_STREAMS_PUB_QOS_STREAM_DATA_WRITER_QOS_HPP_
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
namespace pub
{
namespace qos
{

class OSPL_ISOCPP_IMPL_API StreamDataWriterQosImpl;

}
}
}
}
}

class org::opensplice::streams::pub::qos::StreamDataWriterQosImpl
{
public:
    StreamDataWriterQosImpl();

    StreamDataWriterQosImpl(const StreamDataWriterQosImpl& qos);

    StreamDataWriterQosImpl(dds::streams::core::policy::StreamFlush stream_flush);

    ~StreamDataWriterQosImpl();

    void policy(const dds::streams::core::policy::StreamFlush& stream_flush);

    template <typename POLICY> const POLICY& policy() const;

    template <typename POLICY> POLICY& policy();

    bool operator ==(const StreamDataWriterQosImpl& other) const
    {
        return other.stream_flush_ == stream_flush_;
    }

private:
    dds::streams::core::policy::StreamFlush stream_flush_;
};

namespace org
{
namespace opensplice
{
namespace streams
{
namespace pub
{
namespace qos
{

template<> inline const dds::streams::core::policy::StreamFlush&
StreamDataWriterQosImpl::policy<dds::streams::core::policy::StreamFlush>() const
{
    return stream_flush_;
}

template<> inline dds::streams::core::policy::StreamFlush&
StreamDataWriterQosImpl::policy<dds::streams::core::policy::StreamFlush>()
{
    return stream_flush_;
}

}
}
}
}
}

#endif /* ORG_OPENSPLICE_STREAMS_PUB_QOS_STREAM_DATA_WRITER_QOS_HPP_ */
