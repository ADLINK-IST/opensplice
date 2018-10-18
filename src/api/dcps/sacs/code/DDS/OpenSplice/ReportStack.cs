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

using DDS;
using DDS.OpenSplice.OS;
using System;
using System.Diagnostics;
using System.Runtime.InteropServices;

namespace DDS.OpenSplice
{
    //[SuppressUnmanagedCodeSecurityAttribute()]
    internal class ReportStack
    {
        private static string[] returnCodeNames = {
                "Warning",
                "Internal Error",
                "Unsupported Feature",
                "Bad Parameter",
                "Precondition Not Met",
                "Out of Resources",
                "Not Enabled",
                "Immutable Policy",
                "Inconsistent Policy",
                "Already Deleted",
                "Timeout",
                "No Data",
                "Illegal Operation"
        };

        private static string GetReturnCodePrefix(DDS.ReturnCode returnCode)
        {
            string prefix;
            if ((int) returnCode == -1) {
                prefix = "";
            } else {
                prefix = returnCodeNames[(int) returnCode] + ": ";
            }
            return prefix;
        }

        private static string getTypeName(StackFrame errorFrame)
        {
            System.Reflection.MethodBase method = errorFrame.GetMethod();
            int cut = -1;
            string name;

            if (method.DeclaringType.IsGenericTypeDefinition ||
                method.DeclaringType.IsGenericType)
            {
                cut = method.DeclaringType.Name.IndexOf('`');
            }

            if (cut == -1) {
                name = method.DeclaringType.Name;
            } else {
                name = method.DeclaringType.Name.Remove(cut);
            }

            return name;
        }

        private static string getContext(StackFrame errorFrame)
        {
            System.Reflection.MethodBase method = errorFrame.GetMethod();
            string context = getTypeName(errorFrame) + "." + method.Name;
            return context;
        }

        private static string getFileName(StackFrame errorFrame)
        {
            string fileName = errorFrame.GetFileName();

            // When no debugging symbols available, try to determine fileName based on originating Class.
            if (fileName == null) {
                fileName = getTypeName(errorFrame) + ".cs";
            }
            return fileName;
        }

        internal static void Start()
        {
            Start(null, 0, null, IntPtr.Zero);
        }

        internal static void Flush(SacsSuperClass obj, bool valid)
        {
            if (FlushRequired(valid) != 0) {
                StackFrame callStack = new StackFrame(1, true);
                int domainId = -1;
                if (obj != null) {
                    domainId = obj.MyDomainId;
                }
                Flush(valid, getContext(callStack), getFileName(callStack), callStack.GetFileLineNumber(), domainId);
            }
        }

        internal static void Report(DDS.ReturnCode returnCode, string description)
        {
            ReportType reportType = ReportType.OS_ERROR;
            StackFrame callStack = new StackFrame(1, true);

            Report(reportType,
                   getContext(callStack),
                   getFileName(callStack),
                   callStack.GetFileLineNumber(),
                   returnCode,
                   -1, true,
                   GetReturnCodePrefix(returnCode) + description);
        }

        internal static void Deprecated(string description)
        {
            ReportType reportType = ReportType.OS_API_INFO;
            StackFrame callStack = new StackFrame(1, true);

            Report(reportType,
                   getContext(callStack),
                   getFileName(callStack),
                   callStack.GetFileLineNumber(),
                   DDS.ReturnCode.Ok,
                   -1, true,
                   description);
        }

        internal static void Warning(string description)
        {
            ReportType reportType = ReportType.OS_WARNING;
            StackFrame callStack = new StackFrame(1, true);

            Report(reportType,
                   getContext(callStack),
                   getFileName(callStack),
                   callStack.GetFileLineNumber(),
                   DDS.ReturnCode.Ok,
                   -1, true,
                   description);
        }


        [DllImport("ddskernel", EntryPoint = "os_report_stack_open", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void Start(string fileName, int lineNumber, string signature, IntPtr userInfo);

        [DllImport("ddskernel", EntryPoint = "os_report_flush", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void Flush(bool valid, string reportContext, string fileName, int lineNumber, int domainId);

        [DllImport("ddskernel", EntryPoint = "os_report", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void Report(ReportType type, string reportContext, string fileName, int lineNo, DDS.ReturnCode reportCode, int domainId, bool stack, string description);

        [DllImport("ddskernel", EntryPoint = "os_report_status", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int FlushRequired(bool valid);
    }
}
