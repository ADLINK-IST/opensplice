/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

using System;
using System.Runtime.InteropServices;

namespace DDS.OpenSplice.Database
{
    public enum c_metaKind {
        M_UNDEFINED,
        M_ATTRIBUTE, M_CLASS, M_COLLECTION, M_CONSTANT, M_CONSTOPERAND,
        M_ENUMERATION, M_EXCEPTION, M_EXPRESSION, M_INTERFACE,
        M_LITERAL, M_MEMBER, M_MODULE, M_OPERATION, M_PARAMETER,
        M_PRIMITIVE, M_RELATION, M_BASE, M_STRUCTURE, M_TYPEDEF,
        M_UNION, M_UNIONCASE,
        M_EXTENT, M_EXTENTSYNC,
        M_COUNT
    }

    public enum c_primKind {
        P_UNDEFINED,
        P_ADDRESS, P_BOOLEAN, P_CHAR, P_WCHAR, P_OCTET,
        P_SHORT, P_USHORT, P_LONG, P_ULONG, P_LONGLONG, P_ULONGLONG,
        P_FLOAT, P_DOUBLE, P_VOIDP,
        P_MUTEX, P_LOCK, P_COND,
        P_COUNT
    }

    public enum c_collKind {
        C_UNDEFINED,
        C_LIST, C_ARRAY, C_BAG, C_SET, C_MAP, C_DICTIONARY,
        C_SEQUENCE, C_STRING, C_WSTRING, C_QUERY, C_SCOPE,
        C_COUNT
    }

    public enum c_valueKind {
        V_UNDEFINED,
        V_ADDRESS, V_BOOLEAN, V_OCTET,
        V_SHORT,   V_LONG,   V_LONGLONG,
        V_USHORT,  V_ULONG,  V_ULONGLONG,
        V_FLOAT,   V_DOUBLE,
        V_CHAR,    V_STRING,
        V_WCHAR,   V_WSTRING,
        V_FIXED,   V_OBJECT,
        V_VOIDP,
        V_COUNT
    }

