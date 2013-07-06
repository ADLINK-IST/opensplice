#ifndef ORG_OPENSPLICE_DEMO_ISHAPES_READER_HPP_
#define ORG_OPENSPLICE_DEMO_ISHAPES_READER_HPP_

#include "util.hpp"
#include <vector>
#include <algorithm>

#include <common/example_utilities.h>

namespace demo {
   namespace ishapes {
      template<typename T>
      class Reader;
   }
}
/**
 * This class shows a sample use of the (forward) iterator-based read API.
 * Beware that a back-inserting iterator API also exist.
 *
 */
template<typename T>
class demo::ishapes::Reader: public demo::ishapes::Runner<T> {
public:
   virtual ~Reader() {
   }

public:
   virtual void run(const dds::domain::DomainParticipant& dp,
         const dds::topic::Topic<T>& topic, const Params& params)
   {
      dds::sub::qos::SubscriberQos sqos =
            dp.default_subscriber_qos() << Partition("ishapes");

      Subscriber sub(dp, sqos);

      dds::sub::qos::DataReaderQos drqos =
            sub.default_datareader_qos()
            << History::KeepLast(params.history_depth)
            << Durability::Transient();

      dds::sub::DataReader<T> dr(sub, topic, drqos);

      const uint32_t period = params.period;
      const uint32_t samples = params.samples;
      const uint32_t max_samples = 16;
      const uint32_t sleep_time = period;

      std::vector< Sample<ShapeType> > data(max_samples);


      // AnyTopic work fine
      AnyTopic at = topic;
      dds::topic::Topic<ShapeType> xt = at.get<ShapeType>();

      Filter filter("x > 200 AND y > 100");
      ContentFilteredTopic<ShapeType> cft(topic, "CFCircle", filter);

      // QosProvider...
      QosProvider qos_provider(
            "http://www.opensplice.org/demo/config/qos.xml",
            "ishapes-profile");

      DataReader<ShapeType> cfdr(sub, cft);
      DataReader<ShapeType> dr2(sub, topic, qos_provider.datareader_qos());

      // Query
      Query q(dr, "x < 100 AND y < 100");


      // AnyDataReader
      AnyDataReader adr = q.data_reader();
      DataReader<ShapeType> xdr = adr.get<ShapeType>();

      // Conditions
      dds::sub::cond::ReadCondition rc(dr, params.data_state);
      dds::sub::cond::QueryCondition qc(q, params.data_state);

      Entity e = dr;
      StatusCondition sc(dr);
      sc.enabled_statuses(StatusMask::data_available());

      Subscriber xsub = dr.subscriber();

      /** @todo OSPL-1793 WaitSet is a reference type apparently. Thus the previous code
       * here did not compile as the argless constructor is private.
       * What is the correct usage supposed to look like? */
      WaitSet ws(dds::core::null);
      ws
         .attach_condition(sc)
         .attach_condition(rc)
         .attach_condition(qc);

      LoanedSamples<ShapeType> loaned_samples = dr.read();

      for (uint32_t i = 0; i < samples; ++i) {
         int32_t rs =
               dr.select()
                  .state(params.data_state)
                  .content(q)
                  .read(data.begin(), max_samples);

         std::cout << "==== Read  ==== \n";
         std::for_each(data.begin(), data.begin() + rs, printShapeSample);
         std::cout << std::endl;
         exampleSleepMilliseconds(sleep_time);

         LoanedSamples<ShapeType> loaned_samples =
               dr.select()
                        .state(params.data_state)
                        .content(q)
                        .read();

         std::cout << "==== Loaned Samples Read ==== \n";
         std::for_each(loaned_samples.begin(), loaned_samples.end(), printShapeSample);
         std::cout << std::endl;
         exampleSleepMilliseconds(sleep_time);
      }
   }
};

#endif /* ORG_OPENSPLICE_DEMO_ISHAPES_READER_HPP_ */
