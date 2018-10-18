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
    enum u_kind {
        U_UNDEFINED,
        U_ENTITY, U_PARTICIPANT, U_PUBLISHER, U_WRITER, U_SPLICED, U_SERVICE,
        U_SERVICEMANAGER, U_SUBSCRIBER, U_READER, U_NETWORKREADER,
        U_GROUPQUEUE, U_QUERY, U_DATAVIEW, U_PARTITION, U_TOPIC, U_CFTOPIC,
        U_GROUP, U_WAITSET, U_DOMAIN, U_LISTENER, U_STATUSCONDITION,
        U_COUNT
    }

    class Object
    {

        /*
         *     u_object
         *     u_objectNew(
         *         const u_kind kind,
         *         const u_destructor destructor);
         */
        [DllImport("ddskernel", EntryPoint = "u_objectNew", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr New (
            u_kind kind,
            Delegate destructor);

        /*
         *     u_result
         *     u_objectClose(
         *         const u_object _this);
         */
        [DllImport("ddskernel", EntryPoint = "u_objectClose", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT Close (
            IntPtr _this);

        /*
         *     u_result
         *     u_objectFree(
         *         const u_object _this);
         */
        [DllImport("ddskernel", EntryPoint = "u_objectFree", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT Free (
            IntPtr _this);

        /*
         *     u_bool
         *     u_objectCheck(
         *         const u_object _this);
         */
        [DllImport("ddskernel", EntryPoint = "u_objectCheck", CallingConvention = CallingConvention.Cdecl)]
        public static extern byte Check (
            IntPtr _this);

        /*
         *     u_kind
         *     u_objectKind(
         *         const u_object _this);
         */
        [DllImport("ddskernel", EntryPoint = "u_objectKind", CallingConvention = CallingConvention.Cdecl)]
        public static extern u_kind Kind (
            IntPtr _this);
    }
}
