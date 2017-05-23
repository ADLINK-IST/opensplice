// -- Std C/C++ Include
#include <iostream>
#include <gen/TempControl_DCPS.hpp>
#include <thread>         // std::thread, std::this_thread::sleep_for
#include <chrono>
#include "util.hpp"


int
main(int argc, char* argv[]) {
  char* argvExpression = argv[0];
  if(argc == 2)
  {
      argvExpression = argv[1];
  }
  dds::topic::Filter argvFilter(argvExpression);

  // create a Domain Participant, -1 defaults to value defined in configuration file
  dds::domain::DomainParticipant dp(-1);
  /*segment5-start*/
  // Create the TTempSensor topic
  dds::topic::Topic<tutorial::TempSensorType> topic(dp, "TTempSensor");

  // Define the filter expression
  std::string expression =
    "(temp NOT BETWEEN %0 AND %1) \
    OR \
    (hum NOT BETWEEN %2 and %3)";

  // Define the filter parameters
  std::vector<std::string> params =
    {"20.5", "21.5", "30", "50"};

  // Create the filter for the content-filtered-topic
  dds::topic::Filter filter(expression, params);
  /*segment5-end*/

  if(argc == 2)
  {
      filter = argvFilter;
  }

  /*segment6-start*/
  // Create the ContentFilteredTopic
  dds::topic::ContentFilteredTopic<tutorial::TempSensorType> cfTopic(topic,
								     "CFTTempSensor",
								     filter);

  dds::sub::Subscriber sub(dp);
  //This data reader will only receive data that matches the content filter
  dds::sub::DataReader<tutorial::TempSensorType> dr(sub, cfTopic);
  /*segment6-end*/

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
