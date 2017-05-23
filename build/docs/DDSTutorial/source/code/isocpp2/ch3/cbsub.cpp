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
   // Define the query expression
   std::string expression =
      "(temp NOT BETWEEN (%0 AND %1)) \
        OR \
       (hum NOT BETWEEN (%2 and %3))";

   // Define the query parameters
   std::vector<std::string> params = {"20.5", "21.5", "30", "50"};

   dds::sub::Query query(dr, expression, params);

   auto samples = dr.select().content(query).read();
   /*segment1-end*/

  return 0;
}
