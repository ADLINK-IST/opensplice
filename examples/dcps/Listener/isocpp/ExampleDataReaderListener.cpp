/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
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

void ExampleDataReaderListener::on_requested_incompatible_qos(
    dds::sub::DataReader<ListenerData::Msg>& the_reader,
    const dds::core::status::RequestedIncompatibleQosStatus& status)
{
    std::cout << "\n=== [ExampleDataReaderListener::on_requested_incompatible_qos] : triggered" << std::endl;
}

void ExampleDataReaderListener::on_sample_rejected(
    dds::sub::DataReader<ListenerData::Msg>& the_reader,
    const dds::core::status::SampleRejectedStatus& status)
{
    std::cout << "\n=== [ExampleDataReaderListener::on_sample_rejected] : triggered" << std::endl;
}

void ExampleDataReaderListener::on_liveliness_changed(
    dds::sub::DataReader<ListenerData::Msg>& the_reader,
    const dds::core::status::LivelinessChangedStatus& status)
{
    std::cout << "\n=== [ExampleDataReaderListener::on_liveliness_changed] : triggered" << std::endl;
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

void ExampleDataReaderListener::on_subscription_matched(
    dds::sub::DataReader<ListenerData::Msg>& the_reader,
    const dds::core::status::SubscriptionMatchedStatus& status)
{
    std::cout << "\n=== [ExampleDataReaderListener::on_subscription_matched] : triggered" << std::endl;
}

void ExampleDataReaderListener::on_sample_lost(
    dds::sub::DataReader<ListenerData::Msg>& the_reader,
    const dds::core::status::SampleLostStatus& status)
{
    std::cout << "\n=== [ExampleDataReaderListener::on_sample_lost] : triggered" << std::endl;
}

}}}}
