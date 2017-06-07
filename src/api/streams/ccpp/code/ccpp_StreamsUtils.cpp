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
#include "ccpp_StreamsUtils.h"
#include "ccpp_dds_dcps.h"

namespace DDS {
    namespace Streams {
        LocalFactoryMutex::LocalFactoryMutex()
        {
            os_result result;
            result = os_mutexInit(&lfMutex, NULL);

            if (result != os_resultSuccess) {
                OS_REPORT(OS_ERROR, "DDS::Streams::LocalFactoryMutex", 0, "Unable to create mutex");
            }
        }
        LocalFactoryMutex::~LocalFactoryMutex()
        {
            os_mutexDestroy(&lfMutex);
        }

        const DDS::PublisherQos default_publisher_qos = {
            { INSTANCE_PRESENTATION_QOS, false, false },    /* presentation         */
            { DDS::StringSeq() },                           /* partition            */
            { DDS::octSeq() },                              /* group_data           */
            { true  }                                       /* factory auto_enable  */
        };

        const DDS::SubscriberQos default_subscriber_qos = {
            { INSTANCE_PRESENTATION_QOS, false, false },    /* presentation         */
            { DDS::StringSeq() },                           /* partition            */
            { DDS::octSeq() },                              /* group_data           */
            { true  },                                      /* factory auto_enable  */
            { "", false }                                   /* share                */
        };

        const DDS::TopicQos default_topic_qos = {
            { DDS::octSeq() },                                          /* topicdata */
            { VOLATILE_DURABILITY_QOS },                                /* durability                       */
            { { DURATION_ZERO_SEC, DURATION_ZERO_NSEC },                /* durabilityservice cleanup_delay  */
                KEEP_ALL_HISTORY_QOS, 1L,                               /*   history                        */
                LENGTH_UNLIMITED, LENGTH_UNLIMITED, LENGTH_UNLIMITED    /*   limits                         */
            },                                                          /*                                  */
            { DURATION_INFINITE_SEC, DURATION_INFINITE_NSEC },          /* deadline                         */
            { DURATION_ZERO_SEC, DURATION_ZERO_NSEC },                  /* latency budget                   */
            { AUTOMATIC_LIVELINESS_QOS,                                 /* liveliness                       */
                { DURATION_INFINITE_SEC, DURATION_INFINITE_NSEC }       /*   lease_duration                 */
            },                                                          /*                                  */
            { RELIABLE_RELIABILITY_QOS,                                 /* reliability (default=best_effort) */
                { DURATION_INFINITE_SEC, DURATION_INFINITE_NSEC },      /*   max_blocking_time              */
                false                                                   /*   synchronous                    */
            },                                                          /*                                  */
            { BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS },               /* destination_order (default=reception)   */
            { KEEP_ALL_HISTORY_QOS, 1L },                               /* history                          */
            { LENGTH_UNLIMITED, LENGTH_UNLIMITED, LENGTH_UNLIMITED },   /* resource_limits                  */
            { 0L },                                                     /* transport_priority               */
            { DURATION_INFINITE_SEC, DURATION_INFINITE_NSEC },          /* lifespan                         */
            { SHARED_OWNERSHIP_QOS }                                    /* ownership                        */

        };

        const DDS::DataWriterQos default_datawriter_qos = {
            { VOLATILE_DURABILITY_QOS },                                /* durability               */
            { DURATION_INFINITE_SEC, DURATION_INFINITE_NSEC },          /* deadline                 */
            { DURATION_ZERO_SEC, DURATION_ZERO_NSEC },                  /* latency_budget           */
            { AUTOMATIC_LIVELINESS_QOS,                                 /* liveliness               */
                { DURATION_INFINITE_SEC, DURATION_INFINITE_NSEC }       /*   lease_duration         */
            },                                                          /*                          */
            { RELIABLE_RELIABILITY_QOS,                                 /* reliability (default=best_effort) */
                { 0, 100000000 /* 100ms */ },                           /*   max_blocking_time      */
                false                                                   /*   synchronous            */
            },                                                          /*                          */
            { BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS },               /* destination_order        */
            { KEEP_ALL_HISTORY_QOS, 1L },                               /* history (default=keep_last 1)   */
            { 10L, LENGTH_UNLIMITED, LENGTH_UNLIMITED },                /* resource_limits          */
            { 0L },                                                     /* transport_priority       */
            { DURATION_INFINITE_SEC, DURATION_INFINITE_NSEC },          /* lifespan                 */
            { DDS::octSeq() },                                          /* userdata                 */
            { SHARED_OWNERSHIP_QOS },                                   /* ownership                */
            { 0L },                                                     /* ownership_strength       */
            { false,                                                    /* autodispose              */
                { DURATION_INFINITE_SEC, DURATION_INFINITE_NSEC },      /*   autopurge delay        */
                { DURATION_INFINITE_SEC, DURATION_INFINITE_NSEC }       /*   autounreg delay        */
            }
        };

        const DDS::DataReaderQos default_datareader_qos = {
            { VOLATILE_DURABILITY_QOS },                                /* durability                   */
            { DURATION_INFINITE_SEC, DURATION_INFINITE_NSEC },          /* deadline                     */
            { DURATION_ZERO_SEC, DURATION_ZERO_NSEC },                  /* latency_budget               */
            { AUTOMATIC_LIVELINESS_QOS,                                 /* liveliness                   */
                { DURATION_INFINITE_SEC, DURATION_INFINITE_NSEC }       /*   lease_duration             */
            },                                                          /*                              */
            { RELIABLE_RELIABILITY_QOS,                                 /* reliability                  */
                { DURATION_INFINITE_SEC, DURATION_INFINITE_NSEC },      /*   max_blocking_time          */
                false                                                   /*   synchronous                */
            },                                                          /*                              */
            { BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS },               /* destination_order            */
            { KEEP_ALL_HISTORY_QOS, 1L },                               /* history (default=keep_last 1)*/
            { 1000L, LENGTH_UNLIMITED, LENGTH_UNLIMITED },              /* resource_limits              */
            { DDS::octSeq() },                                          /* userdata                     */
            { SHARED_OWNERSHIP_QOS },                                   /* ownership                    */
            { DURATION_ZERO_SEC, DURATION_ZERO_NSEC },                  /* time_based_filter            */
            { { DURATION_INFINITE_SEC, DURATION_INFINITE_NSEC },        /* autopurge_nowriter delay     */
                { DURATION_INFINITE_SEC, DURATION_INFINITE_NSEC },      /*   autopurge_disposed delay   */
                false,                                                  /*   autopurge_dispose_all      */
                true,                                                   /*   enable_invalid_samples (deprecated) */
                { NO_INVALID_SAMPLES }                                  /*   invalid_sample_visibility (default=minimum)  */
            },                                                          /*                              */
            { false, DDS::StringSeq() },                                /* subscription_keys            */
            { false,                                                    /* use_lifespan                 */
                {DURATION_INFINITE_SEC, DURATION_INFINITE_NSEC}         /*   duration                   */
            },                                                          /*                              */
            { "", false }                                               /* share                        */
        };
    }
}
