// The OpenSplice DDS Community Edition project.
//
// Copyright (C) 2006 to 2009 PrismTech Limited and its licensees.
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
    static internal class TypeSupport
    {
        /* ReturnCode_t
         * register_type(
         *     in DomainParticipant domain,
         *     in string type_name);
         */
        // TODO: This call fails with BadParameter for some reason, so we use 
        // the FooTypeSupport version of this call
        //[DllImport("dcpsgapi", EntryPoint = "gapi_typeSupport_register_type")]
        //public static extern ReturnCode register_type (
        //    IntPtr _this,
        //    IntPtr domain,
        //    string name);

        /*     string
         *     get_type_name();
         */
        [DllImport("dcpsgapi", EntryPoint = "gapi_typeSupport_get_type_name")]
        public static extern IntPtr get_type_name(
            IntPtr _this);

        /* gapi_char *
         * get_description ();
         */
        [DllImport("dcpsgapi", EntryPoint = "gapi_typeSupport_get_description")]
        public static extern IntPtr get_description(
            IntPtr _this);

        /* gapi_string
         * get_key_list ();
         */
        [DllImport("dcpsgapi", EntryPoint = "gapi_typeSupport_get_key_list")]
        public static extern IntPtr get_key_list(
            IntPtr _this);
    }
}
