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
using System.Diagnostics;

namespace DDS.OpenSplice.OS
{
    // from os_report.h
    [StructLayout(LayoutKind.Sequential)]
    public struct ReportInfo
    {
        // char *reportContext;
        public IntPtr reportContext;
        // char *sourceLine;
        public IntPtr sourceLine;
        // char *callStack;
        public IntPtr callStack;
        // os_int32 reportCode;
        public int reportCode;
        // char *description;
        public IntPtr description;
    }

    static internal class Report
    {
        /*
         *     os_reportInfo *
         *     os_reportGetApiInfo(void);
         */
        [DllImport("ddskernel", EntryPoint = "os_reportGetApiInfo", CallingConvention = CallingConvention.Cdecl)]
        internal static extern IntPtr GetApiInfo();
    }
}