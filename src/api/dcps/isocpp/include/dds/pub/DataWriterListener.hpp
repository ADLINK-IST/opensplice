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
#ifndef OSPL_DDS_PUB_DATAWRITERLISTENER_HPP_
#define OSPL_DDS_PUB_DATAWRITERLISTENER_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/pub/DataWriterListener.hpp>

// Implementation

namespace dds
{
namespace pub
{

template <typename T>
NoOpDataWriterListener<T>::~NoOpDataWriterListener() { }

template <typename T>
void NoOpDataWriterListener<T>::on_offered_deadline_missed(dds::pub::DataWriter<T>& writer,
        const dds::core::status::OfferedDeadlineMissedStatus& status) { }

template <typename T>
void NoOpDataWriterListener<T>::on_offered_incompatible_qos(dds::pub::DataWriter<T>& writer,
        const dds::core::status::OfferedIncompatibleQosStatus&  status) { }

template <typename T>
void NoOpDataWriterListener<T>::on_liveliness_lost(dds::pub::DataWriter<T>& writer,
        const dds::core::status::LivelinessLostStatus& status) { }

template <typename T>
void NoOpDataWriterListener<T>::on_publication_matched(dds::pub::DataWriter<T>& writer,
        const dds::core::status::PublicationMatchedStatus& status) { }
}
}

// End of implementation

#endif /* OSPL_DDS_PUB_DATAWRITERLISTENER_HPP_ */
