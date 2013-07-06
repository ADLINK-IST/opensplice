using System;
using System.Threading;

using DDS;
using DDS.OpenSplice;
using DDS.OpenSplice.CustomMarshalers;


namespace DDSAPIHelper
{
    /// <summary>
    /// The DDSEntityManager class is provided as a utility class for
    /// the various OpenSplice DDS operations used in the examples demonstrating
    /// various DDS concepts.
    /// </summary>
    public sealed class DDSEntityManager
    {
        private DomainParticipantFactory dpf;
        private DomainParticipantQos dpQos;
        private IDomainParticipant participant;
        private ITopic topic;
        private IContentFilteredTopic filteredTopic;
        private TopicQos topicQos = new TopicQos();
        private PublisherQos pubQos = new PublisherQos();
        private SubscriberQos subQos = new SubscriberQos();

        private DataWriterQos WQosH = new DataWriterQos();
        private DataReaderQos RQosH = new DataReaderQos();

        private IPublisher publisher;
        private IDataWriter writer;
        private IDataWriter writerStopper;

        private ISubscriber subscriber;
        private IDataReader reader;

        private String typeName;
        private String partitionName;
        private String durabilityKind = "transient";
        private Boolean autoDisposeFlag = false;
        private ReturnCode status = ReturnCode.Error;
        private String exampleName;

        /// <summary>
        /// The constructor to use for creating the various entities.
        /// </summary>
        /// <param name="name">The example name to create an entity manager. This is used to manage different example needs.</param>
        public DDSEntityManager(String name)
        {
            exampleName = name;
        }

        /// <summary>
        /// Create Domain Participant
        /// </summary>
        /// <param name="partitionName">Create the participant and assign the partition name.</param>
        public void createParticipant(String partitionName)
        {
            dpf = DomainParticipantFactory.Instance;
            dpQos = new DomainParticipantQos();
            dpf.GetDefaultParticipantQos(ref dpQos);
            
            ErrorHandler.checkHandle(dpf, "DomainParticipantFactory.Instance");

            participant = dpf.CreateParticipant(DDS.DomainId.Default,dpQos);
            ErrorHandler.checkHandle(participant, "DomainParticipantFactory.CreateParticipant");
            this.partitionName = partitionName;
        }

        /// <summary>
        /// Set the Durability Kind transient | persistent
        /// </summary>
        /// <param name="kind">transient|persistent</param>
        public void setDurabilityKind(String kind)
        {
            durabilityKind = kind;
        }

        /// <summary>
        /// Set the flag for auto disposing of unregistered instances
        /// </summary>
        /// <param name="autoDispose">true | false</param>
        public void setAutoDispose(Boolean autoDisposeFlag)
        {
            this.autoDisposeFlag = autoDisposeFlag;
        }

        /// <summary>
        /// Delete the DomainParticipant.
        /// </summary>
        public void deleteParticipant()
        {
            status = dpf.DeleteParticipant(participant);
            ErrorHandler.checkStatus(status, "DomainParticipantFactory.DeleteParticipant");
        }
        /// <summary>
        /// Register the type we are interested with the DDS Infrastructure
        /// </summary>
        /// <param name="ts">The TypeSupport class</param>
        public void registerType(ITypeSupport ts)
        {
            typeName = ts.TypeName;
            status = ts.RegisterType(participant, typeName);
            ErrorHandler.checkStatus(status, "ITypeSupport.RegisterType");
        }

        /// <summary>
        /// Utility method to create a DDS Topic.
        /// </summary>
        /// <param name="topicName">The Topic name to create the topic with.</param>
        /// <param name="exampleNameToCreateTopicFor">The name of the example we are
        /// creating the topic for. This is used to define any specific Qos that the example
        /// requires.</param>
        /// <returns>The newly created Topic</returns>
        public ITopic createTopic(String topicName)
        {
            status = participant.GetDefaultTopicQos(ref topicQos);
            ErrorHandler.checkStatus(status, "DomainParticipant.GetDefaultTopicQos");

            switch (exampleName)
            {
                case "ContentFilteredTopic":
                case "Durability":
                case "HelloWorld":
                case "WaitSet":
                case "Lifecycle":
                    topicQos.Reliability.Kind = ReliabilityQosPolicyKind.ReliableReliabilityQos;
                    if (durabilityKind.Equals("transient"))
                        topicQos.Durability.Kind = DurabilityQosPolicyKind.TransientDurabilityQos;
                    else
                        topicQos.Durability.Kind = DurabilityQosPolicyKind.PersistentDurabilityQos;
                    break;                
                case "Ownership":
                    topicQos.Reliability.Kind = ReliabilityQosPolicyKind.ReliableReliabilityQos;
                    topicQos.Ownership.Kind = OwnershipQosPolicyKind.ExclusiveOwnershipQos;
                    break;                
                case "QueryCondition":
                    topicQos.Reliability.Kind = ReliabilityQosPolicyKind.ReliableReliabilityQos;
                    break;
                case "Listener":
                    topicQos.Reliability.Kind = ReliabilityQosPolicyKind.ReliableReliabilityQos;
                    topicQos.Durability.Kind = DurabilityQosPolicyKind.TransientDurabilityQos;
                    // DeadlineQoSPolicy : period used to trigger the listener
                    // (on_requested_deadline_missed) 
                    topicQos.Deadline.Period.NanoSec = 0;
                    topicQos.Deadline.Period.Sec = 1;
                    break;
                default:
                    Console.WriteLine("Unidentified example to create topic for.");
                    break;
            }
            
            status = participant.SetDefaultTopicQos(topicQos);
            ErrorHandler.checkStatus(status, "DomainParticipant.SetDefaultTopicQos");
            topic = participant.CreateTopic(
                    topicName,
                    typeName,
                    topicQos,
                    null,
                    StatusKind.Any
                    );
            ErrorHandler.checkHandle(topic, "DomainParticipant.CreateTopic");
            return topic;
        }
        
