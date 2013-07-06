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

#ifndef ORG_OPENSPLICE_CORE_QOSPROVIDER_HPP_
#define ORG_OPENSPLICE_CORE_QOSPROVIDER_HPP_

namespace org {
   namespace opensplice {
      namespace core {
         class QosProvider;
      }
   }
}

class org::opensplice::core::QosProvider {
public:
   explicit QosProvider(const std::string& uri) { }

   explicit QosProvider(const std::string& uri, const std::string& id) { }

   dds::domain::qos::DomainParticipantQos
   participant_qos() {
      return dds::domain::qos::DomainParticipantQos();
   }

   dds::domain::qos::DomainParticipantQos
   participant_qos(const std::string& id) {
      return dds::domain::qos::DomainParticipantQos();
   }


   dds::topic::qos::TopicQos
   topic_qos() {
      return dds::topic::qos::TopicQos();
   }

   dds::topic::qos::TopicQos
   topic_qos(const std::string& id) {
      return dds::topic::qos::TopicQos();
   }


   dds::sub::qos::SubscriberQos
   subscriber_qos() {
      return dds::sub::qos::SubscriberQos();
   }

   dds::sub::qos::SubscriberQos
   subscriber_qos( const std::string& id) {
      return dds::sub::qos::SubscriberQos();
   }

   dds::sub::qos::DataReaderQos
   datareader_qos() {
      return dds::sub::qos::DataReaderQos();
   }

   dds::sub::qos::DataReaderQos
   datareader_qos( const std::string& id) {
      return dds::sub::qos::DataReaderQos();
   }

   dds::pub::qos::PublisherQos
   publisher_qos() {
      return dds::pub::qos::PublisherQos();
   }

   dds::pub::qos::PublisherQos
   publisher_qos( const std::string& id) {
      return dds::pub::qos::PublisherQos();
   }

   dds::pub::qos::DataWriterQos
   datawriter_qos() {
      return dds::pub::qos::DataWriterQos();
   }

   dds::pub::qos::DataWriterQos
   datawriter_qos( const std::string& id) {
      return dds::pub::qos::DataWriterQos();
   }
};

#endif /* ORG_OPENSPLICE_CORE_QOSPROVIDER_HPP_ */
