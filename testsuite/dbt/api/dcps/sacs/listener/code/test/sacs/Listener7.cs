using System.Threading;

namespace test.sacs
{
    /// <date>Jun 2, 2005</date>
    public class Listener7 : Test.Framework.TestCase
    {
        DDS.ReturnCode rc = DDS.ReturnCode.Ok;
        
        public Listener7()
            : base("sacs_listener_tc7", "sacs_listener", "listener", "Listener multithread test."
                , "Listener multithread test.", null)
        {
            this.AddPreItem(new test.sacs.ListenerInit());
            this.AddPostItem(new test.sacs.ListenerDeinit());
        }

        public override Test.Framework.TestResult Run()
        {
            DDS.IDomainParticipant participant;
            DDS.ITopic topic;
            DDS.IPublisher publisher;
            DDS.ISubscriber subscriber;
            mod.tstDataWriter writer;
            mod.tstDataReader reader;
            Test.Framework.TestResult result;
            test.sacs.Listener7.ListenerThread[] threads;
            string expResult = "Listener multithread test succeeded.";
            DDS.ReturnCode[] results;

            result = new Test.Framework.TestResult(expResult, string.Empty, Test.Framework.TestVerdict.Pass,
                Test.Framework.TestVerdict.Fail);
            participant = (DDS.IDomainParticipant)this.ResolveObject("participant");
            writer = (mod.tstDataWriter)this.ResolveObject("datawriter");
            reader = (mod.tstDataReader)this.ResolveObject("datareader");
            topic = (DDS.ITopic)this.ResolveObject("topic");
            publisher = (DDS.IPublisher)this.ResolveObject("publisher");
            subscriber = (DDS.ISubscriber)this.ResolveObject("subscriber");
                        
            threads = new test.sacs.Listener7.ListenerThread[6];
            int i = 0;
            threads[i] = new test.sacs.Listener7.ListenerThread(this, participant, i);
            i++;
            threads[i] = new test.sacs.Listener7.ListenerThread(this, writer, i);
            i++;
            threads[i] = new test.sacs.Listener7.ListenerThread(this, reader, i);
            i++;
            threads[i] = new test.sacs.Listener7.ListenerThread(this, topic, i);
            i++;
            threads[i] = new test.sacs.Listener7.ListenerThread(this, subscriber, i);
            i++;
            threads[i] = new test.sacs.Listener7.ListenerThread(this, publisher, i);
            for (int j = 0; j < threads.Length; j++)
            {
                threads[j].Start();
            }
            results = new DDS.ReturnCode[threads.Length];
            for (int j = 0; j < threads.Length; j++)
            {
                try
                {
                    threads[j].Join();
                    results[j] = threads[j].GetResult();
                }
                catch (System.Exception e)
                {
                    result.Result = string.Format("Thread {0} could not be joined. Exception: {1}", j, e);
                    return result;
                }
            }
            for (int j = 0; j < results.Length; j++)
            {
                if (results[j] != DDS.ReturnCode.Ok)
                {
                    result.Result = string.Format("Listener thread {0} failed.", j);
                    return result;
                }
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }
        
        private static void runListeners(Listener7 listener7, DDS.IEntity entity, int id)
        {
            DDS.ReturnCode rc = DDS.ReturnCode.Error;

            System.Console.Out.WriteLine("Thread {0} started...", id);
            System.Threading.Thread.Sleep(0);
            if (entity is DDS.IDomainParticipant)
            {
                DDS.IDomainParticipantListener listener = new test.sacs.MyParticipantListener();
                DDS.IDomainParticipant domainEntity = (DDS.IDomainParticipant)entity;
                rc = (domainEntity).SetListener(listener, DDS.StatusKind.DataAvailable);
                if (rc == DDS.ReturnCode.Ok)
                {
                    rc = domainEntity.SetListener(null, 0);
                    if (rc == DDS.ReturnCode.Ok)
                    {
                        rc = domainEntity.SetListener(listener, DDS.StatusKind.OfferedDeadlineMissed);
                    }
                }
            }
            else
            {
                if (entity is DDS.IPublisher)
                {
                    DDS.IPublisherListener listener = new test.sacs.MyPublisherListener();
                    DDS.IPublisher domainEntity = (DDS.IPublisher)entity;
                    rc = domainEntity.SetListener(listener, DDS.StatusKind.RequestedDeadlineMissed);
                    if (rc == DDS.ReturnCode.Ok)
                    {
                        rc = domainEntity.SetListener(null, 0);
                        if (rc == DDS.ReturnCode.Ok)
                        {
                            rc = domainEntity.SetListener(listener, DDS.StatusKind.RequestedIncompatibleQos);
                        }
                    }
                }
                else
                {
                    if (entity is DDS.IDataWriter)
                    {
                        DDS.DataWriterListener listener = new test.sacs.MyDataWriterListener();
                        DDS.IDataWriter domainEntity = (DDS.IDataWriter)entity;
                        rc = (domainEntity).SetListener(listener, DDS.StatusKind.RequestedDeadlineMissed);
                        if (rc == DDS.ReturnCode.Ok)
                        {
                            rc = domainEntity.SetListener(null, 0);
                            if (rc == DDS.ReturnCode.Ok)
                            {
                                rc = domainEntity.SetListener(listener, DDS.StatusKind.RequestedIncompatibleQos);
                            }
                        }
                    }
                    else
                    {
                        if (entity is DDS.IDataReader)
                        {
                            DDS.IDataReaderListener listener = new test.sacs.MyDataReaderListener();
                            DDS.IDataReader domainEntity = (DDS.IDataReader)entity;
                            rc = (domainEntity).SetListener(listener, DDS.StatusKind.OfferedDeadlineMissed);
                            if (rc == DDS.ReturnCode.Ok)
                            {
                                rc = domainEntity.SetListener(null, 0);
                                if (rc == DDS.ReturnCode.Ok)
                                {
                                    rc = domainEntity.SetListener(listener, DDS.StatusKind.OfferedIncompatibleQos);
                                }
                            }
                        }
                        else
                        {
                            if (entity is DDS.ITopic)
                            {
                                DDS.TopicListener listener = new test.sacs.MyTopicListener();
                                DDS.ITopic domainEntity = (DDS.ITopic)entity;
                                rc = (domainEntity).SetListener(listener, DDS.StatusKind.InconsistentTopic);
                                if (rc == DDS.ReturnCode.Ok)
                                {
                                    rc = domainEntity.SetListener(null, 0);
                                    if (rc == DDS.ReturnCode.Ok)
                                    {
                                        rc = domainEntity.SetListener(listener, DDS.StatusKind.InconsistentTopic);
                                    }
                                }
                            }
                            else
                            {
                                if (entity is DDS.ISubscriber)
                                {
                                    DDS.ISubscriberListener listener = new test.sacs.MySubscriberListener();
                                    DDS.ISubscriber domainEntity = (DDS.ISubscriber)entity;
                                    rc = (domainEntity).SetListener(listener, DDS.StatusKind.OfferedDeadlineMissed);
                                    if (rc == DDS.ReturnCode.Ok)
                                    {
                                        rc = domainEntity.SetListener(null, 0);
                                        if (rc == DDS.ReturnCode.Ok)
                                        {
                                            rc = domainEntity.SetListener(listener, DDS.StatusKind.DataOnReaders);
                                        }
                                    }
                                }
                                else
                                {
                                    System.Console.Out.WriteLine("Entity type: " + entity.ToString() + " not supported.");
                                }
                            }
                        }
                    }
                }
            }
            System.Console.Out.WriteLine("Thread " + id + " finished.");
        }

        public virtual DDS.ReturnCode GetResult()
        {
            return this.rc;
        }
       
       private class ListenerThread 
        {
            private readonly Listener7 listener7;
            private int id;
            private DDS.IEntity entity;
            private DDS.ReturnCode rc = DDS.ReturnCode.Error;
            private Thread myThread;

            public ListenerThread(Listener7 listener7, DDS.IEntity entity, int id)
            {
                this.listener7 = listener7;
                this.id = id;
                this.entity = entity;
                this.myThread = new Thread(Run);
            }

            public void Run()
            {
                System.Console.Out.WriteLine("Thread " + this.id + " started...");
                System.Threading.Thread.Sleep(0);
                if (this.entity is DDS.IDomainParticipant)
                {
                    DDS.IDomainParticipantListener listener = new test.sacs.MyParticipantListener();
                    DDS.IDomainParticipant domainEntity = (DDS.IDomainParticipant)this.entity;
                    this.rc = (domainEntity).SetListener(listener, DDS.StatusKind.DataAvailable);
                    if (this.rc == DDS.ReturnCode.Ok)
                    {
                        this.rc = domainEntity.SetListener(null, 0);
                        if (this.rc == DDS.ReturnCode.Ok)
                        {
                            this.rc = domainEntity.SetListener(listener, DDS.StatusKind.OfferedDeadlineMissed);
                        }
                    }
                }
                else if (this.entity is DDS.IPublisher)
                {
                    DDS.IPublisherListener listener = new test.sacs.MyPublisherListener();
                    DDS.IPublisher domainEntity = (DDS.IPublisher)this.entity;
                    this.rc = domainEntity.SetListener(listener, DDS.StatusKind.RequestedDeadlineMissed);
                    if (this.rc == DDS.ReturnCode.Ok)
                    {
                        this.rc = domainEntity.SetListener(null, 0);
                        if (this.rc == DDS.ReturnCode.Ok)
                        {
                            this.rc = domainEntity.SetListener(listener, DDS.StatusKind.RequestedIncompatibleQos);
                        }
                    }
                }
                else if (this.entity is DDS.IDataWriter)
                {
                    DDS.DataWriterListener listener = new test.sacs.MyDataWriterListener();
                    DDS.IDataWriter domainEntity = (DDS.IDataWriter)this.entity;
                    this.rc = (domainEntity).SetListener(listener, DDS.StatusKind.RequestedDeadlineMissed);
                    if (this.rc == DDS.ReturnCode.Ok)
                    {
                        this.rc = domainEntity.SetListener(null, 0);
                        if (this.rc == DDS.ReturnCode.Ok)
                        {
                            this.rc = domainEntity.SetListener(listener, DDS.StatusKind.RequestedIncompatibleQos);
                        }
                    }
                }
                else if (this.entity is DDS.IDataReader)
                {
                    DDS.IDataReaderListener listener = new test.sacs.MyDataReaderListener();
                    DDS.IDataReader domainEntity = (DDS.IDataReader)this.entity;
                    this.rc = (domainEntity).SetListener(listener, DDS.StatusKind.OfferedDeadlineMissed);
                    if (this.rc == DDS.ReturnCode.Ok)
                    {
                        this.rc = domainEntity.SetListener(null, 0);
                        if (this.rc == DDS.ReturnCode.Ok)
                        {
                            this.rc = domainEntity.SetListener(listener, DDS.StatusKind.OfferedIncompatibleQos);
                        }
                    }
                }
                else if (this.entity is DDS.ITopic)
                {
                    DDS.TopicListener listener = new test.sacs.MyTopicListener();
                    DDS.ITopic domainEntity = (DDS.ITopic)this.entity;
                    this.rc = (domainEntity).SetListener(listener, DDS.StatusKind.InconsistentTopic);
                    if (this.rc == DDS.ReturnCode.Ok)
                    {
                        this.rc = domainEntity.SetListener(null, 0);
                        if (this.rc == DDS.ReturnCode.Ok)
                        {
                            this.rc = domainEntity.SetListener(listener, DDS.StatusKind.InconsistentTopic);
                        }
                    }
                }
                else if (this.entity is DDS.ISubscriber)
                {
                    DDS.ISubscriberListener listener = new test.sacs.MySubscriberListener();
                    DDS.ISubscriber domainEntity = (DDS.ISubscriber)this.entity;
                    this.rc = (domainEntity).SetListener(listener, DDS.StatusKind.OfferedDeadlineMissed);
                    if (this.rc == DDS.ReturnCode.Ok)
                    {
                        this.rc = domainEntity.SetListener(null, 0);
                        if (this.rc == DDS.ReturnCode.Ok)
                        {
                            this.rc = domainEntity.SetListener(listener, DDS.StatusKind.DataOnReaders);
                        }
                    }
                }
                else
                {
                    System.Console.Out.WriteLine("Entity type: " + this.entity.ToString() + " not supported.");
                }
                System.Console.Out.WriteLine("Thread " + this.id + " finished.");
            }

            public virtual DDS.ReturnCode GetResult()
            {
                return rc;
            }

            internal void Start()
            {
                myThread.Start();
            }

            internal void Join()
            {
               myThread.Join();
            }
        }
    }
}
