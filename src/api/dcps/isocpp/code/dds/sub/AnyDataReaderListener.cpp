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

#ifndef OSPL_DDS_SUB_ANYDATAREADERLISTENER_CPP_
#define OSPL_DDS_SUB_ANYDATAREADERLISTENER_CPP_

#include <dds/sub/AnyDataReaderListener.hpp>

namespace dds
{

namespace sub
{

AnyDataReaderListener::~AnyDataReaderListener() {}

NoOpAnyDataReaderListener::~NoOpAnyDataReaderListener() { }

void NoOpAnyDataReaderListener::on_requested_deadline_missed(
    AnyDataReader&,
    const dds::core::status::RequestedDeadlineMissedStatus&) {}

void NoOpAnyDataReaderListener::on_requested_incompatible_qos(
    AnyDataReader&,
    const dds::core::status::RequestedIncompatibleQosStatus&) {}

void NoOpAnyDataReaderListener::on_sample_rejected(
    AnyDataReader&,
    const dds::core::status::SampleRejectedStatus&) {}

void NoOpAnyDataReaderListener::on_liveliness_changed(
    AnyDataReader&,
    const dds::core::status::LivelinessChangedStatus&) {}

void NoOpAnyDataReaderListener::on_data_available(AnyDataReader&) {}

void NoOpAnyDataReaderListener::on_subscription_matched(
    AnyDataReader&,
    const dds::core::status::SubscriptionMatchedStatus&) {}

void NoOpAnyDataReaderListener::on_sample_lost(
    AnyDataReader&,
    const dds::core::status::SampleLostStatus&) {}

}
}

#endif /* OSPL_DDS_SUB_ANYDATAREADERLISTENER_CPP_ */
