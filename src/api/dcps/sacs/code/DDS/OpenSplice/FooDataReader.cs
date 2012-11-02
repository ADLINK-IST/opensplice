// The OpenSplice DDS Community Edition project.
//
// Copyright (C) 2006 to 2011 PrismTech Limited and its licensees.
// Copyright (C) 2009  L-3 Communications / IS
// 
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License Version 3 dated 29 June 2007, as published by the
//  Free Software Foundation.
// 
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
// 
//  You should have received a copy of the GNU Lesser General Public
//  License along with OpenSplice DDS Community Edition; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

using System;
using System.Runtime.InteropServices;
using DDS.OpenSplice.CustomMarshalers;

namespace DDS.OpenSplice
{
    public static class FooDataReader // : DataReader
    {
        public static ReturnCode Read(
                DataReader reader, 
                ref object[] data, 
                ref SampleInfo[] sampleInfos, 
                int maxSamples,
                SampleStateKind sampleStates, 
                ViewStateKind viewStates, 
                InstanceStateKind instanceStates)
        {
            ReturnCode result = ReturnCode.Ok;
            using (DataReaderMarshaler marshaler = 
                    new DataReaderMarshaler(data, sampleInfos, ref maxSamples, ref result))
            {
                if (result == ReturnCode.Ok)
                {
                    result = Gapi.FooDataReader.read(
                            reader.GapiPeer,
                            marshaler.dataValuesPtr,
                            marshaler.sampleInfosPtr,
                            maxSamples,
                            sampleStates,
                            viewStates,
                            instanceStates);

                    marshaler.CopyOut(ref data, ref sampleInfos);
                }
            }
            return result;
        }

        public static ReturnCode Take(
                DataReader reader, 
                ref object[] data, 
                ref SampleInfo[] sampleInfos, 
                int maxSamples,
                SampleStateKind sampleStates, 
                ViewStateKind viewStates, 
                InstanceStateKind instanceStates)
        {
            ReturnCode result = ReturnCode.Ok;
            using (DataReaderMarshaler marshaler = 
                    new DataReaderMarshaler(data, sampleInfos, ref maxSamples, ref result))
            {
                if (result == ReturnCode.Ok)
                {

                    result = Gapi.FooDataReader.take(
                            reader.GapiPeer,
                            marshaler.dataValuesPtr,
                            marshaler.sampleInfosPtr,
                            maxSamples,
                            sampleStates,
                            viewStates,
                            instanceStates);

                    marshaler.CopyOut(ref data, ref sampleInfos);
                }
            }
            return result;
        }

        public static ReturnCode ReadWithCondition(
                DataReader reader, 
                ref object[] data, 
                ref SampleInfo[] sampleInfos,
                int maxSamples, 
                IReadCondition condition)
        {
            ReturnCode result = ReturnCode.Ok;
            using (DataReaderMarshaler marshaler = 
                    new DataReaderMarshaler(data, sampleInfos, ref maxSamples, ref result))
            {
                if (result == ReturnCode.Ok)
                {

                    result = Gapi.FooDataReader.read_w_condition(
                            reader.GapiPeer,
                            marshaler.dataValuesPtr,
                            marshaler.sampleInfosPtr,
                            maxSamples,
                            ((ReadCondition)condition).GapiPeer);

                    marshaler.CopyOut(ref data, ref sampleInfos);
                }
            }
            return result;
        }

        public static ReturnCode TakeWithCondition(
                DataReader reader, 
                ref object[] data, 
                ref SampleInfo[] sampleInfos,
                int maxSamples, 
                IReadCondition condition)
        {
            ReturnCode result = ReturnCode.Ok;
            using (DataReaderMarshaler marshaler = 
                    new DataReaderMarshaler(data, sampleInfos, ref maxSamples, ref result))
            {
                if (result == ReturnCode.Ok)
                {
                    result = Gapi.FooDataReader.take_w_condition(
                            reader.GapiPeer,
                            marshaler.dataValuesPtr,
                            marshaler.sampleInfosPtr,
                            maxSamples,
                            ((ReadCondition)condition).GapiPeer);

                    marshaler.CopyOut(ref data, ref sampleInfos);
                }
            }
            return result;
        }

        public static ReturnCode ReadNextSample(
                DataReader reader, 
                ref object data, 
                ref SampleInfo sampleInfo)
        {
        //    ReturnCode result = Gapi.FooDataReader.read_next_sample(
        //        reader.GapiPeer,
        //        ref data,
        //        ref sampleInfo);

            return ReturnCode.Unsupported;
        }

