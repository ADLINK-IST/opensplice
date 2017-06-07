#ifndef _OMG_DDS_SUB_LAMBDA_DATA_READER_LISTENER
#define _OMG_DDS_SUB_LAMBDA_DATA_READER_LISTENER

#include <dds/sub/DataReaderListener.hpp>
#include <functional>

namespace dds {
  namespace sub {
    template <typename T>
    class LambdaDataReaderListener;
  }
}

template <typename T>
class dds::sub::LambdaDataReaderListener : public virtual dds::sub::DataReaderListener<T> {

public:
  std::function<void (DataReader<T>&, const dds::core::status::RequestedDeadlineMissedStatus&)> deadline_missed = [](DataReader<T>&, const dds::core::status::RequestedDeadlineMissedStatus&) { };
  std::function<void (DataReader<T>&, const dds::core::status::RequestedIncompatibleQosStatus&)> incompatible_qos = [](DataReader<T>&, const dds::core::status::RequestedIncompatibleQosStatus&) { };
  std::function<void (DataReader<T>&, const dds::core::status::SampleRejectedStatus&)> sample_rejected = [](DataReader<T>&, const dds::core::status::SampleRejectedStatus&) { };
  std::function<void (DataReader<T>&, const dds::core::status::LivelinessChangedStatus&)> liveliness_changes = [](DataReader<T>&, const dds::core::status::LivelinessChangedStatus&) { };

  std::function<void (DataReader<T>&)> data_available = [](DataReader<T>&) { };
  std::function<void (DataReader<T>&, const dds::core::status::SubscriptionMatchedStatus&)> subscription_matched = [](DataReader<T>&, const dds::core::status::SubscriptionMatchedStatus&) { };
  std::function<void (DataReader<T>&, const dds::core::status::SampleLostStatus&)> sample_lost = [](DataReader<T>&, const dds::core::status::SampleLostStatus&) { };

public:
  LambdaDataReaderListener() { }
public:
  virtual void 
  on_requested_deadline_missed(DataReader<T>& reader,
			       const dds::core::status::RequestedDeadlineMissedStatus& status) {
    deadline_missed(reader, status);
  }
  
  virtual void 
  on_requested_incompatible_qos(DataReader<T>& reader,
				const dds::core::status::RequestedIncompatibleQosStatus& status) {
    incompatible_qos(reader, status);
  }

  virtual void 
  on_sample_rejected(DataReader<T>& reader,
		     const dds::core::status::SampleRejectedStatus& status) {
    sample_rejected(reader, status);
  }

  virtual void 
  on_liveliness_changed(DataReader<T>& reader,
			const dds::core::status::LivelinessChangedStatus& status) {
    liveliness_changes(reader, status);
  }

  virtual void 
  on_data_available(DataReader<T>& reader) {
    data_available(reader);
  }

  virtual void 
  on_subscription_matched(DataReader<T>& reader,
			  const dds::core::status::SubscriptionMatchedStatus& status) {
    subscription_matched(reader, status);
  }

  virtual void on_sample_lost(DataReader<T>& reader,
			      const dds::core::status::SampleLostStatus& status) {
    
    sample_lost(reader, status);
  }
  
};
#endif /* _OMG_DDS_SUB_LAMBDA_DATA_READER_LISTENER */
