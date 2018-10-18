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

#ifndef ORG_OPENSPLICE_FORWARD_DECLARATIONS_HPP_
#define ORG_OPENSPLICE_FORWARD_DECLARATIONS_HPP_

namespace dds {
    namespace domain {
        class DomainParticipantListener;

        template <typename DELEGATE>
        class TDomainParticipant;
    }

    namespace sub {
        template <typename T, template <typename Q> class DELEGATE>
        class DataReader;

        template <typename T>
        class DataReaderListener;

        class SubscriberListener;

        template <typename DELEGATE>
        class TSubscriber;

        namespace detail {
            template <typename T>
                class DataReader;

            template <typename T>
                class LoanedSamplesHolder;

            template <typename T, typename SamplesFWIterator>
                class SamplesFWInteratorHolder;

            template <typename T, typename SamplesBIIterator>
                class SamplesBIIteratorHolder;
        }
    }

    namespace pub {
        class PublisherListener;

        template <typename DELEGATE>
        class TPublisher;
    }

    namespace topic {
        template <typename DELEGATE>
        class TTopicDescription;

        template <typename T>
        class TopicListener;

        template <typename T, template <typename Q> class DELEGATE>
        class Topic;

        template <typename T, template <typename Q> class DELEGATE>
        class ContentFilteredTopic;

        namespace detail {
            template <typename T>
            class ContentFilteredTopic;
        }
    }
}


namespace org {
    namespace opensplice {
        namespace core
        {
            class OMG_DDS_API ListenerDispatcher;
        }

        namespace domain {
            class DomainParticipantDelegate;
        }

        namespace sub {
            class SubscriberDelegate;
        }

        namespace pub {
            class PublisherDelegate;
        }

        namespace topic {
            template <class TOPIC>
            class TopicTraits;

            class TopicDescriptionDelegate;
        }
    }
}

#endif /* ORG_OPENSPLICE_FORWARD_DECLARATIONS_HPP_ */
