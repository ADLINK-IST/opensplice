// -- Std C/C++ Include
#include <iostream>
#include <gen/TempControl_DCPS.hpp>
#include <thread>         // std::thread, std::this_thread::sleep_for
#include <chrono>
#include "util.hpp"

/*segment1-start*/
class TempSensorListener :
  public dds::sub::NoOpDataReaderListener<tutorial::TempSensorType>
{
public:
  virtual void on_data_available(
      dds::sub::DataReader<tutorial::TempSensorType>& dr) {
    auto samples =  dr.read();
    std::for_each(samples.begin(), samples.end(),
		  [](const dds::sub::Sample<tutorial::TempSensorType>& s) {
		    std::cout << s.data().id() << std::endl;
		  });
  }
};
/*segment1-end*/

int
main(int argc, char* argv[]) {
  const int duration = 60; // sec
  dds::domain::DomainParticipant dp(org::opensplice::domain::default_id());
  dds::topic::Topic<tutorial::TempSensorType> topic(dp, "TTempSensor");
  dds::sub::Subscriber sub(dp);

  dds::sub::DataReader<tutorial::TempSensorType> dr(sub, topic);

  /*segment2-start*/
  TempSensorListener listener;
  dr.listener(&listener, dds::core::status::StatusMask::data_available());
  /*segment2-end*/

  std::this_thread::sleep_for(std::chrono::seconds(duration));
  dr.listener(nullptr, dds::core::status::StatusMask::none());

  return 0;
}
