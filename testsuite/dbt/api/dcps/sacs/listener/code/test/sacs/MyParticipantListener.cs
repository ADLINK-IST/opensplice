namespace test.sacs
{
	/// <date>Jun 16, 2005</date>
	public class MyParticipantListener : test.sacs.MySubscriberListener, DDS.DomainParticipantListener
	{
		public bool onOfferedDeadlineMissedCalled = false;

		public DDS.OfferedDeadlineMissedStatus odmStatus = null;

		public bool onOfferedIncompatibleQosCalled = false;

		public DDS.OfferedIncompatibleQosStatus oiqStatus = null;

		public bool onLivelinessLostCalled = false;

		public DDS.LivelinessLostStatus llStatus = null;

		public bool onPublicationMatchStatus = false;

		public DDS.PublicationMatchedStatus pmStatus = null;

		public bool onInconsistentTopicCalled = false;

		public DDS.InconsistentTopicStatus ictStatus = null;

		public virtual void On_offered_deadline_missed(DDS.IDataWriter writer, DDS.OfferedDeadlineMissedStatus
			 status)
		{
			onOfferedDeadlineMissedCalled = true;
			odmStatus = status;
		}

		public virtual void On_offered_incompatible_qos(DDS.IDataWriter writer, DDS.OfferedIncompatibleQosStatus
			 status)
		{
			onOfferedIncompatibleQosCalled = true;
			oiqStatus = status;
		}

		public virtual void On_liveliness_lost(DDS.IDataWriter writer, DDS.LivelinessLostStatus
			 status)
		{
			onLivelinessLostCalled = true;
			llStatus = status;
		}

		public virtual void On_publication_matched(DDS.IDataWriter writer, DDS.PublicationMatchedStatus
			 status)
		{
			onPublicationMatchStatus = true;
			pmStatus = status;
		}

		public virtual void On_inconsistent_topic(DDS.ITopic the_topic, DDS.InconsistentTopicStatus
			 status)
		{
			onInconsistentTopicCalled = true;
			ictStatus = status;
		}

		public override void Reset()
		{
			base.Reset();
			onOfferedDeadlineMissedCalled = false;
			odmStatus = null;
			onOfferedIncompatibleQosCalled = false;
			oiqStatus = null;
			onLivelinessLostCalled = false;
			llStatus = null;
			onPublicationMatchStatus = false;
			pmStatus = null;
			onInconsistentTopicCalled = false;
			ictStatus = null;
		}
	}
}
