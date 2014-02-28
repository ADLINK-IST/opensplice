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
    static internal class Condition
    {
        /*
         *     boolean
         *     get_trigger_value();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_condition_get_trigger_value")]
        public static extern byte get_trigger_value(
            IntPtr _this);
    }

    static internal class GuardCondition
    {
        /*     GuardCondition
         *     GuardCondition__alloc (
         *         void);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_guardCondition__alloc")]
        public static extern IntPtr alloc();

        /*     ReturnCode_t
         *     set_trigger_value(
         *         in boolean value);
         * };
         */
        [DllImport("ddskernel", EntryPoint = "gapi_guardCondition_set_trigger_value")]
        public static extern ReturnCode set_trigger_value(
            IntPtr _this,
            byte value);
    }

    static internal class StatusCondition
    {
        /*     StatusKindMask
         *     get_enabled_statuses();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_statusCondition_get_enabled_statuses")]
        public static extern StatusKind get_enabled_statuses(
            IntPtr _this);

        /*     ReturnCode_t
         *     set_enabled_statuses(
         *         in StatusKindMask mask);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_statusCondition_set_enabled_statuses")]
        public static extern ReturnCode set_enabled_statuses(
            IntPtr _this,
            StatusKind mask);

        /*     Entity
         *     get_entity();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_statusCondition_get_entity")]
        public static extern IntPtr get_entity(
            IntPtr _this);
    }

    static internal class ReadCondition
    {
        /*     SampleStateMask
         *     get_sample_state_mask();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_readCondition_get_sample_state_mask")]
        public static extern SampleStateKind get_sample_state_mask(
            IntPtr _this);

        /*     ViewStateMask
         *     get_view_state_mask();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_readCondition_get_view_state_mask")]
        public static extern ViewStateKind get_view_state_mask(
            IntPtr _this);

        /*     InstanceStateMask
         *     get_instance_state_mask();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_readCondition_get_instance_state_mask")]
        public static extern InstanceStateKind get_instance_state_mask(
            IntPtr _this);

        /*     DataReader
         *     get_datareader();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_readCondition_get_datareader")]
        public static extern IntPtr get_datareader(
            IntPtr _this);

        /*     DataReader
         *     get_datareaderview();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_readCondition_get_datareaderview")]
        public static extern IntPtr get_datareaderview(
            IntPtr _this);

    }

    static internal class QueryCondition
    {
        /*     string
         *     get_query_expression();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_queryCondition_get_query_expression")]
        public static extern IntPtr get_query_expression(
            IntPtr _this);

        /*     ReturnCode_t
         *       get_query_parameters(
         *             inout StringSeq query_parameters);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_queryCondition_get_query_parameters")]
        public static extern ReturnCode get_query_parameters(
            IntPtr _this,
            IntPtr query_parameters
            );

        /*     ReturnCode_t
         *     set_query_parameters(
         *         in StringSeq query_parameters);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_queryCondition_set_query_parameters")]
        public static extern ReturnCode set_query_parameters(
            IntPtr _this,
            IntPtr query_parameters
            );
    }
}
