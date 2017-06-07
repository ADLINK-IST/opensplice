// -- Std C/C++ Include
#include <iostream>
#include <gen/TempControl_DCPS.hpp>
#include <thread>         // std::thread, std::this_thread::sleep_for
#include <chrono>
#include "util.hpp"

#define MAXSAMPLES 1000

int
main(int argc, char* argv[]) {
   // This code is not actually meant to run.  It must compile but only
   // serves as example code for the DDStutorial.pdf guide
   dds::domain::DomainParticipant dp(org::opensplice::domain::default_id());
   dds::topic::Topic<tutorial::TempSensorType> topic(dp, "TTempSensor");

   dds::sub::Subscriber sub(dp);
   dds::sub::DataReader<tutorial::TempSensorType> dr(sub, topic);

   /*segment1-start*/
   // Forward iterator using array.
   dds::sub::Sample<tutorial::TempSensorType> samples[MAXSAMPLES];
   unsigned int readSamples = dr.read(&samples, MAXSAMPLES);

   // Forward iterator using vector.
   std::vector<dds::sub::Sample<tutorial::TempSensorType> > fSamples(MAXSAMPLES);
   readSamples = dr.read(fSamples.begin(), MAXSAMPLES);

   // Back-inserting iterator using vector.
   std::vector<dds::sub::Sample<tutorial::TempSensorType> > biSamples;
   uint32_t readBiSamples = dr.read(std::back_inserter(biSamples));
   /*segment1-end*/

   (void)readSamples;

   return 0;
}

