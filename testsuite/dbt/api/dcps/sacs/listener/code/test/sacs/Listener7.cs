namespace test.sacs
{
    /// <date>Jun 2, 2005</date>
    public class Listener7 : Test.Framework.TestCase
    {
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
                catch (System.Exception)
                {
                    result.Result = "Thread " + j + " could not be joined.";
                    return result;
                }
            }
            for (int j = 0; j < results.Length; j++)
            {
                if (results[j] != DDS.ReturnCode.Ok)
                {
                    result.Result = "Listener thread " + j + " failed.";
                    return result;
                }
            }
            result.Result = expResult;
            result.Verdict = Test.Framework.TestVerdict.Pass;
            return result;
        }

        private class ListenerThread
        {
            private int id;

            private DDS.IEntity entity;

            private DDS.ReturnCode rc = DDS.ReturnCode.Error;

            public ListenerThread(Listener7 _enclosing, DDS.IEntity entity, int id)
            {
                this._enclosing = _enclosing;
                this.id = id;
                this.entity = entity;
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
                else
                {
                    if (this.entity is DDS.IPublisher)
                    {
                        // TODO: JLS - Is something missing here, there isn't a IPublisher SetListener method
                        //DDS.IPublisherListener listener = new test.sacs.MyDataWriterListener();
                        //DDS.IPublisher domainEntity = (DDS.IPublisher)this.entity;
                        //this.rc = domainEntity.SetListener(listener, DDS.StatusKind.RequestedDeadlineMissed);
                        //if (this.rc == DDS.ReturnCode.Ok)
                        //{
                        //    this.rc = domainEntity.SetListener(null, 0);
                        //    if (this.rc == DDS.ReturnCode.Ok)
                        //    {
                        //        this.rc = domainEntity.SetListener(listener, DDS.StatusKind.RequestedIncompatibleQos);
                        //    }
                        //}
                    }
                    else
                    {
                        if (this.entity is DDS.IDataWriter)
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
                        else
                        {
                            if (this.entity is DDS.IDataReader)
                            {
                                DDS.DataReaderListener listener = new test.sacs.MyDataReaderListener();
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
                            else
                            {
                                if (this.entity is DDS.ITopic)
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
                                else
                                {
                                    if (this.entity is DDS.ISubscriber)
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
                                }
                            }
                        }
                    }
                }
                System.Console.Out.WriteLine("Thread " + this.id + " finished.");
            }

            public virtual DDS.ReturnCode GetResult()
            {
                return this.rc;
            }

            private readonly Listener7 _enclosing;

            internal void Start()
            {
                throw new System.NotImplementedException();
            }

            internal void Join()
            {
                throw new System.NotImplementedException();
            }
        }
    }
}
