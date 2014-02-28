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
    [StructLayout(LayoutKind.Sequential)]
    public class _TypeSupport 
    {
        //gapi_object 
        public IntPtr   handle;
        //gapi_unsigned_long
        public uint		refCount;
        //gapi_string
        public string   type_name;
        //gapi_string        
        public string   type_keys;
        //gapi_string
        public string   type_def;
        //gapi_typeSupportLoad
        public IntPtr   type_load;
        //gapi_unsigned_long
        public uint     alloc_size;
        //gapi_topicAllocBuffer
        public IntPtr   alloc_buffer;
        //gapi_copyIn
        public Delegate copy_in;
        //gapi_copyOut
        public Delegate copy_out;
        //gapi_copyCache
        public IntPtr   copy_cache;
        //gapi_readerCopy
        public Delegate reader_copy;
        //gapi_writerCopy
        public IntPtr   writer_copy;
        //c_metaObject
        public IntPtr   typeSpec;
        //gapi_boolean
        [MarshalAs(UnmanagedType.U1)]
        public bool useTypeinfo;
    };

    static internal class TypeSupport
    {
        /**
         * The 'Claim' returns not only a reference to the C# representation of the _TypeSupport,
         * but also a pointer to its C-representation (as an 'out' parameter.)
         * This is necessary since the 'Release' operation will need to marshall the changes
         * back into the original C-location. Because of this, the signatures of both the
         * 'Claim' and 'Release' operations have 1 extra parameter when compared the ones on 
         * the Gapi.
         **/
        public static _TypeSupport Claim(IntPtr tsHandle, out IntPtr tsPtr, ref ReturnCode result)
        {
            tsPtr = Gapi.Object.Claim(tsHandle, _ObjectKind.OBJECT_KIND_TYPESUPPORT, ref result);
            return Marshal.PtrToStructure(tsPtr, typeof(_TypeSupport)) as _TypeSupport;
        }
    
        public static IntPtr Release(_TypeSupport typeSupport, IntPtr tsPtr)
        {
            Marshal.StructureToPtr(typeSupport, tsPtr, false);
            return Gapi.Object.Release(tsPtr);
        }
    
        /* ReturnCode_t
         * register_type(
         *     in DomainParticipant domain,
         *     in string type_name);
         */
        // TODO: This call fails with BadParameter for some reason, so we use 
        // the FooTypeSupport version of this call
        //[DllImport("ddskernel", EntryPoint = "gapi_typeSupport_register_type")]
        //public static extern ReturnCode register_type (
        //    IntPtr _this,
        //    IntPtr domain,
        //    string name);

        /*     string
         *     get_type_name();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_typeSupport_get_type_name")]
        public static extern IntPtr get_type_name(
            IntPtr _this);

        /* gapi_char *
         * get_description ();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_typeSupport_get_description")]
        public static extern IntPtr get_description(
            IntPtr _this);

        /* gapi_string
         * get_key_list ();
         */
        [DllImport("ddskernel", EntryPoint = "gapi_typeSupport_get_key_list")]
        public static extern IntPtr get_key_list(
            IntPtr _this);
    }
}
