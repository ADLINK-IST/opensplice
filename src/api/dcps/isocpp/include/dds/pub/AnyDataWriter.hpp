/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2012 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
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

// End of implementation

#endif /* OSPL_DDS_PUB_ANYDATAWRITER_HPP_ */
