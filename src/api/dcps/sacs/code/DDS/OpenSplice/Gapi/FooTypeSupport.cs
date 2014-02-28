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
    static internal class FooTypeSupport
    {
        /* ReturnCode_t
         * register_type(
         *     in DomainParticipant domain,
         *     in string type_name);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_fooTypeSupport_register_type")]
        public static extern ReturnCode register_type(
            IntPtr _this,
            IntPtr domain,
            string name);

        [DllImport("ddskernel", EntryPoint = "gapi_fooTypeSupport__alloc")]
        public static extern IntPtr alloc(
            string type_name,
            string type_keys,
            string type_def,
            IntPtr type_load,
            Delegate copy_in,
            Delegate copy_out,
            uint alloc_size,
            IntPtr alloc_buffer,
            IntPtr writer_copy,
            Delegate reader_copy);
    }
}
