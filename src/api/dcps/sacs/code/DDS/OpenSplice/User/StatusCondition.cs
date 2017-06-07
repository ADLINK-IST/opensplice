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
    static internal class StatusCondition
    {
        /*
         *     u_statusCondition
         *     u_statusConditionNew(
         *         const u_entity entity);
         */
        [DllImport("ddskernel", EntryPoint = "u_statusConditionNew", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr New(
            IntPtr entity);

        /*
         *     u_result
         *     u_statusCondition_set_mask(
         *         const u_statusCondition _this,
         *         u_eventMask eventMask);
         */
        [DllImport("ddskernel", EntryPoint = "u_statusCondition_set_mask", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT SetMask(
            IntPtr _this,
            uint eventMask);

        /*
         *     u_result
         *     u_statusCondition_get_triggerValue (
         *         const u_statusCondition _this,
         *         u_eventMask *triggerValue);
         * };
         */
        [DllImport("ddskernel", EntryPoint = "u_statusCondition_get_triggerValue", CallingConvention = CallingConvention.Cdecl)]
        public static extern V_RESULT GetTriggerValue(
            IntPtr _this,
            ref uint eventMask);
    }
}
