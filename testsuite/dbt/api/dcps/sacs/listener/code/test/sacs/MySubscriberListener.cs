using System.Threading;
using System.Collections.Generic;

namespace test.sacs
{
    /// <date>Jun 14, 2005</date>
    public class MySubscriberListener : test.sacs.MyDataReaderListener, DDS.ISubscriberListener
    {
        public bool onDataOnReadersCalled = false;
        Dictionary<DDS.StatusKind, Semaphore> semaphores = new Dictionary<DDS.StatusKind, Semaphore>();

        public MySubscriberListener(Dictionary<DDS.StatusKind, Semaphore> s)
        {
            semaphores = s;
        }

        public MySubscriberListener()
        {
        }

        public override void Reset()
        {
            base.Reset();
            onDataOnReadersCalled = false;
        }

        #region ISubscriberListener Members

        public void OnDataOnReaders(DDS.ISubscriber entityInterface)
        {
            onDataOnReadersCalled = true;
            Semaphore sem = null;
            if (semaphores.TryGetValue(DDS.StatusKind.DataOnReaders, out sem) == true)
            {
                sem.Release();
            }
        }

        #endregion
    }
}
