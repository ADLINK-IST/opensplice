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
    internal delegate void waitsetActionFn(IntPtr e, IntPtr arg);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    internal delegate int waitsetActionFn2(IntPtr userData, IntPtr arg);

    static internal class WaitSet
    {
        /*
         *     u_waitset
         *     u_waitsetNew();
         */
        [DllImport("ddskernel", EntryPoint = "u_waitsetNew", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr New();

        /*
         *     u_waitset
         *     u_waitsetNew2();
         */
        [DllImport("ddskernel", EntryPoint = "u_waitsetNew2", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr New2();

        /*
         *     void
         *     u_waitsetAnnounceDestruction(
         *         const u_waitset _this);
         */
        [DllImport("ddskernel", EntryPoint = "u_waitsetAnnounceDestruction", CallingConvention = CallingConvention.Cdecl)]
        public static extern void AnnounceDestruction(
            IntPtr _this);

        /*
         *     u_result
         *     u_waitsetNotify(
         *         const u_waitset _this,
         *         void *eventArg);
         */
        [DllImport("ddskernel", EntryPoint = "u_waitsetNotify", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT Notify(
            IntPtr _this,
            IntPtr eventArg);

        /*
         *     u_result
         *     u_waitsetWaitAction (
         *         const u_waitset _this,
         *         u_waitsetAction action,
         *         void *arg,
         *         const os_duration timeout);
         */
        [DllImport("ddskernel", EntryPoint = "u_waitsetWaitAction", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT WaitAction(
            IntPtr _this,
            waitsetActionFn action,
            IntPtr arg,
            long timeout);

        /*
         *     u_result
         *     u_waitsetWaitAction2 (
         *         const u_waitset _this,
         *         u_waitsetAction2 action,
         *         void *arg,
         *         const os_duration timeout);
         */
        [DllImport("ddskernel", EntryPoint = "u_waitsetWaitAction2", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT WaitAction2(
            IntPtr _this,
            waitsetActionFn2 action,
            IntPtr arg,
            long timeout);

        /*
         *     u_result
         *     u_waitsetAttach(
         *         const u_waitset _this,
         *         const u_observable observable,
         *         void *context);
         */
        [DllImport("ddskernel", EntryPoint = "u_waitsetAttach", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT Attach(
            IntPtr _this,
            IntPtr observable,
            IntPtr context);

        /*
         *     u_result
         *     u_waitsetDetach(
         *         const u_waitset _this,
         *         const u_observable observable);
         */
        [DllImport("ddskernel", EntryPoint = "u_waitsetDetach", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT Detach(
            IntPtr _this,
            IntPtr observable);

        /*
         *     u_result
         *     u_waitsetGetEventMask(
         *         const u_waitset _this,
         *         u_eventMask *eventMask);
         */
        [DllImport("ddskernel", EntryPoint = "u_waitsetGetEventMask", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT GetEventMask(
            IntPtr _this,
            ref uint eventMask);

        /*
         *     u_result
         *     u_waitsetSetEventMask(
         *         const u_waitset _this,
         *         u_eventMask eventMask);
         */
        [DllImport("ddskernel", EntryPoint = "u_waitsetSetEventMask", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT SetEventMask(
            IntPtr _this,
            uint eventMask);

        /*
         *     u_result
         *     u_waitsetDetachFromDomain(
         *         const u_waitset _this,
         *         const u_domain domain);
         */
        [DllImport("ddskernel", EntryPoint = "u_waitsetDetachFromDomain", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT DetachFromDomain(
            IntPtr _this,
            IntPtr domain);

        /*
         *     int
         *     u_waitsetGetDomainId(
         *         const u_waitset _this);
         */
        [DllImport("ddskernel", EntryPoint = "u_waitsetGetDomainId", CallingConvention = CallingConvention.Cdecl)]
        public static extern int GetDomainId(
            IntPtr _this);


    }
}
