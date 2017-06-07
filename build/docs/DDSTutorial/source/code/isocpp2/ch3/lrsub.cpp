// -- Std C/C++ Include
#include <iostream>
#include <gen/TempControl_DCPS.hpp>
#include "util.hpp"
#include "LambdaDataReaderListener.hpp"

int
main(int argc, char* argv[]) {
   // This code is not actually meant to run.  It must compile but only
   // serves as example code for the DDStutorial.pdf guide
   dds::domain::DomainParticipant dp(org::opensplice::domain::default_id());
   dds::topic::Topic<tutorial::TempSensorType> topic(dp, "TTempSensor");

   dds::sub::Subscriber sub(dp);
   dds::sub::DataReader<tutorial::TempSensorType> dr(sub, topic);

   /*segment1-start*/
   dds::sub::LambdaDataReaderListener<tutorial::TempSensorType> listener;
   listener.data_available = [](
       dds::sub::DataReader<tutorial::TempSensorType>& dr) {
      auto samples = dr.read();
      std::for_each(samples.begin(), samples.end(),
                    [](const dds::sub::Sample<tutorial::TempSensorType>& s) {
                      std::cout << s.data().id() << std::endl;
                    });
   };

   dr.listener(&listener, dds::core::status::StatusMask::data_available());
   /*segment1-end*/

   return 0;
}
