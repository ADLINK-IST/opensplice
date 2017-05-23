/*segment1-start*/
#include <iostream>
#include <TempControl_DCPS.hpp>

int main(int, char**) {
  dds::domain::DomainParticipant dp(org::opensplice::domain::default_id());
  dds::topic::Topic<tutorial::TempSensorType> topic(dp, "TempSensorTopic");
  dds::pub::Publisher pub(dp);

  //[NOTE #1]: Avoid topic-instance dispose on unregister
  dds::pub::qos::DataWriterQos dwqos = pub.default_datawriter_qos()
  << dds::core::policy::WriterDataLifecycle::ManuallyDisposeUnregisteredInstances();

  //[NOTE #2]: Creating DataWriter with custom QoS.
  // QoS will be covered in detail in article #4.
  dds::pub::DataWriter<tutorial::TempSensorType> dw(pub, topic, dwqos);

  tutorial::TempSensorType data(0, 24.3F, 0.5F, tutorial::CELSIUS);
  dw.write(data);

  tutorial::TempSensorType key;
  short id = 1;
  key.id(id);

  //[NOTE #3] Registering topic-instance explicitly
  dds::core::InstanceHandle h1 = dw.register_instance(key);
  id = 2;
  key.id(id);
  dds::core::InstanceHandle h2 = dw.register_instance(key);
  id = 3;
  key.id(id);
  dds::core::InstanceHandle h3 = dw.register_instance(key);

  dw << tutorial::TempSensorType(1, 24.3F, 0.5F, tutorial::CELSIUS);
  dw << tutorial::TempSensorType(2, 23.5F, 0.6F, tutorial::CELSIUS);
  dw << tutorial::TempSensorType(3, 21.7F, 0.5F, tutorial::CELSIUS);

  // [NOTE #4]: unregister topic-instance with id=1
  dw.unregister_instance(h1);
  // [NOTE #5]: dispose topic-instance with id=2
  dw.dispose_instance(h2);
  //[NOTE #6]:topic-instance with id=3 will be unregistered as
  // result of the dw object destruction

  return 0;
}
/*segment1-end*/
