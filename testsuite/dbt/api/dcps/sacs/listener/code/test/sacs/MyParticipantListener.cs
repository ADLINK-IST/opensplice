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

    }
}
