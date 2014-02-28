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
    static internal class FooDataReaderView
    {
        /* ReturnCode_t
         * read(
         *     inout DataSeq data_values,
         *     inout SampleInfoSeq info_seq,
         *     in long max_samples,
         *     in sampleStateMask sample_states,
         *     in viewStateMask view_states,
         *     in instanceStateMask instance_states);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_fooDataReaderView_read")]
        public static extern ReturnCode read(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            SampleStateKind sample_states,
            ViewStateKind view_states,
            InstanceStateKind instance_states);

        /* ReturnCode_t
         * take(
         *     inout DataSeq data_values,
         *     inout SampleInfoSeq info_seq,
         *     in long max_samples,
         *     in sampleStateMask sample_states,
         *     in viewStateMask view_states,
         *     in instanceStateMask instance_states);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_fooDataReaderView_take")]
        public static extern ReturnCode take(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            SampleStateKind sample_states,
            ViewStateKind view_states,
            InstanceStateKind instance_states);

        /* ReturnCode_t
         * read_next_sample(
         *     inout Data data_values,
         *     inout SampleInfo sample_info);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_fooDataReaderView_read_next_sample")]
        public static extern ReturnCode read_next_sample(
            IntPtr _this,
            IntPtr data_values,
            IntPtr sample_info);

        /* ReturnCode_t
         * take_next_sample(
         *     inout Data data_values,
         *     inout SampleInfo sample_info);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_fooDataReaderView_take_next_sample")]
        public static extern ReturnCode take_next_sample(
            IntPtr _this,
            IntPtr data_values,
            IntPtr sample_info
            );

        /* ReturnCode_t
         * read_instance(
         *     inout DataSeq data_values,
         *     inout SampleInfoSeq info_seq,
         *     in long max_samples,
         *     in InstanceHandle_t a_handle,
         *     in sampleStateMask sample_states,
         *     in viewStateMask view_states,
         *     in instanceStateMask instance_states);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_fooDataReaderView_read_instance")]
        public static extern ReturnCode read_instance(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            InstanceHandle a_handle,
            SampleStateKind sample_states,
            ViewStateKind view_states,
            InstanceStateKind instance_states);

        /* ReturnCode_t
         * take_instance(
         *     inout DataSeq data_values,
         *     inout SampleInfoSeq info_seq,
         *     in long max_samples,
         *     in InstanceHandle_t a_handle,
         *     in sampleStateMask sample_states,
         *     in viewStateMask view_states,
         *     in instanceStateMask instance_states);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_fooDataReaderView_take_instance")]
        public static extern ReturnCode take_instance(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            InstanceHandle a_handle,
            SampleStateKind sample_states,
            ViewStateKind view_states,
            InstanceStateKind instance_states);

        /* ReturnCode_t
         * read_next_instance(
         *     inout DataSeq data_values,
         *     inout SampleInfoSeq info_seq,
         *     in long max_samples,
         *     in InstanceHandle_t a_handle,
         *     in sampleStateMask sample_states,
         *     in viewStateMask view_states,
         *     in instanceStateMask instance_states);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_fooDataReaderView_read_next_instance")]
        public static extern ReturnCode read_next_instance(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            InstanceHandle a_handle,
            SampleStateKind sample_states,
            ViewStateKind view_states,
            InstanceStateKind instance_states);

        /* ReturnCode_t
         * take_next_instance(
         *     inout DataSeq data_values,
         *     inout SampleInfoSeq info_seq,
         *     in long max_samples,
         *     in InstanceHandle_t a_handle,
         *     in sampleStateMask sample_states,
         *     in viewStateMask view_states,
         *     in instanceStateMask instance_states);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_fooDataReaderView_take_next_instance")]
        public static extern ReturnCode take_next_instance(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            InstanceHandle a_handle,
            SampleStateKind sample_states,
            ViewStateKind view_states,
            InstanceStateKind instance_states);

        /* ReturnCode_t
         * read_w_condition(
         *     inout DataSeq data_values,
         *     inout SampleInfoSeq info_seq,
         *     in long max_samples,
         *     in ReadCondition a_condition);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_fooDataReaderView_read_w_condition")]
        public static extern ReturnCode read_w_condition(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            IntPtr a_condition);

        /* ReturnCode_t
         * take_w_condition(
         *     inout DataSeq data_values,
         *     inout SampleInfoSeq info_seq,
         *     in long max_samples,
         *     in ReadCondition a_condition);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_fooDataReaderView_take_w_condition")]
        public static extern ReturnCode take_w_condition(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            IntPtr a_condition);

        /* ReturnCode_t
         * read_next_instance_w_condition(
         *     inout DataSeq data_values,
         *     inout SampleInfoSeq info_seq,
         *     in long max_samples,
         *     in InstanceHandle_t a_handle,
         *     in ReadCondition a_condition);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_fooDataReaderView_read_next_instance_w_condition")]
        public static extern ReturnCode read_next_instance_w_condition(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            InstanceHandle a_handle,
            IntPtr a_condition);

        /* ReturnCode_t
         * take_next_instance_w_condition(
         *     inout DataSeq data_values,
         *     inout SampleInfoSeq info_seq,
         *     in long max_samples,
         *     in InstanceHandle_t a_handle,
         *     in ReadCondition a_condition);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_fooDataReaderView_take_next_instance_w_condition")]
        public static extern ReturnCode take_next_instance_w_condition(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            InstanceHandle a_handle,
            IntPtr a_condition);

        /* Boolean
         * is_loan(
         *     inout DataSeq data_values,
         *     inout SampleInfoSeq info_seq);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_fooDataReaderView_is_loan")]
        public static extern byte is_loan(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq);


        /* ReturnCode_t
         * get_key_value(
         *     inout Data key_holder,
         *     in InstanceHandle_t handle);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_fooDataReaderView_get_key_value")]
        public static extern ReturnCode get_key_value(
            IntPtr _this,
            IntPtr key_holder,
            InstanceHandle handle);

        /* InstanceHandle_t
         * lookup_instance(
         *     in Data instance);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_fooDataReaderView_lookup_instance")]
        public static extern InstanceHandle lookup_instance(
            IntPtr _this,
            IntPtr instance_data);

        /* ReturnCode_t
         * return_loan(
         *     inout DataSeq data_values,
         *     inout SampleInfoSeq info_seq);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_fooDataReaderView_return_loan")]
        public static extern ReturnCode return_loan(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq);
    }
}
