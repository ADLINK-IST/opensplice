namespace test.sacs
{
    /// <date>Jun 14, 2005</date>
    public class MyPublisherListener : test.sacs.MyDataWriterListener, DDS.IPublisherListener
    {
        public bool onDataOnReadersCalled = false;

        public override void Reset()
        {
            base.Reset();
            onDataOnReadersCalled = false;
        }

        #region IPublisherListener Members

        public void OnDataOnReaders(DDS.IPublisher entityInterface)
        {
            onDataOnReadersCalled = true;
        }

        #endregion
    }
}
