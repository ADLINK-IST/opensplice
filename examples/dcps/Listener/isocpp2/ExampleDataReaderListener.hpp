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

class ExampleDataReaderListener :   public virtual dds::sub::DataReaderListener<ListenerData::Msg>, public virtual dds::sub::NoOpDataReaderListener<ListenerData::Msg>
{
public:
    ExampleDataReaderListener();
    virtual ~ExampleDataReaderListener();

    virtual void on_requested_deadline_missed(
        dds::sub::DataReader<ListenerData::Msg>& the_reader,
        const dds::core::status::RequestedDeadlineMissedStatus& status);

    virtual void on_data_available(dds::sub::DataReader<ListenerData::Msg>& the_reader);

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
