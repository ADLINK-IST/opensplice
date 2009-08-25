using DDS;
using DDS.OpenSplice;

namespace DDS
{
    #region IParticipantBuiltinTopicDataDataReader
    public interface IParticipantBuiltinTopicDataDataReader : DDS.IDataReader
    {
        ReturnCode Read(
            ref ParticipantBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos);

        ReturnCode Read(
            ref ParticipantBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples);

        ReturnCode Read(
            ref ParticipantBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            SampleStateKind sampleStates,
            ViewStateKind viewStates,
            InstanceStateKind instanceStates);

        ReturnCode Read(
            ref ParticipantBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            SampleStateKind sampleStates,
            ViewStateKind viewStates,
            InstanceStateKind instanceStates);

        ReturnCode Take(
            ref ParticipantBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos);

        ReturnCode Take(
            ref ParticipantBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples);

        ReturnCode Take(
            ref ParticipantBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            SampleStateKind sampleStates,
            ViewStateKind viewStates,
            InstanceStateKind instanceStates);

        ReturnCode Take(
            ref ParticipantBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            SampleStateKind sampleStates,
            ViewStateKind viewStates,
            InstanceStateKind instanceStates);

        ReturnCode ReadWithCondition(
            ref ParticipantBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            IReadCondition readCondition);

        ReturnCode ReadWithCondition(
            ref ParticipantBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            IReadCondition readCondition);

        ReturnCode TakeWithCondition(
            ref ParticipantBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            IReadCondition readCondition);

        ReturnCode TakeWithCondition(
            ref ParticipantBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            IReadCondition readCondition);

        ReturnCode ReadNextSample(
            ParticipantBuiltinTopicData dataValue,
            SampleInfo sampleInfo);

        ReturnCode TakeNextSample(
            ParticipantBuiltinTopicData dataValue,
            SampleInfo sampleInfo);

        ReturnCode ReadInstance(
            ref ParticipantBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            InstanceHandle instanceHandle);

        ReturnCode ReadInstance(
            ref ParticipantBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle);

        ReturnCode ReadInstance(
            ref ParticipantBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle,
            SampleStateKind sampleStates,
            ViewStateKind viewStates,
            InstanceStateKind instanceStates);

        ReturnCode TakeInstance(
            ref ParticipantBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            InstanceHandle instanceHandle);

        ReturnCode TakeInstance(
            ref ParticipantBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle);

        ReturnCode TakeInstance(
            ref ParticipantBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle,
            SampleStateKind sampleStates,
            ViewStateKind viewStates,
            InstanceStateKind instanceStates);

        ReturnCode ReadNextInstance(
            ref ParticipantBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            InstanceHandle instanceHandle);

        ReturnCode ReadNextInstance(
            ref ParticipantBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle);

        ReturnCode ReadNextInstance(
            ref ParticipantBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle,
            SampleStateKind sampleStates,
            ViewStateKind viewStates,
            InstanceStateKind instanceStates);

        ReturnCode TakeNextInstance(
            ref ParticipantBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            InstanceHandle instanceHandle);

        ReturnCode TakeNextInstance(
            ref ParticipantBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle);

        ReturnCode TakeNextInstance(
            ref ParticipantBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle,
            SampleStateKind sampleStates,
            ViewStateKind viewStates,
            InstanceStateKind instanceStates);

        ReturnCode ReadNextInstanceWithCondition(
            ref ParticipantBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            InstanceHandle instanceHandle,
            IReadCondition readCondition);

        ReturnCode ReadNextInstanceWithCondition(
            ref ParticipantBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle,
            IReadCondition readCondition);

        ReturnCode TakeNextInstanceWithCondition(
            ref ParticipantBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            InstanceHandle instanceHandle,
            IReadCondition readCondition);

        ReturnCode TakeNextInstanceWithCondition(
            ref ParticipantBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle,
            IReadCondition readCondition);

        ReturnCode ReturnLoan(
            ref ParticipantBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos);

        ReturnCode GetKeyValue(
            ParticipantBuiltinTopicData key,
            InstanceHandle handle);

        InstanceHandle LookupInstance(
            ParticipantBuiltinTopicData instance);
    }
    #endregion

