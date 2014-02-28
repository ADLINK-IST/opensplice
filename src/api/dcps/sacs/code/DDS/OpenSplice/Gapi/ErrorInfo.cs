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
using System.Diagnostics;
using System.Runtime.InteropServices;

namespace DDS.OpenSplice
{
    enum ReportType {
        OS_INFO,
        OS_WARNING,
        OS_ERROR,
        OS_CRITICAL,
        OS_FATAL,
        OS_REPAIRED,
        OS_API_INFO
    }

    static internal class OS
    {
       /* void
        *     os_report();
        */
        [DllImport("ddskernel", EntryPoint = "os_report")]
        public static extern void Report(
                ReportType type,
                string reportContext,
                string fileName,
                int lineNo,
                DDS.ErrorCode reportCode,
                string description);

        public static void Report(
                ReportType type,
                string reportContext,
                string fileName,
                DDS.ErrorCode reportCode,
                string description)
        {
            StackFrame callStack = new StackFrame(1, true);
            Report( type, 
                    reportContext, 
                    fileName, 
                    callStack.GetFileLineNumber(),
                    reportCode,
                    description);
        }
    }    
    
    namespace Gapi
    {
        static internal class ErrorInfo
        {
            /*     ErrorInfo
             *     ErrorInfo__alloc (
             *         void);
             */
            [DllImport("ddskernel", EntryPoint = "gapi_errorInfo__alloc")]
            public static extern IntPtr alloc();

            /*     ReturnCode_t
             *     update( );
             */
            [DllImport("ddskernel", EntryPoint = "gapi_errorInfo_update")]
            public static extern ReturnCode update(
                IntPtr _this);

            /*     ReturnCode_t
             *     get_code(
             *         out ErrorCode code);
             */
            [DllImport("ddskernel", EntryPoint = "gapi_errorInfo_get_code")]
            public static extern ReturnCode get_code(
                IntPtr _this,
                out DDS.ErrorCode code);

            /*     ReturnCode_t
             *     get_location(
             *         out String location);
             */
            [DllImport("ddskernel", EntryPoint = "gapi_errorInfo_get_location")]
            public static extern ReturnCode get_location(
                IntPtr _this,
                out string location);

            /*     ReturnCode_t
             *     get_source_line(
             *         out String source_line);
             */
            [DllImport("ddskernel", EntryPoint = "gapi_errorInfo_get_source_line")]
            public static extern ReturnCode get_source_line(
                IntPtr _this,
                out string source_line);

            /*     ReturnCode_t
             *     get_stack_trace(
             *         out String stack_trace);
             */
            [DllImport("ddskernel", EntryPoint = "gapi_errorInfo_get_stack_trace")]
            public static extern ReturnCode get_stack_trace(
                IntPtr _this,
                out string stack_trace);

            /*     ReturnCode_t
             *     get_message(
             *         out String message);
             */
            [DllImport("ddskernel", EntryPoint = "gapi_errorInfo_get_message")]
            public static extern ReturnCode get_message(
                IntPtr _this,
                out string message);
        }
    }
}
