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

namespace DDS.OpenSplice.User
{
    /*
     * interface User {
     */
    static internal class u
    {
        /*
         *     u_result
         *     u_userInitialise (void)
         */
        [DllImport("ddskernel", EntryPoint = "u_userInitialise", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT userInitialise();

        /*
         *     u_domainId_t
         *     u_userGetDomainIdFromEnvUri (void)
         */
        [DllImport("ddskernel", EntryPoint = "u_userGetDomainIdFromEnvUri", CallingConvention = CallingConvention.Cdecl)]
        public static extern int GetDomainIdFromEnvUri();

        /*
         *     os_char *
         *     u_userGetProcessName (void)
         */
        [DllImport("ddskernel", EntryPoint = "u_userGetProcessName", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr userGetProcessName();

        /*
         *     os_timeW
         *     os_timeWGet (void)
         */
        [DllImport("ddskernel", EntryPoint = "os_timeWGet", CallingConvention = CallingConvention.Cdecl)]
        public static extern DDS.OpenSplice.Database.os_timeW timeWGet();

        public static uint DELETE_ENTITIES  = (1 << 0);
        public static uint BLOCK_OPERATIONS = (1 << 1);
        public static uint EXCEPTION        = (1 << 2);

        /*
         *     u_result
         *     u_userDetach (
         *         os_uint32 flags);
         */
        [DllImport("ddskernel", EntryPoint = "u_userDetach", CallingConvention = CallingConvention.Cdecl)]
		public static extern V_RESULT userDetach(uint flags);
    }
}
