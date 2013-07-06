// Std C++ Include
#include <algorithm>

// Utils
#include "util.hpp"

class ShapesListener : public dds::sub::NoOpDataReaderListener<ShapeType> {
public:
  virtual void
  on_data_available(DataReader<ShapeType>& dr)
  {
    std::cout << "----------on_data_available-----------" << std::endl;
    LoanedSamples<ShapeType> samples =
      dr.select()
      .state(DataState::any_data())
      .read();
    std::for_each(samples.begin(), samples.end(), demo::ishapes::printShapeSample);
  }
  virtual void
  on_liveliness_changed(DataReader<ShapeType>& dr,
            const LivelinessChangedStatus& status)
  {
    std::cout << "!!! Liveliness Changed !!!" << std::endl;
  }
};

int main(int argc, char* argv[]) {
  try {
    DomainParticipant dp(0);
    Topic<ShapeType> topic(dp, "Circle");
    Subscriber sub(dp);
    // Beware that the Listener ownership is transfered to the
    // DataReader
    DataReader<ShapeType> dr(sub, topic,
                 dds::sub::qos::DataReaderQos(),
                 new ShapesListener(),
                 dds::core::status::StatusMask::data_available());

    char ch;
    std::cout << "Hit a key to exit: " << std::endl;
    std::cin >> ch;
  } catch (const dds::core::Exception& e) {
    std::cout << e.what() << std::endl;
  }
  return 0;
}
