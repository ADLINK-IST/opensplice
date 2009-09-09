namespace test.sacs
{
	/// <date>Jun 14, 2005</date>
	public class MySubscriberListener : test.sacs.MyDataReaderListener, DDS.SubscriberListener
	{
		public bool onDataOnReadersCalled = false;

		public virtual void On_data_on_readers(DDS.ISubscriber subs)
		{
			onDataOnReadersCalled = true;
		}

		public override void Reset()
		{
			base.Reset();
			onDataOnReadersCalled = false;
		}
	}
}
