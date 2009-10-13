using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using DDS;
using DDS.OpenSplice;
using DDS.OpenSplice.CustomMarshalers;

namespace DDS
{
    #region ParticipantBuiltinTopicDataDataReader
    public class ParticipantBuiltinTopicDataDataReader : DDS.OpenSplice.DataReader, IParticipantBuiltinTopicDataDataReader
    {
        public ParticipantBuiltinTopicDataDataReader(IntPtr gapiPtr)
            : base(gapiPtr)
        { /* empty */ }

        public ReturnCode Read(
            ref ParticipantBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos)
        {
            return Read(ref dataValues, ref sampleInfos, Length.Unlimited, SampleStateKind.Any,
                ViewStateKind.Any, InstanceStateKind.Any);
        }

        public ReturnCode Read(
            ref ParticipantBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples)
        {
            return Read(ref dataValues, ref sampleInfos, maxSamples, SampleStateKind.Any,
                ViewStateKind.Any, InstanceStateKind.Any);
        }

        public ReturnCode Read(
            ref ParticipantBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            SampleStateKind sampleStates,
            ViewStateKind viewStates,
            InstanceStateKind instanceStates)
        {
            return Read(ref dataValues, ref sampleInfos, Length.Unlimited, sampleStates,
                viewStates, instanceStates);
        }

        public ReturnCode Read(
                ref ParticipantBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates)
        {
            object[] objectValues = dataValues;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.Read(
                        this,
                        ref objectValues,
                        ref sampleInfos,
                        maxSamples,
                        sampleStates,
                        viewStates,
                        instanceStates);
            dataValues = (ParticipantBuiltinTopicData[])objectValues;
            return result;
        }

        public ReturnCode Take(
            ref ParticipantBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos)
        {
            return Take(ref dataValues, ref sampleInfos, Length.Unlimited);
        }

        public ReturnCode Take(
            ref ParticipantBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples)
        {
            return Take(ref dataValues, ref sampleInfos, maxSamples, SampleStateKind.Any,
                ViewStateKind.Any, InstanceStateKind.Any);
        }

        public ReturnCode Take(
            ref ParticipantBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            SampleStateKind sampleStates,
            ViewStateKind viewStates,
            InstanceStateKind instanceStates)
        {
            return Take(ref dataValues, ref sampleInfos, Length.Unlimited, sampleStates,
                viewStates, instanceStates);
        }

        public ReturnCode Take(
                ref ParticipantBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates)
        {
            object[] objectValues = dataValues;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.Take(
                        this,
                        ref objectValues,
                        ref sampleInfos,
                        maxSamples,
                        sampleStates,
                        viewStates,
                        instanceStates);
            dataValues = (ParticipantBuiltinTopicData[])objectValues;
            return result;
        }

        public ReturnCode ReadWithCondition(
            ref ParticipantBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            IReadCondition readCondition)
        {
            return ReadWithCondition(ref dataValues, ref sampleInfos,
                Length.Unlimited, readCondition);
        }

        public ReturnCode ReadWithCondition(
                ref ParticipantBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                IReadCondition readCondition)
        {
            object[] objectValues = dataValues;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.ReadWithCondition(
                        this,
                        ref objectValues,
                        ref sampleInfos,
                        maxSamples,
                        readCondition);
            dataValues = (ParticipantBuiltinTopicData[])objectValues;
            return result;
        }

        public ReturnCode TakeWithCondition(
            ref ParticipantBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            IReadCondition readCondition)
        {
            return TakeWithCondition(ref dataValues, ref sampleInfos,
                Length.Unlimited, readCondition);
        }

        public ReturnCode TakeWithCondition(
                ref ParticipantBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                IReadCondition readCondition)
        {
            object[] objectValues = dataValues;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.TakeWithCondition(
                        this,
                        ref objectValues,
                        ref sampleInfos,
                        maxSamples,
                        readCondition);
            dataValues = (ParticipantBuiltinTopicData[])objectValues;
            return result;
        }

        public ReturnCode ReadNextSample(
                ParticipantBuiltinTopicData dataValue,
                SampleInfo sampleInfo)
        {
            object objectValues = dataValue;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.ReadNextSample(
                        this,
                        ref objectValues,
                        ref sampleInfo);
            dataValue = (ParticipantBuiltinTopicData)objectValues;
            return result;
        }

        public ReturnCode TakeNextSample(
                ParticipantBuiltinTopicData dataValue,
                SampleInfo sampleInfo)
        {
            object objectValues = dataValue;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.TakeNextSample(
                        this,
                        ref objectValues,
                        ref sampleInfo);
            dataValue = (ParticipantBuiltinTopicData)objectValues;
            return result;
        }

        public ReturnCode ReadInstance(
            ref ParticipantBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            InstanceHandle instanceHandle)
        {
            return ReadInstance(ref dataValues, ref sampleInfos, Length.Unlimited, instanceHandle);
        }

        public ReturnCode ReadInstance(
            ref ParticipantBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle)
        {
            return ReadInstance(ref dataValues, ref sampleInfos, maxSamples, instanceHandle,
                SampleStateKind.Any, ViewStateKind.Any, InstanceStateKind.Any);
        }

        public ReturnCode ReadInstance(
                ref ParticipantBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                InstanceHandle instanceHandle,
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates)
        {
            object[] objectValues = dataValues;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.ReadInstance(
                        this,
                        ref objectValues,
                        ref sampleInfos,
                        maxSamples,
                        instanceHandle,
                        sampleStates,
                        viewStates,
                        instanceStates);
            dataValues = (ParticipantBuiltinTopicData[])objectValues;
            return result;
        }

        public ReturnCode TakeInstance(
            ref ParticipantBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            InstanceHandle instanceHandle)
        {
            return TakeInstance(ref dataValues, ref sampleInfos, Length.Unlimited, instanceHandle);
        }

        public ReturnCode TakeInstance(
            ref ParticipantBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle)
        {
            return TakeInstance(ref dataValues, ref sampleInfos, maxSamples, instanceHandle,
                SampleStateKind.Any, ViewStateKind.Any, InstanceStateKind.Any);
        }

        public ReturnCode TakeInstance(
                ref ParticipantBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                InstanceHandle instanceHandle,
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates)
        {
            object[] objectValues = dataValues;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.TakeInstance(
                        this,
                        ref objectValues,
                        ref sampleInfos,
                        maxSamples,
                        instanceHandle,
                        sampleStates,
                        viewStates,
                        instanceStates);
            dataValues = (ParticipantBuiltinTopicData[])objectValues;
            return result;
        }

        public ReturnCode ReadNextInstance(
            ref ParticipantBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            InstanceHandle instanceHandle)
        {
            return ReadNextInstance(ref dataValues, ref sampleInfos, Length.Unlimited, instanceHandle);
        }

        public ReturnCode ReadNextInstance(
            ref ParticipantBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle)
        {
            return ReadNextInstance(ref dataValues, ref sampleInfos, maxSamples, instanceHandle,
                SampleStateKind.Any, ViewStateKind.Any, InstanceStateKind.Any);
        }

        public ReturnCode ReadNextInstance(
                ref ParticipantBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                InstanceHandle instanceHandle,
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates)
        {
            object[] objectValues = dataValues;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.ReadNextInstance(
                        this,
                        ref objectValues,
                        ref sampleInfos,
                        maxSamples,
                        instanceHandle,
                        sampleStates,
                        viewStates,
                        instanceStates);
            dataValues = (ParticipantBuiltinTopicData[])objectValues;
            return result;
        }

        public ReturnCode TakeNextInstance(
            ref ParticipantBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            InstanceHandle instanceHandle)
        {
            return TakeNextInstance(ref dataValues, ref sampleInfos, Length.Unlimited, instanceHandle);
        }

        public ReturnCode TakeNextInstance(
            ref ParticipantBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle)
        {
            return TakeNextInstance(ref dataValues, ref sampleInfos, maxSamples, instanceHandle,
                SampleStateKind.Any, ViewStateKind.Any, InstanceStateKind.Any);
        }

        public ReturnCode TakeNextInstance(
                ref ParticipantBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                InstanceHandle instanceHandle,
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates)
        {
            object[] objectValues = dataValues;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.TakeNextInstance(
                        this,
                        ref objectValues,
                        ref sampleInfos,
                        maxSamples,
                        instanceHandle,
                        sampleStates,
                        viewStates,
                        instanceStates);
            dataValues = (ParticipantBuiltinTopicData[])objectValues;
            return result;
        }

        public ReturnCode ReadNextInstanceWithCondition(
                ref ParticipantBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                InstanceHandle instanceHandle,
                IReadCondition readCondition)
        {
            return ReadNextInstanceWithCondition(
                ref dataValues,
                ref sampleInfos,
                Length.Unlimited,
                instanceHandle,
                readCondition);
        }

        public ReturnCode ReadNextInstanceWithCondition(
                ref ParticipantBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                InstanceHandle instanceHandle,
                IReadCondition readCondition)
        {
            object[] objectValues = dataValues;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.ReadNextInstanceWithCondition(
                        this,
                        ref objectValues,
                        ref sampleInfos,
                        maxSamples,
                        instanceHandle,
                        readCondition);
            dataValues = (ParticipantBuiltinTopicData[])objectValues;
            return result;
        }

        public ReturnCode TakeNextInstanceWithCondition(
                ref ParticipantBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                InstanceHandle instanceHandle,
                IReadCondition readCondition)
        {
            return TakeNextInstanceWithCondition(
                ref dataValues,
                ref sampleInfos,
                Length.Unlimited,
                instanceHandle,
                readCondition);
        }

        public ReturnCode TakeNextInstanceWithCondition(
                ref ParticipantBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                InstanceHandle instanceHandle,
                IReadCondition readCondition)
        {
            object[] objectValues = dataValues;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.TakeNextInstanceWithCondition(
                        this,
                        ref objectValues,
                        ref sampleInfos,
                        maxSamples,
                        instanceHandle,
                        readCondition);

            dataValues = (ParticipantBuiltinTopicData[])objectValues;
            return result;
        }

        public ReturnCode ReturnLoan(
                ref ParticipantBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos)
        {
            ReturnCode result;

            if (dataValues != null && sampleInfos != null)
            {
                if (dataValues != null && sampleInfos != null)
                {
                    if (dataValues.Length == sampleInfos.Length)
                    {
                        dataValues = null;
                        sampleInfos = null;
                        result = ReturnCode.Ok;
                    }
                    else
                    {
                        result = ReturnCode.PreconditionNotMet;
                    }
                }
                else
                {
                    if ((dataValues == null) && (sampleInfos == null))
                    {
                        result = ReturnCode.Ok;
                    }
                    else
                    {
                        result = ReturnCode.PreconditionNotMet;
                    }
                }
            }
            else
            {
                result = ReturnCode.BadParameter;
            }

            return result;
        }

        public ReturnCode GetKeyValue(
                ParticipantBuiltinTopicData key,
                InstanceHandle handle)
        {
            return
                DDS.OpenSplice.FooDataReader.GetKeyValue(
                        this,
                        key,
                        handle);
        }

