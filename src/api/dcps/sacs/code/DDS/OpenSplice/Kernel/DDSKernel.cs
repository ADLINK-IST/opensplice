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
using DDS.OpenSplice.Database;
using DDS.OpenSplice.kernelModuleI;

namespace DDS.OpenSplice.Kernel {

    public enum V_RESULT {
        UNDEFINED = (3 << 8),
        OK,
        INTERRUPTED,
        NOT_ENABLED,
        OUT_OF_MEMORY,
        INTERNAL_ERROR,
        ILL_PARAM,
        CLASS_MISMATCH,
        DETACHING,
        TIMEOUT,
        OUT_OF_RESOURCES,
        INCONSISTENT_QOS,
        IMMUTABLE_POLICY,
        PRECONDITION_NOT_MET,
        ALREADY_DELETED,
        HANDLE_EXPIRED,
        NO_DATA,
        UNSUPPORTED
    }

    public enum V_COPYIN_RESULT {
        INVALID,
        OK,
        OUT_OF_MEMORY
    }

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    internal delegate V_RESULT statusActionFn(IntPtr info, IntPtr arg);

    enum V_EVENT : uint {
        UNDEFINED                   = 0x00000000,
        OBJECT_DESTROYED            = 0x00000001 << 0,
        INCONSISTENT_TOPIC          = 0x00000001 << 1,  /* 0x00000002U : 2 */
        SAMPLE_REJECTED             = 0x00000001 << 2,  /* 0x00000004U : 4 */
        SAMPLE_LOST                 = 0x00000001 << 3,  /* 0x00000008U : 8 */
        OFFERED_DEADLINE_MISSED     = 0x00000001 << 4,  /* 0x00000010U : 16 */
        REQUESTED_DEADLINE_MISSED   = 0x00000001 << 5,  /* 0x00000020U : 32 */
        OFFERED_INCOMPATIBLE_QOS    = 0x00000001 << 6,  /* 0x00000040U : 64 */
        REQUESTED_INCOMPATIBLE_QOS  = 0x00000001 << 7,  /* 0x00000080U : 128 */
        LIVELINESS_ASSERT           = 0x00000001 << 8,  /* 0x00000100U : 256 */
        LIVELINESS_CHANGED          = 0x00000001 << 9,  /* 0x00000200U : 512 */
        LIVELINESS_LOST             = 0x00000001 << 10, /* 0x00000400U : 1024 */
        SERVICES_CHANGES            = 0x00000001 << 11, /* 0x00000800U : 2048 */
        DATA_AVAILABLE              = 0x00000001 << 12, /* 0x00001000U : 4096 */
        PUBLICATION_MATCHED         = 0x00000001 << 13, /* 0x00002000U : 8192 */
        SUBSCRIPTION_MATCHED        = 0x00000001 << 14, /* 0x00004000U : 16384 */
        NEW_GROUP                   = 0x00000001 << 15, /* 0x00008000U : 32768 */
        SERVICESTATE_CHANGED        = 0x00000001 << 16, /* 0x00010000U : 65536 */
        LEASE_RENEWED               = 0x00000001 << 17, /* 0x00020000U : 131072 */
        LEASE_EXPIRED               = 0x00000001 << 18, /* 0x00040000U : 262144 */
        TRIGGER                     = 0x00000001 << 19, /* 0x00080000U : 524288 */
        TIMEOUT                     = 0x00000001 << 20, /* 0x00100000U : 1048576 */
        TERMINATE                   = 0x00000001 << 21, /* 0x00200000U : 2097152 */
        HISTORY_DELETE              = 0x00000001 << 22, /* 0x00400000U : 4194304 */
        HISTORY_REQUEST             = 0x00000001 << 23, /* 0x00800000U : 8388608 */
        PERSISTENT_SNAPSHOT         = 0x00000001 << 24, /* 0x01000000U : 16777216 */
        ALL_DATA_DISPOSED           = 0x00000001 << 25, /* 0x02000000U : 33554432 */
        ON_DATA_ON_READERS          = 0x00000001 << 26, /* 0x04000000U : 67108864 */
        CONNECT_WRITER              = 0x00000001 << 27, /* 0x08000000U : 134217728 */
        PREPARE_DELETE              = 0x00000001 << 28, /* 0x10000000U : 268435456 */
        MASK_ALL                    = 0xffffffff
    }

    enum v_handleResult {
        V_HANDLE_OK,      /* The handle is valid */
        V_HANDLE_EXPIRED, /* The handle was valid once but is not valid anymore */
        V_HANDLE_ILLEGAL, /* The handle is bogus */
        V_HANDLE_SUSPENDED/* The handle server is suspended. */
    }

    public static class Constants
    {
       public const uint V_POLICY_ID_COUNT = 28;
    }

    [StructLayoutAttribute(LayoutKind.Sequential)]
    internal struct v_handle {
        /*
         *     v_handleResult
         *     v_handleClaim(
         *         v_handle handle,
         *         v_object *o)
         */
        [DllImport("ddskernel", EntryPoint = "v_handleClaim", CallingConvention = CallingConvention.Cdecl)]
        internal static extern v_handleResult Claim(v_handle_s handle, out IntPtr o);

        /*
         *     v_handleResult
         *     v_handleRelease(
         *         v_handle handle)
         */
        [DllImport("ddskernel", EntryPoint = "v_handleRelease", CallingConvention = CallingConvention.Cdecl)]
        internal static extern v_handleResult Release(v_handle_s handle);
    }

    static public class DataReaderInstance {
        // from v_dataReaderInstance.h

        //OS_API c_ulong
        //v_dataReaderInstanceGetNotEmptyInstanceCount (
        //    v_dataReaderInstance _this);
        [DllImport("ddskernel", EntryPoint = "v_dataReaderInstanceGetNotEmptyInstanceCount", CallingConvention = CallingConvention.Cdecl)]
        public static extern uint GetNotEmptyInstanceCount(IntPtr _this);
    }

    static public class InstanceHandle {
        // from u_instanceHandle.h

        /*
         *     u_instanceHandle
         *     u_instanceHandleNew (
         *         v_public object);
         */
        [DllImport("ddskernel", EntryPoint = "u_instanceHandleNew", CallingConvention = CallingConvention.Cdecl)]
        public static extern long New(IntPtr obj);
    }

    enum v_statusResult {
        Success,
        Fail
    };

    static public class Status {
        // from v_status.h

        /*
         *     c_ulong
         *     v_statusGetMask (
         *         v_status s);
         */
        [DllImport("ddskernel", EntryPoint = "v_statusGetMask", CallingConvention = CallingConvention.Cdecl)]
        public static extern uint GetMask(IntPtr s);

        /*
         *     v_statusResult
         *     v_statusReset(
         *         v_status s,
         *         c_ulong mask);
         */
        [DllImport("ddskernel", EntryPoint = "v_statusReset", CallingConvention = CallingConvention.Cdecl)]
        internal static extern v_statusResult Reset(IntPtr s, V_EVENT mask);
    }

    static public class Entity {
        // from v_entity.h

        /*
         *     v_status
         *     v_entityStatus (
         *         v_entity _this);
         */
        [DllImport("ddskernel", EntryPoint = "v_entityStatus", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr Status(IntPtr _this);
    }

    static public class Parser {
        // from v_kernelParser.h

        /*
         *     q_expr
         *     v_parser_parse (
         *         const char *queryString);
         */
        [DllImport("ddskernel", EntryPoint = "v_parser_parse", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr parse(string queryString);
    }
}
