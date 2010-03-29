namespace test.sacs
{
    /// <date>Jun 14, 2005</date>
    public class MyDataWriterListener : DDS.DataWriterListener
    {
        public bool onOfferedDeadlineMissedCalled = false;
        public DDS.OfferedDeadlineMissedStatus odmStatus;

        public bool onOfferedIncompatibleQosCalled = false;
        public DDS.OfferedIncompatibleQosStatus oiqStatus;

        public bool onLivelinessLostCalled = false;
        public DDS.LivelinessLostStatus llStatus;

        public bool onPublicationMatchStatus = false;
        public DDS.PublicationMatchedStatus pmStatus;

        public override void OnOfferedDeadlineMissed(DDS.IDataWriter writer, DDS.OfferedDeadlineMissedStatus status)
        {
            onOfferedDeadlineMissedCalled = true;
            odmStatus = status;
        }

        public override void OnOfferedIncompatibleQos(DDS.IDataWriter writer, DDS.OfferedIncompatibleQosStatus status)
        {
            onOfferedIncompatibleQosCalled = true;
            oiqStatus = status;
        }

        public override void OnLivelinessLost(DDS.IDataWriter writer, DDS.LivelinessLostStatus status)
        {
            onLivelinessLostCalled = true;
            llStatus = status;
        }

        public override void OnPublicationMatched(DDS.IDataWriter writer, DDS.PublicationMatchedStatus status)
        {
            onPublicationMatchStatus = true;
            pmStatus = status;
        }

        public virtual void Reset()
        {
            onOfferedDeadlineMissedCalled = false;
            odmStatus = new DDS.OfferedDeadlineMissedStatus();
            onOfferedIncompatibleQosCalled = false;
            oiqStatus = new DDS.OfferedIncompatibleQosStatus();
            onLivelinessLostCalled = false;
            llStatus = new DDS.LivelinessLostStatus();
            onPublicationMatchStatus = false;
            pmStatus = new DDS.PublicationMatchedStatus();
        }
    }
}