        public InstanceHandle LookupInstance(
                ParticipantBuiltinTopicData instance)
        {
            return
                DDS.OpenSplice.FooDataReader.LookupInstance(
                        this,
                        instance);
        }

    }
    #endregion
    
    #region ParticipantBuiltinTopicDataDataWriter
    public class ParticipantBuiltinTopicDataDataWriter : DDS.OpenSplice.DataWriter, IParticipantBuiltinTopicDataDataWriter
    {
        public ParticipantBuiltinTopicDataDataWriter(IntPtr gapiPtr)
            : base(gapiPtr)
        { /* empty */ }

        public InstanceHandle RegisterInstance(
                ParticipantBuiltinTopicData instanceData)
        {
            return DDS.OpenSplice.FooDataWriter.RegisterInstance(
                        this,
                        instanceData);
        }

        public InstanceHandle RegisterInstanceWithTimestamp(
                ParticipantBuiltinTopicData instanceData,
                Time sourceTimestamp)
        {
            return DDS.OpenSplice.FooDataWriter.RegisterInstanceWithTimestamp(
                        this,
                        instanceData,
                        sourceTimestamp);
        }

        public ReturnCode UnregisterInstance(
                ParticipantBuiltinTopicData instanceData,
                InstanceHandle instanceHandle)
        {
            return DDS.OpenSplice.FooDataWriter.UnregisterInstance(
                        this,
                        instanceData,
                        instanceHandle);
        }

        public ReturnCode UnregisterInstanceWithTimestamp(
                ParticipantBuiltinTopicData instanceData,
                InstanceHandle instanceHandle,
                Time sourceTimestamp)
        {
            return DDS.OpenSplice.FooDataWriter.UnregisterInstanceWithTimestamp(
                        this,
                        instanceData,
                        instanceHandle,
                        sourceTimestamp);
        }

        public ReturnCode Write(ParticipantBuiltinTopicData instanceData)
        {
            return Write(instanceData, InstanceHandle.Nil);
        }

        public ReturnCode Write(
                ParticipantBuiltinTopicData instanceData,
                InstanceHandle instanceHandle)
        {
            return DDS.OpenSplice.FooDataWriter.Write(
                        this,
                        instanceData,
                        instanceHandle);
        }

        public ReturnCode WriteWithTimestamp(
                ParticipantBuiltinTopicData instanceData,
                Time sourceTimestamp)
        {
            return WriteWithTimestamp(instanceData, InstanceHandle.Nil, sourceTimestamp);
        }

        public ReturnCode WriteWithTimestamp(
                ParticipantBuiltinTopicData instanceData,
                InstanceHandle instanceHandle,
                Time sourceTimestamp)
        {
            return DDS.OpenSplice.FooDataWriter.WriteWithTimestamp(
                        this,
                        instanceData,
                        instanceHandle,
                        sourceTimestamp);
        }

        public ReturnCode Dispose(
                ParticipantBuiltinTopicData instanceData,
                InstanceHandle instanceHandle)
        {
            return DDS.OpenSplice.FooDataWriter.Dispose(
                        this,
                        instanceData,
                        instanceHandle);
        }

        public ReturnCode DisposeWithTimestamp(
                ParticipantBuiltinTopicData instanceData,
                InstanceHandle instanceHandle,
                Time sourceTimestamp)
        {
            return DDS.OpenSplice.FooDataWriter.DisposeWithTimestamp(
                        this,
                        instanceData,
                        instanceHandle,
                        sourceTimestamp);
        }

        public ReturnCode WriteDispose(
                ParticipantBuiltinTopicData instanceData)
        {
            return WriteDispose(instanceData, InstanceHandle.Nil);
        }

        public ReturnCode WriteDispose(
                ParticipantBuiltinTopicData instanceData,
                InstanceHandle instanceHandle)
        {
            return DDS.OpenSplice.FooDataWriter.WriteDispose(
                        this,
                        instanceData,
                        instanceHandle);
        }

        public ReturnCode WriteDisposeWithTimestamp(
                ParticipantBuiltinTopicData instanceData,
                Time sourceTimestamp)
        {
            return WriteDisposeWithTimestamp(instanceData, InstanceHandle.Nil, sourceTimestamp);
        }

        public ReturnCode WriteDisposeWithTimestamp(
                ParticipantBuiltinTopicData instanceData,
                InstanceHandle instanceHandle,
                Time sourceTimestamp)
        {
            return DDS.OpenSplice.FooDataWriter.WriteDisposeWithTimestamp(
                        this,
                        instanceData,
                        instanceHandle,
                        sourceTimestamp);
        }

        public ReturnCode GetKeyValue(
                ParticipantBuiltinTopicData key,
                InstanceHandle instanceHandle)
        {
            ReturnCode result = DDS.OpenSplice.FooDataWriter.GetKeyValue(
                                    this,
                                    key,
                                    instanceHandle);
            return result;
        }

        public InstanceHandle LookupInstance(
            ParticipantBuiltinTopicData instanceData)
        {
            InstanceHandle result = DDS.OpenSplice.FooDataWriter.LookupInstance(
                                        this,
                                        instanceData);
            return result;
        }
    }
    #endregion

    #region ParticipantBuiltinTopicDataTypeSupport
    public class ParticipantBuiltinTopicDataTypeSupport : DDS.OpenSplice.TypeSupport
    {
        private const string typeName = "DDS::ParticipantBuiltinTopicData";
        private const string keyList = "key";
        private const string metaDescriptor = "<MetaData version=\"1.0.0\"><Module name=\"DDS\"><TypeDef name=\"BuiltinTopicKey_t\"><Array size=\"3\"><Long/></Array></TypeDef><Struct name=\"UserDataQosPolicy\"><Member name=\"value\"><Sequence><Octet/></Sequence></Member></Struct><Struct name=\"ParticipantBuiltinTopicData\"><Member name=\"key\"><Type name=\"DDS::BuiltinTopicKey_t\"/></Member><Member name=\"user_data\"><Type name=\"DDS::UserDataQosPolicy\"/></Member></Struct></Module></MetaData>";

        public ParticipantBuiltinTopicDataTypeSupport()
            : base(typeof(ParticipantBuiltinTopicData), new MarshalerTypeGenerator())
        { }

        public ParticipantBuiltinTopicDataTypeSupport(
                IMarshalerTypeGenerator generator)
            : base(typeof(ParticipantBuiltinTopicData), generator)
        { }

        public ParticipantBuiltinTopicDataTypeSupport(
                DatabaseMarshaler marshaler)
            : base(typeof(ParticipantBuiltinTopicData), marshaler)
        { }

        public override string TypeName
        {
            get
            {
                return typeName;
            }
        }

        public override string Description
        {
            get
            {
                return metaDescriptor;
            }
        }

        public override string KeyList
        {
            get
            {
                return keyList;
            }
        }

        public override DDS.OpenSplice.DataWriter CreateDataWriter(IntPtr gapiPtr)
        {
            return new ParticipantBuiltinTopicDataDataWriter(gapiPtr);
        }

        public override DDS.OpenSplice.DataReader CreateDataReader(IntPtr gapiPtr)
        {
            return new ParticipantBuiltinTopicDataDataReader(gapiPtr);
        }
    }
    #endregion
    #region TopicBuiltinTopicDataDataReader
    public class TopicBuiltinTopicDataDataReader : DDS.OpenSplice.DataReader, ITopicBuiltinTopicDataDataReader
    {
        public TopicBuiltinTopicDataDataReader(IntPtr gapiPtr)
            : base(gapiPtr)
        { /* empty */ }

        public ReturnCode Read(
            ref TopicBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos)
        {
            return Read(ref dataValues, ref sampleInfos, Length.Unlimited, SampleStateKind.Any,
                ViewStateKind.Any, InstanceStateKind.Any);
        }

        public ReturnCode Read(
            ref TopicBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples)
        {
            return Read(ref dataValues, ref sampleInfos, maxSamples, SampleStateKind.Any,
                ViewStateKind.Any, InstanceStateKind.Any);
        }

        public ReturnCode Read(
            ref TopicBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            SampleStateKind sampleStates,
            ViewStateKind viewStates,
            InstanceStateKind instanceStates)
        {
            return Read(ref dataValues, ref sampleInfos, Length.Unlimited, sampleStates,
                viewStates, instanceStates);
        }

        public ReturnCode Read(
                ref TopicBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates)
        {
            object[] objectValues = dataValues;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.Read(
                        this,
                        ref objectValues,
                        ref sampleInfos,
                        maxSamples,
                        sampleStates,
                        viewStates,
                        instanceStates);
            dataValues = (TopicBuiltinTopicData[])objectValues;
            return result;
        }

        public ReturnCode Take(
            ref TopicBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos)
        {
            return Take(ref dataValues, ref sampleInfos, Length.Unlimited);
        }

        public ReturnCode Take(
            ref TopicBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples)
        {
            return Take(ref dataValues, ref sampleInfos, maxSamples, SampleStateKind.Any,
                ViewStateKind.Any, InstanceStateKind.Any);
        }

        public ReturnCode Take(
            ref TopicBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            SampleStateKind sampleStates,
            ViewStateKind viewStates,
            InstanceStateKind instanceStates)
        {
            return Take(ref dataValues, ref sampleInfos, Length.Unlimited, sampleStates,
                viewStates, instanceStates);
        }

        public ReturnCode Take(
                ref TopicBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates)
        {
            object[] objectValues = dataValues;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.Take(
                        this,
                        ref objectValues,
                        ref sampleInfos,
                        maxSamples,
                        sampleStates,
                        viewStates,
                        instanceStates);
            dataValues = (TopicBuiltinTopicData[])objectValues;
            return result;
        }

        public ReturnCode ReadWithCondition(
            ref TopicBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            IReadCondition readCondition)
        {
            return ReadWithCondition(ref dataValues, ref sampleInfos,
                Length.Unlimited, readCondition);
        }

        public ReturnCode ReadWithCondition(
                ref TopicBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                IReadCondition readCondition)
        {
            object[] objectValues = dataValues;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.ReadWithCondition(
                        this,
                        ref objectValues,
                        ref sampleInfos,
                        maxSamples,
                        readCondition);
            dataValues = (TopicBuiltinTopicData[])objectValues;
            return result;
        }

        public ReturnCode TakeWithCondition(
            ref TopicBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            IReadCondition readCondition)
        {
            return TakeWithCondition(ref dataValues, ref sampleInfos,
                Length.Unlimited, readCondition);
        }

        public ReturnCode TakeWithCondition(
                ref TopicBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                IReadCondition readCondition)
        {
            object[] objectValues = dataValues;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.TakeWithCondition(
                        this,
                        ref objectValues,
                        ref sampleInfos,
                        maxSamples,
                        readCondition);
            dataValues = (TopicBuiltinTopicData[])objectValues;
            return result;
        }

