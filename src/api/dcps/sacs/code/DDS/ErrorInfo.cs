/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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
using DDS.OpenSplice;
using System.Runtime.InteropServices;
using DDS.OpenSplice.CustomMarshalers;

namespace DDS
{
    public class ErrorInfo : SacsSuperClass
    {
        private bool valid = false;
        private string reportContext;
        private string sourceLine;
        private string callStack;
        private int reportCode;
        private string description;

        public ErrorInfo()
        {
            init();
        }

        internal ReturnCode init()
        {
            // Because there is no UserLayer ref to be stored, do NOT forward to SacsSuperClass.init()!!
            // Parent class is only needed for its WriteLock and ReadLock calls...
            return DDS.ReturnCode.Ok;
        }

        internal override ReturnCode wlReq_deinit()
        {
            // Because there is no UserLayer ref to be stored, do NOT forward to SacsSuperClass.wlReq_deinit()!!
            // Parent class is only needed for its WriteLock and ReadLock calls...
            return DDS.ReturnCode.Ok;
        }

        public ReturnCode Update()
        {
            ReturnCode result;

            IntPtr osInfoPtr = DDS.OpenSplice.OS.Report.GetApiInfo();
            if (osInfoPtr != IntPtr.Zero)
            {
                DDS.OpenSplice.OS.ReportInfo osInfo;
                Type osInfoType = typeof(DDS.OpenSplice.OS.ReportInfo);

                osInfo = (DDS.OpenSplice.OS.ReportInfo) Marshal.PtrToStructure(osInfoPtr, osInfoType);
                reportContext = BaseMarshaler.ReadString(osInfo.reportContext);
                sourceLine = BaseMarshaler.ReadString(osInfo.sourceLine);
                callStack = BaseMarshaler.ReadString(osInfo.callStack);
                reportCode = osInfo.reportCode;
                description = BaseMarshaler.ReadString(osInfo.description);
                valid = true;
                result = DDS.ReturnCode.Ok;
            }
            else
            {
                result = DDS.ReturnCode.NoData;
                valid = false;
            }
            return result;
        }

        public ReturnCode GetCode(out ErrorCode code)
        {
            ReturnCode result;

            if (valid)
            {
                code = (ErrorCode)this.reportCode;
                result = DDS.ReturnCode.Ok;
            }
            else
            {
                code = ErrorCode.Undefined;
                result = DDS.ReturnCode.NoData;
            }
            return result;
        }

        public ReturnCode GetCode(out ReturnCode code)
        {
            ReturnCode result;

            if (valid)
            {
                code = (DDS.ReturnCode)DDS.OpenSplice.Common.ErrorInfo.ReportCodeToCode(this.reportCode);
                result = DDS.ReturnCode.Ok;
            }
            else
            {
                code = ReturnCode.Error;
                result = DDS.ReturnCode.NoData;
            }
            return result;
        }

        public ReturnCode GetMessage(out string message)
        {
            ReturnCode result;

            if (valid)
            {
                message = this.description;
                result = DDS.ReturnCode.Ok;
            }
            else
            {
                message = null;
                result = DDS.ReturnCode.NoData;
            }
            return result;
        }

        public ReturnCode GetLocation(out string location)
        {
            ReturnCode result;

            if (valid)
            {
                location = this.reportContext;
                result = DDS.ReturnCode.Ok;
            }
            else
            {
                location = null;
                result = DDS.ReturnCode.NoData;
            }
            return result;
        }

        public ReturnCode GetSourceLine(out string sourceLine)
        {
            ReturnCode result;

            if (valid)
            {
                sourceLine = this.sourceLine;
                result = DDS.ReturnCode.Ok;
            }
            else
            {
                sourceLine = null;
                result = DDS.ReturnCode.NoData;
            }
            return result;
        }

        public ReturnCode GetStackTrace(out string stackTrace)
        {
            ReturnCode result;

            if (valid)
            {
                stackTrace = this.callStack;
                result = DDS.ReturnCode.Ok;
            }
            else
            {
                stackTrace = null;
                result = DDS.ReturnCode.NoData;
            }
            return result;
        }
    }
}
