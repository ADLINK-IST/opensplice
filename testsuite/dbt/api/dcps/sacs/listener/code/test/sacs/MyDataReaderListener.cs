namespace test.sacs
{
    /// <date>Jun 8, 2005</date>
    public class MyDataReaderListener : DDS.IDataReaderListener
    {
        public bool onRequestedDeadlineMissedCalled = false;
        public DDS.RequestedDeadlineMissedStatus rdmStatus;

        public bool onRequestedIncompatibleQosCalled = false;
        public DDS.RequestedIncompatibleQosStatus riqStatus;

        public bool onSampleRejectedCalled = false;
        public DDS.SampleRejectedStatus srStatus;

        public bool onLivelinessChangedCalled = false;
        public DDS.LivelinessChangedStatus lcStatus;

        public bool onDataAvailableCalled = false;

        public bool onSubscriptionMatchCalled = false;
        public DDS.SubscriptionMatchedStatus smStatus;

        public bool onSampleLostCalled = false;
        public DDS.SampleLostStatus slStatus;

        public virtual void OnRequestedDeadlineMissed(DDS.IDataReader reader, DDS.RequestedDeadlineMissedStatus
             status)
        {
            onRequestedDeadlineMissedCalled = true;
            rdmStatus = status;
        }

        public virtual void OnRequestedIncompatibleQos(DDS.IDataReader reader, DDS.RequestedIncompatibleQosStatus
             status)
        {
            onRequestedIncompatibleQosCalled = true;
            riqStatus = status;
        }

        public virtual void OnSampleRejected(DDS.IDataReader reader, DDS.SampleRejectedStatus
             status)
        {
            onSampleRejectedCalled = true;
            srStatus = status;
        }

        public virtual void OnLivelinessChanged(DDS.IDataReader reader, DDS.LivelinessChangedStatus
             status)
        {
            
            onLivelinessChangedCalled = true;
            lcStatus = status;
        }

        public virtual void OnDataAvailable(DDS.IDataReader reader)
        {
            onDataAvailableCalled = true;
        }

        public virtual void OnSubscriptionMatched(DDS.IDataReader reader, DDS.SubscriptionMatchedStatus
             status)
        {
            onSubscriptionMatchCalled = true;
            smStatus = status;
        }

        public virtual void OnSampleLost(DDS.IDataReader reader, DDS.SampleLostStatus status
            )
        {
            onSampleLostCalled = true;
            slStatus = status;
        }

        public virtual void Reset()
        {
            onRequestedDeadlineMissedCalled = false;
            rdmStatus = new DDS.RequestedDeadlineMissedStatus();
            onRequestedIncompatibleQosCalled = false;
            riqStatus = new DDS.RequestedIncompatibleQosStatus();
            onSampleRejectedCalled = false;
            srStatus = new DDS.SampleRejectedStatus();
            onLivelinessChangedCalled = false;
            lcStatus = new DDS.LivelinessChangedStatus();
            onDataAvailableCalled = false;
            onSubscriptionMatchCalled = false;
            smStatus = new DDS.SubscriptionMatchedStatus();
            onSampleLostCalled = false;
            slStatus = new DDS.SampleLostStatus();
        }
    }
}