        public ReturnCode ReadNextSample(
                TopicBuiltinTopicData dataValue,
                SampleInfo sampleInfo)
        {
            object objectValues = dataValue;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.ReadNextSample(
                        this,
                        ref objectValues,
                        ref sampleInfo);
            dataValue = (TopicBuiltinTopicData)objectValues;
            return result;
        }

        public ReturnCode TakeNextSample(
                TopicBuiltinTopicData dataValue,
                SampleInfo sampleInfo)
        {
            object objectValues = dataValue;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.TakeNextSample(
                        this,
                        ref objectValues,
                        ref sampleInfo);
            dataValue = (TopicBuiltinTopicData)objectValues;
            return result;
        }

        public ReturnCode ReadInstance(
            ref TopicBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            InstanceHandle instanceHandle)
        {
            return ReadInstance(ref dataValues, ref sampleInfos, Length.Unlimited, instanceHandle);
        }

        public ReturnCode ReadInstance(
            ref TopicBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle)
        {
            return ReadInstance(ref dataValues, ref sampleInfos, maxSamples, instanceHandle,
                SampleStateKind.Any, ViewStateKind.Any, InstanceStateKind.Any);
        }

        public ReturnCode ReadInstance(
                ref TopicBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                InstanceHandle instanceHandle,
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates)
        {
            object[] objectValues = dataValues;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.ReadInstance(
                        this,
                        ref objectValues,
                        ref sampleInfos,
                        maxSamples,
                        instanceHandle,
                        sampleStates,
                        viewStates,
                        instanceStates);
            dataValues = (TopicBuiltinTopicData[])objectValues;
            return result;
        }

        public ReturnCode TakeInstance(
            ref TopicBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            InstanceHandle instanceHandle)
        {
            return TakeInstance(ref dataValues, ref sampleInfos, Length.Unlimited, instanceHandle);
        }

        public ReturnCode TakeInstance(
            ref TopicBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle)
        {
            return TakeInstance(ref dataValues, ref sampleInfos, maxSamples, instanceHandle,
                SampleStateKind.Any, ViewStateKind.Any, InstanceStateKind.Any);
        }

        public ReturnCode TakeInstance(
                ref TopicBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                InstanceHandle instanceHandle,
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates)
        {
            object[] objectValues = dataValues;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.TakeInstance(
                        this,
                        ref objectValues,
                        ref sampleInfos,
                        maxSamples,
                        instanceHandle,
                        sampleStates,
                        viewStates,
                        instanceStates);
            dataValues = (TopicBuiltinTopicData[])objectValues;
            return result;
        }

        public ReturnCode ReadNextInstance(
            ref TopicBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            InstanceHandle instanceHandle)
        {
            return ReadNextInstance(ref dataValues, ref sampleInfos, Length.Unlimited, instanceHandle);
        }

        public ReturnCode ReadNextInstance(
            ref TopicBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle)
        {
            return ReadNextInstance(ref dataValues, ref sampleInfos, maxSamples, instanceHandle,
                SampleStateKind.Any, ViewStateKind.Any, InstanceStateKind.Any);
        }

        public ReturnCode ReadNextInstance(
                ref TopicBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                InstanceHandle instanceHandle,
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates)
        {
            object[] objectValues = dataValues;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.ReadNextInstance(
                        this,
                        ref objectValues,
                        ref sampleInfos,
                        maxSamples,
                        instanceHandle,
                        sampleStates,
                        viewStates,
                        instanceStates);
            dataValues = (TopicBuiltinTopicData[])objectValues;
            return result;
        }

        public ReturnCode TakeNextInstance(
            ref TopicBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            InstanceHandle instanceHandle)
        {
            return TakeNextInstance(ref dataValues, ref sampleInfos, Length.Unlimited, instanceHandle);
        }

        public ReturnCode TakeNextInstance(
            ref TopicBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle)
        {
            return TakeNextInstance(ref dataValues, ref sampleInfos, maxSamples, instanceHandle,
                SampleStateKind.Any, ViewStateKind.Any, InstanceStateKind.Any);
        }

        public ReturnCode TakeNextInstance(
                ref TopicBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                InstanceHandle instanceHandle,
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates)
        {
            object[] objectValues = dataValues;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.TakeNextInstance(
                        this,
                        ref objectValues,
                        ref sampleInfos,
                        maxSamples,
                        instanceHandle,
                        sampleStates,
                        viewStates,
                        instanceStates);
            dataValues = (TopicBuiltinTopicData[])objectValues;
            return result;
        }

        public ReturnCode ReadNextInstanceWithCondition(
                ref TopicBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                InstanceHandle instanceHandle,
                IReadCondition readCondition)
        {
            return ReadNextInstanceWithCondition(
                ref dataValues,
                ref sampleInfos,
                Length.Unlimited,
                instanceHandle,
                readCondition);
        }

        public ReturnCode ReadNextInstanceWithCondition(
                ref TopicBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                InstanceHandle instanceHandle,
                IReadCondition readCondition)
        {
            object[] objectValues = dataValues;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.ReadNextInstanceWithCondition(
                        this,
                        ref objectValues,
                        ref sampleInfos,
                        maxSamples,
                        instanceHandle,
                        readCondition);
            dataValues = (TopicBuiltinTopicData[])objectValues;
            return result;
        }

        public ReturnCode TakeNextInstanceWithCondition(
                ref TopicBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                InstanceHandle instanceHandle,
                IReadCondition readCondition)
        {
            return TakeNextInstanceWithCondition(
                ref dataValues,
                ref sampleInfos,
                Length.Unlimited,
                instanceHandle,
                readCondition);
        }

        public ReturnCode TakeNextInstanceWithCondition(
                ref TopicBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                InstanceHandle instanceHandle,
                IReadCondition readCondition)
        {
            object[] objectValues = dataValues;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.TakeNextInstanceWithCondition(
                        this,
                        ref objectValues,
                        ref sampleInfos,
                        maxSamples,
                        instanceHandle,
                        readCondition);

            dataValues = (TopicBuiltinTopicData[])objectValues;
            return result;
        }

        public ReturnCode ReturnLoan(
                ref TopicBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos)
        {
            ReturnCode result;

            if (dataValues != null && sampleInfos != null)
            {
                if (dataValues != null && sampleInfos != null)
                {
                    if (dataValues.Length == sampleInfos.Length)
                    {
                        dataValues = null;
                        sampleInfos = null;
                        result = ReturnCode.Ok;
                    }
                    else
                    {
                        result = ReturnCode.PreconditionNotMet;
                    }
                }
                else
                {
                    if ((dataValues == null) && (sampleInfos == null))
                    {
                        result = ReturnCode.Ok;
                    }
                    else
                    {
                        result = ReturnCode.PreconditionNotMet;
                    }
                }
            }
            else
            {
                result = ReturnCode.BadParameter;
            }

            return result;
        }

        public ReturnCode GetKeyValue(
                TopicBuiltinTopicData key,
                InstanceHandle handle)
        {
            return
                DDS.OpenSplice.FooDataReader.GetKeyValue(
                        this,
                        key,
                        handle);
        }

        public InstanceHandle LookupInstance(
                TopicBuiltinTopicData instance)
        {
            return
                DDS.OpenSplice.FooDataReader.LookupInstance(
                        this,
                        instance);
        }

    }
    #endregion
    
    #region TopicBuiltinTopicDataDataWriter
    public class TopicBuiltinTopicDataDataWriter : DDS.OpenSplice.DataWriter, ITopicBuiltinTopicDataDataWriter
    {
        public TopicBuiltinTopicDataDataWriter(IntPtr gapiPtr)
            : base(gapiPtr)
        { /* empty */ }

        public InstanceHandle RegisterInstance(
                TopicBuiltinTopicData instanceData)
        {
            return DDS.OpenSplice.FooDataWriter.RegisterInstance(
                        this,
                        instanceData);
        }

        public InstanceHandle RegisterInstanceWithTimestamp(
                TopicBuiltinTopicData instanceData,
                Time sourceTimestamp)
        {
            return DDS.OpenSplice.FooDataWriter.RegisterInstanceWithTimestamp(
                        this,
                        instanceData,
                        sourceTimestamp);
        }

        public ReturnCode UnregisterInstance(
                TopicBuiltinTopicData instanceData,
                InstanceHandle instanceHandle)
        {
            return DDS.OpenSplice.FooDataWriter.UnregisterInstance(
                        this,
                        instanceData,
                        instanceHandle);
        }

        public ReturnCode UnregisterInstanceWithTimestamp(
                TopicBuiltinTopicData instanceData,
                InstanceHandle instanceHandle,
                Time sourceTimestamp)
        {
            return DDS.OpenSplice.FooDataWriter.UnregisterInstanceWithTimestamp(
                        this,
                        instanceData,
                        instanceHandle,
                        sourceTimestamp);
        }

        public ReturnCode Write(TopicBuiltinTopicData instanceData)
        {
            return Write(instanceData, InstanceHandle.Nil);
        }

        public ReturnCode Write(
                TopicBuiltinTopicData instanceData,
                InstanceHandle instanceHandle)
        {
            return DDS.OpenSplice.FooDataWriter.Write(
                        this,
                        instanceData,
                        instanceHandle);
        }

        public ReturnCode WriteWithTimestamp(
                TopicBuiltinTopicData instanceData,
                Time sourceTimestamp)
        {
            return WriteWithTimestamp(instanceData, InstanceHandle.Nil, sourceTimestamp);
        }

        public ReturnCode WriteWithTimestamp(
                TopicBuiltinTopicData instanceData,
                InstanceHandle instanceHandle,
                Time sourceTimestamp)
        {
            return DDS.OpenSplice.FooDataWriter.WriteWithTimestamp(
                        this,
                        instanceData,
                        instanceHandle,
                        sourceTimestamp);
        }

        public ReturnCode Dispose(
                TopicBuiltinTopicData instanceData,
                InstanceHandle instanceHandle)
        {
            return DDS.OpenSplice.FooDataWriter.Dispose(
                        this,
                        instanceData,
                        instanceHandle);
        }

        public ReturnCode DisposeWithTimestamp(
                TopicBuiltinTopicData instanceData,
                InstanceHandle instanceHandle,
                Time sourceTimestamp)
        {
            return DDS.OpenSplice.FooDataWriter.DisposeWithTimestamp(
                        this,
                        instanceData,
                        instanceHandle,
                        sourceTimestamp);
        }

        public ReturnCode WriteDispose(
                TopicBuiltinTopicData instanceData)
        {
            return WriteDispose(instanceData, InstanceHandle.Nil);
        }

        public ReturnCode WriteDispose(
                TopicBuiltinTopicData instanceData,
                InstanceHandle instanceHandle)
        {
            return DDS.OpenSplice.FooDataWriter.WriteDispose(
                        this,
                        instanceData,
                        instanceHandle);
        }

        public ReturnCode WriteDisposeWithTimestamp(
                TopicBuiltinTopicData instanceData,
                Time sourceTimestamp)
        {
            return WriteDisposeWithTimestamp(instanceData, InstanceHandle.Nil, sourceTimestamp);
        }