        public static ReturnCode TakeNextSample(
                DataReader reader, 
                ref object data, 
                ref SampleInfo sampleInfo)
        {
            //ReturnCode result = Gapi.FooDataReader.take_next_sample(
            //    reader.GapiPeer,
            //    ref data,
            //    ref sampleInfo);

            return ReturnCode.Unsupported;
        }

        public static ReturnCode ReadInstance(
                DataReader reader, 
                ref object[] data, 
                ref SampleInfo[] sampleInfos, 
                int maxSamples,
                InstanceHandle instanceHandle, 
                SampleStateKind sampleStates, 
                ViewStateKind viewStates, 
                InstanceStateKind instanceStates)
        {
            ReturnCode result = ReturnCode.Ok;
            using (DataReaderMarshaler marshaler = 
                    new DataReaderMarshaler(data, sampleInfos, ref maxSamples, ref result))
            {
                if (result == ReturnCode.Ok)
                {
                    result = Gapi.FooDataReader.read_instance(
                            reader.GapiPeer,
                            marshaler.dataValuesPtr,
                            marshaler.sampleInfosPtr,
                            maxSamples,
                            instanceHandle,
                            sampleStates,
                            viewStates,
                            instanceStates);

                    marshaler.CopyOut(ref data, ref sampleInfos);
                }
            }
            return result;
        }

        public static ReturnCode TakeInstance(
                DataReader reader, 
                ref object[] data, 
                ref SampleInfo[] sampleInfos, 
                int maxSamples,
                InstanceHandle instanceHandle, 
                SampleStateKind sampleStates, 
                ViewStateKind viewStates, 
                InstanceStateKind instanceStates)
        {
            ReturnCode result = ReturnCode.Ok;
            using (DataReaderMarshaler marshaler = 
                    new DataReaderMarshaler(data, sampleInfos, ref maxSamples, ref result))
            {
                if (result == ReturnCode.Ok)
                {
                    result = Gapi.FooDataReader.take_instance(
                            reader.GapiPeer,
                            marshaler.dataValuesPtr,
                            marshaler.sampleInfosPtr,
                            maxSamples,
                            instanceHandle,
                            sampleStates,
                            viewStates,
                            instanceStates);

                    marshaler.CopyOut(ref data, ref sampleInfos);
                }
            }
            return result;
        }

        public static ReturnCode ReadNextInstance(
                DataReader reader, 
                ref object[] data, 
                ref SampleInfo[] sampleInfos, 
                int maxSamples,
                InstanceHandle instanceHandle, 
                SampleStateKind sampleStates, 
                ViewStateKind viewStates, 
                InstanceStateKind instanceStates)
        {
            ReturnCode result = ReturnCode.Ok;
            using (DataReaderMarshaler marshaler = 
                    new DataReaderMarshaler(data, sampleInfos, ref maxSamples, ref result))
            {
                if (result == ReturnCode.Ok)
                {
                    result = Gapi.FooDataReader.read_next_instance(
                            reader.GapiPeer,
                            marshaler.dataValuesPtr,
                            marshaler.sampleInfosPtr,
                            maxSamples,
                            instanceHandle,
                            sampleStates,
                            viewStates,
                            instanceStates);

                    marshaler.CopyOut(ref data, ref sampleInfos);
                }
            }
            return result;
        }

        public static ReturnCode TakeNextInstance(
                DataReader reader, 
                ref object[] data, 
                ref SampleInfo[] sampleInfos, 
                int maxSamples,
                InstanceHandle instanceHandle, 
                SampleStateKind sampleStates, 
                ViewStateKind viewStates, 
                InstanceStateKind instanceStates)
        {
            ReturnCode result = ReturnCode.Ok;
            using (DataReaderMarshaler marshaler = 
                    new DataReaderMarshaler(data, sampleInfos, ref maxSamples, ref result))
            {
                if (result == ReturnCode.Ok)
                {
                    result = Gapi.FooDataReader.take_next_instance(
                        reader.GapiPeer,
                        marshaler.dataValuesPtr,
                        marshaler.sampleInfosPtr,
                        maxSamples,
                        instanceHandle,
                        sampleStates,
                        viewStates,
                        instanceStates);

                    marshaler.CopyOut(ref data, ref sampleInfos);
                }
            }
            return result;
        }

