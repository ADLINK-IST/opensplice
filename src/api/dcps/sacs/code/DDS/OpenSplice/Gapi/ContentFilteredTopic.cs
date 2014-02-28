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
    static internal class ContentFilteredTopic
    {
        /*     string
         *     get_filter_expression();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_contentFilteredTopic_get_filter_expression")]
        public static extern IntPtr get_filter_expression(
            IntPtr _this);

        /*     ReturnCode_t
         *     get_expression_parameters(
         *         inout StringSeq expression_parameters);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_contentFilteredTopic_get_expression_parameters")]
        public static extern ReturnCode get_expression_parameters(
            IntPtr _this,
            IntPtr expression_parameters
            );

        /*     ReturnCode_t
         *     set_expression_parameters(
         *         in StringSeq expression_parameters);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_contentFilteredTopic_set_expression_parameters")]
        public static extern ReturnCode set_expression_parameters(
            IntPtr _this,
            IntPtr expression_parameters);

        /*     Topic
         *     get_related_topic();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_contentFilteredTopic_get_related_topic")]
        public static extern IntPtr get_related_topic(
            IntPtr _this);
    }
}