        /// <summary>
        /// Create a ContentFilteredTopic
        /// </summary>
        /// <param name="topicFName">The name of the topic.</param>
        /// <param name="topic">The topic to which the filter is applied.</param>
        /// <param name="arg">The handle to a sequence of strings with the parameter 
        /// value used in the SQL expression.</param>
        public void createContentFilter(String topicName, ITopic topic, String arg)
        {
            String[] tab = new String[1];
            tab[0] = arg;
            String filter = "ticker = %0";
            
            filteredTopic =
                participant.CreateContentFilteredTopic(topicName, topic, filter, tab);
            ErrorHandler.checkHandle(filteredTopic, "DomainParticipant.CreateContentFilteredTopic");
        }
        
        /// <summary>
        /// Delete a ContentFilteredTopic
        /// </summary>
        public void deleteContentFilteredTopic()
        {
            if (filteredTopic == null)
            {
                return;
            }
            else
            {
                status = participant.DeleteContentFilteredTopic(filteredTopic);
                ErrorHandler.checkStatus(status, "DDS.DomainParticipant.DeleteContentFilteredTopic");
            }
        }

        /// <summary>
        /// Delete the Topic
        /// </summary>
        public void deleteTopic()
        {
            status = participant.DeleteTopic(topic);
            ErrorHandler.checkStatus(
                    status, "DDS.DomainParticipant.DeleteTopic");
        }

        /// <summary>
        /// Create a Publisher
        /// </summary>
        public void createPublisher()
        {
            status = participant.GetDefaultPublisherQos(ref pubQos);
            ErrorHandler.checkStatus(status, "DomainParticipant.GetDefaultPublisherQos");

            pubQos.Partition.Name = new String[1];
            pubQos.Partition.Name[0] = partitionName;
            publisher = participant.CreatePublisher(
                    pubQos, null, StatusKind.Any);
            ErrorHandler.checkHandle(publisher, "DomainParticipant.CreatePublisher");
        }

        /// <summary>
        /// Delete a Publisher
        /// </summary>
        public void deletePublisher()
        {
            status = participant.DeletePublisher(publisher);
            ErrorHandler.checkStatus(status, "DomainParticipant.DeletePublisher");
        }

        /// <summary>
        /// Create DataWriter(s)
        /// </summary>
        public void createWriter()
        {
            status = publisher.GetDefaultDataWriterQos(ref WQosH);
            ErrorHandler.checkStatus(status, "Publisher.GetDefaultDataWriterQos");
            status = publisher.CopyFromTopicQos(ref WQosH, topicQos);
            ErrorHandler.checkStatus(status, "Publisher.CopyFromTopicQos");
            if (durabilityKind.Equals("transient"))
                WQosH.Durability.Kind = DurabilityQosPolicyKind.TransientDurabilityQos;
            else
                WQosH.Durability.Kind = DurabilityQosPolicyKind.PersistentDurabilityQos;

            WQosH.WriterDataLifecycle.AutodisposeUnregisteredInstances = autoDisposeFlag;
            writer = publisher.CreateDataWriter(
                    topic,
                    WQosH,
                    null,
                    StatusKind.Any);
            ErrorHandler.checkHandle(writer, "Publisher.CreateDataWriter");

            if (exampleName.Equals("Lifecycle"))
            {
                writerStopper = publisher.CreateDataWriter(
                    topic,
                    WQosH,
                    null,
                    StatusKind.Any);
                ErrorHandler.checkHandle(writerStopper, "Publisher.CreateDataWriter");
            }
        }

        public void createWriterWithStrength(int strength)
        {
            status = publisher.GetDefaultDataWriterQos(ref WQosH);
            ErrorHandler.checkStatus(status, "Publisher.GetDefaultDataWriterQos");
            status = publisher.CopyFromTopicQos(ref WQosH, topicQos);
            ErrorHandler.checkStatus(status, "Publisher.CopyFromTopicQos");
            if (durabilityKind.Equals("transient"))
                WQosH.Durability.Kind = DurabilityQosPolicyKind.TransientDurabilityQos;
            else
                WQosH.Durability.Kind = DurabilityQosPolicyKind.PersistentDurabilityQos;

            WQosH.WriterDataLifecycle.AutodisposeUnregisteredInstances = autoDisposeFlag;
			WQosH.OwnershipStrength.Value = strength;
            writer = publisher.CreateDataWriter(
                    topic,
                    WQosH,
                    null,
                    StatusKind.Any);
            ErrorHandler.checkHandle(writer, "Publisher.CreateDataWriter");

            if (exampleName.Equals("Lifecycle"))
            {
                writerStopper = publisher.CreateDataWriter(
                    topic,
                    WQosH,
                    null,
                    StatusKind.Any);
                ErrorHandler.checkHandle(writerStopper, "Publisher.CreateDataWriter");
            }
        }

