// -- Std C/C++ Include
#include <iostream>
#include <gen/TempControl_DCPS.hpp>
#include <thread>         // std::thread, std::this_thread::sleep_for
#include <chrono>
#include "util.hpp"


int
main(int argc, char* argv[])
{
  if (argc < 2) {
    std::cout << "USAGE:\n\t tspub <sensor-id>" << std::endl;
    return -1;
  }
  int sid = atoi(argv[1]);
  const int N = 100;

  /*segment1-start*/
  // create a Domain Participant, -1 defaults to value defined in configuration file
  dds::domain::DomainParticipant dp(-1);
  /*segment1-end*/

#if 0
  //Example code - needs to compile but not run
  /*segment1a-start*/
  // Creates a domain participant in the domain identified by
  // the number 18
  dds::domain::DomainParticipant dp2(18);
  /*segment1a-end*/
#endif

  /*segment2-start*/
  // Create the topic
  dds::topic::Topic<tutorial::TempSensorType> topic(dp, "TTempSensor");
  /*segment2-end*/

  /*segment3-start*/
  // Create the Publisher and DataWriter
  dds::pub::Publisher pub(dp);
  dds::pub::DataWriter<tutorial::TempSensorType> dw(pub, topic);
  /*segment3-end*/

  /*segment4-start*/
  // Write the data
  tutorial::TempSensorType sensor(1, 26.0F, 70.0F, tutorial::CELSIUS);
  dw.write(sensor);
  /*segment4-end*/

  /*segment5-start*/
  // Write data using streaming operators (same as calling dw.write(...))
  dw << tutorial::TempSensorType(2, 26.5F, 74.0F, tutorial::CELSIUS);
  /*segment5-end*/

  const float avgT = 25;
  const float avgH = 0.6;
  const float deltaT = 5;
  const float deltaH = 0.15;
  // Initialize random number generation with a seed
  srandom(clock());

  // Write some temperature randomly changing around a set point
  float temp = avgT + ((random()*deltaT)/RAND_MAX);
  float hum  = avgH + ((random()*deltaH)/RAND_MAX);

  tutorial::TempSensorType sensor2( sid, temp, hum, tutorial::CELSIUS );

  // Write the data
  for (unsigned int i = 0; i < N; ++i) {
    dw.write(sensor2);
    std::cout << "DW << " << sensor2 << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    temp = avgT + ((random()*deltaT)/RAND_MAX);
    sensor2.temp(temp);
    hum = avgH + ((random()*deltaH)/RAND_MAX);
    sensor2.hum(hum);
  }

  return 0;
}
