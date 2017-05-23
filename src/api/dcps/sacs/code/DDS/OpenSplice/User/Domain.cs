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
using DDS.OpenSplice.Database;

namespace DDS.OpenSplice.User
{
    internal static class Domain
    {
        /*
         *     u_result
         *     u_domain_load_xml_descriptor (
         *         const u_domain _this,
         *         const os_char *xml_descriptor;
         */
        [DllImport("ddskernel", EntryPoint = "u_domain_load_xml_descriptor", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT LoadXmlDescriptor(
            IntPtr _this,
            string xmlDescriptor);

        /*
         *     u_result
         *     u_domainCreatePersistentSnapshot(
         *         const u_domain _this,
         *         const os_char * partition_expression,
         *         const os_char * topic_expression,
         *         const os_char * uri);
         */
        [DllImport("ddskernel", EntryPoint = "u_domainCreatePersistentSnapshot", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT CreatePersistentSnapshot(
            IntPtr _this,
            string partitionExpression,
            string topicExpression,
            string uri);
    }
}
