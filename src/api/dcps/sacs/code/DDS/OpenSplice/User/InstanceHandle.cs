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
using DDS.OpenSplice.kernelModule;

namespace DDS.OpenSplice.User
{
    /*
     * interface User {
     */
    static internal class InstanceHandle
    {
        /*
         *     u_instanceHandle
         *     u_instanceHandleNew(
         *         v_public object)
         */
        [DllImport("ddskernel", EntryPoint = "u_instanceHandleNew", CallingConvention = CallingConvention.Cdecl)]
        public static extern long New(IntPtr obj);

        /*
         *     u_instanceHandle
         *     u_instanceHandleFromGID(
         *         v_gid gid)
         */
        [DllImport("ddskernel", EntryPoint = "u_instanceHandleFromGID", CallingConvention = CallingConvention.Cdecl)]
        public static extern long FromGID(v_gid_s gid);

        /*
         *     v_gid
         *     u_instanceHandleToGID(
         *         u_instanceHandle handle)
         */
        [DllImport("ddskernel", EntryPoint = "u_instanceHandleToGID", CallingConvention = CallingConvention.Cdecl)]
        public static extern v_gid_s ToGID(long handle);
    }
}