    #region IParticipantBuiltinTopicDataDataWriter
    public interface IParticipantBuiltinTopicDataDataWriter : DDS.IDataWriter
    {
        InstanceHandle RegisterInstance(
            ParticipantBuiltinTopicData instanceData);

        InstanceHandle RegisterInstanceWithTimestamp(
            ParticipantBuiltinTopicData instanceData,
            Time sourceTimestamp);

        ReturnCode UnregisterInstance(
            ParticipantBuiltinTopicData instanceData,
            InstanceHandle instanceHandle);

        ReturnCode UnregisterInstanceWithTimestamp(
            ParticipantBuiltinTopicData instanceData,
            InstanceHandle instanceHandle,
            Time sourceTimestamp);

        ReturnCode Write(
            ParticipantBuiltinTopicData instanceData);

        ReturnCode Write(
            ParticipantBuiltinTopicData instanceData,
            InstanceHandle instanceHandle);

        ReturnCode WriteWithTimestamp(
            ParticipantBuiltinTopicData instanceData,
            Time sourceTimestamp);

        ReturnCode WriteWithTimestamp(
            ParticipantBuiltinTopicData instanceData,
            InstanceHandle instanceHandle,
            Time sourceTimestamp);

        ReturnCode Dispose(
            ParticipantBuiltinTopicData instanceData,
            InstanceHandle instanceHandle);

        ReturnCode DisposeWithTimestamp(
            ParticipantBuiltinTopicData instanceData,
            InstanceHandle instanceHandle,
            Time sourceTimestamp);

        ReturnCode WriteDispose(
            ParticipantBuiltinTopicData instanceData);

        ReturnCode WriteDispose(
            ParticipantBuiltinTopicData instanceData,
            InstanceHandle instanceHandle);

        ReturnCode WriteDisposeWithTimestamp(
            ParticipantBuiltinTopicData instanceData,
            Time sourceTimestamp);

        ReturnCode WriteDisposeWithTimestamp(
            ParticipantBuiltinTopicData instanceData,
            InstanceHandle instanceHandle,
            Time sourceTimestamp);

        ReturnCode GetKeyValue(
            ParticipantBuiltinTopicData key,
            InstanceHandle instanceHandle);

        InstanceHandle LookupInstance(
            ParticipantBuiltinTopicData instanceData);
    }
    #endregion

    #region ITopicBuiltinTopicDataDataReader
    public interface ITopicBuiltinTopicDataDataReader : DDS.IDataReader
    {
        ReturnCode Read(
            ref TopicBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos);

        ReturnCode Read(
            ref TopicBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples);

        ReturnCode Read(
            ref TopicBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            SampleStateKind sampleStates,
            ViewStateKind viewStates,
            InstanceStateKind instanceStates);

        ReturnCode Read(
            ref TopicBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            SampleStateKind sampleStates,
            ViewStateKind viewStates,
            InstanceStateKind instanceStates);

        ReturnCode Take(
            ref TopicBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos);

        ReturnCode Take(
            ref TopicBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples);

        ReturnCode Take(
            ref TopicBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            SampleStateKind sampleStates,
            ViewStateKind viewStates,
            InstanceStateKind instanceStates);

        ReturnCode Take(
            ref TopicBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            SampleStateKind sampleStates,
            ViewStateKind viewStates,
            InstanceStateKind instanceStates);

        ReturnCode ReadWithCondition(
            ref TopicBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            IReadCondition readCondition);

        ReturnCode ReadWithCondition(
            ref TopicBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            IReadCondition readCondition);

        ReturnCode TakeWithCondition(
            ref TopicBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            IReadCondition readCondition);

        ReturnCode TakeWithCondition(
            ref TopicBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            IReadCondition readCondition);

        ReturnCode ReadNextSample(
            TopicBuiltinTopicData dataValue,
            SampleInfo sampleInfo);

        ReturnCode TakeNextSample(
            TopicBuiltinTopicData dataValue,
            SampleInfo sampleInfo);

        ReturnCode ReadInstance(
            ref TopicBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            InstanceHandle instanceHandle);

        ReturnCode ReadInstance(
            ref TopicBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle);

        ReturnCode ReadInstance(
            ref TopicBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle,
            SampleStateKind sampleStates,
            ViewStateKind viewStates,
            InstanceStateKind instanceStates);