        public ReturnCode WriteDisposeWithTimestamp(
                TopicBuiltinTopicData instanceData,
                InstanceHandle instanceHandle,
                Time sourceTimestamp)
        {
            return DDS.OpenSplice.FooDataWriter.WriteDisposeWithTimestamp(
                        this,
                        instanceData,
                        instanceHandle,
                        sourceTimestamp);
        }

        public ReturnCode GetKeyValue(
                TopicBuiltinTopicData key,
                InstanceHandle instanceHandle)
        {
            ReturnCode result = DDS.OpenSplice.FooDataWriter.GetKeyValue(
                                    this,
                                    key,
                                    instanceHandle);
            return result;
        }

        public InstanceHandle LookupInstance(
            TopicBuiltinTopicData instanceData)
        {
            InstanceHandle result = DDS.OpenSplice.FooDataWriter.LookupInstance(
                                        this,
                                        instanceData);
            return result;
        }
    }
    #endregion

    #region TopicBuiltinTopicDataTypeSupport
    public class TopicBuiltinTopicDataTypeSupport : DDS.OpenSplice.TypeSupport
    {
        private const string typeName = "DDS::TopicBuiltinTopicData";
        private const string keyList = "key";
        private const string metaDescriptor = "<MetaData version=\"1.0.0\"><Module name=\"DDS\"><TypeDef name=\"BuiltinTopicKey_t\"><Array size=\"3\"><Long/></Array></TypeDef><Enum name=\"DurabilityQosPolicyKind\"><Element name=\"VOLATILE_DURABILITY_QOS\"/><Element name=\"TRANSIENT_LOCAL_DURABILITY_QOS\"/><Element name=\"TRANSIENT_DURABILITY_QOS\"/><Element name=\"PERSISTENT_DURABILITY_QOS\"/></Enum><Struct name=\"DurabilityQosPolicy\"><Member name=\"kind\"><Type name=\"DDS::DurabilityQosPolicyKind\"/></Member></Struct><Struct name=\"Duration_t\"><Member name=\"sec\"><Long/></Member><Member name=\"nanosec\"><ULong/></Member></Struct><Enum name=\"HistoryQosPolicyKind\"><Element name=\"KEEP_LAST_HISTORY_QOS\"/><Element name=\"KEEP_ALL_HISTORY_QOS\"/></Enum><Struct name=\"DurabilityServiceQosPolicy\"><Member name=\"service_cleanup_delay\"><Type name=\"DDS::Duration_t\"/></Member><Member name=\"history_kind\"><Type name=\"DDS::HistoryQosPolicyKind\"/></Member><Member name=\"history_depth\"><Long/></Member><Member name=\"max_samples\"><Long/></Member><Member name=\"max_instances\"><Long/></Member><Member name=\"max_samples_per_instance\"><Long/></Member></Struct><Struct name=\"DeadlineQosPolicy\"><Member name=\"period\"><Type name=\"DDS::Duration_t\"/></Member></Struct><Struct name=\"LatencyBudgetQosPolicy\"><Member name=\"duration\"><Type name=\"DDS::Duration_t\"/></Member></Struct><Enum name=\"LivelinessQosPolicyKind\"><Element name=\"AUTOMATIC_LIVELINESS_QOS\"/><Element name=\"MANUAL_BY_PARTICIPANT_LIVELINESS_QOS\"/><Element name=\"MANUAL_BY_TOPIC_LIVELINESS_QOS\"/></Enum><Struct name=\"LivelinessQosPolicy\"><Member name=\"kind\"><Type name=\"DDS::LivelinessQosPolicyKind\"/></Member><Member name=\"lease_duration\"><Type name=\"DDS::Duration_t\"/></Member></Struct><Enum name=\"ReliabilityQosPolicyKind\"><Element name=\"BEST_EFFORT_RELIABILITY_QOS\"/><Element name=\"RELIABLE_RELIABILITY_QOS\"/></Enum><Struct name=\"ReliabilityQosPolicy\"><Member name=\"kind\"><Type name=\"DDS::ReliabilityQosPolicyKind\"/></Member><Member name=\"max_blocking_time\"><Type name=\"DDS::Duration_t\"/></Member></Struct><Struct name=\"TransportPriorityQosPolicy\"><Member name=\"value\"><Long/></Member></Struct><Struct name=\"LifespanQosPolicy\"><Member name=\"duration\"><Type name=\"DDS::Duration_t\"/></Member></Struct><Enum name=\"DestinationOrderQosPolicyKind\"><Element name=\"BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS\"/><Element name=\"BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS\"/></Enum><Struct name=\"DestinationOrderQosPolicy\"><Member name=\"kind\"><Type name=\"DDS::DestinationOrderQosPolicyKind\"/></Member></Struct><Struct name=\"HistoryQosPolicy\"><Member name=\"kind\"><Type name=\"DDS::HistoryQosPolicyKind\"/></Member><Member name=\"depth\"><Long/></Member></Struct><Struct name=\"ResourceLimitsQosPolicy\"><Member name=\"max_samples\"><Long/></Member><Member name=\"max_instances\"><Long/></Member><Member name=\"max_samples_per_instance\"><Long/></Member></Struct><Enum name=\"OwnershipQosPolicyKind\"><Element name=\"SHARED_OWNERSHIP_QOS\"/><Element name=\"EXCLUSIVE_OWNERSHIP_QOS\"/></Enum><Struct name=\"OwnershipQosPolicy\"><Member name=\"kind\"><Type name=\"DDS::OwnershipQosPolicyKind\"/></Member></Struct><Struct name=\"TopicDataQosPolicy\"><Member name=\"value\"><Sequence><Octet/></Sequence></Member></Struct><Struct name=\"TopicBuiltinTopicData\"><Member name=\"key\"><Type name=\"DDS::BuiltinTopicKey_t\"/></Member><Member name=\"name\"><String/></Member><Member name=\"type_name\"><String/></Member><Member name=\"durability\"><Type name=\"DDS::DurabilityQosPolicy\"/></Member><Member name=\"durability_service\"><Type name=\"DDS::DurabilityServiceQosPolicy\"/></Member><Member name=\"deadline\"><Type name=\"DDS::DeadlineQosPolicy\"/></Member><Member name=\"latency_budget\"><Type name=\"DDS::LatencyBudgetQosPolicy\"/></Member><Member name=\"liveliness\"><Type name=\"DDS::LivelinessQosPolicy\"/></Member><Member name=\"reliability\"><Type name=\"DDS::ReliabilityQosPolicy\"/></Member><Member name=\"transport_priority\"><Type name=\"DDS::TransportPriorityQosPolicy\"/></Member><Member name=\"lifespan\"><Type name=\"DDS::LifespanQosPolicy\"/></Member><Member name=\"destination_order\"><Type name=\"DDS::DestinationOrderQosPolicy\"/></Member><Member name=\"history\"><Type name=\"DDS::HistoryQosPolicy\"/></Member><Member name=\"resource_limits\"><Type name=\"DDS::ResourceLimitsQosPolicy\"/></Member><Member name=\"ownership\"><Type name=\"DDS::OwnershipQosPolicy\"/></Member><Member name=\"topic_data\"><Type name=\"DDS::TopicDataQosPolicy\"/></Member></Struct></Module></MetaData>";

        public TopicBuiltinTopicDataTypeSupport()
            : base(typeof(TopicBuiltinTopicData), new MarshalerTypeGenerator())
        { }

        public TopicBuiltinTopicDataTypeSupport(
                IMarshalerTypeGenerator generator)
            : base(typeof(TopicBuiltinTopicData), generator)
        { }

        public TopicBuiltinTopicDataTypeSupport(
                DatabaseMarshaler marshaler)
            : base(typeof(TopicBuiltinTopicData), marshaler)
        { }

        public override string TypeName
        {
            get
            {
                return typeName;
            }
        }

        public override string Description
        {
            get
            {
                return metaDescriptor;
            }
        }

        public override string KeyList
        {
            get
            {
                return keyList;
            }
        }

        public override DDS.OpenSplice.DataWriter CreateDataWriter(IntPtr gapiPtr)
        {
            return new TopicBuiltinTopicDataDataWriter(gapiPtr);
        }

        public override DDS.OpenSplice.DataReader CreateDataReader(IntPtr gapiPtr)
        {
            return new TopicBuiltinTopicDataDataReader(gapiPtr);
        }
    }
    #endregion
    #region PublicationBuiltinTopicDataDataReader
    public class PublicationBuiltinTopicDataDataReader : DDS.OpenSplice.DataReader, IPublicationBuiltinTopicDataDataReader
    {
        public PublicationBuiltinTopicDataDataReader(IntPtr gapiPtr)
            : base(gapiPtr)
        { /* empty */ }

        public ReturnCode Read(
            ref PublicationBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos)
        {
            return Read(ref dataValues, ref sampleInfos, Length.Unlimited, SampleStateKind.Any,
                ViewStateKind.Any, InstanceStateKind.Any);
        }

        public ReturnCode Read(
            ref PublicationBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples)
        {
            return Read(ref dataValues, ref sampleInfos, maxSamples, SampleStateKind.Any,
                ViewStateKind.Any, InstanceStateKind.Any);
        }

        public ReturnCode Read(
            ref PublicationBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            SampleStateKind sampleStates,
            ViewStateKind viewStates,
            InstanceStateKind instanceStates)
        {
            return Read(ref dataValues, ref sampleInfos, Length.Unlimited, sampleStates,
                viewStates, instanceStates);
        }

        public ReturnCode Read(
                ref PublicationBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates)
        {
            object[] objectValues = dataValues;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.Read(
                        this,
                        ref objectValues,
                        ref sampleInfos,
                        maxSamples,
                        sampleStates,
                        viewStates,
                        instanceStates);
            dataValues = (PublicationBuiltinTopicData[])objectValues;
            return result;
        }

        public ReturnCode Take(
            ref PublicationBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos)
        {
            return Take(ref dataValues, ref sampleInfos, Length.Unlimited);
        }

        public ReturnCode Take(
            ref PublicationBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples)
        {
            return Take(ref dataValues, ref sampleInfos, maxSamples, SampleStateKind.Any,
                ViewStateKind.Any, InstanceStateKind.Any);
        }

        public ReturnCode Take(
            ref PublicationBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            SampleStateKind sampleStates,
            ViewStateKind viewStates,
            InstanceStateKind instanceStates)
        {
            return Take(ref dataValues, ref sampleInfos, Length.Unlimited, sampleStates,
                viewStates, instanceStates);
        }

        public ReturnCode Take(
                ref PublicationBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates)
        {
            object[] objectValues = dataValues;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.Take(
                        this,
                        ref objectValues,
                        ref sampleInfos,
                        maxSamples,
                        sampleStates,
                        viewStates,
                        instanceStates);
            dataValues = (PublicationBuiltinTopicData[])objectValues;
            return result;
        }

        public ReturnCode ReadWithCondition(
            ref PublicationBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            IReadCondition readCondition)
        {
            return ReadWithCondition(ref dataValues, ref sampleInfos,
                Length.Unlimited, readCondition);
        }