    [StructLayout(LayoutKind.Explicit)]
    public struct c_value
    {
        [FieldOffset(0)]
        public c_valueKind kind;
        [FieldOffset(8)]
        public IntPtr Address;
        [FieldOffset(8)]
        public short Short;
        [FieldOffset(8)]
        public int Long;
        [FieldOffset(8)]
        public long LongLong;
        [FieldOffset(8)]
        public byte Octet;
        [FieldOffset(8)]
        public ushort UShort;
        [FieldOffset(8)]
        public uint ULong;
        [FieldOffset(8)]
        public ulong ULongLong;
        [FieldOffset(8)]
        public byte Char;
        [FieldOffset(8)]
        public char WChar;
        [FieldOffset(8)]
        public float Float;
        [FieldOffset(8)]
        public double Double;
        [FieldOffset(8)]
        public IntPtr String;
        [FieldOffset(8)]
        public IntPtr WString;
        [FieldOffset(8)]
        public IntPtr Fixed;
        [FieldOffset(8)]
        public byte Boolean;
        [FieldOffset(8)]
        public IntPtr Object;
        [FieldOffset(8)]
        public IntPtr Voidp;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct c_time
    {
        public int seconds;
        public uint nanoseconds;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct os_timeW
    {
        public ulong wt;
    }

    static public class q
    {
        // from c_base.h

        //OS_API void
        //q_parse (
        //    const c_char *expression);
        [DllImport("ddskernel", EntryPoint = "q_parse", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr parse(string expression);

        //OS_API q_expr
        //q_dispose (
        //    q_expr expr);
        [DllImport("ddskernel", EntryPoint = "q_dispose", CallingConvention = CallingConvention.Cdecl)]
        public static extern void dispose(IntPtr expr);
    }

    static public class c
    {
        // from c_base.h

        //OS_API void
        //c_free (
        //    c_object object);
        [DllImport("ddskernel", EntryPoint = "c_free", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr free(IntPtr _this);

        //OS_API c_string
        //c_stringNew (
        //    c_base _this,
        //    const c_char *str);
        [DllImport("ddskernel", EntryPoint = "c_stringNew_s", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr stringNew(IntPtr _this, string str);

        //OS_API c_base
        //c_getBase (
        //    c_object object);
        [DllImport("ddskernel", EntryPoint = "c_getBase", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr getBase(IntPtr _object);

        //OS_API c_type
        //c_resolve (
        //    c_base _this,
        //    const c_char *typeName);
        [DllImport("ddskernel", EntryPoint = "c_resolve", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr resolve(IntPtr _this, string typeName);

        //OS_API c_array
        //c_arrayNew (
        //    c_type subType,
        //    c_long size);
        [DllImport("ddskernel", EntryPoint = "c_arrayNew_s", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr arrayNew(IntPtr subType, int size);

        //OS_API c_array
        //c_newArray (
        //    c_collectionType arrayType,
        //    c_long size);
        [DllImport("ddskernel", EntryPoint = "c_newBaseArrayObject_s", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr newArray(IntPtr collectionType, int size);

        //OS_API c_sequence
        //c_sequenceNew (
        //    c_type subType,
        //    c_long maxsize,
        //    c_long size);
        [DllImport("ddskernel", EntryPoint = "c_sequenceNew_s", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr sequenceNew(IntPtr subType, int maxsize, int size);

        //OS_API c_sequence
        //c_newSequence (
        //    c_collectionType seqType,
        //    c_long size);
        [DllImport("ddskernel", EntryPoint = "c_newBaseArrayObject_s", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr newSequence(IntPtr collectionType, int size);

        // from c_metabase.h

        //OS_API c_metaObject
        //c_metaResolve(
        //    c_metaObject scope,
        //    const c_char *name);
        [DllImport("ddskernel", EntryPoint = "c_metaResolve", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr metaResolve(IntPtr scope, string name);

        //OS_API c_specifier
        //c_metaResolveSpecifier(
        //    c_metaObject scope,
        //    const c_char *name);
        [DllImport("ddskernel", EntryPoint = "c_metaResolveSpecifier", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr metaResolveSpecifier(IntPtr scope, string name);

        //OS_API c_long
        //c_arraySize(
        //    c_array _this);
        [DllImport("ddskernel", EntryPoint = "c_arraySize", CallingConvention = CallingConvention.Cdecl)]
        public static extern int arraySize(IntPtr _this);

        //OS_API c_type
        //c_metaArrayTypeNew(
        //    c_metaObject scope,
        //    const c_char *name,
        //    c_type subType,
        //    c_long maxSize);
        [DllImport("ddskernel", EntryPoint = "c_metaArrayTypeNew", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr metaArrayTypeNew(IntPtr scope, string name, IntPtr subType, int maxSize);

        //OS_API c_type
        //c_metaSequenceTypeNew(
        //    c_metaObject scope,
        //    const c_char *name,
        //    c_type subType,
        //    c_long maxSize);
        [DllImport("ddskernel", EntryPoint = "c_metaSequenceTypeNew", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr metaSequenceTypeNew(IntPtr scope, string name, IntPtr subType, int maxSize);

        //OS_API c_type
        //c_getType (
        //    c_object object);
        [DllImport("ddskernel", EntryPoint = "c_getType", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr getType(IntPtr _object);

        //OS_API c_type
        //c_typeActualType (
        //    c_type type);
        [DllImport("ddskernel", EntryPoint = "c_typeActualType", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr typeActualType(IntPtr _type);

        //#define c_specifierType(_this) \
        //        c_type(c_specifier(_this)->type)
        public static IntPtr specifierType(IntPtr _this)
        {
            c_specifier specifier = Marshal.PtrToStructure(_this, typeof(c_specifier)) as c_specifier;
            return specifier.type;
        }

        //OS_API c_long
        //c_iterLength (
        //    c_iter iter);
        [DllImport("ddskernel", EntryPoint = "c_iterLength", CallingConvention = CallingConvention.Cdecl)]
        public static extern int iterLength(IntPtr iter);

        //OS_API void *
        //c_iterTakeFirst (
        //    c_iter iter);
        [DllImport("ddskernel", EntryPoint = "c_iterTakeFirst", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr iterTakeFirst(IntPtr iter);

        //OS_API void *
        //c_iterFree (
        //    c_iter iter);
        [DllImport("ddskernel", EntryPoint = "c_iterFree", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr iterFree(IntPtr iter);
    }

#pragma warning disable 169
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
    public class c_specifier
    {
        //c_metaKind kind
        public int kind;

        //c_string name; char*
        public IntPtr name;
        //c_type type;
        public IntPtr type;
    }

    [StructLayout(LayoutKind.Sequential)]
    public class c_member : c_specifier
    {
        //c_address offset;      /* implementation memory mapping */
        public IntPtr offset;
    }

    // NOTE: In version 3.4.3 arraySize is a macro, so we have to implment the macro!

    [StructLayout(LayoutKind.Sequential)]
    public class c_collectionType
    {
        //c_metaKind kind
        public int kind;

        //c_metaObject definedIn;
        public IntPtr definedIn;
        //c_string name;
        public IntPtr name;

        //c_address alignment;
        public IntPtr alignment;
        //c_base base;
        public IntPtr _base;
        //c_ulong objectCount;
        public uint objectCount;
        //c_address size;
        public IntPtr size;

        //c_collKind kind;
        public int kind2;
        //c_long maxSize;
        public int maxSize;
        //c_type subType;
        public IntPtr subType;
    }
}
#pragma warning restore 169
