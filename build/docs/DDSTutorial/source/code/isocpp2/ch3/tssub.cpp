// -- Std C/C++ Include
#include <iostream>
#include <gen/TempControl_DCPS.hpp>
#include <thread>         // std::thread, std::this_thread::sleep_for
#include <chrono> 
#include "util.hpp"


int
main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cout << "USAGE:\n\ttssub <filter-expression>"
              << std::endl;
    exit(-1);
  }
  dds::domain::DomainParticipant dp(-1);
  dds::topic::Topic<tutorial::TempSensorType> topic(dp, "TTempSensor");
  
  // Create the filter for the content-filtered-topic
  dds::topic::Filter filter(argv[1]);
  dds::topic::ContentFilteredTopic<tutorial::TempSensorType> cfTopic(topic, 
								     "CFTTempSensor", 
								     filter);

  dds::sub::Subscriber sub(dp);
  dds::sub::DataReader<tutorial::TempSensorType> dr(sub, cfTopic);

  while (true) {
    auto samples = dr.read();
    std::for_each(samples.begin(),
		  samples.end(),
		  [](const dds::sub::Sample<tutorial::TempSensorType>& s) {
		    std::cout << s.data() << std::endl;
		  });
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  return 0;
}
