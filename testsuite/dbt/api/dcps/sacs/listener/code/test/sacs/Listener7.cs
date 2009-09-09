namespace test.sacs
{
	/// <date>Jun 2, 2005</date>
	public class Listener7 : Test.Framework.TestCase
	{
		public Listener7() : base("sacs_listener_tc7", "sacs_listener", "listener", "Listener multithread test."
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

			public override void Run()
			{
				System.Console.Out.WriteLine("Thread " + this.id + " started...");
                System.Threading.Thread.Sleep(0);
				if (this.entity is DDS.IDomainParticipant)
				{
					DDS.DomainParticipantListener listener = new test.sacs.MyParticipantListener();
					DDS.IDomainParticipant domainEntity = (DDS.IDomainParticipant)this.entity;
					DDS.Listener l = domainEntity.Get_listener();
					if (l == null)
					{
						this.rc = (domainEntity).Set_listener(listener, DDS.DATA_AVAILABLE_STATUS.Value);
						if (this.rc == DDS.ReturnCode.Ok)
						{
							this.rc = domainEntity.Set_listener(null, 0);
							if (this.rc == DDS.ReturnCode.Ok)
							{
								this.rc = domainEntity.Set_listener(listener, DDS.OFFERED_DEADLINE_MISSED_STATUS.
									Value);
							}
						}
					}
					else
					{
						this.rc = DDS.RETCODE_ERROR.Value;
					}
				}
				else
				{
					if (this.entity is DDS.IPublisher)
					{
						DDS.PublisherListener listener = new test.sacs.MyDataWriterListener();
						DDS.IPublisher domainEntity = (DDS.IPublisher)this.entity;
						DDS.Listener l = domainEntity.Get_listener();
						if (l == null)
						{
							this.rc = (domainEntity).Set_listener(listener, DDS.REQUESTED_DEADLINE_MISSED_STATUS
								.Value);
							if (this.rc == DDS.ReturnCode.Ok)
							{
								this.rc = domainEntity.Set_listener(null, 0);
								if (this.rc == DDS.ReturnCode.Ok)
								{
									this.rc = domainEntity.Set_listener(listener, DDS.REQUESTED_INCOMPATIBLE_QOS_STATUS
										.Value);
								}
							}
						}
						else
						{
							this.rc = DDS.RETCODE_ERROR.Value;
						}
					}
					else
					{
						if (this.entity is DDS.IDataWriter)
						{
							DDS.DataWriterListener listener = new test.sacs.MyDataWriterListener();
							DDS.IDataWriter domainEntity = (DDS.IDataWriter)this.entity;
							DDS.Listener l = domainEntity.Get_listener();
							if (l == null)
							{
								this.rc = (domainEntity).Set_listener(listener, DDS.REQUESTED_DEADLINE_MISSED_STATUS
									.Value);
								if (this.rc == DDS.ReturnCode.Ok)
								{
									this.rc = domainEntity.Set_listener(null, 0);
									if (this.rc == DDS.ReturnCode.Ok)
									{
										this.rc = domainEntity.Set_listener(listener, DDS.REQUESTED_INCOMPATIBLE_QOS_STATUS
											.Value);
									}
								}
							}
							else
							{
								this.rc = DDS.RETCODE_ERROR.Value;
							}
						}
						else
						{
							if (this.entity is DDS.IDataReader)
							{
								DDS.DataReaderListener listener = new test.sacs.MyDataReaderListener();
								DDS.IDataReader domainEntity = (DDS.IDataReader)this.entity;
								DDS.Listener l = domainEntity.Get_listener();
								if (l == null)
								{
									this.rc = (domainEntity).Set_listener(listener, DDS.OFFERED_DEADLINE_MISSED_STATUS
										.Value);
									if (this.rc == DDS.ReturnCode.Ok)
									{
										this.rc = domainEntity.Set_listener(null, 0);
										if (this.rc == DDS.ReturnCode.Ok)
										{
											this.rc = domainEntity.Set_listener(listener, DDS.OFFERED_INCOMPATIBLE_QOS_STATUS
												.Value);
										}
									}
								}
								else
								{
									this.rc = DDS.RETCODE_ERROR.Value;
								}
							}
							else
							{
								if (this.entity is DDS.ITopic)
								{
									DDS.TopicListener listener = new test.sacs.MyTopicListener();
									DDS.ITopic domainEntity = (DDS.ITopic)this.entity;
									DDS.Listener l = domainEntity.Get_listener();
									if (l == null)
									{
										this.rc = (domainEntity).Set_listener(listener, DDS.INCONSISTENT_TOPIC_STATUS.Value
											);
										if (this.rc == DDS.ReturnCode.Ok)
										{
											this.rc = domainEntity.Set_listener(null, 0);
											if (this.rc == DDS.ReturnCode.Ok)
											{
												this.rc = domainEntity.Set_listener(listener, DDS.INCONSISTENT_TOPIC_STATUS.Value
													);
											}
										}
									}
									else
									{
										this.rc = DDS.RETCODE_ERROR.Value;
									}
								}
								else
								{
									if (this.entity is DDS.ISubscriber)
									{
										DDS.SubscriberListener listener = new test.sacs.MySubscriberListener();
										DDS.ISubscriber domainEntity = (DDS.ISubscriber)this.entity;
										DDS.Listener l = domainEntity.Get_listener();
										if (l == null)
										{
											this.rc = (domainEntity).Set_listener(listener, DDS.OFFERED_DEADLINE_MISSED_STATUS
												.Value);
											if (this.rc == DDS.ReturnCode.Ok)
											{
												this.rc = domainEntity.Set_listener(null, 0);
												if (this.rc == DDS.ReturnCode.Ok)
												{
													this.rc = domainEntity.Set_listener(listener, DDS.DATA_ON_READERS_STATUS.Value);
												}
											}
										}
										else
										{
											this.rc = DDS.RETCODE_ERROR.Value;
										}
									}
									else
									{
										System.Console.Out.WriteLine("Entity type: " + Sharpen.Runtime.GetClassForObject(
											this.entity).GetSimpleName() + " not supported.");
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
