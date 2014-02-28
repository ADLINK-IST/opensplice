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
    static internal class Entity
    {
       /* ReturnCode_t
        *     enable();
        */
        [DllImport("ddskernel", EntryPoint = "gapi_entity_enable")]
        public static extern ReturnCode enable(IntPtr _this);

        /*
         *     StatusCondition
         *     get_statuscondition();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_entity_get_statuscondition")]
        public static extern IntPtr get_statuscondition(IntPtr _this);

        /*
         *     StatusKindMask
         *     get_status_changes();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_entity_get_status_changes")]
        public static extern StatusKind get_status_changes(IntPtr _this);

        /*
         *     InstanceHandle_t
         *     get_instance_handle();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_entity_get_instance_handle")]
        public static extern long get_instance_handle(IntPtr _this);

       /*
        * // ----------------------------------------------------------------------
        * // Administrative gapi functions.
        * // ----------------------------------------------------------------------
        */
        [DllImport("ddskernel", EntryPoint = "gapi_object_set_user_data")]
        public static extern void set_user_data(IntPtr _this, IntPtr userData, Delegate deleteAction, IntPtr deleteActionArg);

        [DllImport("ddskernel", EntryPoint = "gapi_object_get_user_data")]
        public static extern IntPtr get_user_data(IntPtr _this);
    }
}
