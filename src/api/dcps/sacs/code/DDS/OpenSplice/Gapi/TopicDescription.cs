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
    static internal class TopicDescription
    {
        /*     string
         *     get_type_name();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_topicDescription_get_type_name")]
        public static extern IntPtr get_type_name(
            IntPtr _this);

        /*     string
         *     get_name();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_topicDescription_get_name")]
        public static extern IntPtr get_name(
            IntPtr _this);

        /*     DomainParticipant
         *     get_participant();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_topicDescription_get_participant")]
        public static extern IntPtr get_participant(
            IntPtr _this);
    }
}
