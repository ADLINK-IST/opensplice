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
using DDS.OpenSplice.Kernel;

namespace DDS.OpenSplice.User
{
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void observableActionFn(IntPtr p, IntPtr arg);

    public class Observable
    {
        /*
         *     void *
         *     u_observableGetUserData(
         *         const u_observable _this);
         */
        [DllImport("ddskernel", EntryPoint = "u_observableGetUserData", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr GetUserData (
            IntPtr _this);

        /*
         *     void *
         *     u_observableSetUserData(
         *         const u_observable _this,
         *         void *userData);
         */
        [DllImport("ddskernel", EntryPoint = "u_observableSetUserData", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr SetUserData (
            IntPtr _this,
            IntPtr userData);

        /*
         *     u_result
         *     u_observableAction(
         *         const u_observable _this,
         *         void (*action)(v_public p, void *arg),
         *         void *arg);
         */
        [DllImport("ddskernel", EntryPoint = "u_observableAction", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT Action (
            IntPtr _this,
            observableActionFn action,
            IntPtr arg);
    }
}