#ifndef DDS_STREAMS_PUB_STREAMDATAWRITER_HPP_
#define DDS_STREAMS_PUB_STREAMDATAWRITER_HPP_
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

#include <spec/dds/streams/pub/StreamDataWriter.hpp>

namespace dds
{
namespace streams
{
namespace pub
{

#ifdef OSPL_2893_COMPILER_BUG
#define DELEGATE dds::streams::pub::detail::StreamDataWriter
template <typename T>
class StreamDataWriter<T, dds::streams::pub::detail::StreamDataWriter> : public dds::core::TEntity< dds::streams::pub::detail::StreamDataWriter<T> >
{
public:
    OMG_DDS_BASIC_REF_TYPE(StreamDataWriter, ::dds::core::TEntity, DELEGATE<T>)
#endif

    #ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    StreamDataWriter<T, DELEGATE>::
    #endif
    StreamDataWriter(const std::string& stream_name,
                     #ifndef OSPL_2893_COMPILER_BUG
                     const dds::streams::pub::qos::StreamDataWriterQos& qos
                     #else
                     const dds::streams::pub::qos::StreamDataWriterQos& qos = dds::streams::pub::qos::StreamDataWriterQos()
                     #endif
    ) : ::dds::core::TEntity< DELEGATE<T> >(new DELEGATE<T>(stream_name, qos)) {}

    #ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    StreamDataWriter<T, DELEGATE>::
    #endif
    StreamDataWriter(uint32_t domain_id,
                     const std::string& stream_name,
                     #ifndef OSPL_2893_COMPILER_BUG
                     const dds::streams::pub::qos::StreamDataWriterQos& qos
                     #else
                     const dds::streams::pub::qos::StreamDataWriterQos& qos = dds::streams::pub::qos::StreamDataWriterQos()
                     #endif
    ) : ::dds::core::TEntity< DELEGATE<T> >(new DELEGATE<T>(domain_id, stream_name, qos)) {}

    #ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    StreamDataWriter<T, DELEGATE>::
    #endif
    StreamDataWriter(const dds::pub::Publisher& publisher,
                     const std::string& stream_name,
                     #ifndef OSPL_2893_COMPILER_BUG
                     const dds::streams::pub::qos::StreamDataWriterQos& qos
                     #else
                     const dds::streams::pub::qos::StreamDataWriterQos& qos = dds::streams::pub::qos::StreamDataWriterQos()
                     #endif
    ) : ::dds::core::TEntity< DELEGATE<T> >(new DELEGATE<T>(publisher, stream_name, qos)) {}

    #ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    #endif
    StreamDataWriter<T, DELEGATE>&
    #ifndef OSPL_2893_COMPILER_BUG
    StreamDataWriter<T, DELEGATE>::
    #endif
    stream(uint32_t id)
    {
        this->delegate()->stream(id);
        return *this;
    }

    #ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    #endif
    uint32_t
    #ifndef OSPL_2893_COMPILER_BUG
    StreamDataWriter<T, DELEGATE>::
    #endif
    stream()
    {
        return this->delegate()->stream();
    }

    #ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    #endif
    StreamDataWriter<T, DELEGATE>&
    #ifndef OSPL_2893_COMPILER_BUG
    StreamDataWriter<T, DELEGATE>::
    #endif
    append(const T& data)
    {
        this->delegate()->append(data);
        return *this;
    }

    #ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    #endif
    template <typename FWIterator>
    StreamDataWriter<T, DELEGATE>&
    #ifndef OSPL_2893_COMPILER_BUG
    StreamDataWriter<T, DELEGATE>::
    #endif
    append(const FWIterator& begin, const FWIterator& end)
    {
        FWIterator i = begin;
        while(i != end)
        {
            this->delegate()->append(*i);
            ++i;
        }
        return *this;
    }

    #ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    #endif
    StreamDataWriter<T, DELEGATE>&
    #ifndef OSPL_2893_COMPILER_BUG
    StreamDataWriter<T, DELEGATE>::
    #endif
    operator<<(const T& data)
    {
        this->append(data);
        return *this;
    }

    #ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    #endif
    StreamDataWriter<T, DELEGATE>&
    #ifndef OSPL_2893_COMPILER_BUG
    StreamDataWriter<T, DELEGATE>::
    #endif
    flush()
    {
        this->delegate()->flush();
        return *this;
    }

    #ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    #endif
    const dds::streams::pub::qos::StreamDataWriterQos&
    #ifndef OSPL_2893_COMPILER_BUG
    StreamDataWriter<T, DELEGATE>::
    #endif
    qos() const
    {
        return this->delegate()->qos();
    }

    #ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    #endif
    void
    #ifndef OSPL_2893_COMPILER_BUG
    StreamDataWriter<T, DELEGATE>::
    #endif
    qos(const dds::streams::pub::qos::StreamDataWriterQos& qos)
    {
        this->delegate()->qos(qos);
    }

    #ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    #endif
    dds::streams::pub::qos::StreamDataWriterQos&
    #ifndef OSPL_2893_COMPILER_BUG
    StreamDataWriter<T, DELEGATE>::
    #endif
    operator<<(const dds::streams::pub::qos::StreamDataWriterQos& qos)
    {
        this->qos(qos);
        return (dds::streams::pub::qos::StreamDataWriterQos&)this->qos();
    }

    #ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    #endif
    const StreamDataWriter<T, DELEGATE>&
    #ifndef OSPL_2893_COMPILER_BUG
    StreamDataWriter<T, DELEGATE>::
    #endif
    operator>>(dds::streams::pub::qos::StreamDataWriterQos& qos) const
    {
        qos = this->qos();
        return *this;
    }
#ifdef OSPL_2893_COMPILER_BUG
};
#endif

}
}
}

#endif /* DDS_STREAMS_PUB_STREAMDATAWRITER_HPP_ */
