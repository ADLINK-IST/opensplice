using System;
using DDS;

namespace DDS
{
    public class ParticipantBuiltinTopicDataTypeSupport : DDS.OpenSplice.TypeSupport
    {
        private static string TypeName = "DDS::ParticipantBuiltinTopicData";
        private static string KeyList = "key";
        private static string MetaDescriptor = "<MetaData version=\"1.0.0\"><Module name=\"DDS\"><TypeDef name=\"BuiltinTopicKey_t\"><Array size=\"3\"><Long/></Array></TypeDef><Struct name=\"UserDataQosPolicy\"><Member name=\"value\"><Sequence><Octet/></Sequence></Member></Struct><Struct name=\"ParticipantBuiltinTopicData\"><Member name=\"key\"><Type name=\"DDS::BuiltinTopicKey_t\"/></Member><Member name=\"user_data\"><Type name=\"DDS::UserDataQosPolicy\"/></Member></Struct></Module></MetaData>";

        public ParticipantBuiltinTopicDataTypeSupport()
            : base(TypeName,
                    KeyList,
                    MetaDescriptor,
                    ParticipantBuiltinTopicDataMarshaler.CopyIn,
                    ParticipantBuiltinTopicDataMarshaler.CopyOut,
                    ReaderAlloc)
        { }


        //public static void CopyIn(object from, IntPtr to)
        //{
        //    // Generate some marshaling stuff here.
        //}

        //public static void CopyOut(IntPtr from, object to)
        //{
        //    // Generate some unmarshaling stuff here.
        //}

        public static object[] ReaderAlloc(int length)
        {
            throw new NotImplementedException();
        }

        // Note: These overrides allow us to cast to the specific DataWriter/DataReader(s) for 
        // our type. This removes the need for "Helpers"
        public override DDS.OpenSplice.DataWriter CreateDataWriter(IntPtr gapiPtr)
        {
            throw new NotImplementedException();
        }

        public override DDS.OpenSplice.DataReader CreateDataReader(IntPtr gapiPtr)
        {
            throw new NotImplementedException();
        }

        protected override void InitType(IntPtr participant)
        {
            throw new NotImplementedException();
        }
    }
}