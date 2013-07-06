#include "util.hpp"

int main(int argc, char* argv[]) {
  try {
    DomainParticipant dp(0);
    Topic<ShapeType> topic(dp, "Circle");
    Publisher pub(dp);
    DataWriter<ShapeType> dw(pub, topic);

    const int32_t N = 1000;
    uint32_t sleep_time = 300000;
    for (int32_t i = 0; i < N; ++i) {
      ShapeType bc = {"RED", i, i, 60};
      ShapeType rc = {"BLUE", N-i, N-i, 60};
      dw.write(bc);
      // You can also write with streaming operators!
      dw << rc;
      std::cout << "." << std::flush;
      exampleSleepMilliseconds(sleep_time);
    }

  } catch (const dds::core::Exception& e) {
    std::cout << e.what() << std::endl;
  }
  return 0;
}
