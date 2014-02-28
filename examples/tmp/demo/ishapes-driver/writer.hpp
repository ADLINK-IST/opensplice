#ifndef ORG_OPENSPLICE_DEMO_ISHAPES_WRITER_HPP_
#define ORG_OPENSPLICE_DEMO_ISHAPES_WRITER_HPP_

#include "util.hpp"
#include <vector>
#include <algorithm>

#include "common/example_utilities.h"

namespace demo {
   namespace ishapes {
      template <typename T>
      class Writer;
   }
}
/**
 * This class shows a sample use of the (forward) iterator-based read API.
 * Beware that a back-inserting iterator API also exist.
 *
 */
template <typename T>
class demo::ishapes::Writer : public demo::ishapes::Runner<T> {
public:
   virtual ~Writer() { }

public:
   virtual void run(const dds::domain::DomainParticipant& dp,
         const dds::topic::Topic<T>& topic,
         const Params& params)
   {
      dds::pub::qos::PublisherQos pqos =
            dp.default_publisher_qos() << Partition("ishapes");

      dds::pub::Publisher pub(dp, pqos);

      dds::pub::qos::DataWriterQos dwqos =
            pub.default_datawriter_qos() << Durability::Transient() << Reliability::Reliable();

      dds::pub::DataWriter<T> dw(pub, topic, dwqos);

      const uint32_t period = params.period;
      const uint32_t samples = params.samples;
      uint32_t sleep_time = period * 1000;

      srand(clock());
      const uint32_t x0 = 10;
      const uint32_t y0 = 10;
      const uint32_t r = 200;
      const uint32_t dx = 5;
      const uint32_t dy = 7;

      // AnyDataWriter work just fine...
      AnyDataWriter adw = dw;
      DataWriter<ShapeType> xdw = adw.get<ShapeType>();
      std::cout << "Topic Name = " << xdw.topic().name()
                      << "\tType Name = " << xdw.topic().type_name() << std::endl;

      // ShapeType s = {params.color, x0, y0, params.shape_size};
      ShapeType s = {params.color.c_str(), x0 , y0, params.shape_size};

      std::cout << ">> Writing Data...";
               std::flush(std::cout);
      for (uint32_t i = 0; i < samples; ++i) {
         // Regular write
         dw.write(s);

         // Stream write
         dw << s;

         s.x = (s.x + dx) % r;
         s.y = (s.y + dy) % r;

         exampleSleepMilliseconds(sleep_time); // period is in ms
      }
   }
};


#endif /* ORG_OPENSPLICE_DEMO_ISHAPES_READER_HPP_ */
