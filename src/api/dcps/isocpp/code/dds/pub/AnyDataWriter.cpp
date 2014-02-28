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


/**
 * @file
 */

#ifndef OSPL_DDS_PUB_ANYDATAWRITER_CPP_
#define OSPL_DDS_PUB_ANYDATAWRITER_CPP_

#include <dds/pub/AnyDataWriter.hpp>

namespace dds
{
namespace pub
{

const dds::pub::qos::DataWriterQos& AnyDataWriter::qos() const
{
    return holder_->qos();
}

void AnyDataWriter::qos(const ::dds::pub::qos::DataWriterQos& q)
{
    holder_->qos(q);
}

const std::string& AnyDataWriter::topic_name() const
{
    return holder_->topic_name();
}

const std::string& AnyDataWriter::type_name() const
{
    return holder_->type_name();
}

const dds::pub::Publisher& AnyDataWriter::publisher() const
{
    return holder_->publisher();
}

void AnyDataWriter::wait_for_acknowledgments(const dds::core::Duration& timeout)
{
    holder_->wait_for_acknowledgments(timeout);
}

void AnyDataWriter::close()
{
    holder_->close();
}

void AnyDataWriter::retain(bool b)
{
    holder_->retain(b);
}

const detail::DWHolderBase* AnyDataWriter::operator->() const
{
    return holder_.get();
}

detail::DWHolderBase* AnyDataWriter::operator->()
{
    return holder_.get();
}

}
}

#endif /* OSPL_DDS_PUB_ANYDATAWRITER_CPP_ */
