// -- Std C/C++ Include
#include <iostream>
#include <gen/TempControl_DCPS.hpp>
#include <thread>         // std::thread, std::this_thread::sleep_for
#include <chrono>
#include "util.hpp"


int
main(int argc, char* argv[]) {
   // This code is not actually meant to run.  It must compile but only
   // serves as example code for the DDStutorial.pdf guide
   dds::domain::DomainParticipant dp(org::opensplice::domain::default_id());
   dds::topic::Topic<tutorial::TempSensorType> topic(dp, "TTempSensor");
   dds::sub::Subscriber sub(dp);
   dds::sub::DataReader<tutorial::TempSensorType> dr(sub, topic);

   /*segment1-start*/
   tutorial::TempSensorType key;
   key.id() = 123;
   auto handle = dr.lookup_instance(key);

   auto samples = dr.select().instance(handle).read();
   /*segment1-end*/

  return 0;
}