        ReturnCode TakeInstance(
            ref TopicBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            InstanceHandle instanceHandle);

        ReturnCode TakeInstance(
            ref TopicBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle);

        ReturnCode TakeInstance(
            ref TopicBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle,
            SampleStateKind sampleStates,
            ViewStateKind viewStates,
            InstanceStateKind instanceStates);

        ReturnCode ReadNextInstance(
            ref TopicBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            InstanceHandle instanceHandle);

        ReturnCode ReadNextInstance(
            ref TopicBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle);

        ReturnCode ReadNextInstance(
            ref TopicBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle,
            SampleStateKind sampleStates,
            ViewStateKind viewStates,
            InstanceStateKind instanceStates);

        ReturnCode TakeNextInstance(
            ref TopicBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            InstanceHandle instanceHandle);

        ReturnCode TakeNextInstance(
            ref TopicBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle);

        ReturnCode TakeNextInstance(
            ref TopicBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle,
            SampleStateKind sampleStates,
            ViewStateKind viewStates,
            InstanceStateKind instanceStates);

        ReturnCode ReadNextInstanceWithCondition(
            ref TopicBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            InstanceHandle instanceHandle,
            IReadCondition readCondition);

        ReturnCode ReadNextInstanceWithCondition(
            ref TopicBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle,
            IReadCondition readCondition);

        ReturnCode TakeNextInstanceWithCondition(
            ref TopicBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            InstanceHandle instanceHandle,
            IReadCondition readCondition);

        ReturnCode TakeNextInstanceWithCondition(
            ref TopicBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle,
            IReadCondition readCondition);

        ReturnCode ReturnLoan(
            ref TopicBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos);

        ReturnCode GetKeyValue(
            TopicBuiltinTopicData key,
            InstanceHandle handle);

        InstanceHandle LookupInstance(
            TopicBuiltinTopicData instance);
    }
    #endregion

    #region ITopicBuiltinTopicDataDataWriter
    public interface ITopicBuiltinTopicDataDataWriter : DDS.IDataWriter
    {
        InstanceHandle RegisterInstance(
            TopicBuiltinTopicData instanceData);

        InstanceHandle RegisterInstanceWithTimestamp(
            TopicBuiltinTopicData instanceData,
            Time sourceTimestamp);

        ReturnCode UnregisterInstance(
            TopicBuiltinTopicData instanceData,
            InstanceHandle instanceHandle);

        ReturnCode UnregisterInstanceWithTimestamp(
            TopicBuiltinTopicData instanceData,
            InstanceHandle instanceHandle,
            Time sourceTimestamp);

        ReturnCode Write(
            TopicBuiltinTopicData instanceData);

        ReturnCode Write(
            TopicBuiltinTopicData instanceData,
            InstanceHandle instanceHandle);

        ReturnCode WriteWithTimestamp(
            TopicBuiltinTopicData instanceData,
            Time sourceTimestamp);

        ReturnCode WriteWithTimestamp(
            TopicBuiltinTopicData instanceData,
            InstanceHandle instanceHandle,
            Time sourceTimestamp);

        ReturnCode Dispose(
            TopicBuiltinTopicData instanceData,
            InstanceHandle instanceHandle);

        ReturnCode DisposeWithTimestamp(
            TopicBuiltinTopicData instanceData,
            InstanceHandle instanceHandle,
            Time sourceTimestamp);

        ReturnCode WriteDispose(
            TopicBuiltinTopicData instanceData);

        ReturnCode WriteDispose(
            TopicBuiltinTopicData instanceData,
            InstanceHandle instanceHandle);

        ReturnCode WriteDisposeWithTimestamp(
            TopicBuiltinTopicData instanceData,
            Time sourceTimestamp);

        ReturnCode WriteDisposeWithTimestamp(
            TopicBuiltinTopicData instanceData,
            InstanceHandle instanceHandle,
            Time sourceTimestamp);

        ReturnCode GetKeyValue(
            TopicBuiltinTopicData key,
            InstanceHandle instanceHandle);

        InstanceHandle LookupInstance(
            TopicBuiltinTopicData instanceData);
    }
    #endregion

