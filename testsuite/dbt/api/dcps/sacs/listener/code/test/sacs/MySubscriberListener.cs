namespace test.sacs
{
    /// <date>Jun 14, 2005</date>
    public class MySubscriberListener : test.sacs.MyDataReaderListener, DDS.ISubscriberListener
    {
        public bool onDataOnReadersCalled = false;

        public override void Reset()
        {
            base.Reset();
            onDataOnReadersCalled = false;
        }

        #region ISubscriberListener Members

        public void OnDataOnReaders(DDS.ISubscriber entityInterface)
        {
            onDataOnReadersCalled = true;
        }

        #endregion
    }
}
