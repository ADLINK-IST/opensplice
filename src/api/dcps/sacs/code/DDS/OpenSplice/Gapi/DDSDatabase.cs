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
    static internal class DDSDatabase
    {
        // from c_base.h

		//OS_API void
		//c_free (
		//    c_object object);
		[DllImport("ddsdatabase", EntryPoint = "c_free")]
		public static extern IntPtr free(IntPtr _this);

        //OS_API c_string
        //c_stringNew (
        //    c_base _this,
        //    const c_char *str);
        [DllImport("ddsdatabase", EntryPoint = "c_stringNew")]
        public static extern IntPtr stringNew(IntPtr _this, string str);

        //OS_API c_array
        //c_arrayNew (
        //    c_type subType,
        //    c_long size);
        [DllImport("ddsdatabase", EntryPoint = "c_arrayNew")]
        public static extern IntPtr arrayNew(IntPtr subType, int size);

		//OS_API c_array
		//c_newArray (
		//    c_collectionType arrayType,
		//    c_long size);
        [DllImport("ddsdatabase", EntryPoint = "c_newArray")]
        public static extern IntPtr newArray(IntPtr collectionType, int size);

        // from c_metabase.h

        //OS_API c_metaObject
        //c_metaResolve(
        //    c_metaObject scope,
        //    const c_char *name);
        [DllImport("ddsdatabase", EntryPoint = "c_metaResolve")]
        public static extern IntPtr metaResolve(IntPtr scope, string name);

        //OS_API c_long
        //c_arraySize(
        //    c_array _this);
        [DllImport("ddsdatabase", EntryPoint = "c_arraySize")]
        public static extern int arraySize(IntPtr _this);

		//OS_API c_type
		//c_metaArrayTypeNew(
		//    c_metaObject scope,
		//    const c_char *name,
		//    c_type subType,
		//    c_long maxSize);
		[DllImport("ddsdatabase", EntryPoint = "c_metaArrayTypeNew")]
		public static extern IntPtr metaArrayTypeNew(IntPtr scope, string name, IntPtr subType, int maxSize);

        //OS_API c_type
        //c_getType (
        //    c_object object);
        [DllImport("ddsdatabase", EntryPoint = "c_getType")]
        public static extern IntPtr getType(IntPtr _object);
    }

    [StructLayout(LayoutKind.Sequential)]
    public class c_structure
    {
        //c_metaKind kind
        int kind;

        //c_metaObject definedIn;
        IntPtr definedIn;
        //c_string name;
        IntPtr name;

        //c_address alignment;
        IntPtr alignment;
        //c_base base;
        IntPtr _base;
        //c_ulong objectCount;
        public uint objectCount;
        //c_address size;
        IntPtr size;

        // in v 3.4.3
        //c_array blobs; 
        //IntPtr blobs;

        //c_array members;    /* c_member */
        public IntPtr members;
        //c_array references; /* optimization */
        IntPtr references;
        //c_scope scope;
        IntPtr scope;
    }

    [StructLayout(LayoutKind.Sequential)]
    public class c_member
    {
        //c_metaKind kind
        public int kind;

        //c_string name; char*
        public IntPtr name;
        //c_type type;
        public IntPtr type;

        //c_address offset;      /* implementation memory mapping */
        public IntPtr offset;
    }

    // NOTE: In version 3.4.3 arraySize is a macro, so we have to implment the macro!

    [StructLayout(LayoutKind.Sequential)]
    public class c_collectionType
    {
        //c_metaKind kind
        int kind;

        //c_metaObject definedIn;
        IntPtr definedIn;
        //c_string name;
        public IntPtr name;

        //c_address alignment;
        IntPtr alignment;
        //c_base base;
        IntPtr _base;
        //c_ulong objectCount;
        public uint objectCount;
        //c_address size;
        IntPtr size;

        //c_collKind kind;
        int kind2;
        //c_long maxSize;
        public int maxSize;
        //c_type subType;
        IntPtr subType;
    }
}