    #region IPublicationBuiltinTopicDataDataReader
    public interface IPublicationBuiltinTopicDataDataReader : DDS.IDataReader
    {
        ReturnCode Read(
            ref PublicationBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos);

        ReturnCode Read(
            ref PublicationBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples);

        ReturnCode Read(
            ref PublicationBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            SampleStateKind sampleStates,
            ViewStateKind viewStates,
            InstanceStateKind instanceStates);

        ReturnCode Read(
            ref PublicationBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            SampleStateKind sampleStates,
            ViewStateKind viewStates,
            InstanceStateKind instanceStates);

        ReturnCode Take(
            ref PublicationBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos);

        ReturnCode Take(
            ref PublicationBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples);

        ReturnCode Take(
            ref PublicationBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            SampleStateKind sampleStates,
            ViewStateKind viewStates,
            InstanceStateKind instanceStates);

        ReturnCode Take(
            ref PublicationBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            SampleStateKind sampleStates,
            ViewStateKind viewStates,
            InstanceStateKind instanceStates);

        ReturnCode ReadWithCondition(
            ref PublicationBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            IReadCondition readCondition);

        ReturnCode ReadWithCondition(
            ref PublicationBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            IReadCondition readCondition);

        ReturnCode TakeWithCondition(
            ref PublicationBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            IReadCondition readCondition);

        ReturnCode TakeWithCondition(
            ref PublicationBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            IReadCondition readCondition);

        ReturnCode ReadNextSample(
            PublicationBuiltinTopicData dataValue,
            SampleInfo sampleInfo);

        ReturnCode TakeNextSample(
            PublicationBuiltinTopicData dataValue,
            SampleInfo sampleInfo);

        ReturnCode ReadInstance(
            ref PublicationBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            InstanceHandle instanceHandle);

        ReturnCode ReadInstance(
            ref PublicationBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle);

        ReturnCode ReadInstance(
            ref PublicationBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle,
            SampleStateKind sampleStates,
            ViewStateKind viewStates,
            InstanceStateKind instanceStates);

        ReturnCode TakeInstance(
            ref PublicationBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            InstanceHandle instanceHandle);

        ReturnCode TakeInstance(
            ref PublicationBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle);

        ReturnCode TakeInstance(
            ref PublicationBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle,
            SampleStateKind sampleStates,
            ViewStateKind viewStates,
            InstanceStateKind instanceStates);

        ReturnCode ReadNextInstance(
            ref PublicationBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            InstanceHandle instanceHandle);

        ReturnCode ReadNextInstance(
            ref PublicationBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle);

        ReturnCode ReadNextInstance(
            ref PublicationBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle,
            SampleStateKind sampleStates,
            ViewStateKind viewStates,
            InstanceStateKind instanceStates);

        ReturnCode TakeNextInstance(
            ref PublicationBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            InstanceHandle instanceHandle);

        ReturnCode TakeNextInstance(
            ref PublicationBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle);

        ReturnCode TakeNextInstance(
            ref PublicationBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle,
            SampleStateKind sampleStates,
            ViewStateKind viewStates,
            InstanceStateKind instanceStates);

        ReturnCode ReadNextInstanceWithCondition(
            ref PublicationBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            InstanceHandle instanceHandle,
            IReadCondition readCondition);

        ReturnCode ReadNextInstanceWithCondition(
            ref PublicationBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle,
            IReadCondition readCondition);

        ReturnCode TakeNextInstanceWithCondition(
            ref PublicationBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            InstanceHandle instanceHandle,
            IReadCondition readCondition);

        ReturnCode TakeNextInstanceWithCondition(
            ref PublicationBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle,
            IReadCondition readCondition);

        ReturnCode ReturnLoan(
            ref PublicationBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos);

        ReturnCode GetKeyValue(
            PublicationBuiltinTopicData key,
            InstanceHandle handle);

        InstanceHandle LookupInstance(
            PublicationBuiltinTopicData instance);
    }
    #endregion

    #region IPublicationBuiltinTopicDataDataWriter
    public interface IPublicationBuiltinTopicDataDataWriter : DDS.IDataWriter
    {
        InstanceHandle RegisterInstance(
            PublicationBuiltinTopicData instanceData);

        InstanceHandle RegisterInstanceWithTimestamp(
            PublicationBuiltinTopicData instanceData,
            Time sourceTimestamp);

