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
#ifndef OSPL_DDS_PUB_ANYDATAWRITER_HPP_
#define OSPL_DDS_PUB_ANYDATAWRITER_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/pub/AnyDataWriter.hpp>

// Implementation

namespace dds
{
namespace pub
{

inline AnyDataWriter::AnyDataWriter(const dds::core::null_type&)
    : holder_()
{
}

template <typename T>
AnyDataWriter::AnyDataWriter(const dds::pub::DataWriter<T>& dw)
    : holder_(new dds::pub::detail::DWHolder<T>(dw)) { }

template <typename T> AnyDataWriter&
AnyDataWriter::operator =(const dds::pub::DataWriter<T>& rhs)
{
    holder_.reset(new detail::DWHolder<T>(rhs));
    return *this;
}

inline AnyDataWriter& AnyDataWriter::swap(AnyDataWriter& rhs)
{
    holder_.swap(rhs.holder_);
    return *this;
}

inline AnyDataWriter& AnyDataWriter::operator =(AnyDataWriter rhs)
{
    return this->swap(rhs);
}

template <typename T>
dds::pub::DataWriter<T> AnyDataWriter::get()
{
    OMG_DDS_STATIC_ASSERT(::dds::topic::is_topic_type<T>::value == 1);
    detail::DWHolder<T>* h = dynamic_cast<detail::DWHolder<T>* >(holder_.get());
    if(h == 0)
    {
        throw ::dds::core::InvalidDowncastError("invalid type");
    }
    return h->get();
}

}
}
#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    void dds::pub::DataWriter<T, DELEGATE>::close()
    {
        this->delegate()->close();
        dds::pub::AnyDataWriter adw(*this);
        org::opensplice::core::retain_remove<dds::pub::AnyDataWriter>(adw);
    }

    template <typename T, template <typename Q> class DELEGATE>
    void dds::pub::DataWriter<T, DELEGATE>::retain()
    {
        this->delegate()->retain();
        dds::pub::AnyDataWriter adr(*this);
        org::opensplice::core::retain_add<dds::pub::AnyDataWriter>(adr);
    }
#endif
// End of implementation

#endif /* OSPL_DDS_PUB_ANYDATAWRITER_HPP_ */
