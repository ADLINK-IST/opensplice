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