        public ReturnCode ReadWithCondition(
                ref PublicationBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                IReadCondition readCondition)
        {
            object[] objectValues = dataValues;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.ReadWithCondition(
                        this,
                        ref objectValues,
                        ref sampleInfos,
                        maxSamples,
                        readCondition);
            dataValues = (PublicationBuiltinTopicData[])objectValues;
            return result;
        }

        public ReturnCode TakeWithCondition(
            ref PublicationBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            IReadCondition readCondition)
        {
            return TakeWithCondition(ref dataValues, ref sampleInfos,
                Length.Unlimited, readCondition);
        }

        public ReturnCode TakeWithCondition(
                ref PublicationBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                IReadCondition readCondition)
        {
            object[] objectValues = dataValues;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.TakeWithCondition(
                        this,
                        ref objectValues,
                        ref sampleInfos,
                        maxSamples,
                        readCondition);
            dataValues = (PublicationBuiltinTopicData[])objectValues;
            return result;
        }

        public ReturnCode ReadNextSample(
                PublicationBuiltinTopicData dataValue,
                SampleInfo sampleInfo)
        {
            object objectValues = dataValue;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.ReadNextSample(
                        this,
                        ref objectValues,
                        ref sampleInfo);
            dataValue = (PublicationBuiltinTopicData)objectValues;
            return result;
        }

        public ReturnCode TakeNextSample(
                PublicationBuiltinTopicData dataValue,
                SampleInfo sampleInfo)
        {
            object objectValues = dataValue;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.TakeNextSample(
                        this,
                        ref objectValues,
                        ref sampleInfo);
            dataValue = (PublicationBuiltinTopicData)objectValues;
            return result;
        }

        public ReturnCode ReadInstance(
            ref PublicationBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            InstanceHandle instanceHandle)
        {
            return ReadInstance(ref dataValues, ref sampleInfos, Length.Unlimited, instanceHandle);
        }

        public ReturnCode ReadInstance(
            ref PublicationBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle)
        {
            return ReadInstance(ref dataValues, ref sampleInfos, maxSamples, instanceHandle,
                SampleStateKind.Any, ViewStateKind.Any, InstanceStateKind.Any);
        }

        public ReturnCode ReadInstance(
                ref PublicationBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                InstanceHandle instanceHandle,
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates)
        {
            object[] objectValues = dataValues;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.ReadInstance(
                        this,
                        ref objectValues,
                        ref sampleInfos,
                        maxSamples,
                        instanceHandle,
                        sampleStates,
                        viewStates,
                        instanceStates);
            dataValues = (PublicationBuiltinTopicData[])objectValues;
            return result;
        }

        public ReturnCode TakeInstance(
            ref PublicationBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            InstanceHandle instanceHandle)
        {
            return TakeInstance(ref dataValues, ref sampleInfos, Length.Unlimited, instanceHandle);
        }

        public ReturnCode TakeInstance(
            ref PublicationBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle)
        {
            return TakeInstance(ref dataValues, ref sampleInfos, maxSamples, instanceHandle,
                SampleStateKind.Any, ViewStateKind.Any, InstanceStateKind.Any);
        }

        public ReturnCode TakeInstance(
                ref PublicationBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                InstanceHandle instanceHandle,
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates)
        {
            object[] objectValues = dataValues;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.TakeInstance(
                        this,
                        ref objectValues,
                        ref sampleInfos,
                        maxSamples,
                        instanceHandle,
                        sampleStates,
                        viewStates,
                        instanceStates);
            dataValues = (PublicationBuiltinTopicData[])objectValues;
            return result;
        }

        public ReturnCode ReadNextInstance(
            ref PublicationBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            InstanceHandle instanceHandle)
        {
            return ReadNextInstance(ref dataValues, ref sampleInfos, Length.Unlimited, instanceHandle);
        }

        public ReturnCode ReadNextInstance(
            ref PublicationBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle)
        {
            return ReadNextInstance(ref dataValues, ref sampleInfos, maxSamples, instanceHandle,
                SampleStateKind.Any, ViewStateKind.Any, InstanceStateKind.Any);
        }

        public ReturnCode ReadNextInstance(
                ref PublicationBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                InstanceHandle instanceHandle,
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates)
        {
            object[] objectValues = dataValues;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.ReadNextInstance(
                        this,
                        ref objectValues,
                        ref sampleInfos,
                        maxSamples,
                        instanceHandle,
                        sampleStates,
                        viewStates,
                        instanceStates);
            dataValues = (PublicationBuiltinTopicData[])objectValues;
            return result;
        }

        public ReturnCode TakeNextInstance(
            ref PublicationBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            InstanceHandle instanceHandle)
        {
            return TakeNextInstance(ref dataValues, ref sampleInfos, Length.Unlimited, instanceHandle);
        }

        public ReturnCode TakeNextInstance(
            ref PublicationBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle)
        {
            return TakeNextInstance(ref dataValues, ref sampleInfos, maxSamples, instanceHandle,
                SampleStateKind.Any, ViewStateKind.Any, InstanceStateKind.Any);
        }

        public ReturnCode TakeNextInstance(
                ref PublicationBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                InstanceHandle instanceHandle,
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates)
        {
            object[] objectValues = dataValues;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.TakeNextInstance(
                        this,
                        ref objectValues,
                        ref sampleInfos,
                        maxSamples,
                        instanceHandle,
                        sampleStates,
                        viewStates,
                        instanceStates);
            dataValues = (PublicationBuiltinTopicData[])objectValues;
            return result;
        }

        public ReturnCode ReadNextInstanceWithCondition(
                ref PublicationBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                InstanceHandle instanceHandle,
                IReadCondition readCondition)
        {
            return ReadNextInstanceWithCondition(
                ref dataValues,
                ref sampleInfos,
                Length.Unlimited,
                instanceHandle,
                readCondition);
        }

        public ReturnCode ReadNextInstanceWithCondition(
                ref PublicationBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                InstanceHandle instanceHandle,
                IReadCondition readCondition)
        {
            object[] objectValues = dataValues;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.ReadNextInstanceWithCondition(
                        this,
                        ref objectValues,
                        ref sampleInfos,
                        maxSamples,
                        instanceHandle,
                        readCondition);
            dataValues = (PublicationBuiltinTopicData[])objectValues;
            return result;
        }

        public ReturnCode TakeNextInstanceWithCondition(
                ref PublicationBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                InstanceHandle instanceHandle,
                IReadCondition readCondition)
        {
            return TakeNextInstanceWithCondition(
                ref dataValues,
                ref sampleInfos,
                Length.Unlimited,
                instanceHandle,
                readCondition);
        }

        public ReturnCode TakeNextInstanceWithCondition(
                ref PublicationBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                InstanceHandle instanceHandle,
                IReadCondition readCondition)
        {
            object[] objectValues = dataValues;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.TakeNextInstanceWithCondition(
                        this,
                        ref objectValues,
                        ref sampleInfos,
                        maxSamples,
                        instanceHandle,
                        readCondition);

            dataValues = (PublicationBuiltinTopicData[])objectValues;
            return result;
        }

        public ReturnCode ReturnLoan(
                ref PublicationBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos)
        {
            ReturnCode result;

            if (dataValues != null && sampleInfos != null)
            {
                if (dataValues != null && sampleInfos != null)
                {
                    if (dataValues.Length == sampleInfos.Length)
                    {
                        dataValues = null;
                        sampleInfos = null;
                        result = ReturnCode.Ok;
                    }
                    else
                    {
                        result = ReturnCode.PreconditionNotMet;
                    }
                }
                else
                {
                    if ((dataValues == null) && (sampleInfos == null))
                    {
                        result = ReturnCode.Ok;
                    }
                    else
                    {
                        result = ReturnCode.PreconditionNotMet;
                    }
                }
            }
            else
            {
                result = ReturnCode.BadParameter;
            }

            return result;
        }

        public ReturnCode GetKeyValue(
                PublicationBuiltinTopicData key,
                InstanceHandle handle)
        {
            return
                DDS.OpenSplice.FooDataReader.GetKeyValue(
                        this,
                        key,
                        handle);
        }

        public InstanceHandle LookupInstance(
                PublicationBuiltinTopicData instance)
        {
            return
                DDS.OpenSplice.FooDataReader.LookupInstance(
                        this,
                        instance);
        }

    }
    #endregion
    
    #region PublicationBuiltinTopicDataDataWriter
    public class PublicationBuiltinTopicDataDataWriter : DDS.OpenSplice.DataWriter, IPublicationBuiltinTopicDataDataWriter
    {
        public PublicationBuiltinTopicDataDataWriter(IntPtr gapiPtr)
            : base(gapiPtr)
        { /* empty */ }

        public InstanceHandle RegisterInstance(
                PublicationBuiltinTopicData instanceData)
        {
            return DDS.OpenSplice.FooDataWriter.RegisterInstance(
                        this,
                        instanceData);
        }

        public InstanceHandle RegisterInstanceWithTimestamp(
                PublicationBuiltinTopicData instanceData,
                Time sourceTimestamp)
        {
            return DDS.OpenSplice.FooDataWriter.RegisterInstanceWithTimestamp(
                        this,
                        instanceData,
                        sourceTimestamp);
        }

        public ReturnCode UnregisterInstance(
                PublicationBuiltinTopicData instanceData,
                InstanceHandle instanceHandle)
        {
            return DDS.OpenSplice.FooDataWriter.UnregisterInstance(
                        this,
                        instanceData,
                        instanceHandle);
        }

        public ReturnCode UnregisterInstanceWithTimestamp(
                PublicationBuiltinTopicData instanceData,
                InstanceHandle instanceHandle,
                Time sourceTimestamp)
        {
            return DDS.OpenSplice.FooDataWriter.UnregisterInstanceWithTimestamp(
                        this,
                        instanceData,
                        instanceHandle,
                        sourceTimestamp);
        }

        public ReturnCode Write(PublicationBuiltinTopicData instanceData)
        {
            return Write(instanceData, InstanceHandle.Nil);
        }

        public ReturnCode Write(
                PublicationBuiltinTopicData instanceData,
                InstanceHandle instanceHandle)
        {
            return DDS.OpenSplice.FooDataWriter.Write(
                        this,
                        instanceData,
                        instanceHandle);
        }

        public ReturnCode WriteWithTimestamp(
                PublicationBuiltinTopicData instanceData,
                Time sourceTimestamp)
        {
            return WriteWithTimestamp(instanceData, InstanceHandle.Nil, sourceTimestamp);
        }

        public ReturnCode WriteWithTimestamp(
                PublicationBuiltinTopicData instanceData,
                InstanceHandle instanceHandle,
                Time sourceTimestamp)
        {
            return DDS.OpenSplice.FooDataWriter.WriteWithTimestamp(
                        this,
                        instanceData,
                        instanceHandle,
                        sourceTimestamp);
        }

        public ReturnCode Dispose(
                PublicationBuiltinTopicData instanceData,
                InstanceHandle instanceHandle)
        {
            return DDS.OpenSplice.FooDataWriter.Dispose(
                        this,
                        instanceData,
                        instanceHandle);
        }

