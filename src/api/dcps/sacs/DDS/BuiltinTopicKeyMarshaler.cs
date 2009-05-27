using System;
using System.Runtime.InteropServices;
using DDS;

namespace DDS
{
    public class BuiltinTopicKeyMarshaler
    {
        private static bool firstPass = true;
        private static uint offset_SystemId;
        private static uint offset_LocalId;
        private static uint offset_Serial;

        private static void InitOffsets(IntPtr c_base)
        {
            //uint[] offsets = OpenSplice.TypeSupport.GetOffsets(c_base, "DDS_BuiltinTopicKey", 3);

            //offset_SystemId = offsets[0];
            //offset_LocalId = offsets[1];
            //offset_Serial = offsets[2];


            //IntPtr dbPtr = OpenSplice.Gapi.DDSDatabase.metaResolve(c_base, "DDS_BuiltinTopicKey");

            //OpenSplice.Gapi.c_structure dbType = Marshal.PtrToStructure(dbPtr, typeof(OpenSplice.Gapi.c_structure))
            //    as OpenSplice.Gapi.c_structure;

            //// assert this
            //if (OpenSplice.Gapi.DDSDatabase.arraySize(dbType.members) != 3) { }

            //int ptrSize = Marshal.SizeOf(typeof(IntPtr));

            //// SystemId
            //IntPtr memberPtr = dbType.members;
            //OpenSplice.Gapi.c_member dbMember = Marshal.PtrToStructure(dbType.members, typeof(OpenSplice.Gapi.c_member))
            //    as OpenSplice.Gapi.c_member;
            //offset_SystemId = (uint)dbMember.offset.ToInt32();

            //// LocalId
            //memberPtr = new IntPtr(dbType.members.ToInt64() + ptrSize * 1);
            //dbMember = Marshal.PtrToStructure(memberPtr, typeof(OpenSplice.Gapi.c_member))
            //    as OpenSplice.Gapi.c_member;
            //offset_LocalId = (uint)dbMember.offset.ToInt32();

            //// Serial
            //memberPtr = new IntPtr(dbType.members.ToInt64() + ptrSize * 2);
            //dbMember = Marshal.PtrToStructure(memberPtr, typeof(OpenSplice.Gapi.c_member))
            //    as OpenSplice.Gapi.c_member;
            //offset_Serial = (uint)dbMember.offset.ToInt32();

        }



        public static void CopyIn(IntPtr basePtr, /*BuiltinTopicKey*/ IntPtr from, IntPtr to)//, int offset)
        {
            if (firstPass)
            {
                firstPass = false;
                InitOffsets(basePtr);
            }

            // Normal copying semantics here.
        }

        public static void CopyOut(IntPtr basePtr, IntPtr from, /*BuiltinTopicKey*/ IntPtr to)//, int offset)
        {
            if (firstPass)
            {
                firstPass = false;
                InitOffsets(basePtr);
            }

            // Normal copying semantics here.
        }
    }


    public class ParticipantBuiltinTopicDataMarshaler
    {
        private static bool FirstPass = true;
        private static uint offset_Key;
        private static uint offset_UserData;

        private static void InitOffsets(IntPtr c_base)
        {

            //uint[] offsets = OpenSplice.TypeSupport.GetOffsets(c_base, "DDS_BuiltinTopicKey", 2);

            //offset_Key = offsets[0];
            //offset_UserData = offsets[1];
        }


        public static bool CopyIn(IntPtr c_base, /*ParticipantBuiltinTopicData*/ IntPtr from, IntPtr to)//, int offset)
        {
            //if (FirstPass) InitOffsets(c_base);

            // Normal copying semantics here.
            throw new NotImplementedException();
        }

        public static void CopyOut(IntPtr from, /*ParticipantBuiltinTopicData*/ ref object to, int offset)
        {
            //if (FirstPass) InitOffsets(c_base);

            // Normal copying semantics here.
            throw new NotImplementedException();

        }
    }

}