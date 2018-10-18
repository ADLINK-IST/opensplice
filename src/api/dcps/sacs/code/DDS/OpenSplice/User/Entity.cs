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
    static internal class Entity
    {
        /*
         *     u_result
         *     u_entityInit(
         *         const u_entity _this);
         */
        [DllImport("ddskernel", EntryPoint = "u_entityInit", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT Init(
            IntPtr _this);

        /*
         *     u_result
         *     u_entityDeinit(
         *         const u_entity _this);
         */
        [DllImport("ddskernel", EntryPoint = "u_entityDeinit", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT Deinit(
            IntPtr _this);

        /*
         *     u_result
         *     u_entityEnable(
         *         const u_entity _this);
         */
        [DllImport("ddskernel", EntryPoint = "u_entityEnable", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT Enable(
            IntPtr _this);

        /*
         *     u_result
         *     u_entitySetListener(
         *         const u_entity _this,
         *         u_listener listener,
         *         u_eventMask interest);
         */
        [DllImport("ddskernel", EntryPoint = "u_entitySetListener", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT SetListener(
            IntPtr _this,
            IntPtr listener,
            IntPtr listenerData,
            uint interest);

        /*
         *     u_bool
         *     u_entityEnabled (
         *         const u_entity _this);
         */
        [DllImport("ddskernel", EntryPoint = "u_entityEnabled", CallingConvention = CallingConvention.Cdecl)]
        public static extern byte Enabled(
            IntPtr _this);

        /*
         *     u_result
         *     u_entityWalkEntities(
         *         const u_entity _this,
         *         u_bool (*action)(v_entity e, void *arg),
         *         void *arg);
         */
        [DllImport("ddskernel", EntryPoint = "u_entityWalkEntities", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT WalkEntities(
            IntPtr _this,
            Delegate action,
            IntPtr arg);

        /*
         *     u_result
         *     u_entityWalkDependantEntities(
         *         const u_entity _this,
         *         u_bool (*action)(v_entity e, void *arg),
         *         void *arg);
         */
        [DllImport("ddskernel", EntryPoint = "u_entityWalkDependantEntities", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT WalkDependantEntities(
            IntPtr _this,
            Delegate action,
            IntPtr arg);

        /*
         *     u_instanceHandle
         *     u_entityGetInstanceHandle(
         *         const u_entity _this);
         */
        [DllImport("ddskernel", EntryPoint = "u_entityGetInstanceHandle", CallingConvention = CallingConvention.Cdecl)]
        public static extern long GetInstanceHandle(
            IntPtr _this);

        /*
         *     os_char *
         *     u_entityName(
         *         const u_entity _this);
         */
        [DllImport("ddskernel", EntryPoint = "u_entityName", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr Name(
            IntPtr _this);

        /*
         *     u_result
         *     u_entityGetEventState (
         *         const u_entity _this,
         *         u_eventMask *eventState);
         */
        [DllImport("ddskernel", EntryPoint = "u_entityGetEventState", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT GetEventState(
            IntPtr _this,
            uint eventState);

        /*
         *     u_result
         *     u_entityGetXMLQos (
         *         const u_entity _this,
         *         os_char **xml);
         */
        [DllImport("ddskernel", EntryPoint = "u_entityGetXMLQos", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT GetXMLQos(
            IntPtr _this,
            IntPtr xml);

        /*
         *     u_result
         *     u_entitySetXMLQos (
         *         const u_entity _this,
         *         const os_char *xml);
         */
        [DllImport("ddskernel", EntryPoint = "u_entitySetXMLQos", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT SetXMLQos(
            IntPtr _this,
            string xml);

        /*
         *     u_bool
         *     u_entityDisableCallbacks(
         *         const u_entity _this);
         */
        [DllImport("ddskernel", EntryPoint = "u_entityDisableCallbacks", CallingConvention = CallingConvention.Cdecl)]
        public static extern byte DisableCallbacks(
            IntPtr _this);

       [DllImport("ddskernel", EntryPoint = "u_entityGetProperty", CallingConvention = CallingConvention.Cdecl)]
       public static extern V_RESULT GetProperty(
            IntPtr _this,
            string name,
            ref IntPtr value);

       [DllImport("ddskernel", EntryPoint = "u_entitySetProperty", CallingConvention = CallingConvention.Cdecl)]
       public static extern V_RESULT SetProperty(
            IntPtr _this,
            string name,
            string value);

    }
}
