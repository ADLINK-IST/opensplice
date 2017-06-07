/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

using System;
using System.Runtime.InteropServices;
using DDS.OpenSplice.CustomMarshalers;
using DDS.OpenSplice.Kernel;

namespace DDS.OpenSplice
{
    public struct SampleCopyArg<T>
    {
        public T[] samples;
        public SampleInfo[] sampleInfos;
        public uint index;
    }

    public abstract class FooDataReader<T, TMarshaler> : DataReader
            where T : class
            where TMarshaler : FooDatabaseMarshaler<T>
    {
        private TMarshaler sampleMarshaler;

        public FooDataReader(DatabaseMarshaler marshaler)
        {
            sampleMarshaler = marshaler as TMarshaler;
        }

        internal ReturnCode CheckReaderPreConditions(
                T[] data,
                SampleInfo[] sampleInfos,
                int maxSamples,
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates)
        {
            ReturnCode result;

            if (maxSamples >= Length.Unlimited)
            {
                if (DataReader.SampleStateMaskIsValid(sampleStates) &&
                    DataReader.ViewStateMaskIsValid(viewStates) &&
                    DataReader.InstanceStateMaskIsValid(instanceStates))
                {
                    if ((data == null && sampleInfos == null) ||
                            (data != null && sampleInfos != null && data.Length == sampleInfos.Length))
                    {
                        if (data == null || data.Length == 0 ||
                                maxSamples == DDS.Length.Unlimited || data.Length >= maxSamples)
                        {
                            result = DDS.ReturnCode.Ok;
                        }
                        else
                        {
                            result = DDS.ReturnCode.PreconditionNotMet;
                        }
                    }
                    else
                    {
                        result = DDS.ReturnCode.PreconditionNotMet;
                    }
                }
                else
                {
                    result = DDS.ReturnCode.BadParameter;
                }
            }
            else
            {
                result = DDS.ReturnCode.BadParameter;
            }
            return result;
        }

        internal ReturnCode wlReq_CheckReaderPreConditions(
                T[] data,
                SampleInfo[] sampleInfos,
                int maxSamples,
                ReadCondition condition)
        {
            ReturnCode result;

            if (maxSamples >= Length.Unlimited)
            {
                if (rlReq_ConditionList.Contains(condition))
                {
                    if ((data == null && sampleInfos == null) ||
                            (data != null && sampleInfos != null && data.Length == sampleInfos.Length))
                    {
                        if (data == null || data.Length == 0 || data.Length >= maxSamples || maxSamples == DDS.Length.Unlimited)
                        {
                            result = DDS.ReturnCode.Ok;
                        }
                        else
                        {
                            result = DDS.ReturnCode.PreconditionNotMet;
                        }
                    }
                    else
                    {
                        result = DDS.ReturnCode.PreconditionNotMet;
                    }
                }
                else
                {
                    result = DDS.ReturnCode.PreconditionNotMet;
                }
            }
            else
            {
                result = DDS.ReturnCode.BadParameter;
            }
            return result;
        }

        internal int RealMaxSamples(T[] data, int maxSamples)
        {
            int realMax;

            if (data != null && data.Length > 0 && maxSamples == DDS.Length.Unlimited)
            {
                realMax = data.Length;
            }
            else
            {
                realMax = maxSamples;
            }
            return realMax;
        }

        internal void CopySampleAndSampleInfo(
                IntPtr sample,
                IntPtr sampleInfo,
                IntPtr argPtr)
        {
            GCHandle argHandle = GCHandle.FromIntPtr(argPtr);
            SampleCopyArg<T> arg = (SampleCopyArg<T>) argHandle.Target;
            //object sampleElement = arg.samples[arg.index];
            //CopyOut(sample, ref sampleElement, 0);
            sampleMarshaler.CopyOut(sample, ref arg.samples[arg.index]);
            //arg.samples[arg.index] = sampleElement;
            if (arg.sampleInfos[arg.index] == null) arg.sampleInfos[arg.index] = new SampleInfo();
            SampleInfoMarshaler.CopyOut(sampleInfo, ref arg.sampleInfos[arg.index]);
            arg.index++;
            argHandle.Target = arg;
        }

        internal ReturnCode ReaderCopy(
                IntPtr sampleList,
                ref T[] data,
                ref SampleInfo[] sampleInfos)
        {
            T[] newData;
            SampleInfo[] newSampleInfos;
            ReturnCode result = DDS.ReturnCode.Ok;

            uint length = Common.SampleList.Length(sampleList);
            if (data != null && length == data.Length)
            {
                newData = data;
                newSampleInfos = sampleInfos;
            }
            else
            {
                newData = new T[length];
                newSampleInfos = new SampleInfo[length];
                if (data != null)
                {
                    for (uint i = 0; i < length && i < data.Length; i++)
                    {
                        newData[i] = data[i];
                        newSampleInfos[i] = sampleInfos[i];
                    }
                }
            }

            if (length > 0)
            {
                SampleCopyArg<T> arg = new SampleCopyArg<T>();
                arg.samples = newData;
                arg.sampleInfos = newSampleInfos;
                arg.index = 0;
                GCHandle argHandle = GCHandle.Alloc(arg, GCHandleType.Normal);

                result = uResultToReturnCode(User.Reader.ProtectCopyOutEnter(rlReq_UserPeer));
                if (result == DDS.ReturnCode.Ok) {
                    int r = Common.SampleList.Flush(sampleList, CopySampleAndSampleInfo, GCHandle.ToIntPtr(argHandle));
                    User.Reader.ProtectCopyOutExit(rlReq_UserPeer);
                    if (r < 0) {
                        result = DDS.ReturnCode.AlreadyDeleted;
                    }
                }
                data = arg.samples;
                sampleInfos = arg.sampleInfos;
                argHandle.Free();
            }
            else
            {
                // newData and newSampleInfos have already been set to arrays with 0 elements.
                data = newData;
                sampleInfos = newSampleInfos;
                result = DDS.ReturnCode.NoData;
            }
            return result;
        }

        public virtual ReturnCode Read(
                ref T[] data,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates)
        {
            ReturnCode result;

            ReportStack.Start();
            IntPtr sampleList = Common.SampleList.New(0);
            if (sampleList == IntPtr.Zero)
            {
                result = DDS.ReturnCode.OutOfResources;
                ReportStack.Report(result, "Unable to allocate sampleList.");
            }
            else
            {
                lock (this)
                {
                    if (this.rlReq_isAlive)
                    {
                        result = CheckReaderPreConditions(data, sampleInfos, maxSamples, sampleStates, viewStates, instanceStates);
                        if (result == DDS.ReturnCode.Ok)
                        {
                            uint mask = StateMask(sampleStates, viewStates, instanceStates);
                            int realMax = RealMaxSamples(data, maxSamples);
                            Common.SampleList.Reset(sampleList, realMax);
                            result = uResultToReturnCode(
                                    User.Reader.Read(rlReq_UserPeer, mask, Common.SampleList.ReaderAction, sampleList, DDS.Duration.Zero.OsDuration));
                        }
                    }
                    else
                    {
                        result = DDS.ReturnCode.AlreadyDeleted;
                    }
                }
                if (result == DDS.ReturnCode.Ok || result == ReturnCode.NoData)
                {
                    result = ReaderCopy(sampleList, ref data, ref sampleInfos);
                }
                
                Common.SampleList.Free(sampleList);
            }
            ReportStack.Flush(this, (result != ReturnCode.Ok)&&(result != ReturnCode.NoData));
            return result;
        }

        public virtual ReturnCode Take(
                ref T[] data,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates)
        {
            ReturnCode result;
            ReportStack.Start();
            IntPtr sampleList = Common.SampleList.New(0);
            if (sampleList == IntPtr.Zero)
            {
                result = DDS.ReturnCode.OutOfResources;
                ReportStack.Report(result, "Unable to allocate sampleList.");
            }
            else
            {
                lock (this)
                {
                    if (this.rlReq_isAlive)
                    {
                        result = CheckReaderPreConditions(data, sampleInfos, maxSamples, sampleStates, viewStates, instanceStates);
                        if (result == DDS.ReturnCode.Ok)
                        {
                            uint mask = StateMask(sampleStates, viewStates, instanceStates);
                            int realMax = RealMaxSamples(data, maxSamples);
                            Common.SampleList.Reset(sampleList, realMax);
                            result = uResultToReturnCode(
                                    User.Reader.Take(rlReq_UserPeer, mask, Common.SampleList.ReaderAction, sampleList, DDS.Duration.Zero.OsDuration));
                        }
                    }
                    else
                    {
                        result = DDS.ReturnCode.AlreadyDeleted;
                    }
                }
                if (result == DDS.ReturnCode.Ok || result == ReturnCode.NoData)
                {
                    result = ReaderCopy(sampleList, ref data, ref sampleInfos);
                }
                Common.SampleList.Free(sampleList);
            }
            ReportStack.Flush(this, (result != ReturnCode.Ok)&&(result != ReturnCode.NoData));
            return result;
        }

        public virtual ReturnCode ReadWithCondition (
                ref T[] data,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                IReadCondition condition)
        {
            ReturnCode result;
            ReadCondition rcObj = condition as ReadCondition;

            ReportStack.Start ();
            if (rcObj == null)
            {
                result = DDS.ReturnCode.BadParameter;
                ReportStack.Report(result, "condition is invalid (null), or not of type " +
                                            "DDS::OpenSplice::ReadCondition.");
            }
            else
            {
                IntPtr sampleList = Common.SampleList.New(0);
                if (sampleList == IntPtr.Zero)
                {
                    result = DDS.ReturnCode.OutOfResources;
                    ReportStack.Report(result, "Unable to allocate sampleList.");
                }
                else
                {
                    lock(this)
                    {
                        if (this.rlReq_isAlive)
                        {
                            result = wlReq_CheckReaderPreConditions(data, sampleInfos, maxSamples, rcObj);
                            if (result == DDS.ReturnCode.Ok)
                            {
                                int realMax = RealMaxSamples(data, maxSamples);
                                Common.SampleList.Reset(sampleList, realMax);
                                result = rcObj.Read(sampleList);
                            }
                        }
                        else
                        {
                            result = DDS.ReturnCode.AlreadyDeleted;
                        }
                    }
                    if (result == DDS.ReturnCode.Ok || result == ReturnCode.NoData)
                    {
                        result = ReaderCopy(sampleList, ref data, ref sampleInfos);
                    }
                    Common.SampleList.Free(sampleList);
                }
            }
            ReportStack.Flush(this, (result != ReturnCode.Ok)&&(result != ReturnCode.NoData));
            return result;
        }

        public virtual ReturnCode TakeWithCondition(
                ref T[] data,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                IReadCondition condition)
        {
            ReturnCode result;
            ReadCondition rcObj = condition as ReadCondition;

            ReportStack.Start();
            if (rcObj == null)
            {
                result = DDS.ReturnCode.BadParameter;
                ReportStack.Report(result, "condition is invalid (null), or not of type " +
                                   "DDS::OpenSplice::ReadCondition.");
            }
            else
            {
                IntPtr sampleList = Common.SampleList.New(0);
                if (sampleList == IntPtr.Zero)
                {
                    result = DDS.ReturnCode.OutOfResources;
                    ReportStack.Report(result, "Unable to allocate sampleList.");
                }
                else
                {
                    lock(this)
                    {
                        if (this.rlReq_isAlive)
                        {
                            result = wlReq_CheckReaderPreConditions(data, sampleInfos, maxSamples, rcObj);
                            if (result == DDS.ReturnCode.Ok)
                            {
                                int realMax = RealMaxSamples(data, maxSamples);
                                Common.SampleList.Reset(sampleList, realMax);
                                result = rcObj.Take(sampleList);
                            }
                        }
                        else
                        {
                            result = DDS.ReturnCode.AlreadyDeleted;
                        }
                    }
                    if (result == DDS.ReturnCode.Ok || result == ReturnCode.NoData)
                    {
                        result = ReaderCopy(sampleList, ref data, ref sampleInfos);
                    }
                    Common.SampleList.Free(sampleList);
                }
            }
            ReportStack.Flush(this, (result != ReturnCode.Ok)&&(result != ReturnCode.NoData));
            return result;
        }

        public virtual ReturnCode ReadNextSample(
                ref T data,
                ref SampleInfo sampleInfo)
        {
            return ReturnCode.Unsupported;
        }

        public virtual ReturnCode TakeNextSample(
                ref T data,
                ref SampleInfo sampleInfo)
        {
            return ReturnCode.Unsupported;
        }

        public virtual ReturnCode ReadInstance(
                ref T[] data,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                InstanceHandle instanceHandle,
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates)
        {
            ReturnCode result;

            ReportStack.Start();
            IntPtr sampleList = Common.SampleList.New(0);
            if (sampleList == IntPtr.Zero)
            {
                result = DDS.ReturnCode.OutOfResources;
                ReportStack.Report(result, "Unable to allocate sampleList.");
            }
            else
            {
                lock (this)
                {
                    if (this.rlReq_isAlive)
                    {
                        result = CheckReaderPreConditions(data, sampleInfos, maxSamples, sampleStates, viewStates, instanceStates);
                        if (result == DDS.ReturnCode.Ok)
                        {
                            uint mask = StateMask(sampleStates, viewStates, instanceStates);
                            int realMax = RealMaxSamples(data, maxSamples);
                            Common.SampleList.Reset(sampleList, realMax);
                            result = uResultToReturnCode(
                                    User.Reader.ReadInstance(
                                            rlReq_UserPeer,
                                            instanceHandle,
                                            mask,
                                            Common.SampleList.ReaderAction,
                                            sampleList, DDS.Duration.Zero.OsDuration));
                        }
                    }
                    else
                    {
                        result = DDS.ReturnCode.AlreadyDeleted;
                    }
                }
                if (result == DDS.ReturnCode.Ok || result == ReturnCode.NoData)
                {
                    result = ReaderCopy(sampleList, ref data, ref sampleInfos);
                }
                Common.SampleList.Free(sampleList);
            }
            ReportStack.Flush(this, (result != ReturnCode.Ok)&&(result != ReturnCode.NoData));
            return result;
        }

        public virtual ReturnCode TakeInstance(
                ref T[] data,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                InstanceHandle instanceHandle,
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates)
        {
            ReturnCode result;

            ReportStack.Start();
            IntPtr sampleList = Common.SampleList.New(0);
            if (sampleList == IntPtr.Zero)
            {
                result = DDS.ReturnCode.OutOfResources;
                ReportStack.Report(result, "Unable to allocate sampleList.");
            }
            else
            {
                lock (this)
                {
                    if (this.rlReq_isAlive)
                    {
                        result = CheckReaderPreConditions(data, sampleInfos, maxSamples, sampleStates, viewStates, instanceStates);
                        if (result == DDS.ReturnCode.Ok)
                        {
                            uint mask = StateMask(sampleStates, viewStates, instanceStates);
                            int realMax = RealMaxSamples(data, maxSamples);
                            Common.SampleList.Reset(sampleList, realMax);
                            result = uResultToReturnCode(
                                    User.Reader.TakeInstance(
                                            rlReq_UserPeer,
                                            instanceHandle,
                                            mask,
                                            Common.SampleList.ReaderAction,
                                            sampleList, DDS.Duration.Zero.OsDuration));
                        }
                    }
                    else
                    {
                        result = DDS.ReturnCode.AlreadyDeleted;
                    }
                }
                if (result == DDS.ReturnCode.Ok || result == ReturnCode.NoData)
                {
                    result = ReaderCopy(sampleList, ref data, ref sampleInfos);
                }
                Common.SampleList.Free(sampleList);
            }
            ReportStack.Flush(this, (result != ReturnCode.Ok)&&(result != ReturnCode.NoData));
            return result;
        }

        public virtual ReturnCode ReadNextInstance(
                ref T[] data,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                InstanceHandle instanceHandle,
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates)
        {
            ReturnCode result;

            ReportStack.Start();
            IntPtr sampleList = Common.SampleList.New(0);
            if (sampleList == IntPtr.Zero)
            {
                result = DDS.ReturnCode.OutOfResources;
                ReportStack.Report(result, "Unable to allocate sampleList.");
            }
            else
            {
                lock (this)
                {
                    if (this.rlReq_isAlive)
                    {
                        result = CheckReaderPreConditions(data, sampleInfos, maxSamples, sampleStates, viewStates, instanceStates);
                        if (result == DDS.ReturnCode.Ok)
                        {
                            uint mask = StateMask(sampleStates, viewStates, instanceStates);
                            int realMax = RealMaxSamples(data, maxSamples);
                            Common.SampleList.Reset(sampleList, realMax);
                            result = uResultToReturnCode(
                                    User.Reader.ReadNextInstance(
                                            rlReq_UserPeer,
                                            instanceHandle,
                                            mask,
                                            Common.SampleList.ReaderAction,
                                            sampleList, DDS.Duration.Zero.OsDuration));
                        }
                    }
                    else
                    {
                        result = DDS.ReturnCode.AlreadyDeleted;
                    }
                }
                if (result == DDS.ReturnCode.Ok || result == ReturnCode.NoData)
                {
                    result = ReaderCopy(sampleList, ref data, ref sampleInfos);
                }
                Common.SampleList.Free(sampleList);
            }
            ReportStack.Flush(this, (result != ReturnCode.Ok)&&(result != ReturnCode.NoData));
            return result;
        }

        public virtual ReturnCode TakeNextInstance(
                ref T[] data,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                InstanceHandle instanceHandle,
                SampleStateKind sampleStates,
                ViewStateKind viewStates,
                InstanceStateKind instanceStates)
        {
            ReturnCode result;

            ReportStack.Start();
            IntPtr sampleList = Common.SampleList.New(0);
            if (sampleList == IntPtr.Zero)
            {
                result = DDS.ReturnCode.OutOfResources;
                ReportStack.Report(result, "Unable to allocate sampleList.");
            }
            else
            {
                lock (this)
                {
                    if (this.rlReq_isAlive)
                    {
                        result = CheckReaderPreConditions(data, sampleInfos, maxSamples, sampleStates, viewStates, instanceStates);
                        if (result == DDS.ReturnCode.Ok)
                        {
                            uint mask = StateMask(sampleStates, viewStates, instanceStates);
                            int realMax = RealMaxSamples(data, maxSamples);
                            Common.SampleList.Reset(sampleList, realMax);
                            result = uResultToReturnCode(
                                    User.Reader.TakeNextInstance(
                                            rlReq_UserPeer,
                                            instanceHandle,
                                            mask,
                                            Common.SampleList.ReaderAction,
                                            sampleList, DDS.Duration.Zero.OsDuration));
                        }
                    }
                    else
                    {
                        result = DDS.ReturnCode.AlreadyDeleted;
                    }
                }
                if (result == DDS.ReturnCode.Ok || result == ReturnCode.NoData)
                {
                    result = ReaderCopy(sampleList, ref data, ref sampleInfos);
                }
                Common.SampleList.Free(sampleList);
            }
            ReportStack.Flush(this, (result != ReturnCode.Ok)&&(result != ReturnCode.NoData));
            return result;
        }

        public virtual ReturnCode ReadNextInstanceWithCondition(
                ref T[] data,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                InstanceHandle instanceHandle,
                IReadCondition condition)
        {
            ReturnCode result;
            ReadCondition rcObj = condition as ReadCondition;

            ReportStack.Start();
            if (rcObj == null)
            {
                result = DDS.ReturnCode.BadParameter;
                ReportStack.Report(result, "condition is invalid (null), or not of type " +
                                   "DDS::OpenSplice::ReadCondition.");
            }
            else
            {
                IntPtr sampleList = Common.SampleList.New(0);
                if (sampleList == IntPtr.Zero)
                {
                    result = DDS.ReturnCode.OutOfResources;
                    ReportStack.Report(result, "Unable to allocate sampleList.");
                }
                else
                {
                    lock(this)
                    {
                        if (this.rlReq_isAlive)
                        {
                            result = wlReq_CheckReaderPreConditions(data, sampleInfos, maxSamples, rcObj);
                            if (result == DDS.ReturnCode.Ok)
                            {
                                int realMax = RealMaxSamples(data, maxSamples);
                                Common.SampleList.Reset(sampleList, realMax);
                                result = rcObj.ReadNextInstance(instanceHandle, sampleList);
                            }
                        }
                        else
                        {
                            result = DDS.ReturnCode.AlreadyDeleted;
                        }
                    }
                    if (result == DDS.ReturnCode.Ok || result == ReturnCode.NoData)
                    {
                        result = ReaderCopy(sampleList, ref data, ref sampleInfos);
                    }
                    Common.SampleList.Free(sampleList);
                }
            }
            ReportStack.Flush(this, (result != ReturnCode.Ok)&&(result != ReturnCode.NoData));
            return result;
        }

        public virtual ReturnCode TakeNextInstanceWithCondition(
                ref T[] data,
                ref SampleInfo[] sampleInfos,
                int maxSamples,
                InstanceHandle instanceHandle,
                IReadCondition condition)
        {
            ReturnCode result;
            ReadCondition rcObj = condition as ReadCondition;

            ReportStack.Start();
            if (rcObj == null)
            {
                result = DDS.ReturnCode.BadParameter;
                ReportStack.Report(result, "condition is invalid (null), or not of type " +
                                   "DDS::OpenSplice::ReadCondition.");
            }
            else
            {
                IntPtr sampleList = Common.SampleList.New(0);
                if (sampleList == IntPtr.Zero)
                {
                    result = DDS.ReturnCode.OutOfResources;
                    ReportStack.Report(result, "Unable to allocate sampleList.");
                }
                else
                {
                    lock(this)
                    {
                        if (this.rlReq_isAlive)
                        {
                            result = wlReq_CheckReaderPreConditions(data, sampleInfos, maxSamples, rcObj);
                            if (result == DDS.ReturnCode.Ok)
                            {
                                int realMax = RealMaxSamples(data, maxSamples);
                                Common.SampleList.Reset(sampleList, realMax);
                                result = rcObj.TakeNextInstance(instanceHandle, sampleList);
                            }
                        }
                        else
                        {
                            result = DDS.ReturnCode.AlreadyDeleted;
                        }
                    }
                    if (result == DDS.ReturnCode.Ok || result == ReturnCode.NoData)
                    {
                        result = ReaderCopy(sampleList, ref data, ref sampleInfos);
                    }
                    Common.SampleList.Free(sampleList);
                }
            }
            ReportStack.Flush(this, (result != ReturnCode.Ok)&&(result != ReturnCode.NoData));
            return result;
        }

        public virtual ReturnCode ReturnLoan(
                ref T[] data,
                ref SampleInfo[] sampleInfos)
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;

            ReportStack.Start();
            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    if (data != null && sampleInfos != null) {
                        if (data.Length == sampleInfos.Length) {
                            data = null;
                            sampleInfos = null;
                            result = DDS.ReturnCode.Ok;
                        } else {
                            result = DDS.ReturnCode.PreconditionNotMet;
                            ReportStack.Report(result, "data and sampleInfo arrays have unequal length");
                        }
                    } else {
                        result = DDS.ReturnCode.PreconditionNotMet;
                        ReportStack.Report(result, "data and/or sampleInfo arrays may not be null");
                    }
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);
            return result;
        }

        public virtual ReturnCode GetKeyValue(
                ref T key,
                InstanceHandle instanceHandle)
        {
            ReportStack.Start();
            GCHandle keyHandle = GCHandle.Alloc(key, GCHandleType.Normal);
            ReturnCode result = uResultToReturnCode(
                    User.DataReader.CopyKeysFromInstanceHandle(
                            rlReq_UserPeer,
                            instanceHandle,
                            sampleMarshaler.CopyOut,
                            GCHandle.ToIntPtr(keyHandle)));
            key = keyHandle.Target as T;
            keyHandle.Free();
            ReportStack.Flush(this, result != ReturnCode.Ok);
            
            return result;
        }

        public virtual InstanceHandle LookupInstance(
                T instance)
        {
            ReportStack.Start();
            InstanceHandle handle = InstanceHandle.Nil;
            long uHandle = handle;
            GCHandle keyHandle = GCHandle.Alloc(instance, GCHandleType.Normal);
            ReturnCode result = uResultToReturnCode(
                    User.DataReader.LookupInstance(
                            rlReq_UserPeer,
                            GCHandle.ToIntPtr(keyHandle),
                            sampleMarshaler.CopyIn,
                            ref uHandle));
            handle = uHandle;
            keyHandle.Free();
            ReportStack.Flush(this, result != ReturnCode.Ok);
            
            return handle;
        }
    }
}