        public static ReturnCode ReadNextInstanceWithCondition(
                DataReader reader, 
                ref object[] data, 
                ref SampleInfo[] sampleInfos,
                int maxSamples, 
                InstanceHandle instanceHandle, 
                IReadCondition condition)
        {
            ReturnCode result = ReturnCode.Ok;
            using (DataReaderMarshaler marshaler = new 
                    DataReaderMarshaler(data, sampleInfos, ref maxSamples, ref result))
            {
                if (result == ReturnCode.Ok)
                {
                    result = Gapi.FooDataReader.read_next_instance_w_condition(
                            reader.GapiPeer,
                            marshaler.dataValuesPtr,
                            marshaler.sampleInfosPtr,
                            maxSamples,
                            instanceHandle,
                            ((ReadCondition)condition).GapiPeer);

                    marshaler.CopyOut(ref data, ref sampleInfos);
                }
            }
            return result;
        }

        public static ReturnCode TakeNextInstanceWithCondition(
                DataReader reader, 
                ref object[] data, 
                ref SampleInfo[] sampleInfos,
                int maxSamples, 
                InstanceHandle instanceHandle, 
                IReadCondition condition)
        {
            ReturnCode result = ReturnCode.Ok;
            using (DataReaderMarshaler marshaler = 
                    new DataReaderMarshaler(data, sampleInfos, ref maxSamples, ref result))
            {
                if (result == ReturnCode.Ok)
                {
                    result = Gapi.FooDataReader.take_next_instance_w_condition(
                        reader.GapiPeer,
                        marshaler.dataValuesPtr,
                        marshaler.sampleInfosPtr,
                        maxSamples,
                        instanceHandle,
                        ((ReadCondition)condition).GapiPeer);

                    marshaler.CopyOut(ref data, ref sampleInfos);
                }
            }
            return result;
        }

        public static ReturnCode ReturnLoan(
                DataReader reader, 
                ref object[] data, 
                ref SampleInfo[] sampleInfos)
        {
            GCHandle tmpGCHandleData = GCHandle.Alloc(data, GCHandleType.Normal);
            IntPtr dataValuesPtr = GCHandle.ToIntPtr(tmpGCHandleData);

            GCHandle tmpGCHandleInfo = GCHandle.Alloc(sampleInfos, GCHandleType.Normal);
            IntPtr sampleInfosPtr = GCHandle.ToIntPtr(tmpGCHandleInfo);

            ReturnCode result = Gapi.FooDataReader.return_loan(
                reader.GapiPeer,
                dataValuesPtr,
                sampleInfosPtr);

            tmpGCHandleData.Free();
            tmpGCHandleInfo.Free();

            return result;
        }

        public static bool IsLoan(
                DataReader reader, 
                ref object[] data, 
                ref SampleInfo[] sampleInfos)
        {
            GCHandle tmpGCHandleData = GCHandle.Alloc(data, GCHandleType.Normal);
            IntPtr dataValuesPtr = GCHandle.ToIntPtr(tmpGCHandleData);

            GCHandle tmpGCHandleInfo = GCHandle.Alloc(sampleInfos, GCHandleType.Normal);
            IntPtr sampleInfosPtr = GCHandle.ToIntPtr(tmpGCHandleInfo);

            byte result = Gapi.FooDataReader.is_loan(
                reader.GapiPeer,
                dataValuesPtr,
                sampleInfosPtr);

            tmpGCHandleData.Free();
            tmpGCHandleInfo.Free();

            return result != 0;
        }

        public static ReturnCode GetKeyValue(
                DataReader reader, 
                ref object key, 
                InstanceHandle instanceHandle)
        {
            GCHandle tmpGCHandleData = GCHandle.Alloc(key, GCHandleType.Normal);
            ReturnCode result =  Gapi.FooDataReader.get_key_value(
                    reader.GapiPeer,
                    GCHandle.ToIntPtr(tmpGCHandleData),
                    instanceHandle);
            tmpGCHandleData.Free();
            return result;
        }

        public static InstanceHandle LookupInstance(
                DataReader reader, 
                object instance)
        {
            GCHandle tmpGCHandleData = GCHandle.Alloc(instance, GCHandleType.Normal);            
            InstanceHandle handle = Gapi.FooDataReader.lookup_instance(
                    reader.GapiPeer,
                    GCHandle.ToIntPtr(tmpGCHandleData));
            tmpGCHandleData.Free();
            return handle;
        }
    }
}