        ReturnCode UnregisterInstance(
            PublicationBuiltinTopicData instanceData,
            InstanceHandle instanceHandle);

        ReturnCode UnregisterInstanceWithTimestamp(
            PublicationBuiltinTopicData instanceData,
            InstanceHandle instanceHandle,
            Time sourceTimestamp);

        ReturnCode Write(
            PublicationBuiltinTopicData instanceData);

        ReturnCode Write(
            PublicationBuiltinTopicData instanceData,
            InstanceHandle instanceHandle);

        ReturnCode WriteWithTimestamp(
            PublicationBuiltinTopicData instanceData,
            Time sourceTimestamp);

        ReturnCode WriteWithTimestamp(
            PublicationBuiltinTopicData instanceData,
            InstanceHandle instanceHandle,
            Time sourceTimestamp);

        ReturnCode Dispose(
            PublicationBuiltinTopicData instanceData,
            InstanceHandle instanceHandle);

        ReturnCode DisposeWithTimestamp(
            PublicationBuiltinTopicData instanceData,
            InstanceHandle instanceHandle,
            Time sourceTimestamp);

        ReturnCode WriteDispose(
            PublicationBuiltinTopicData instanceData);

        ReturnCode WriteDispose(
            PublicationBuiltinTopicData instanceData,
            InstanceHandle instanceHandle);

        ReturnCode WriteDisposeWithTimestamp(
            PublicationBuiltinTopicData instanceData,
            Time sourceTimestamp);

        ReturnCode WriteDisposeWithTimestamp(
            PublicationBuiltinTopicData instanceData,
            InstanceHandle instanceHandle,
            Time sourceTimestamp);

        ReturnCode GetKeyValue(
            PublicationBuiltinTopicData key,
            InstanceHandle instanceHandle);

        InstanceHandle LookupInstance(
            PublicationBuiltinTopicData instanceData);
    }
    #endregion

    #region ISubscriptionBuiltinTopicDataDataReader
    public interface ISubscriptionBuiltinTopicDataDataReader : DDS.IDataReader
    {
        ReturnCode Read(
            ref SubscriptionBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos);

        ReturnCode Read(
            ref SubscriptionBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples);

        ReturnCode Read(
            ref SubscriptionBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            SampleStateKind sampleStates,
            ViewStateKind viewStates,
            InstanceStateKind instanceStates);

        ReturnCode Read(
            ref SubscriptionBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            SampleStateKind sampleStates,
            ViewStateKind viewStates,
            InstanceStateKind instanceStates);

        ReturnCode Take(
            ref SubscriptionBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos);

        ReturnCode Take(
            ref SubscriptionBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples);

        ReturnCode Take(
            ref SubscriptionBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            SampleStateKind sampleStates,
            ViewStateKind viewStates,
            InstanceStateKind instanceStates);

        ReturnCode Take(
            ref SubscriptionBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            SampleStateKind sampleStates,
            ViewStateKind viewStates,
            InstanceStateKind instanceStates);

        ReturnCode ReadWithCondition(
            ref SubscriptionBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            IReadCondition readCondition);

        ReturnCode ReadWithCondition(
            ref SubscriptionBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            IReadCondition readCondition);

        ReturnCode TakeWithCondition(
            ref SubscriptionBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            IReadCondition readCondition);

        ReturnCode TakeWithCondition(
            ref SubscriptionBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            IReadCondition readCondition);

        ReturnCode ReadNextSample(
            SubscriptionBuiltinTopicData dataValue,
            SampleInfo sampleInfo);

        ReturnCode TakeNextSample(
            SubscriptionBuiltinTopicData dataValue,
            SampleInfo sampleInfo);

        ReturnCode ReadInstance(
            ref SubscriptionBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            InstanceHandle instanceHandle);

        ReturnCode ReadInstance(
            ref SubscriptionBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle);

        ReturnCode ReadInstance(
            ref SubscriptionBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle,
            SampleStateKind sampleStates,
            ViewStateKind viewStates,
            InstanceStateKind instanceStates);

        ReturnCode TakeInstance(
            ref SubscriptionBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            InstanceHandle instanceHandle);

        ReturnCode TakeInstance(
            ref SubscriptionBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle);

