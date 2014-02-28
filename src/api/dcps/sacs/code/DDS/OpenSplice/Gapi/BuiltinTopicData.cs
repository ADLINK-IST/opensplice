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
    static internal class ParticipantBuiltinTopicDataDataReader
    {
        /*
         * ParticipantBuiltinTopicData
         */
        [DllImport("ddskernel", EntryPoint = "gapi_participantBuiltinTopicDataDataReader_read")]
        public static extern ReturnCode read(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            SampleStateKind sample_states,
            ViewStateKind view_states,
            InstanceStateKind instance_states);

        [DllImport("ddskernel", EntryPoint = "gapi_participantBuiltinTopicDataDataReader_take")]
        public static extern ReturnCode take(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            SampleStateKind sample_states,
            ViewStateKind view_states,
            InstanceStateKind instance_states);

        [DllImport("ddskernel", EntryPoint = "gapi_participantBuiltinTopicDataDataReader_read_w_condition")]
        public static extern ReturnCode read_w_condition(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            IntPtr a_condition);

        [DllImport("ddskernel", EntryPoint = "gapi_participantBuiltinTopicDataDataReader_take_w_condition")]
        public static extern ReturnCode take_w_condition(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            IntPtr a_condition);

        [DllImport("ddskernel", EntryPoint = "gapi_participantBuiltinTopicDataDataReader_read_next_sample")]
        public static extern ReturnCode read_next_sample(
            IntPtr _this,
            IntPtr data_values,
            IntPtr sample_info);

        [DllImport("ddskernel", EntryPoint = "gapi_participantBuiltinTopicDataDataReader_take_next_sample")]
        public static extern ReturnCode take_next_sample(
            IntPtr _this,
            IntPtr data_values,
            IntPtr sample_info);

        [DllImport("ddskernel", EntryPoint = "gapi_participantBuiltinTopicDataDataReader_read_instance")]
        public static extern ReturnCode read_instance(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            InstanceHandle a_handle,
            SampleStateKind sample_states,
            ViewStateKind view_states,
            InstanceStateKind instance_states);

        [DllImport("ddskernel", EntryPoint = "gapi_participantBuiltinTopicDataDataReader_take_instance")]
        public static extern ReturnCode take_instance(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            InstanceHandle a_handle,
            SampleStateKind sample_states,
            ViewStateKind view_states,
            InstanceStateKind instance_states);

        [DllImport("ddskernel", EntryPoint = "gapi_participantBuiltinTopicDataDataReader_read_next_instance")]
        public static extern ReturnCode read_next_instance(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            InstanceHandle a_handle,
            SampleStateKind sample_states,
            ViewStateKind view_states,
            InstanceStateKind instance_states);

        [DllImport("ddskernel", EntryPoint = "gapi_participantBuiltinTopicDataDataReader_take_next_instance")]
        public static extern ReturnCode take_next_instance(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            InstanceHandle a_handle,
            SampleStateKind sample_states,
            ViewStateKind view_states,
            InstanceStateKind instance_states);

        [DllImport("ddskernel", EntryPoint = "gapi_participantBuiltinTopicDataDataReader_read_next_instance_w_condition")]
        public static extern ReturnCode read_next_instance_w_condition(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            InstanceHandle a_handle,
            IntPtr a_condition);

        [DllImport("ddskernel", EntryPoint = "gapi_participantBuiltinTopicDataDataReader_take_next_instance_w_condition")]
        public static extern ReturnCode take_next_instance_w_condition(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            InstanceHandle a_handle,
            IntPtr a_condition);

        [DllImport("ddskernel", EntryPoint = "gapi_participantBuiltinTopicDataDataReader_return_loan")]
        public static extern ReturnCode return_loan(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq);

        [DllImport("ddskernel", EntryPoint = "gapi_participantBuiltinTopicDataDataReader_get_key_value")]
        public static extern ReturnCode get_key_value(
            IntPtr _this,
            IntPtr key_holder,
            InstanceHandle handle);
    }

    static internal class TopicBuiltinTopicDataDataReader
    {
        /*
         * TopicBuiltinTopicData
         */
        [DllImport("ddskernel", EntryPoint = "gapi_topicBuiltinTopicDataDataReader_read")]
        public static extern ReturnCode read(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            SampleStateKind sample_states,
            ViewStateKind view_states,
            InstanceStateKind instance_states);

        [DllImport("ddskernel", EntryPoint = "gapi_topicBuiltinTopicDataDataReader_take")]
        public static extern ReturnCode take(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            SampleStateKind sample_states,
            ViewStateKind view_states,
            InstanceStateKind instance_states);

        [DllImport("ddskernel", EntryPoint = "gapi_topicBuiltinTopicDataDataReader_read_w_condition")]
        public static extern ReturnCode read_w_condition(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            IntPtr a_condition);

        [DllImport("ddskernel", EntryPoint = "gapi_topicBuiltinTopicDataDataReader_take_w_condition")]
        public static extern ReturnCode take_w_condition(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            IntPtr a_condition);

        [DllImport("ddskernel", EntryPoint = "gapi_topicBuiltinTopicDataDataReader_read_next_sample")]
        public static extern ReturnCode read_next_sample(
            IntPtr _this,
            IntPtr data_values,
            IntPtr sample_info);

        [DllImport("ddskernel", EntryPoint = "gapi_topicBuiltinTopicDataDataReader_take_next_sample")]
        public static extern ReturnCode take_next_sample(
            IntPtr _this,
            IntPtr data_values,
            IntPtr sample_info);

        [DllImport("ddskernel", EntryPoint = "gapi_topicBuiltinTopicDataDataReader_read_instance")]
        public static extern ReturnCode read_instance(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            InstanceHandle a_handle,
            SampleStateKind sample_states,
            ViewStateKind view_states,
            InstanceStateKind instance_states);

        [DllImport("ddskernel", EntryPoint = "gapi_topicBuiltinTopicDataDataReader_take_instance")]
        public static extern ReturnCode take_instance(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            InstanceHandle a_handle,
            SampleStateKind sample_states,
            ViewStateKind view_states,
            InstanceStateKind instance_states);

        [DllImport("ddskernel", EntryPoint = "gapi_topicBuiltinTopicDataDataReader_read_next_instance")]
        public static extern ReturnCode read_next_instance(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            InstanceHandle a_handle,
            SampleStateKind sample_states,
            ViewStateKind view_states,
            InstanceStateKind instance_states);

        [DllImport("ddskernel", EntryPoint = "gapi_topicBuiltinTopicDataDataReader_take_next_instance")]
        public static extern ReturnCode take_next_instance(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            InstanceHandle a_handle,
            SampleStateKind sample_states,
            ViewStateKind view_states,
            InstanceStateKind instance_states);

        [DllImport("ddskernel", EntryPoint = "gapi_topicBuiltinTopicDataDataReader_read_next_instance_w_condition")]
        public static extern ReturnCode read_next_instance_w_condition(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            InstanceHandle a_handle,
            IntPtr a_condition);

        [DllImport("ddskernel", EntryPoint = "gapi_topicBuiltinTopicDataDataReader_take_next_instance_w_condition")]
        public static extern ReturnCode take_next_instance_w_condition(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            InstanceHandle a_handle,
            IntPtr a_condition);

        [DllImport("ddskernel", EntryPoint = "gapi_topicBuiltinTopicDataDataReader_return_loan")]
        public static extern ReturnCode return_loan(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq);

        [DllImport("ddskernel", EntryPoint = "gapi_topicBuiltinTopicDataDataReader_get_key_value")]
        public static extern ReturnCode get_key_value(
            IntPtr _this,
            IntPtr key_holder,
            InstanceHandle handle);
    }

    static internal class PublicationBuiltinTopicDataDataReader
    {
        /*
         * PublicationBuiltinTopicData
         */
        [DllImport("ddskernel", EntryPoint = "gapi_publicationBuiltinTopicDataDataReader_read")]
        public static extern ReturnCode read(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            SampleStateKind sample_states,
            ViewStateKind view_states,
            InstanceStateKind instance_states);

        [DllImport("ddskernel", EntryPoint = "gapi_publicationBuiltinTopicDataDataReader_take")]
        public static extern ReturnCode take(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            SampleStateKind sample_states,
            ViewStateKind view_states,
            InstanceStateKind instance_states);

        [DllImport("ddskernel", EntryPoint = "gapi_publicationBuiltinTopicDataDataReader_read_w_condition")]
        public static extern ReturnCode read_w_condition(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            IntPtr a_condition);

        [DllImport("ddskernel", EntryPoint = "gapi_publicationBuiltinTopicDataDataReader_take_w_condition")]
        public static extern ReturnCode take_w_condition(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            IntPtr a_condition);

        [DllImport("ddskernel", EntryPoint = "gapi_publicationBuiltinTopicDataDataReader_read_next_sample")]
        public static extern ReturnCode read_next_sample(
            IntPtr _this,
            IntPtr data_values,
            IntPtr sample_info);

        [DllImport("ddskernel", EntryPoint = "gapi_publicationBuiltinTopicDataDataReader_take_next_sample")]
        public static extern ReturnCode take_next_sample(
            IntPtr _this,
            IntPtr data_values,
            IntPtr sample_info);

        [DllImport("ddskernel", EntryPoint = "gapi_publicationBuiltinTopicDataDataReader_read_instance")]
        public static extern ReturnCode read_instance(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            InstanceHandle a_handle,
            SampleStateKind sample_states,
            ViewStateKind view_states,
            InstanceStateKind instance_states);

        [DllImport("ddskernel", EntryPoint = "gapi_publicationBuiltinTopicDataDataReader_take_instance")]
        public static extern ReturnCode take_instance(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            InstanceHandle a_handle,
            SampleStateKind sample_states,
            ViewStateKind view_states,
            InstanceStateKind instance_states);

        [DllImport("ddskernel", EntryPoint = "gapi_publicationBuiltinTopicDataDataReader_read_next_instance")]
        public static extern ReturnCode read_next_instance(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            InstanceHandle a_handle,
            SampleStateKind sample_states,
            ViewStateKind view_states,
            InstanceStateKind instance_states);

        [DllImport("ddskernel", EntryPoint = "gapi_publicationBuiltinTopicDataDataReader_take_next_instance")]
        public static extern ReturnCode take_next_instance(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            InstanceHandle a_handle,
            SampleStateKind sample_states,
            ViewStateKind view_states,
            InstanceStateKind instance_states);

        [DllImport("ddskernel", EntryPoint = "gapi_publicationBuiltinTopicDataDataReader_read_next_instance_w_condition")]
        public static extern ReturnCode read_next_instance_w_condition(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            InstanceHandle a_handle,
            IntPtr a_condition);

        [DllImport("ddskernel", EntryPoint = "gapi_publicationBuiltinTopicDataDataReader_take_next_instance_w_condition")]
        public static extern ReturnCode take_next_instance_w_condition(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            InstanceHandle a_handle,
            IntPtr a_condition);

        [DllImport("ddskernel", EntryPoint = "gapi_publicationBuiltinTopicDataDataReader_return_loan")]
        public static extern ReturnCode return_loan(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq);

        [DllImport("ddskernel", EntryPoint = "gapi_publicationBuiltinTopicDataDataReader_get_key_value")]
        public static extern ReturnCode get_key_value(
            IntPtr _this,
            IntPtr key_holder,
            InstanceHandle handle);
    }

    static internal class SubscriptionBuiltinTopicDataDataReader
    {
        /*
         * SubscriptionBuiltinTopicData
         */
        [DllImport("ddskernel", EntryPoint = "gapi_subscriptionBuiltinTopicDataDataReader_read")]
        public static extern ReturnCode read(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            SampleStateKind sample_states,
            ViewStateKind view_states,
            InstanceStateKind instance_states);

        [DllImport("ddskernel", EntryPoint = "gapi_subscriptionBuiltinTopicDataDataReader_take")]
        public static extern ReturnCode take(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            SampleStateKind sample_states,
            ViewStateKind view_states,
            InstanceStateKind instance_states);

        [DllImport("ddskernel", EntryPoint = "gapi_subscriptionBuiltinTopicDataDataReader_read_w_condition")]
        public static extern ReturnCode read_w_condition(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            IntPtr a_condition);

        [DllImport("ddskernel", EntryPoint = "gapi_subscriptionBuiltinTopicDataDataReader_take_w_condition")]
        public static extern ReturnCode take_w_condition(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            IntPtr a_condition);

        [DllImport("ddskernel", EntryPoint = "gapi_subscriptionBuiltinTopicDataDataReader_read_next_sample")]
        public static extern ReturnCode read_next_sample(
            IntPtr _this,
            IntPtr data_values,
            IntPtr sample_info);

        [DllImport("ddskernel", EntryPoint = "gapi_subscriptionBuiltinTopicDataDataReader_take_next_sample")]
        public static extern ReturnCode take_next_sample(
            IntPtr _this,
            IntPtr data_values,
            IntPtr sample_info);

        [DllImport("ddskernel", EntryPoint = "gapi_subscriptionBuiltinTopicDataDataReader_read_instance")]
        public static extern ReturnCode read_instance(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            InstanceHandle a_handle,
            SampleStateKind sample_states,
            ViewStateKind view_states,
            InstanceStateKind instance_states);

        [DllImport("ddskernel", EntryPoint = "gapi_subscriptionBuiltinTopicDataDataReader_take_instance")]
        public static extern ReturnCode take_instance(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            InstanceHandle a_handle,
            SampleStateKind sample_states,
            ViewStateKind view_states,
            InstanceStateKind instance_states);

        [DllImport("ddskernel", EntryPoint = "gapi_subscriptionBuiltinTopicDataDataReader_read_next_instance")]
        public static extern ReturnCode read_next_instance(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            InstanceHandle a_handle,
            SampleStateKind sample_states,
            ViewStateKind view_states,
            InstanceStateKind instance_states);

        [DllImport("ddskernel", EntryPoint = "gapi_subscriptionBuiltinTopicDataDataReader_take_next_instance")]
        public static extern ReturnCode take_next_instance(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            InstanceHandle a_handle,
            SampleStateKind sample_states,
            ViewStateKind view_states,
            InstanceStateKind instance_states);

        [DllImport("ddskernel", EntryPoint = "gapi_subscriptionBuiltinTopicDataDataReader_read_next_instance_w_condition")]
        public static extern ReturnCode read_next_instance_w_condition(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            InstanceHandle a_handle,
            IntPtr a_condition);

        [DllImport("ddskernel", EntryPoint = "gapi_subscriptionBuiltinTopicDataDataReader_take_next_instance_w_condition")]
        public static extern ReturnCode take_next_instance_w_condition(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq,
            int max_samples,
            InstanceHandle a_handle,
            IntPtr a_condition);

        [DllImport("ddskernel", EntryPoint = "gapi_subscriptionBuiltinTopicDataDataReader_return_loan")]
        public static extern ReturnCode return_loan(
            IntPtr _this,
            IntPtr data_values,
            IntPtr info_seq);

        [DllImport("ddskernel", EntryPoint = "gapi_subscriptionBuiltinTopicDataDataReader_get_key_value")]
        public static extern ReturnCode get_key_value(
            IntPtr _this,
            IntPtr key_holder,
            InstanceHandle handle);
    }
}
