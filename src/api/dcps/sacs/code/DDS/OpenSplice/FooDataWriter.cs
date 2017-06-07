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

using DDS.OpenSplice.Kernel;
using DDS.OpenSplice.CustomMarshalers;

namespace DDS.OpenSplice
{
    public abstract class FooDataWriter<T, TMarshaler> : DataWriter
            where T : class
            where TMarshaler : FooDatabaseMarshaler<T>
    {
        private TMarshaler sampleMarshaler;

        public FooDataWriter(DatabaseMarshaler marshaler)
        {
            sampleMarshaler = marshaler as TMarshaler;
        }

        public virtual InstanceHandle RegisterInstance(
                T instanceData,
                Time sourceTimestamp)
        {
            ReturnCode result = DDS.ReturnCode.BadParameter;
            InstanceHandle handle = DDS.InstanceHandle.Nil;
            
            ReportStack.Start();
            if ((sourceTimestamp == Time.Current) ||
                (QosManager.countErrors(sourceTimestamp) == 0)) {
                long uHandle = handle;
                GCHandle tmpGCHandle = GCHandle.Alloc(instanceData, GCHandleType.Normal);
                result = uResultToReturnCode(
                        User.Writer.RegisterInstance(
                                rlReq_UserPeer,
                                sampleMarshaler.CopyIn,
                                GCHandle.ToIntPtr(tmpGCHandle),
                                sourceTimestamp.OsTimeW,
                                ref uHandle));
                handle = uHandle;
                tmpGCHandle.Free();
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return handle;
        }

        public virtual ReturnCode UnregisterInstance(
                T instanceData,
                InstanceHandle instanceHandle,
                Time sourceTimestamp)
        {
            ReturnCode result = DDS.ReturnCode.BadParameter;

            ReportStack.Start();
            if ((sourceTimestamp == Time.Current) ||
                (QosManager.countErrors(sourceTimestamp) == 0)) {
                GCHandle tmpGCHandle = GCHandle.Alloc(instanceData, GCHandleType.Normal);
                result = uResultToReturnCode(
                        User.Writer.UnregisterInstance(
                                rlReq_UserPeer,
                                sampleMarshaler.CopyIn,
                                GCHandle.ToIntPtr(tmpGCHandle),
                                sourceTimestamp.OsTimeW,
                                instanceHandle));
                tmpGCHandle.Free();
            }
            ReportStack.Flush(this, (result != ReturnCode.Ok) && (result != ReturnCode.Timeout));

            return result;
        }

        public virtual ReturnCode Write(
                T instanceData,
                InstanceHandle instanceHandle,
                Time sourceTimestamp)
        {
            ReturnCode result = DDS.ReturnCode.BadParameter;

            ReportStack.Start();
            if ((sourceTimestamp == Time.Current) ||
                (QosManager.countErrors(sourceTimestamp)) == 0) {
                GCHandle tmpGCHandle = GCHandle.Alloc(instanceData, GCHandleType.Normal);
                result = uResultToReturnCode(
                        User.Writer.Write(
                                rlReq_UserPeer,
                                sampleMarshaler.CopyIn,
                                GCHandle.ToIntPtr(tmpGCHandle),
                                sourceTimestamp.OsTimeW,
                                instanceHandle));
                tmpGCHandle.Free();
            }
            ReportStack.Flush(this, (result != ReturnCode.Ok) && (result != ReturnCode.Timeout));

            return result;
        }

        public virtual ReturnCode Dispose(
                T instanceData,
                InstanceHandle instanceHandle,
                Time sourceTimestamp)
        {
            ReturnCode result = DDS.ReturnCode.BadParameter;

            ReportStack.Start();
            if ((sourceTimestamp == Time.Current) ||
                (QosManager.countErrors(sourceTimestamp) == 0)) {
                GCHandle tmpGCHandle = GCHandle.Alloc(instanceData, GCHandleType.Normal);
                result = uResultToReturnCode(
                        User.Writer.Dispose(
                                rlReq_UserPeer,
                                sampleMarshaler.CopyIn,
                                GCHandle.ToIntPtr(tmpGCHandle),
                                sourceTimestamp.OsTimeW,
                                instanceHandle));
                tmpGCHandle.Free();
            }
            ReportStack.Flush(this, (result != ReturnCode.Ok) && (result != ReturnCode.Timeout));

            return result;
        }

        public virtual ReturnCode WriteDispose(
                T instanceData,
                InstanceHandle instanceHandle,
                Time sourceTimestamp)
        {
            ReturnCode result = DDS.ReturnCode.BadParameter;

            ReportStack.Start();
            if ((sourceTimestamp == Time.Current) ||
                (QosManager.countErrors(sourceTimestamp) == 0)) {
                GCHandle tmpGCHandle = GCHandle.Alloc(instanceData, GCHandleType.Normal);
                result = uResultToReturnCode(
                        User.Writer.WriteDispose(
                                rlReq_UserPeer,
                                sampleMarshaler.CopyIn,
                                GCHandle.ToIntPtr(tmpGCHandle),
                                sourceTimestamp.OsTimeW,
                                instanceHandle));
                tmpGCHandle.Free();
            }
            ReportStack.Flush(this, (result != ReturnCode.Ok) && (result != ReturnCode.Timeout));

            return result;
        }

        public virtual ReturnCode GetKeyValue(
                ref T key,
                InstanceHandle instanceHandle)
        {
            ReportStack.Start();
            GCHandle keyHandle = GCHandle.Alloc(key, GCHandleType.Normal);
            ReturnCode result = uResultToReturnCode(
                    User.Writer.CopyKeysFromInstanceHandle(
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
                T instanceData)
        {
            InstanceHandle handle = InstanceHandle.Nil;

            ReportStack.Start();
            long uHandle = handle;
            GCHandle keyHandle = GCHandle.Alloc(instanceData, GCHandleType.Normal);
            ReturnCode result = uResultToReturnCode(
                    User.Writer.LookupInstance(
                            rlReq_UserPeer,
                            sampleMarshaler.CopyIn,
                            GCHandle.ToIntPtr(keyHandle),
                            ref uHandle));
            handle = uHandle;
            keyHandle.Free();
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return handle;
        }
    }
}
