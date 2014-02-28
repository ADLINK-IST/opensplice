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
using DDS.OpenSplice.Database;

namespace DDS.OpenSplice.Gapi {

    static internal class MetaData {
        /* gapi_type
         *     gapi_typeActualType(gapi_type typeBase);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_typeActualType")]
        public static extern IntPtr typeActualType(IntPtr typeBase);

        /* c_metaKind
         *     gapi_metaData_baseObjectKind(gapi_baseObject objBase);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_metaData_baseObjectKind")]
        public static extern c_metaKind baseObjectKind(IntPtr objBase);

        /* gapi_type
         *     gapi_metaData_specifierType(gapi_specifier specBase);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_metaData_specifierType")]
        public static extern IntPtr specifierType(IntPtr specBase);

        /* const gapi_char *
         *     gapi_metaData_specifierName(gapi_specifier specBase);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_metaData_specifierName")]
        public static extern IntPtr _specifierName(IntPtr specBase);

        public static string specifierName(IntPtr specBase)
        {
            return Marshal.PtrToStringAnsi(_specifierName(specBase));
        }

        /* gapi_long
         *     gapi_metaData_typeSize(gapi_type typeBase);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_metaData_typeSize")]
        public static extern int typeSize(IntPtr typeBase);


        /* gapi_long
         *     gapi_metaData_enumerationCount(gapi_enumeration enumBase);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_metaData_enumerationCount")]
        public static extern int enumerationCount(IntPtr enumBase);

        /* c_primKind
         *     gapi_metaData_primitiveKind(gapi_primitive primBase);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_metaData_primitiveKind")]
        public static extern c_primKind primitiveKind(IntPtr primBase);

        /* c_collKind
         *     gapi_metaData_collectionTypeKind(gapi_collectionType collBase);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_metaData_collectionTypeKind")]
        public static extern c_collKind collectionTypeKind(IntPtr collBase);

        /* gapi_long
         *     gapi_metaData_collectionTypeMaxSize(gapi_collectionType collBase);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_metaData_collectionTypeMaxSize")]
        public static extern int collectionTypeMaxSize(IntPtr collBase);

        /* gapi_type
         *     gapi_metaData_collectionTypeSubType(gapi_collectionType collBase);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_metaData_collectionTypeSubType")]
        public static extern IntPtr collectionTypeSubType(IntPtr collBase);

        /* gapi_long
         *     gapi_metaData_structureMemberCount(gapi_structure structBase);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_metaData_structureMemberCount")]
        public static extern int structureMemberCount(IntPtr structBase);

        /* gapi_member
         *     gapi_metaData_structureMember(gapi_structure structBase, c_long index);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_metaData_structureMember")]
        public static extern IntPtr structureMember(IntPtr structBase, int index);

        /* gapi_type
         *     gapi_metaData_memberType(gapi_member memberBase);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_metaData_memberType")]
        public static extern IntPtr memberType(IntPtr memberBase);

        /* gapi_unsigned_long
         *     gapi_metaData_memberOffset(gapi_member memberBase);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_metaData_memberOffset")]
        public static extern uint memberOffset(IntPtr memberBase);

        /* gapi_long
         *     gapi_metaData_unionUnionCaseCount(gapi_union unionBase);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_metaData_unionUnionCaseCount")]
        public static extern int unionUnionCaseCount(IntPtr unionBase);

        /* gapi_unionCase
         *     gapi_metaData_unionUnionCase(gapi_union unionBase, c_long index);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_metaData_unionUnionCase")]
        public static extern IntPtr unionUnionCase(IntPtr unionBase, int index);

        /* gapi_type
         *     gapi_metaData_unionCaseType(gapi_unionCase caseBase);
         */
        [DllImport("ddskernel", EntryPoint = "gapi_metaData_unionCaseType")]
        public static extern IntPtr unionCaseType(IntPtr caseBase);
    }

}
