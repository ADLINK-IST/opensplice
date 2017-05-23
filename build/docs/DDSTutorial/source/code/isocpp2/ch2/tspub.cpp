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

  // create a Domain Participant, -1 defaults to value defined in configuration file
  dds::domain::DomainParticipant dp(-1);
  /*segment1-start*/
  dds::topic::Topic<tutorial::TempSensorType> topic(dp, "TTempSensor");
  dds::topic::Topic<tutorial::KeylessTempSensorType> kltsTopic(dp,
                                                               "KLTempSensorTopic");
  /*segment1-end*/
  dds::pub::Publisher pub(dp);

  /*segment2-start*/
  dds::pub::DataWriter<tutorial::KeylessTempSensorType> kldw(pub, kltsTopic);
  tutorial::KeylessTempSensorType klts(1, 26.0F, 70.0F, tutorial::CELSIUS);
  kldw.write(klts);
  kldw << tutorial::KeylessTempSensorType(2, 26.0F, 70.0F, tutorial::CELSIUS);
  /*segment2-end*/

  /*segment3-start*/
  dds::pub::DataWriter<tutorial::TempSensorType> dw(pub, topic);
  tutorial::TempSensorType ts(1, 26.0F, 70.0F, tutorial::CELSIUS);
  dw.write(ts);
  dw << tutorial::TempSensorType(2, 26.0F, 70.0F, tutorial::CELSIUS);
  /*segment3-end*/

  const float avgT = 25;
  const float avgH = 0.6;
  const float deltaT = 5;
  const float deltaH = 0.15;
  // Initialize random number generation with a seed
  srandom(clock());

  // Write some temperature randomly changing around a set point
  float temp = avgT + ((random()*deltaT)/RAND_MAX);
  float hum  = avgH + ((random()*deltaH)/RAND_MAX);

  tutorial::TempSensorType sensor( sid, temp, hum, tutorial::CELSIUS );

  for (unsigned int i = 0; i < N; ++i) {
    dw.write(sensor);
    std::cout << "DW << " << sensor << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    temp = avgT + ((random()*deltaT)/RAND_MAX);
    sensor.temp(temp);
    hum = avgH + ((random()*deltaH)/RAND_MAX);
    sensor.hum(hum);
  }
  return 0;
}
