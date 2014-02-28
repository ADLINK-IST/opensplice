// The OpenSplice DDS Community Edition project.
//
// Copyright (C) 2006 to 2011 PrismTech Limited and its licensees.
// Copyright (C) 2009  L-3 Communications / IS
// 
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License Version 3 dated 29 June 2007, as published by the
//  Free Software Foundation.
// 
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
// 
//  You should have received a copy of the GNU Lesser General Public
//  License along with OpenSplice DDS Community Edition; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

using System;
using System.Runtime.InteropServices;

namespace DDS.OpenSplice.Gapi
{
    static internal class WaitSet
    {
        /*     WaitSet
         *     WaitSet__alloc (
         *         void);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_waitSet__alloc")]
        public static extern IntPtr alloc();

        /*     ReturnCode_t
         *     wait(
         *         inout ConditionSeq active_conditions,
         *         in Duration_t timeout);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_waitSet_wait")]
        public static extern ReturnCode wait(
            IntPtr _this,
            IntPtr active_conditions,
            ref Duration timeout);

        /*     ReturnCode_t
         *     attach_condition(
         *         in Condition cond);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_waitSet_attach_condition")]
        public static extern ReturnCode attach_condition(
            IntPtr _this,
            IntPtr cond);

        /*     ReturnCode_t
         *     detach_condition(
         *         in Condition cond);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_waitSet_detach_condition")]
        public static extern ReturnCode detach_condition(
            IntPtr _this,
            IntPtr cond);

        /*     ReturnCode_t
         *     get_conditions(
         *         inout ConditionSeq attached_conditions);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_waitSet_get_conditions")]
        public static extern ReturnCode get_conditions(
            IntPtr _this,
            IntPtr attached_conditions);
    }
}