        /// <summary>
        /// Method to delete a data writer.
        /// </summary>
        /// <param name="dataWriter">The DataWriter instance to delete.</param>
        public void deleteWriter(IDataWriter dataWriter)
        {
            status = publisher.DeleteDataWriter(dataWriter);
            ErrorHandler.checkStatus(status, "Publisher.DeleteDataWriter");
        }

        /// <summary>
        /// Creates a Subscriber
        /// </summary>
        public void createSubscriber()
        {
            status = participant.GetDefaultSubscriberQos(ref subQos);
            ErrorHandler.checkStatus(status, "DomainParticipant.GetDefaultSubscriberQos");

            subQos.Partition.Name = new String[1];
            subQos.Partition.Name[0] = partitionName;
            subscriber = participant.CreateSubscriber(
                    subQos, null, StatusKind.Any);
            ErrorHandler.checkHandle(subscriber, "DomainParticipant.CreateSubscriber");
        }

        /// <summary>
        /// Deletes the class's Subscriber
        /// </summary>
        public void deleteSubscriber()
        {
            status = participant.DeleteSubscriber(subscriber);
            ErrorHandler.checkStatus(status, "Participant.DeleteSubscriber");
        }

        /// <summary>
        /// Creates a DataReader
        /// </summary>
        /// <param name="exampleNameToCreateReaderFor">The example name to create the
        /// DataReader for. This param is used to define any specific Qos values the 
        /// example requires.</param>
        /// <param name="filtered">This param determines whether a reader will be created
        /// for a normal or filtered topic.</param>
        public void createReader(Boolean filtered)
        {
            status = subscriber.GetDefaultDataReaderQos(ref RQosH);
            ErrorHandler.checkStatus(status, "Subscriber.GetDefaultDataReaderQoS");
            status = subscriber.CopyFromTopicQos(ref RQosH, topicQos);
            ErrorHandler.checkStatus(status, "Subscriber.CopyFromTopicQoS");

            switch (exampleName)
            {                
                case "ContentFilteredTopic": 
                case "Listener":
                    RQosH.Durability.Kind = DurabilityQosPolicyKind.TransientDurabilityQos;                    
                    break;
                case "Durability":
                    if (durabilityKind.Equals("transient"))
                        RQosH.Durability.Kind = DurabilityQosPolicyKind.TransientDurabilityQos;
                    else
                        RQosH.Durability.Kind = DurabilityQosPolicyKind.PersistentDurabilityQos;

                    break;
                case "HelloWorld":
                case "Ownership":
                case "WaitSet":
                case "QueryCondition":
                case "Lifecycle":
                    break;
                default:
                    break;
            }
            if (filtered)
            {
                reader = subscriber.CreateDataReader(
                        filteredTopic,
                        RQosH,
                        null,
                        StatusKind.Any);
            }
            else
            {
                reader = subscriber.CreateDataReader(
                        topic,
                        RQosH,
                        null,
                        StatusKind.Any);
            }
            ErrorHandler.checkHandle(reader, "Subscriber.CreateDataReader");
        }

        /// <summary>
        /// Accessor method to get the class's DataReader
        /// </summary>
        /// <returns></returns>
        public IDataReader getReader()
        {
            return reader;
        }

        /// <summary>
        /// Accessor method to get the class's DataWriter
        /// </summary>
        /// <returns></returns>
        public IDataWriter getWriter()
        {
            return writer;
        }

        /// <summary>
        /// Accessor method to get the class's DataWriter stopper. Only available for the Lifecycle example.
        /// </summary>
        /// <returns></returns>
        public IDataWriter getWriterStopper()
        {
            return writerStopper;
        }

        /// <summary>
        /// Accessor method to get the class's Publisher
        /// </summary>
        /// <returns></returns>
        public IPublisher getPublisher()
        {
            return publisher;
        }

        /// <summary>
        /// Accessor method to get the class's Subscriber
        /// </summary>
        /// <returns></returns>
        public ISubscriber getSubscriber()
        {
            return subscriber;
        }

        /// <summary>
        /// Accessor method to get the class's Topic
        /// </summary>
        /// <returns></returns>
        public ITopic getTopic()
        {
            return topic;
        }

        /// <summary>
        /// Accessor method to get the class's DomainParticipant
        /// </summary>
        /// <returns></returns>
        public IDomainParticipant getParticipant()
        {
            return participant;
        }

        /// <summary>
        /// Accessor method to get the class's DomainParticipantFactory
        /// </summary>
        /// <returns></returns>
        public IDomainParticipantFactory getDomainParticipantFactory()
        {
            return dpf;
        }
    }
}
