// -- Std C/C++ Include
#include <iostream>
#include <gen/TempControl_DCPS.hpp>
#include <thread>         // std::thread, std::this_thread::sleep_for
#include <chrono> 
#include "util.hpp"


int main(int argc, char* argv[]) {

/*segment3-start*/
  // create a Domain Participant, -1 defaults to value defined in configuration file
  dds::domain::DomainParticipant dp(-1);
  // create the Topic
  dds::topic::Topic<tutorial::TempSensorType> topic(dp, "TTempSensor");
  // create a Subscriber
  dds::sub::Subscriber sub(dp);
  // create a DataReader
  dds::sub::DataReader<tutorial::TempSensorType> dr(sub, topic);

  while (true) {
    auto samples = dr.read();
    std::for_each(samples.begin(),
		  samples.end(),
		  [](const dds::sub::Sample<tutorial::TempSensorType>& s) {
		    std::cout << s.data() << std::endl;
		  });
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
/*segment3-end*/
  return 0;
}