        public ReturnCode DisposeWithTimestamp(
                PublicationBuiltinTopicData instanceData,
                InstanceHandle instanceHandle,
                Time sourceTimestamp)
        {
            return DDS.OpenSplice.FooDataWriter.DisposeWithTimestamp(
                        this,
                        instanceData,
                        instanceHandle,
                        sourceTimestamp);
        }

        public ReturnCode WriteDispose(
                PublicationBuiltinTopicData instanceData)
        {
            return WriteDispose(instanceData, InstanceHandle.Nil);
        }

        public ReturnCode WriteDispose(
                PublicationBuiltinTopicData instanceData,
                InstanceHandle instanceHandle)
        {
            return DDS.OpenSplice.FooDataWriter.WriteDispose(
                        this,
                        instanceData,
                        instanceHandle);
        }

        public ReturnCode WriteDisposeWithTimestamp(
                PublicationBuiltinTopicData instanceData,
                Time sourceTimestamp)
        {
            return WriteDisposeWithTimestamp(instanceData, InstanceHandle.Nil, sourceTimestamp);
        }

        public ReturnCode WriteDisposeWithTimestamp(
                PublicationBuiltinTopicData instanceData,
                InstanceHandle instanceHandle,
                Time sourceTimestamp)
        {
            return DDS.OpenSplice.FooDataWriter.WriteDisposeWithTimestamp(
                        this,
                        instanceData,
                        instanceHandle,
                        sourceTimestamp);
        }

        public ReturnCode GetKeyValue(
                PublicationBuiltinTopicData key,
                InstanceHandle instanceHandle)
        {
            ReturnCode result = DDS.OpenSplice.FooDataWriter.GetKeyValue(
                                    this,
                                    key,
                                    instanceHandle);
            return result;
        }

        public InstanceHandle LookupInstance(
            PublicationBuiltinTopicData instanceData)
        {
            InstanceHandle result = DDS.OpenSplice.FooDataWriter.LookupInstance(
                                        this,
                                        instanceData);
            return result;
        }
    }
    #endregion

    #region PublicationBuiltinTopicDataTypeSupport
    public class PublicationBuiltinTopicDataTypeSupport : DDS.OpenSplice.TypeSupport
    {
        private const string typeName = "DDS::PublicationBuiltinTopicData";
        private const string keyList = "key";
        private const string metaDescriptor = "<MetaData version=\"1.0.0\"><Module name=\"DDS\"><TypeDef name=\"BuiltinTopicKey_t\"><Array size=\"3\"><Long/></Array></TypeDef><Enum name=\"DurabilityQosPolicyKind\"><Element name=\"VOLATILE_DURABILITY_QOS\"/><Element name=\"TRANSIENT_LOCAL_DURABILITY_QOS\"/><Element name=\"TRANSIENT_DURABILITY_QOS\"/><Element name=\"PERSISTENT_DURABILITY_QOS\"/></Enum><Struct name=\"DurabilityQosPolicy\"><Member name=\"kind\"><Type name=\"DDS::DurabilityQosPolicyKind\"/></Member></Struct><Struct name=\"Duration_t\"><Member name=\"sec\"><Long/></Member><Member name=\"nanosec\"><ULong/></Member></Struct><Struct name=\"DeadlineQosPolicy\"><Member name=\"period\"><Type name=\"DDS::Duration_t\"/></Member></Struct><Struct name=\"LatencyBudgetQosPolicy\"><Member name=\"duration\"><Type name=\"DDS::Duration_t\"/></Member></Struct><Enum name=\"LivelinessQosPolicyKind\"><Element name=\"AUTOMATIC_LIVELINESS_QOS\"/><Element name=\"MANUAL_BY_PARTICIPANT_LIVELINESS_QOS\"/><Element name=\"MANUAL_BY_TOPIC_LIVELINESS_QOS\"/></Enum><Struct name=\"LivelinessQosPolicy\"><Member name=\"kind\"><Type name=\"DDS::LivelinessQosPolicyKind\"/></Member><Member name=\"lease_duration\"><Type name=\"DDS::Duration_t\"/></Member></Struct><Enum name=\"ReliabilityQosPolicyKind\"><Element name=\"BEST_EFFORT_RELIABILITY_QOS\"/><Element name=\"RELIABLE_RELIABILITY_QOS\"/></Enum><Struct name=\"ReliabilityQosPolicy\"><Member name=\"kind\"><Type name=\"DDS::ReliabilityQosPolicyKind\"/></Member><Member name=\"max_blocking_time\"><Type name=\"DDS::Duration_t\"/></Member></Struct><Struct name=\"LifespanQosPolicy\"><Member name=\"duration\"><Type name=\"DDS::Duration_t\"/></Member></Struct><Struct name=\"UserDataQosPolicy\"><Member name=\"value\"><Sequence><Octet/></Sequence></Member></Struct><Enum name=\"OwnershipQosPolicyKind\"><Element name=\"SHARED_OWNERSHIP_QOS\"/><Element name=\"EXCLUSIVE_OWNERSHIP_QOS\"/></Enum><Struct name=\"OwnershipQosPolicy\"><Member name=\"kind\"><Type name=\"DDS::OwnershipQosPolicyKind\"/></Member></Struct><Struct name=\"OwnershipStrengthQosPolicy\"><Member name=\"value\"><Long/></Member></Struct><Enum name=\"PresentationQosPolicyAccessScopeKind\"><Element name=\"INSTANCE_PRESENTATION_QOS\"/><Element name=\"TOPIC_PRESENTATION_QOS\"/><Element name=\"GROUP_PRESENTATION_QOS\"/></Enum><Struct name=\"PresentationQosPolicy\"><Member name=\"access_scope\"><Type name=\"DDS::PresentationQosPolicyAccessScopeKind\"/></Member><Member name=\"coherent_access\"><Boolean/></Member><Member name=\"ordered_access\"><Boolean/></Member></Struct><TypeDef name=\"StringSeq\"><Sequence><String/></Sequence></TypeDef><Struct name=\"PartitionQosPolicy\"><Member name=\"name\"><Type name=\"DDS::StringSeq\"/></Member></Struct><Struct name=\"TopicDataQosPolicy\"><Member name=\"value\"><Sequence><Octet/></Sequence></Member></Struct><Struct name=\"GroupDataQosPolicy\"><Member name=\"value\"><Sequence><Octet/></Sequence></Member></Struct><Struct name=\"PublicationBuiltinTopicData\"><Member name=\"key\"><Type name=\"DDS::BuiltinTopicKey_t\"/></Member><Member name=\"participant_key\"><Type name=\"DDS::BuiltinTopicKey_t\"/></Member><Member name=\"topic_name\"><String/></Member><Member name=\"type_name\"><String/></Member><Member name=\"durability\"><Type name=\"DDS::DurabilityQosPolicy\"/></Member><Member name=\"deadline\"><Type name=\"DDS::DeadlineQosPolicy\"/></Member><Member name=\"latency_budget\"><Type name=\"DDS::LatencyBudgetQosPolicy\"/></Member><Member name=\"liveliness\"><Type name=\"DDS::LivelinessQosPolicy\"/></Member><Member name=\"reliability\"><Type name=\"DDS::ReliabilityQosPolicy\"/></Member><Member name=\"lifespan\"><Type name=\"DDS::LifespanQosPolicy\"/></Member><Member name=\"user_data\"><Type name=\"DDS::UserDataQosPolicy\"/></Member><Member name=\"ownership\"><Type name=\"DDS::OwnershipQosPolicy\"/></Member><Member name=\"ownership_strength\"><Type name=\"DDS::OwnershipStrengthQosPolicy\"/></Member><Member name=\"presentation\"><Type name=\"DDS::PresentationQosPolicy\"/></Member><Member name=\"partition\"><Type name=\"DDS::PartitionQosPolicy\"/></Member><Member name=\"topic_data\"><Type name=\"DDS::TopicDataQosPolicy\"/></Member><Member name=\"group_data\"><Type name=\"DDS::GroupDataQosPolicy\"/></Member></Struct></Module></MetaData>";

        public PublicationBuiltinTopicDataTypeSupport()
            : base(typeof(PublicationBuiltinTopicData), new MarshalerTypeGenerator())
        { }

        public PublicationBuiltinTopicDataTypeSupport(
                IMarshalerTypeGenerator generator)
            : base(typeof(PublicationBuiltinTopicData), generator)
        { }

        public PublicationBuiltinTopicDataTypeSupport(
                DatabaseMarshaler marshaler)
            : base(typeof(PublicationBuiltinTopicData), marshaler)
        { }

        public override string TypeName
        {
            get
            {
                return typeName;
            }
        }

        public override string Description
        {
            get
            {
                return metaDescriptor;
            }
        }

        public override string KeyList
        {
            get
            {
                return keyList;
            }
        }

        public override DDS.OpenSplice.DataWriter CreateDataWriter(IntPtr gapiPtr)
        {
            return new PublicationBuiltinTopicDataDataWriter(gapiPtr);
        }

        public override DDS.OpenSplice.DataReader CreateDataReader(IntPtr gapiPtr)
        {
            return new PublicationBuiltinTopicDataDataReader(gapiPtr);
        }
    }
    #endregion
    #region SubscriptionBuiltinTopicDataDataReader
    public class SubscriptionBuiltinTopicDataDataReader : DDS.OpenSplice.DataReader, ISubscriptionBuiltinTopicDataDataReader
    {
        public SubscriptionBuiltinTopicDataDataReader(IntPtr gapiPtr)
            : base(gapiPtr)
        { /* empty */ }

        public ReturnCode Read(
            ref SubscriptionBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos)
        {
            return Read(ref dataValues, ref sampleInfos, Length.Unlimited, SampleStateKind.Any,
                ViewStateKind.Any, InstanceStateKind.Any);
        }

        public ReturnCode Read(
            ref SubscriptionBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples)
        {
            return Read(ref dataValues, ref sampleInfos, maxSamples, SampleStateKind.Any,
                ViewStateKind.Any, InstanceStateKind.Any);
        }

        public ReturnCode Read(
            ref SubscriptionBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            SampleStateKind sampleStates,
            ViewStateKind viewStates,
            InstanceStateKind instanceStates)
        {
            return Read(ref dataValues, ref sampleInfos, Length.Unlimited, sampleStates,
                viewStates, instanceStates);
        }

        public ReturnCode Read(
                ref SubscriptionBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates)
        {
            object[] objectValues = dataValues;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.Read(
                        this,
                        ref objectValues,
                        ref sampleInfos,
                        maxSamples,
                        sampleStates,
                        viewStates,
                        instanceStates);
            dataValues = (SubscriptionBuiltinTopicData[])objectValues;
            return result;
        }

        public ReturnCode Take(
            ref SubscriptionBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos)
        {
            return Take(ref dataValues, ref sampleInfos, Length.Unlimited);
        }

        public ReturnCode Take(
            ref SubscriptionBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples)
        {
            return Take(ref dataValues, ref sampleInfos, maxSamples, SampleStateKind.Any,
                ViewStateKind.Any, InstanceStateKind.Any);
        }

