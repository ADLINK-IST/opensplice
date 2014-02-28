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
    static internal class DataReaderView
    {
        /*     ReturnCode_t
         *     set_qos(
         *         in DataReaderViewQos qos);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_dataReaderView_set_qos")]
        public static extern ReturnCode set_qos(
            IntPtr _this,
            IntPtr qos);

        /*     ReturnCode_t
         *     get_qos(
         *         inout DataReaderViewQos qos);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_dataReaderView_get_qos")]
        public static extern ReturnCode get_qos(
            IntPtr _this,
            IntPtr qos);

        /*     Datareader
         *     get_datareader();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_dataReaderView_get_datareader")]
        public static extern IntPtr get_datareader(
            IntPtr _this);

        /*     ReadCondition
         *     create_readcondition(
         *         in SampleStateMask sample_states,
         *         in ViewStateMask view_states,
         *         in InstanceStateMask instance_states);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_dataReaderView_create_readcondition")]
        public static extern IntPtr create_readcondition(
            IntPtr _this,
            SampleStateKind sample_states,
            ViewStateKind view_states,
            InstanceStateKind instance_states);

        /*     QueryCondition
         *     create_querycondition(
         *         in SampleStateMask sample_states,
         *         in ViewStateMask view_states,
         *         in InstanceStateMask instance_states,
         *         in string query_expression,
         *         in StringSeq query_parameters);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_dataReaderView_create_querycondition")]
        public static extern IntPtr create_querycondition(
            IntPtr _this,
            SampleStateKind sample_states,
            ViewStateKind view_states,
            InstanceStateKind instance_states,
            string query_expression,
            IntPtr query_parameters);

        /*     ReturnCode_t
         *     delete_readcondition(
         *         in ReadCondition a_condition);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_dataReaderView_delete_readcondition")]
        public static extern ReturnCode delete_readcondition(
            IntPtr _this,
            IntPtr a_condition);

        /*     ReturnCode_t
         *     delete_contained_entities();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_dataReaderView_delete_contained_entities")]
        public static extern ReturnCode delete_contained_entities(
            IntPtr _this);
    }
}