        ReturnCode TakeInstance(
            ref SubscriptionBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle,
            SampleStateKind sampleStates,
            ViewStateKind viewStates,
            InstanceStateKind instanceStates);

        ReturnCode ReadNextInstance(
            ref SubscriptionBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            InstanceHandle instanceHandle);

        ReturnCode ReadNextInstance(
            ref SubscriptionBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle);

        ReturnCode ReadNextInstance(
            ref SubscriptionBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle,
            SampleStateKind sampleStates,
            ViewStateKind viewStates,
            InstanceStateKind instanceStates);

        ReturnCode TakeNextInstance(
            ref SubscriptionBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            InstanceHandle instanceHandle);

        ReturnCode TakeNextInstance(
            ref SubscriptionBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle);

        ReturnCode TakeNextInstance(
            ref SubscriptionBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle,
            SampleStateKind sampleStates,
            ViewStateKind viewStates,
            InstanceStateKind instanceStates);

        ReturnCode ReadNextInstanceWithCondition(
            ref SubscriptionBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            InstanceHandle instanceHandle,
            IReadCondition readCondition);

        ReturnCode ReadNextInstanceWithCondition(
            ref SubscriptionBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle,
            IReadCondition readCondition);

        ReturnCode TakeNextInstanceWithCondition(
            ref SubscriptionBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            InstanceHandle instanceHandle,
            IReadCondition readCondition);

        ReturnCode TakeNextInstanceWithCondition(
            ref SubscriptionBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle,
            IReadCondition readCondition);

        ReturnCode ReturnLoan(
            ref SubscriptionBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos);

        ReturnCode GetKeyValue(
            SubscriptionBuiltinTopicData key,
            InstanceHandle handle);

        InstanceHandle LookupInstance(
            SubscriptionBuiltinTopicData instance);
    }
    #endregion

    #region ISubscriptionBuiltinTopicDataDataWriter
    public interface ISubscriptionBuiltinTopicDataDataWriter : DDS.IDataWriter
    {
        InstanceHandle RegisterInstance(
            SubscriptionBuiltinTopicData instanceData);

        InstanceHandle RegisterInstanceWithTimestamp(
            SubscriptionBuiltinTopicData instanceData,
            Time sourceTimestamp);

        ReturnCode UnregisterInstance(
            SubscriptionBuiltinTopicData instanceData,
            InstanceHandle instanceHandle);

        ReturnCode UnregisterInstanceWithTimestamp(
            SubscriptionBuiltinTopicData instanceData,
            InstanceHandle instanceHandle,
            Time sourceTimestamp);

        ReturnCode Write(
            SubscriptionBuiltinTopicData instanceData);

        ReturnCode Write(
            SubscriptionBuiltinTopicData instanceData,
            InstanceHandle instanceHandle);

        ReturnCode WriteWithTimestamp(
            SubscriptionBuiltinTopicData instanceData,
            Time sourceTimestamp);

        ReturnCode WriteWithTimestamp(
            SubscriptionBuiltinTopicData instanceData,
            InstanceHandle instanceHandle,
            Time sourceTimestamp);

        ReturnCode Dispose(
            SubscriptionBuiltinTopicData instanceData,
            InstanceHandle instanceHandle);

        ReturnCode DisposeWithTimestamp(
            SubscriptionBuiltinTopicData instanceData,
            InstanceHandle instanceHandle,
            Time sourceTimestamp);

        ReturnCode WriteDispose(
            SubscriptionBuiltinTopicData instanceData);

        ReturnCode WriteDispose(
            SubscriptionBuiltinTopicData instanceData,
            InstanceHandle instanceHandle);

        ReturnCode WriteDisposeWithTimestamp(
            SubscriptionBuiltinTopicData instanceData,
            Time sourceTimestamp);

        ReturnCode WriteDisposeWithTimestamp(
            SubscriptionBuiltinTopicData instanceData,
            InstanceHandle instanceHandle,
            Time sourceTimestamp);

        ReturnCode GetKeyValue(
            SubscriptionBuiltinTopicData key,
            InstanceHandle instanceHandle);

        InstanceHandle LookupInstance(
            SubscriptionBuiltinTopicData instanceData);
    }
    #endregion

}

