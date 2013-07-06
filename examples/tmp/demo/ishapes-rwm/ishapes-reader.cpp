// Std C++ Include
#include <algorithm>

// Utils
#include "util.hpp"

int main(int argc, char* argv[]) {
  try {
    DomainParticipant dp(0);
    Topic<ShapeType> topic(dp, "Circle");
    Subscriber sub(dp);
    DataReader<ShapeType> dr(sub, topic);

    uint32_t sleepTime = 100;

    while (true) {
      // If you can use C++11 then the "auto" keywork and "lambda"
      // function can make this code even nicer.

       /*
        * This assignement has move semantics. Also assignements
        * between LoanedSamples have move semantics a la C++11.
        * We should also add a member function to do:
        *
        * LoanedSamples<ShapeType> s1 = ...;
        * LoanedSamples<ShapeType> s2;
        *
        * s2 = move(s1); // LoanedSamples<T> move(LoanedSamples<T>& s)
        *                // This will call the copy ctor under the hood.
        * s2 = s1; // error!
        */
       ShapeType s = {"BLUE", 0, 0, 0};
       dds::core::InstanceHandle handle = dr.lookup_instance(s);

       /*
        * Make available older style API
        * (essentially make public as opposed to private)
        */
      LoanedSamples<ShapeType> samples =
            dr.select()
               .instance(handle)
               .state(DataState::any_data())
            .read();

      /* Ex1:
       *
       * SharedSample<ShapeType> ss = samples.share();
       * auto ss2 = ss; // This create "ref-counted alias" and does
       * not use the move semantics.
       */
      std::cout << "--------------------------------------------" << std::endl;
      std::for_each(samples.begin(), samples.end(), demo::ishapes::printShapeSample);
      exampleSleepMilliseconds(sleepTime);
    }
  } catch (const dds::core::Exception& e) {
    std::cout << e.what() << std::endl;
  }
  return 0;
}
