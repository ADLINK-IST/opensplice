// -- Std C/C++ Include
#include <iostream>
#include <gen/TempControl_DCPS.hpp>
#include <thread>         // std::thread, std::this_thread::sleep_for
#include <chrono>
#include "util.hpp"


int
main(int argc, char* argv[]) {
   dds::domain::DomainParticipant dp(org::opensplice::domain::default_id());
   dds::topic::Topic<tutorial::TempSensorType> topic(dp, "TTempSensor");
   dds::sub::Subscriber sub(dp);
   dds::sub::DataReader<tutorial::TempSensorType> dr(sub, topic);

   /*segment1-start*/
   dds::sub::LoanedSamples<tutorial::TempSensorType> samples;
   samples = dr.select().state(dds::sub::status::DataState::any()).read();
   /*segment1-end*/

   /*segment2-start*/
   samples = dr.select().state(dds::sub::status::SampleState::not_read()).read();
   /*segment2-end*/

   /*segment3-start*/
   samples = dr.select().state(dds::sub::status::DataState::new_data()).read();
   /*segment3-end*/

   /*segment4-start*/
   dds::sub::status::DataState ds;
   ds << dds::sub::status::SampleState::not_read()
      << dds::sub::status::ViewState::new_view()
      << dds::sub::status::InstanceState::alive();

   samples = dr.select().state(ds).read();
   /*segment4-end*/

   /*segment5-start*/
   auto samples2 = dr.read();
   /*segment5-end*/

  return 0;
}
