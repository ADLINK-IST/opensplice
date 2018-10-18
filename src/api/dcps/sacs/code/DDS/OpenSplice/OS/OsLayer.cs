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
using System.Runtime.InteropServices;
using System.Diagnostics;

namespace DDS.OpenSplice.OS
{
    internal enum ReportType {
        OS_DEBUG,
        OS_INFO,
        OS_WARNING,
        OS_API_INFO,
        OS_ERROR,
        OS_CRITICAL,
        OS_FATAL,
        OS_REPAIRED,
        OS_NONE
    }

    //[SuppressUnmanagedCodeSecurityAttribute()]
    static internal class os
    {
        /*
         *     void *
         *     os_malloc(
         *         os_size_t size);
         */
        [DllImport("ddskernel", EntryPoint = "os_malloc", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr malloc(IntPtr size);

        /*
         *     void
         *     os_free(
         *         void *ptr);
         */
        [DllImport("ddskernel", EntryPoint = "os_free", CallingConvention = CallingConvention.Cdecl)]
        public static extern void free(IntPtr ptr);

       /*
        *     void
        *         os_report();
        */
        [DllImport("ddskernel", EntryPoint = "os_report", CallingConvention = CallingConvention.Cdecl)]
        public static extern void report(
                ReportType type,
                string reportContext,
                string fileName,
                int lineNo,
                DDS.ReturnCode reportCode,
                int domainId,
                bool stack,
                string description,
                IntPtr args);

        public static void report(
                ReportType type,
                string reportContext,
                string fileName,
                DDS.ReturnCode reportCode,
                string description)
        {
            StackFrame callStack = new StackFrame(1, true);
            report( type,
                    reportContext,
                    fileName,
                    callStack.GetFileLineNumber(),
                    reportCode,
                    -1,
                    true,
                    description,
                    IntPtr.Zero);
        }
    }
}