        public ReturnCode Take(
            ref SubscriptionBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            SampleStateKind sampleStates,
            ViewStateKind viewStates,
            InstanceStateKind instanceStates)
        {
            return Take(ref dataValues, ref sampleInfos, Length.Unlimited, sampleStates,
                viewStates, instanceStates);
        }

        public ReturnCode Take(
                ref SubscriptionBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates)
        {
            object[] objectValues = dataValues;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.Take(
                        this,
                        ref objectValues,
                        ref sampleInfos,
                        maxSamples,
                        sampleStates,
                        viewStates,
                        instanceStates);
            dataValues = (SubscriptionBuiltinTopicData[])objectValues;
            return result;
        }

        public ReturnCode ReadWithCondition(
            ref SubscriptionBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            IReadCondition readCondition)
        {
            return ReadWithCondition(ref dataValues, ref sampleInfos,
                Length.Unlimited, readCondition);
        }

        public ReturnCode ReadWithCondition(
                ref SubscriptionBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                IReadCondition readCondition)
        {
            object[] objectValues = dataValues;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.ReadWithCondition(
                        this,
                        ref objectValues,
                        ref sampleInfos,
                        maxSamples,
                        readCondition);
            dataValues = (SubscriptionBuiltinTopicData[])objectValues;
            return result;
        }

        public ReturnCode TakeWithCondition(
            ref SubscriptionBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            IReadCondition readCondition)
        {
            return TakeWithCondition(ref dataValues, ref sampleInfos,
                Length.Unlimited, readCondition);
        }

        public ReturnCode TakeWithCondition(
                ref SubscriptionBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                IReadCondition readCondition)
        {
            object[] objectValues = dataValues;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.TakeWithCondition(
                        this,
                        ref objectValues,
                        ref sampleInfos,
                        maxSamples,
                        readCondition);
            dataValues = (SubscriptionBuiltinTopicData[])objectValues;
            return result;
        }

        public ReturnCode ReadNextSample(
                SubscriptionBuiltinTopicData dataValue,
                SampleInfo sampleInfo)
        {
            object objectValues = dataValue;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.ReadNextSample(
                        this,
                        ref objectValues,
                        ref sampleInfo);
            dataValue = (SubscriptionBuiltinTopicData)objectValues;
            return result;
        }

        public ReturnCode TakeNextSample(
                SubscriptionBuiltinTopicData dataValue,
                SampleInfo sampleInfo)
        {
            object objectValues = dataValue;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.TakeNextSample(
                        this,
                        ref objectValues,
                        ref sampleInfo);
            dataValue = (SubscriptionBuiltinTopicData)objectValues;
            return result;
        }

        public ReturnCode ReadInstance(
            ref SubscriptionBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            InstanceHandle instanceHandle)
        {
            return ReadInstance(ref dataValues, ref sampleInfos, Length.Unlimited, instanceHandle);
        }

        public ReturnCode ReadInstance(
            ref SubscriptionBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle)
        {
            return ReadInstance(ref dataValues, ref sampleInfos, maxSamples, instanceHandle,
                SampleStateKind.Any, ViewStateKind.Any, InstanceStateKind.Any);
        }

        public ReturnCode ReadInstance(
                ref SubscriptionBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                InstanceHandle instanceHandle,
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates)
        {
            object[] objectValues = dataValues;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.ReadInstance(
                        this,
                        ref objectValues,
                        ref sampleInfos,
                        maxSamples,
                        instanceHandle,
                        sampleStates,
                        viewStates,
                        instanceStates);
            dataValues = (SubscriptionBuiltinTopicData[])objectValues;
            return result;
        }

        public ReturnCode TakeInstance(
            ref SubscriptionBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            InstanceHandle instanceHandle)
        {
            return TakeInstance(ref dataValues, ref sampleInfos, Length.Unlimited, instanceHandle);
        }

        public ReturnCode TakeInstance(
            ref SubscriptionBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle)
        {
            return TakeInstance(ref dataValues, ref sampleInfos, maxSamples, instanceHandle,
                SampleStateKind.Any, ViewStateKind.Any, InstanceStateKind.Any);
        }

        public ReturnCode TakeInstance(
                ref SubscriptionBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                InstanceHandle instanceHandle,
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates)
        {
            object[] objectValues = dataValues;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.TakeInstance(
                        this,
                        ref objectValues,
                        ref sampleInfos,
                        maxSamples,
                        instanceHandle,
                        sampleStates,
                        viewStates,
                        instanceStates);
            dataValues = (SubscriptionBuiltinTopicData[])objectValues;
            return result;
        }

        public ReturnCode ReadNextInstance(
            ref SubscriptionBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            InstanceHandle instanceHandle)
        {
            return ReadNextInstance(ref dataValues, ref sampleInfos, Length.Unlimited, instanceHandle);
        }

        public ReturnCode ReadNextInstance(
            ref SubscriptionBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle)
        {
            return ReadNextInstance(ref dataValues, ref sampleInfos, maxSamples, instanceHandle,
                SampleStateKind.Any, ViewStateKind.Any, InstanceStateKind.Any);
        }

        public ReturnCode ReadNextInstance(
                ref SubscriptionBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                InstanceHandle instanceHandle,
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates)
        {
            object[] objectValues = dataValues;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.ReadNextInstance(
                        this,
                        ref objectValues,
                        ref sampleInfos,
                        maxSamples,
                        instanceHandle,
                        sampleStates,
                        viewStates,
                        instanceStates);
            dataValues = (SubscriptionBuiltinTopicData[])objectValues;
            return result;
        }

        public ReturnCode TakeNextInstance(
            ref SubscriptionBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            InstanceHandle instanceHandle)
        {
            return TakeNextInstance(ref dataValues, ref sampleInfos, Length.Unlimited, instanceHandle);
        }

        public ReturnCode TakeNextInstance(
            ref SubscriptionBuiltinTopicData[] dataValues,
            ref SampleInfo[] sampleInfos,
            int maxSamples,
            InstanceHandle instanceHandle)
        {
            return TakeNextInstance(ref dataValues, ref sampleInfos, maxSamples, instanceHandle,
                SampleStateKind.Any, ViewStateKind.Any, InstanceStateKind.Any);
        }

        public ReturnCode TakeNextInstance(
                ref SubscriptionBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                InstanceHandle instanceHandle,
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates)
        {
            object[] objectValues = dataValues;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.TakeNextInstance(
                        this,
                        ref objectValues,
                        ref sampleInfos,
                        maxSamples,
                        instanceHandle,
                        sampleStates,
                        viewStates,
                        instanceStates);
            dataValues = (SubscriptionBuiltinTopicData[])objectValues;
            return result;
        }

        public ReturnCode ReadNextInstanceWithCondition(
                ref SubscriptionBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                InstanceHandle instanceHandle,
                IReadCondition readCondition)
        {
            return ReadNextInstanceWithCondition(
                ref dataValues,
                ref sampleInfos,
                Length.Unlimited,
                instanceHandle,
                readCondition);
        }

        public ReturnCode ReadNextInstanceWithCondition(
                ref SubscriptionBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                InstanceHandle instanceHandle,
                IReadCondition readCondition)
        {
            object[] objectValues = dataValues;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.ReadNextInstanceWithCondition(
                        this,
                        ref objectValues,
                        ref sampleInfos,
                        maxSamples,
                        instanceHandle,
                        readCondition);
            dataValues = (SubscriptionBuiltinTopicData[])objectValues;
            return result;
        }

        public ReturnCode TakeNextInstanceWithCondition(
                ref SubscriptionBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                InstanceHandle instanceHandle,
                IReadCondition readCondition)
        {
            return TakeNextInstanceWithCondition(
                ref dataValues,
                ref sampleInfos,
                Length.Unlimited,
                instanceHandle,
                readCondition);
        }

        public ReturnCode TakeNextInstanceWithCondition(
                ref SubscriptionBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                InstanceHandle instanceHandle,
                IReadCondition readCondition)
        {
            object[] objectValues = dataValues;
            ReturnCode result =
                DDS.OpenSplice.FooDataReader.TakeNextInstanceWithCondition(
                        this,
                        ref objectValues,
                        ref sampleInfos,
                        maxSamples,
                        instanceHandle,
                        readCondition);

            dataValues = (SubscriptionBuiltinTopicData[])objectValues;
            return result;
        }

        public ReturnCode ReturnLoan(
                ref SubscriptionBuiltinTopicData[] dataValues,
                ref SampleInfo[] sampleInfos)
        {
            ReturnCode result;

            if (dataValues != null && sampleInfos != null)
            {
                if (dataValues != null && sampleInfos != null)
                {
                    if (dataValues.Length == sampleInfos.Length)
                    {
                        dataValues = null;
                        sampleInfos = null;
                        result = ReturnCode.Ok;
                    }
                    else
                    {
                        result = ReturnCode.PreconditionNotMet;
                    }
                }
                else
                {
                    if ((dataValues == null) && (sampleInfos == null))
                    {
                        result = ReturnCode.Ok;
                    }
                    else
                    {
                        result = ReturnCode.PreconditionNotMet;
                    }
                }
            }
            else
            {
                result = ReturnCode.BadParameter;
            }

            return result;
        }

        public ReturnCode GetKeyValue(
                SubscriptionBuiltinTopicData key,
                InstanceHandle handle)
        {
            return
                DDS.OpenSplice.FooDataReader.GetKeyValue(
                        this,
                        key,
                        handle);
        }

        public InstanceHandle LookupInstance(
                SubscriptionBuiltinTopicData instance)
        {
            return
                DDS.OpenSplice.FooDataReader.LookupInstance(
                        this,
                        instance);
        }

    }
    #endregion
    
    #region SubscriptionBuiltinTopicDataDataWriter
    public class SubscriptionBuiltinTopicDataDataWriter : DDS.OpenSplice.DataWriter, ISubscriptionBuiltinTopicDataDataWriter
    {
        public SubscriptionBuiltinTopicDataDataWriter(IntPtr gapiPtr)
            : base(gapiPtr)
        { /* empty */ }

        public InstanceHandle RegisterInstance(
                SubscriptionBuiltinTopicData instanceData)
        {
            return DDS.OpenSplice.FooDataWriter.RegisterInstance(
                        this,
                        instanceData);
        }

        public InstanceHandle RegisterInstanceWithTimestamp(
                SubscriptionBuiltinTopicData instanceData,
                Time sourceTimestamp)
        {
            return DDS.OpenSplice.FooDataWriter.RegisterInstanceWithTimestamp(
                        this,
                        instanceData,
                        sourceTimestamp);
        }

        public ReturnCode UnregisterInstance(
                SubscriptionBuiltinTopicData instanceData,
                InstanceHandle instanceHandle)
        {
            return DDS.OpenSplice.FooDataWriter.UnregisterInstance(
                        this,
                        instanceData,
                        instanceHandle);
        }

