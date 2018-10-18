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
#ifndef OSPL_DDS_SUB_ANYDATAREADER_HPP_
#define OSPL_DDS_SUB_ANYDATAREADER_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/sub/AnyDataReader.hpp>

// Implementation
namespace dds
{
namespace sub
{

inline AnyDataReader::AnyDataReader(const dds::core::null_type&)
    : holder_()
{
}

template <typename T>
AnyDataReader::AnyDataReader(const dds::sub::DataReader<T>& dr)
    : holder_(new detail::DRHolder<T>(dr)) { }

inline const detail::DRHolderBase* AnyDataReader::operator->() const
{
    return holder_.get();
}

inline dds::sub::detail::DRHolderBase* AnyDataReader::operator->()
{
    return holder_.get();
}


inline AnyDataReader& AnyDataReader::swap(AnyDataReader& rhs)
{
    holder_.swap(rhs.holder_);
    return *this;
}

template <typename T>
AnyDataReader& AnyDataReader::operator =(const DataReader<T>& rhs)
{
    holder_.reset(new detail::DRHolder<T>(rhs));
    return *this;
}

inline AnyDataReader& AnyDataReader::operator =(AnyDataReader rhs)
{
    return this->swap(rhs);
}

template <typename T>
dds::sub::DataReader<T> AnyDataReader::get()
{
    OMG_DDS_STATIC_ASSERT(::dds::topic::is_topic_type<T>::value == 1);
    detail::DRHolder<T>* h = dynamic_cast<detail::DRHolder<T>* >(holder_.get());
    if(h == 0)
    {
        throw dds::core::InvalidDowncastError("invalid type");
    }
    return h->get();
}

}
}

#ifndef OSPL_2893_COMPILER_BUG
template <typename T, template <typename Q> class DELEGATE>
void dds::sub::DataReader<T, DELEGATE>::close()
{
    try
    {
        this->delegate()->close();
        dds::sub::AnyDataReader adr(*this);
        org::opensplice::core::retain_remove<dds::sub::AnyDataReader>(adr);
    }
    catch(int i)
    {
        (void)i;
    }
}

template <typename T, template <typename Q> class DELEGATE>
void dds::sub::DataReader<T, DELEGATE>::retain()
{
    this->delegate()->retain();
    dds::sub::AnyDataReader adr(*this);
    org::opensplice::core::retain_add<dds::sub::AnyDataReader>(adr);
}
#endif

// End of implementation

#endif /* OSPL_DDS_SUB_ANYDATAREADER_HPP_ */
