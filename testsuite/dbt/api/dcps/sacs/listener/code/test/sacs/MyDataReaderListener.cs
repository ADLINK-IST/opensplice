namespace test.sacs
{
	/// <date>Jun 8, 2005</date>
	public class MyDataReaderListener : DDS.DataReaderListener
	{
		public bool onRequestedDeadlineMissedCalled = false;

		public DDS.RequestedDeadlineMissedStatus rdmStatus = null;

		public bool onRequestedIncompatibleQosCalled = false;

		public DDS.RequestedIncompatibleQosStatus riqStatus = null;

		public bool onSampleRejectedCalled = false;

		public DDS.SampleRejectedStatus srStatus = null;

		public bool onLivelinessChangedCalled = false;

		public DDS.LivelinessChangedStatus lcStatus = null;

		public bool onDataAvailableCalled = false;

		public bool onSubscriptionMatchCalled = false;

		public DDS.SubscriptionMatchedStatus smStatus = null;

		public bool onSampleLostCalled = false;

		public DDS.SampleLostStatus slStatus = null;

		public virtual void On_requested_deadline_missed(DDS.IDataReader reader, DDS.RequestedDeadlineMissedStatus
			 status)
		{
			onRequestedDeadlineMissedCalled = true;
			rdmStatus = status;
		}

		public virtual void On_requested_incompatible_qos(DDS.IDataReader reader, DDS.RequestedIncompatibleQosStatus
			 status)
		{
			onRequestedIncompatibleQosCalled = true;
			riqStatus = status;
		}

		public virtual void On_sample_rejected(DDS.IDataReader reader, DDS.SampleRejectedStatus
			 status)
		{
			onSampleRejectedCalled = true;
			srStatus = status;
		}

		public virtual void On_liveliness_changed(DDS.IDataReader reader, DDS.LivelinessChangedStatus
			 status)
		{
			onLivelinessChangedCalled = true;
			lcStatus = status;
		}

		public virtual void On_data_available(DDS.IDataReader reader)
		{
			onDataAvailableCalled = true;
		}

		public virtual void On_subscription_matched(DDS.IDataReader reader, DDS.SubscriptionMatchedStatus
			 status)
		{
			onSubscriptionMatchCalled = true;
			smStatus = status;
		}

		public virtual void On_sample_lost(DDS.IDataReader reader, DDS.SampleLostStatus status
			)
		{
			onSampleLostCalled = true;
			slStatus = status;
		}

		public virtual void Reset()
		{
			onRequestedDeadlineMissedCalled = false;
			rdmStatus = null;
			onRequestedIncompatibleQosCalled = false;
			riqStatus = null;
			onSampleRejectedCalled = false;
			srStatus = null;
			onLivelinessChangedCalled = false;
			lcStatus = null;
			onDataAvailableCalled = false;
			onSubscriptionMatchCalled = false;
			smStatus = null;
			onSampleLostCalled = false;
			slStatus = null;
		}
	}
}
