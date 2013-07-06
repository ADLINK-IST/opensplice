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

#ifndef __EXAMPLEDATAREADERLISTENER_H__
#define __EXAMPLEDATAREADERLISTENER_H__

#include <iostream>

#include "ListenerData_DCPS.hpp"

namespace examples { namespace dcps { namespace Listener { namespace isocpp {

/**
 * @addtogroup examplesdcpsListenerisocpp
 */
/** @{*/
/** @file */

class ExampleDataReaderListener : public virtual dds::sub::DataReaderListener<ListenerData::Msg>
{
public:
    ExampleDataReaderListener();
    virtual ~ExampleDataReaderListener();

    virtual void on_requested_deadline_missed(
        dds::sub::DataReader<ListenerData::Msg>& the_reader,
        const dds::core::status::RequestedDeadlineMissedStatus& status);

    virtual void on_requested_incompatible_qos(
        dds::sub::DataReader<ListenerData::Msg>& the_reader,
        const dds::core::status::RequestedIncompatibleQosStatus& status);

    virtual void on_sample_rejected(
        dds::sub::DataReader<ListenerData::Msg>& the_reader,
        const dds::core::status::SampleRejectedStatus& status);

    virtual void on_liveliness_changed(
        dds::sub::DataReader<ListenerData::Msg>& the_reader,
        const dds::core::status::LivelinessChangedStatus& status);

    virtual void on_data_available(dds::sub::DataReader<ListenerData::Msg>& the_reader);

    virtual void on_subscription_matched(
        dds::sub::DataReader<ListenerData::Msg>& the_reader,
        const dds::core::status::SubscriptionMatchedStatus& status);

    virtual void on_sample_lost(
        dds::sub::DataReader<ListenerData::Msg>& the_reader,
        const dds::core::status::SampleLostStatus& status);

    /** Is set to true when the listener has been notified of
     * ExampleDataReaderListener::on_data_available and has read some data */
    bool data_received_;

    /** Is set to true when the listener has been notified of
     * ExampleDataReaderListener::on_requested_deadline_missed  */
    bool deadline_expired_;
};

/** @}*/

}}}}

#endif
