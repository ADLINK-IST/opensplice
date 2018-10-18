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
#ifndef CCPP_STREAMSUTILS_H_
#define CCPP_STREAMSUTILS_H_

#include "streams_ccpp.h"
#include "os_mutex.h"
#include "ccpp_streams_if.h"

namespace DDS {
    namespace Streams {
        class LocalFactoryMutex {
            public:
                os_mutex lfMutex;
                LocalFactoryMutex();
                ~LocalFactoryMutex();
        };

        extern OS_STREAMS_API const DDS::SubscriberQos default_subscriber_qos;
        extern OS_STREAMS_API const DDS::PublisherQos default_publisher_qos;
        extern OS_STREAMS_API const DDS::TopicQos default_topic_qos;
        extern OS_STREAMS_API const DDS::DataWriterQos default_datawriter_qos;
        extern OS_STREAMS_API const DDS::DataReaderQos default_datareader_qos;
    }
}

#undef OS_STREAMS_API

#endif /* CCPP_STREAMSUTILS_H_ */


