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
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void listenerActionFn(IntPtr e, IntPtr arg);

    /*
     * interface User {
     */
    static internal class Listener
    {
        /*
         *     u_listener
         *     u_listenerNew(
         *         const u_participant p)
         */
        [DllImport("ddskernel", EntryPoint = "u_listenerNew", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr New(IntPtr p);

        /*
         *     u_result
         *     u_listenerDeinit(
         *         u_object _this)
         */
        [DllImport("ddskernel", EntryPoint = "u_listenerDeinit", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT Deinit(IntPtr _this);

        /*
         *     u_result
         *     u_listenerNotify(
         *         const u_listener _this)
         */
        [DllImport("ddskernel", EntryPoint = "u_listenerNotify", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT Notify(IntPtr _this);

        /*
         *     u_result
         *     u_listenerTrigger(
         *         const u_listener _this)
         */
        [DllImport("ddskernel", EntryPoint = "u_listenerTrigger", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT Trigger(IntPtr _this);

        /*
         *     u_result
         *     u_listenerWait(
         *         const u_listener _this,
         *         u_listenerAction action,
         *         void *arg,
         *         os_duration timeout)
         */
        [DllImport("ddskernel", EntryPoint = "u_listenerWait", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT Wait(IntPtr _this, listenerActionFn action, IntPtr arg, long timeout);
    }
}
