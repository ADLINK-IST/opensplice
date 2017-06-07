/*segment1-start*/
#include <thread>
#include <chrono>
#include <TempControl_DCPS.hpp>

int main(int, char**) {
  dds::domain::DomainParticipant dp(org::opensplice::domain::default_id());
  dds::topic::Topic<tutorial::TempSensorType> topic(dp, "TempSensorTopic");
  dds::pub::Publisher pub(dp);

  dds::pub::DataWriter<tutorial::TempSensorType> dw(pub, topic);

  //[NOTE #1]: Instances implicitly registered as part
  // of the write.
  // {id, temp hum scale}
  dw << tutorial::TempSensorType(1, 25.0F, 65.0F, tutorial::CELSIUS);
  dw << tutorial::TempSensorType(2, 26.0F, 70.0F, tutorial::CELSIUS);
  dw << tutorial::TempSensorType(3, 27.0F, 75.0F, tutorial::CELSIUS);

  std::this_thread::sleep_for(std::chrono::seconds(10));

  //[NOTE #2]: Instances automatically unregistered and
  // disposed as result of the destruction of the dw object

  return 0;
}
/*segment1-end*/
