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
