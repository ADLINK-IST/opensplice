#ifndef ORG_OPENSPLICE_DEMO_ISHAPES_DRIVER_UTIL_HPP_
#define ORG_OPENSPLICE_DEMO_ISHAPES_DRIVER_UTIL_HPP_

#include <iostream>
// DDS Include
#include <dds/dds.hpp>

// Generated Files
//#include <org/opensplice/core/config.hpp>
#include "ccpp_ishapes.h"

REGISTER_TOPIC_TRAITS(org::opensplice::demo::ShapeType)

using namespace org::opensplice::demo;

using namespace dds::core;
using namespace dds::core::policy;
using namespace dds::core::cond;
using namespace dds::core::status;

using namespace dds::domain;

using namespace dds::topic;

using namespace dds::pub;
using namespace dds::pub::qos;

using namespace dds::sub;
using namespace dds::sub::status;

std::ostream&
operator <<(std::ostream& os, const org::opensplice::demo::ShapeType& s);

std::ostream&
operator <<(std::ostream& os, const dds::sub::SampleInfo& si);

namespace demo {
   namespace ishapes {
      struct Params;

      void printShape(const org::opensplice::demo::ShapeType& s);
      void printShapeSample(const dds::sub::Sample<org::opensplice::demo::ShapeType>& s);

      template<typename T>
      class Runner {
      public:
         virtual ~Runner() {
         }
         virtual void run(const dds::domain::DomainParticipant& dp,
               const dds::topic::Topic<T>& topic,
               const Params& params) = 0;
      };

      template<typename T>
      class IdleRunner: public Runner<T> {
      public:
         virtual ~IdleRunner() {
         }
      public:
         virtual void run(const dds::domain::DomainParticipant& dp,
               const dds::topic::Topic<T>& topic,
               const Params& params) {

         }
      };

      typedef ::dds::core::smart_ptr_traits<demo::ishapes::Runner<ShapeType> >::ref_type Runner_t;

      struct Params {
          Params() :
              shape("Circle"),
              color("RED"),
              period(1000),
              samples(10),
              shape_size(80),
              history_depth(1),
              data_state(dds::sub::status::DataState::new_data())
          { }
          std::string shape;
          std::string color;
          uint32_t period;
          uint32_t samples;
          uint32_t shape_size;
          uint32_t history_depth;
          dds::sub::status::DataState data_state;
       };

       struct Config {
          Params params;
          Runner_t runner;
       };

   }
} /** demo :: ishapes */


#endif /* ORG_OPENSPLICE_DEMO_ISHAPES_DRIVER_UTIL_HPP_ */
