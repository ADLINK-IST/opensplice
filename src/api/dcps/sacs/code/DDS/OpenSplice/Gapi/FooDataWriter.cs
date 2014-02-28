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
    static internal class FooDataWriter
    {
        /* InstanceHandle_t
         * register_instance(
         *     in Data instance_data);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_fooDataWriter_register_instance")]
        public static extern long register_instance(
            IntPtr _this,
            IntPtr instance_data);

        /* InstanceHandle_t
         * register_instance_w_timestamp(
         *    in Data instance_data,
         *     in Time_t source_timestamp);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_fooDataWriter_register_instance_w_timestamp")]
        public static extern long register_instance_w_timestamp(
            IntPtr _this,
            IntPtr instance_data,
            ref Time source_timestamp);

        /* ReturnCode_t
         * unregister_instance(
         *     in Data instance_data,
         *     in InstanceHandle_t handle);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_fooDataWriter_unregister_instance")]
        public static extern ReturnCode unregister_instance(
            IntPtr _this,
            IntPtr instance_data,
            long handle);

        /* ReturnCode_t
         * unregister_instance_w_timestamp(
         *     in Data instance_data,
         *     in InstanceHandle_t handle,
         *     in Time_t source_timestamp);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_fooDataWriter_unregister_instance_w_timestamp")]
        public static extern ReturnCode unregister_instance_w_timestamp(
            IntPtr _this,
            IntPtr instance_data,
            long handle,
            ref Time source_timestamp);

        /* ReturnCode_t
         * write(
         *     in Data instance_data,
         *     in InstanceHandle_t handle);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_fooDataWriter_write")]
        public static extern ReturnCode write(
            IntPtr _this,
            IntPtr instance_data,
            long handle);

        /* ReturnCode_t
         * write_w_timestamp(
         *     in Data instance_data,
         *     in InstanceHandle_t handle,
         *     in Time_t source_timestamp);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_fooDataWriter_write_w_timestamp")]
        public static extern ReturnCode write_w_timestamp(
            IntPtr _this,
            IntPtr instance_data,
            long handle,
            ref Time source_timestamp);

        /* ReturnCode_t
         * dispose(
         *     in Data instance_data,
         *     in InstanceHandle_t instance_handle);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_fooDataWriter_dispose")]
        public static extern ReturnCode dispose(
            IntPtr _this,
            IntPtr instance_data,
            long instance_handle);

        /* ReturnCode_t
         * dispose_w_timestamp(
         *     in Data instance_data,
         *     in InstanceHandle_t instance_handle,
         *     in Time_t source_timestamp);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_fooDataWriter_dispose_w_timestamp")]
        public static extern ReturnCode dispose_w_timestamp(
            IntPtr _this,
            IntPtr instance_data,
            long instance_handle,
            ref Time source_timestamp);

        /* ReturnCode_t
         * writedispose(
         *     in Data instance_data,
         *     in InstanceHandle_t instance_handle);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_fooDataWriter_writedispose")]
        public static extern ReturnCode writedispose(
            IntPtr _this,
            IntPtr instance_data,
            long instance_handle);

        /* ReturnCode_t
         * writedispose_w_timestamp(
         *     in Data instance_data,
         *     in InstanceHandle_t instance_handle,
         *     in Time_t source_timestamp);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_fooDataWriter_writedispose_w_timestamp")]
        public static extern ReturnCode writedispose_w_timestamp(
            IntPtr _this,
            IntPtr instance_data,
            long instance_handle,
            ref Time source_timestamp);

        /* ReturnCode_t
         * get_key_value(
         *     inout Data key_holder,
         *     in InstanceHandle_t handle);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_fooDataWriter_get_key_value")]
        public static extern ReturnCode get_key_value(
            IntPtr _this,
            IntPtr key_holder,
            long handle);

        /* InstanceHandle_t
         *   lookup_instance(
         *       in Data instance_data);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_fooDataWriter_lookup_instance")]
        public static extern long lookup_instance(
            IntPtr _this,
            IntPtr instance_data);
    }
}
