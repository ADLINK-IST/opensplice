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
#ifndef OSPL_DDS_SUB_DATAREADERLISTENER_HPP_
#define OSPL_DDS_SUB_DATAREADERLISTENER_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/sub/DataReaderListener.hpp>

// Implementation

namespace dds
{
namespace sub
{

template <typename T>
DataReaderListener<T>::~DataReaderListener() { }

template <typename T>
NoOpDataReaderListener<T>::~NoOpDataReaderListener() { }

template <typename T>
void NoOpDataReaderListener<T>::on_requested_deadline_missed(
    DataReader<T>& reader,
    const dds::core::status::RequestedDeadlineMissedStatus& status) { }

template <typename T>
void NoOpDataReaderListener<T>::on_requested_incompatible_qos(
    DataReader<T>& reader,
    const dds::core::status::RequestedIncompatibleQosStatus& status) { }

template <typename T>
void NoOpDataReaderListener<T>::on_sample_rejected(
    DataReader<T>& reader,
    const dds::core::status::SampleRejectedStatus& status) { }

template <typename T>
void NoOpDataReaderListener<T>::on_liveliness_changed(
    DataReader<T>& reader,
    const dds::core::status::LivelinessChangedStatus& status) { }

template <typename T>
void NoOpDataReaderListener<T>::on_data_available(DataReader<T>& reader) { }

template <typename T>
void NoOpDataReaderListener<T>::on_subscription_matched(
    DataReader<T>& reader,
    const dds::core::status::SubscriptionMatchedStatus& status) { }

template <typename T>
void NoOpDataReaderListener<T>::on_sample_lost(
    DataReader<T>& reader,
    const dds::core::status::SampleLostStatus& status) { }
}
}

// End of implementation

#endif /* OSPL_DDS_SUB_DATAREADERLISTENER_HPP_ */
