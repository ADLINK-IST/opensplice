using System;
using System.Threading;
using System.Collections.Generic;

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
        Dictionary<DDS.StatusKind, Semaphore> semaphores = new Dictionary<DDS.StatusKind, Semaphore>();

        public MyDataReaderListener(Dictionary<DDS.StatusKind, Semaphore> s)
        {
            semaphores = s;
        }

        public MyDataReaderListener()
        {
        }

        public virtual void OnRequestedDeadlineMissed(DDS.IDataReader reader, DDS.RequestedDeadlineMissedStatus
             status)
        {
            onRequestedDeadlineMissedCalled = true;
            rdmStatus = status;
            Semaphore sem = null;
            if (semaphores.TryGetValue(DDS.StatusKind.RequestedDeadlineMissed, out sem) == true)
            {
                sem.Release();
            }
        }

        public virtual void OnRequestedIncompatibleQos(DDS.IDataReader reader, DDS.RequestedIncompatibleQosStatus
             status)
        {
            onRequestedIncompatibleQosCalled = true;
            riqStatus = status;
            Semaphore sem = null;
            if (semaphores.TryGetValue(DDS.StatusKind.RequestedIncompatibleQos, out sem) == true)
            {
                sem.Release();
            }

        }

        public virtual void OnSampleRejected(DDS.IDataReader reader, DDS.SampleRejectedStatus
             status)
        {
            onSampleRejectedCalled = true;
            srStatus = status;
            Semaphore sem = null;
            if (semaphores.TryGetValue(DDS.StatusKind.SampleRejected, out sem) == true)
            {
                sem.Release();
            }
        }

        public virtual void OnLivelinessChanged(DDS.IDataReader reader, DDS.LivelinessChangedStatus
             status)
        {

            onLivelinessChangedCalled = true;
            lcStatus = status;
            Semaphore sem = null;
            if (semaphores.TryGetValue(DDS.StatusKind.LivelinessChanged, out sem) == true)
            {
                sem.Release();
            }
        }

        public virtual void OnDataAvailable(DDS.IDataReader reader)
        {
            onDataAvailableCalled = true;
            Semaphore sem = null;
            if (semaphores.TryGetValue(DDS.StatusKind.DataAvailable, out sem) == true)
            {
                sem.Release();
            }
        }

        public virtual void OnSubscriptionMatched(DDS.IDataReader reader, DDS.SubscriptionMatchedStatus
             status)
        {
            onSubscriptionMatchCalled = true;
            smStatus = status;
            Semaphore sem = null;
            if (semaphores.TryGetValue(DDS.StatusKind.SubscriptionMatched, out sem) == true)
            {
                sem.Release();
            }
        }

        public virtual void OnSampleLost(DDS.IDataReader reader, DDS.SampleLostStatus status
            )
        {
            onSampleLostCalled = true;
            slStatus = status;
            Semaphore sem = null;
            if (semaphores.TryGetValue(DDS.StatusKind.SampleLost, out sem) == true)
            {
                sem.Release();
            }
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
