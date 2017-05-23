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
  // Create the WaitSet
  dds::core::cond::WaitSet ws;
  // Create a ReadCondition for our DataReader and configure it for new data
  dds::sub::cond::ReadCondition rc(dr,
          dds::sub::status::DataState::new_data(),
          [](const dds::sub::ReadCondition& srcCond) {
              dds::sub::DataReader<tutorial::TempSensorType> srcReader = srcCond.data_reader();
              // Read the data
              auto samples = srcReader.read();
              std::for_each(samples.begin(),
                  samples.end(),
                  [](const dds::sub::Sample<tutorial::TempSensorType>& s) {
                    std::cout << s.data() << std::endl;
                  });
          });
  // Attach the condition
  ws += rc;

  // Wait for new data to be available
  ws.dispatch();
  /*segment1-end*/

  return 0;
}
