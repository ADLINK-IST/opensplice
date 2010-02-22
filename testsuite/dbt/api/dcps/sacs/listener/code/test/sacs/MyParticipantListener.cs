namespace test.sacs
{
    /// <date>Jun 16, 2005</date>
    public class MyParticipantListener : test.sacs.MySubscriberListener, DDS.IDomainParticipantListener
    {
        public bool onOfferedDeadlineMissedCalled = false;

        public DDS.OfferedDeadlineMissedStatus odmStatus;

        public bool onOfferedIncompatibleQosCalled = false;

        public DDS.OfferedIncompatibleQosStatus oiqStatus;

        public bool onLivelinessLostCalled = false;

        public DDS.LivelinessLostStatus llStatus;

        public bool onPublicationMatchStatus = false;

        public DDS.PublicationMatchedStatus pmStatus;

        public bool onInconsistentTopicCalled = false;

        public DDS.InconsistentTopicStatus ictStatus;

        new public bool onLivelinessChangedCalled = false;
        public DDS.LivelinessChangedStatus llcStatus;

        new public bool onDataAvailableCalled = false;

        new public bool onRequestedDeadlineMissedCalled = false;
        public DDS.RequestedDeadlineMissedStatus rdmStatus;
        new public bool onRequestedIncompatibleQosCalled = false;
        public DDS.RequestedIncompatibleQosStatus rcqStatus;

        public override void Reset()
        {
            base.Reset();
            onOfferedDeadlineMissedCalled = false;
            odmStatus = new DDS.OfferedDeadlineMissedStatus();
            onOfferedIncompatibleQosCalled = false;
            oiqStatus = new DDS.OfferedIncompatibleQosStatus();
            onLivelinessLostCalled = false;
            llStatus = new DDS.LivelinessLostStatus();
            onPublicationMatchStatus = false;
            pmStatus = new DDS.PublicationMatchedStatus();
            onInconsistentTopicCalled = false;
            ictStatus = new DDS.InconsistentTopicStatus();
            onDataAvailableCalled = false;
            onRequestedDeadlineMissedCalled = false;
            rdmStatus = new DDS.RequestedDeadlineMissedStatus();
            onRequestedIncompatibleQosCalled = false;
            rcqStatus = new DDS.RequestedIncompatibleQosStatus();
            onLivelinessChangedCalled = false;
            llcStatus = new DDS.LivelinessChangedStatus();
        }

        #region ITopicListener Members

        void DDS.ITopicListener.OnInconsistentTopic(DDS.ITopic entityInterface, DDS.InconsistentTopicStatus status)
        {
            onInconsistentTopicCalled = true;
            ictStatus = status;
        }

        #endregion

        #region IDataWriterListener Members

        void DDS.IDataWriterListener.OnLivelinessLost(DDS.IDataWriter entityInterface, DDS.LivelinessLostStatus status)
        {
            onLivelinessLostCalled = true;
            llStatus = status;
        }

        void DDS.IDataWriterListener.OnOfferedDeadlineMissed(DDS.IDataWriter entityInterface, DDS.OfferedDeadlineMissedStatus status)
        {
            onOfferedDeadlineMissedCalled = true;
            odmStatus = status;
        }

        void DDS.IDataWriterListener.OnOfferedIncompatibleQos(DDS.IDataWriter entityInterface, DDS.OfferedIncompatibleQosStatus status)
        {
            onOfferedIncompatibleQosCalled = true;
            oiqStatus = status;
        }

        void DDS.IDataWriterListener.OnPublicationMatched(DDS.IDataWriter entityInterface, DDS.PublicationMatchedStatus status)
        {
            onPublicationMatchStatus = true;
            pmStatus = status;
        }

        #endregion

        #region ISubscriberListener Members

        void DDS.ISubscriberListener.OnDataOnReaders(DDS.ISubscriber entityInterface)
        {
            throw new System.NotImplementedException();
        }

        #endregion

        #region IDataReaderListener Members

        void DDS.IDataReaderListener.OnDataAvailable(DDS.IDataReader entityInterface)
        {
            onDataAvailableCalled = true;
        }

        void DDS.IDataReaderListener.OnLivelinessChanged(DDS.IDataReader entityInterface, DDS.LivelinessChangedStatus status)
        {
            onLivelinessChangedCalled = true;
            llcStatus = status;
        }

        void DDS.IDataReaderListener.OnRequestedDeadlineMissed(DDS.IDataReader entityInterface, DDS.RequestedDeadlineMissedStatus status)
        {

            onRequestedDeadlineMissedCalled = true;
            rdmStatus = status;
        }

        void DDS.IDataReaderListener.OnRequestedIncompatibleQos(DDS.IDataReader entityInterface, DDS.RequestedIncompatibleQosStatus status)
        {
            onRequestedIncompatibleQosCalled = true;
            riqStatus = status;
        }

        void DDS.IDataReaderListener.OnSampleLost(DDS.IDataReader entityInterface, DDS.SampleLostStatus status)
        {
            throw new System.NotImplementedException();
        }

        void DDS.IDataReaderListener.OnSampleRejected(DDS.IDataReader entityInterface, DDS.SampleRejectedStatus status)
        {
            throw new System.NotImplementedException();
        }

        void DDS.IDataReaderListener.OnSubscriptionMatched(DDS.IDataReader entityInterface, DDS.SubscriptionMatchedStatus status)
        {
            throw new System.NotImplementedException();
        }

        #endregion
    }
}
