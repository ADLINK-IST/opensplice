/* Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Inc.
 * Copyright 2010, Real-Time Innovations, Inc.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <dds/pub/AnyDataWriterListener.hpp>

namespace dds
{
namespace pub
{

AnyDataWriterListener::~AnyDataWriterListener() { }

NoOpAnyDataWriterListener::~NoOpAnyDataWriterListener() { }

void
NoOpAnyDataWriterListener::on_offered_deadline_missed
(dds::pub::AnyDataWriter&,
 const ::dds::core::status::OfferedDeadlineMissedStatus&)
{ }

void
NoOpAnyDataWriterListener::on_offered_incompatible_qos
(dds::pub::AnyDataWriter&,
 const ::dds::core::status::OfferedIncompatibleQosStatus&)
{ }

void
NoOpAnyDataWriterListener::on_liveliness_lost
(dds::pub::AnyDataWriter&,
 const ::dds::core::status::LivelinessLostStatus&)
{ }

void
NoOpAnyDataWriterListener::on_publication_matched
(dds::pub::AnyDataWriter&,
 const ::dds::core::status::PublicationMatchedStatus&)
{ }

}
}