        public ReturnCode UnregisterInstanceWithTimestamp(
                SubscriptionBuiltinTopicData instanceData,
                InstanceHandle instanceHandle,
                Time sourceTimestamp)
        {
            return DDS.OpenSplice.FooDataWriter.UnregisterInstanceWithTimestamp(
                        this,
                        instanceData,
                        instanceHandle,
                        sourceTimestamp);
        }

        public ReturnCode Write(SubscriptionBuiltinTopicData instanceData)
        {
            return Write(instanceData, InstanceHandle.Nil);
        }

        public ReturnCode Write(
                SubscriptionBuiltinTopicData instanceData,
                InstanceHandle instanceHandle)
        {
            return DDS.OpenSplice.FooDataWriter.Write(
                        this,
                        instanceData,
                        instanceHandle);
        }

        public ReturnCode WriteWithTimestamp(
                SubscriptionBuiltinTopicData instanceData,
                Time sourceTimestamp)
        {
            return WriteWithTimestamp(instanceData, InstanceHandle.Nil, sourceTimestamp);
        }

        public ReturnCode WriteWithTimestamp(
                SubscriptionBuiltinTopicData instanceData,
                InstanceHandle instanceHandle,
                Time sourceTimestamp)
        {
            return DDS.OpenSplice.FooDataWriter.WriteWithTimestamp(
                        this,
                        instanceData,
                        instanceHandle,
                        sourceTimestamp);
        }

        public ReturnCode Dispose(
                SubscriptionBuiltinTopicData instanceData,
                InstanceHandle instanceHandle)
        {
            return DDS.OpenSplice.FooDataWriter.Dispose(
                        this,
                        instanceData,
                        instanceHandle);
        }

        public ReturnCode DisposeWithTimestamp(
                SubscriptionBuiltinTopicData instanceData,
                InstanceHandle instanceHandle,
                Time sourceTimestamp)
        {
            return DDS.OpenSplice.FooDataWriter.DisposeWithTimestamp(
                        this,
                        instanceData,
                        instanceHandle,
                        sourceTimestamp);
        }

        public ReturnCode WriteDispose(
                SubscriptionBuiltinTopicData instanceData)
        {
            return WriteDispose(instanceData, InstanceHandle.Nil);
        }

        public ReturnCode WriteDispose(
                SubscriptionBuiltinTopicData instanceData,
                InstanceHandle instanceHandle)
        {
            return DDS.OpenSplice.FooDataWriter.WriteDispose(
                        this,
                        instanceData,
                        instanceHandle);
        }

        public ReturnCode WriteDisposeWithTimestamp(
                SubscriptionBuiltinTopicData instanceData,
                Time sourceTimestamp)
        {
            return WriteDisposeWithTimestamp(instanceData, InstanceHandle.Nil, sourceTimestamp);
        }

        public ReturnCode WriteDisposeWithTimestamp(
                SubscriptionBuiltinTopicData instanceData,
                InstanceHandle instanceHandle,
                Time sourceTimestamp)
        {
            return DDS.OpenSplice.FooDataWriter.WriteDisposeWithTimestamp(
                        this,
                        instanceData,
                        instanceHandle,
                        sourceTimestamp);
        }

        public ReturnCode GetKeyValue(
                SubscriptionBuiltinTopicData key,
                InstanceHandle instanceHandle)
        {
            ReturnCode result = DDS.OpenSplice.FooDataWriter.GetKeyValue(
                                    this,
                                    key,
                                    instanceHandle);
            return result;
        }

        public InstanceHandle LookupInstance(
            SubscriptionBuiltinTopicData instanceData)
        {
            InstanceHandle result = DDS.OpenSplice.FooDataWriter.LookupInstance(
                                        this,
                                        instanceData);
            return result;
        }
    }
    #endregion

    #region SubscriptionBuiltinTopicDataTypeSupport
    public class SubscriptionBuiltinTopicDataTypeSupport : DDS.OpenSplice.TypeSupport
    {
        private const string typeName = "DDS::SubscriptionBuiltinTopicData";
        private const string keyList = "key";
        private const string metaDescriptor = "<MetaData version=\"1.0.0\"><Module name=\"DDS\"><TypeDef name=\"BuiltinTopicKey_t\"><Array size=\"3\"><Long/></Array></TypeDef><Enum name=\"DurabilityQosPolicyKind\"><Element name=\"VOLATILE_DURABILITY_QOS\"/><Element name=\"TRANSIENT_LOCAL_DURABILITY_QOS\"/><Element name=\"TRANSIENT_DURABILITY_QOS\"/><Element name=\"PERSISTENT_DURABILITY_QOS\"/></Enum><Struct name=\"DurabilityQosPolicy\"><Member name=\"kind\"><Type name=\"DDS::DurabilityQosPolicyKind\"/></Member></Struct><Struct name=\"Duration_t\"><Member name=\"sec\"><Long/></Member><Member name=\"nanosec\"><ULong/></Member></Struct><Struct name=\"DeadlineQosPolicy\"><Member name=\"period\"><Type name=\"DDS::Duration_t\"/></Member></Struct><Struct name=\"LatencyBudgetQosPolicy\"><Member name=\"duration\"><Type name=\"DDS::Duration_t\"/></Member></Struct><Enum name=\"LivelinessQosPolicyKind\"><Element name=\"AUTOMATIC_LIVELINESS_QOS\"/><Element name=\"MANUAL_BY_PARTICIPANT_LIVELINESS_QOS\"/><Element name=\"MANUAL_BY_TOPIC_LIVELINESS_QOS\"/></Enum><Struct name=\"LivelinessQosPolicy\"><Member name=\"kind\"><Type name=\"DDS::LivelinessQosPolicyKind\"/></Member><Member name=\"lease_duration\"><Type name=\"DDS::Duration_t\"/></Member></Struct><Enum name=\"ReliabilityQosPolicyKind\"><Element name=\"BEST_EFFORT_RELIABILITY_QOS\"/><Element name=\"RELIABLE_RELIABILITY_QOS\"/></Enum><Struct name=\"ReliabilityQosPolicy\"><Member name=\"kind\"><Type name=\"DDS::ReliabilityQosPolicyKind\"/></Member><Member name=\"max_blocking_time\"><Type name=\"DDS::Duration_t\"/></Member></Struct><Enum name=\"OwnershipQosPolicyKind\"><Element name=\"SHARED_OWNERSHIP_QOS\"/><Element name=\"EXCLUSIVE_OWNERSHIP_QOS\"/></Enum><Struct name=\"OwnershipQosPolicy\"><Member name=\"kind\"><Type name=\"DDS::OwnershipQosPolicyKind\"/></Member></Struct><Enum name=\"DestinationOrderQosPolicyKind\"><Element name=\"BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS\"/><Element name=\"BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS\"/></Enum><Struct name=\"DestinationOrderQosPolicy\"><Member name=\"kind\"><Type name=\"DDS::DestinationOrderQosPolicyKind\"/></Member></Struct><Struct name=\"UserDataQosPolicy\"><Member name=\"value\"><Sequence><Octet/></Sequence></Member></Struct><Struct name=\"TimeBasedFilterQosPolicy\"><Member name=\"minimum_separation\"><Type name=\"DDS::Duration_t\"/></Member></Struct><Enum name=\"PresentationQosPolicyAccessScopeKind\"><Element name=\"INSTANCE_PRESENTATION_QOS\"/><Element name=\"TOPIC_PRESENTATION_QOS\"/><Element name=\"GROUP_PRESENTATION_QOS\"/></Enum><Struct name=\"PresentationQosPolicy\"><Member name=\"access_scope\"><Type name=\"DDS::PresentationQosPolicyAccessScopeKind\"/></Member><Member name=\"coherent_access\"><Boolean/></Member><Member name=\"ordered_access\"><Boolean/></Member></Struct><TypeDef name=\"StringSeq\"><Sequence><String/></Sequence></TypeDef><Struct name=\"PartitionQosPolicy\"><Member name=\"name\"><Type name=\"DDS::StringSeq\"/></Member></Struct><Struct name=\"TopicDataQosPolicy\"><Member name=\"value\"><Sequence><Octet/></Sequence></Member></Struct><Struct name=\"GroupDataQosPolicy\"><Member name=\"value\"><Sequence><Octet/></Sequence></Member></Struct><Struct name=\"SubscriptionBuiltinTopicData\"><Member name=\"key\"><Type name=\"DDS::BuiltinTopicKey_t\"/></Member><Member name=\"participant_key\"><Type name=\"DDS::BuiltinTopicKey_t\"/></Member><Member name=\"topic_name\"><String/></Member><Member name=\"type_name\"><String/></Member><Member name=\"durability\"><Type name=\"DDS::DurabilityQosPolicy\"/></Member><Member name=\"deadline\"><Type name=\"DDS::DeadlineQosPolicy\"/></Member><Member name=\"latency_budget\"><Type name=\"DDS::LatencyBudgetQosPolicy\"/></Member><Member name=\"liveliness\"><Type name=\"DDS::LivelinessQosPolicy\"/></Member><Member name=\"reliability\"><Type name=\"DDS::ReliabilityQosPolicy\"/></Member><Member name=\"ownership\"><Type name=\"DDS::OwnershipQosPolicy\"/></Member><Member name=\"destination_order\"><Type name=\"DDS::DestinationOrderQosPolicy\"/></Member><Member name=\"user_data\"><Type name=\"DDS::UserDataQosPolicy\"/></Member><Member name=\"time_based_filter\"><Type name=\"DDS::TimeBasedFilterQosPolicy\"/></Member><Member name=\"presentation\"><Type name=\"DDS::PresentationQosPolicy\"/></Member><Member name=\"partition\"><Type name=\"DDS::PartitionQosPolicy\"/></Member><Member name=\"topic_data\"><Type name=\"DDS::TopicDataQosPolicy\"/></Member><Member name=\"group_data\"><Type name=\"DDS::GroupDataQosPolicy\"/></Member></Struct></Module></MetaData>";

        public SubscriptionBuiltinTopicDataTypeSupport()
            : base(typeof(SubscriptionBuiltinTopicData),new MarshalerTypeGenerator())
        { }

        public SubscriptionBuiltinTopicDataTypeSupport(
                IMarshalerTypeGenerator generator)
            : base(typeof(SubscriptionBuiltinTopicData), generator)
        { }

        public SubscriptionBuiltinTopicDataTypeSupport(
                DatabaseMarshaler marshaler)
            : base(typeof(SubscriptionBuiltinTopicData), marshaler)
        { }

        public override string TypeName
        {
            get
            {
                return typeName;
            }
        }

        public override string Description
        {
            get
            {
                return metaDescriptor;
            }
        }

        public override string KeyList
        {
            get
            {
                return keyList;
            }
        }

        public override DDS.OpenSplice.DataWriter CreateDataWriter(IntPtr gapiPtr)
        {
            return new SubscriptionBuiltinTopicDataDataWriter(gapiPtr);
        }

        public override DDS.OpenSplice.DataReader CreateDataReader(IntPtr gapiPtr)
        {
            return new SubscriptionBuiltinTopicDataDataReader(gapiPtr);
        }
    }
    #endregion
}

