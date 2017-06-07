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
 * @ingroup examplesdcpsListenerisocpp
 */

#include "ExampleDataReaderListener.hpp"

namespace examples { namespace dcps { namespace Listener { namespace isocpp {

ExampleDataReaderListener::ExampleDataReaderListener() : data_received_(false),
                                                         deadline_expired_(false) {}

ExampleDataReaderListener::~ExampleDataReaderListener() {}

void ExampleDataReaderListener::on_requested_deadline_missed(
    dds::sub::DataReader<ListenerData::Msg>& the_reader,
    const dds::core::status::RequestedDeadlineMissedStatus& status)
{
    std::cout << "\n=== [ExampleDataReaderListener::on_requested_deadline_missed] : triggered" << std::endl;
    std::cout << "\n=== [ExampleDataReaderListener::on_requested_deadline_missed] : stopping" << std::endl;
    deadline_expired_ = true;
}


void ExampleDataReaderListener::on_data_available(dds::sub::DataReader<ListenerData::Msg>& dr)
{
    dds::sub::LoanedSamples<ListenerData::Msg> samples = dr.take();
    if((*samples.begin()).info().valid())
    {
        std::cout << "\n=== [ExampleDataReaderListener::on_data_available] - samples.length() : " << samples.length() << std::endl;
        for (dds::sub::LoanedSamples<ListenerData::Msg>::const_iterator sample = samples.begin();
             sample < samples.end();
             ++sample)
        {
                data_received_ = true;
                std::cout << "=== [Subscriber] message received :" << std::endl;
                std::cout << "    userID  : " << sample->data().userID() << std::endl;
                std::cout << "    Message : \"" << sample->data().message() << "\"" << std::endl;
        }
    }
}

}}}}
