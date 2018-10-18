using System;
using DDS.OpenSplice;
using DDS.OpenSplice.OS;
using System.Runtime.InteropServices;

namespace DDS.OpenSplice.CustomMarshalers
{
    /**
     * Common base class for all Marshalers that marshal to
     * and from gapi datatypes.
     */
    internal abstract class UserMarshaler<TUserType, TSacsType> : BaseMarshaler, IDisposable
            where TUserType : class, new()
            where TSacsType : class, new()
    {
        protected TUserType nativeImg = null;
        protected IntPtr userPtr;

        private bool cleanupRequired = true;
        private readonly Type type;
        private readonly int size;

        internal abstract DDS.ReturnCode CopyIn(TSacsType from, ref TUserType to);
        internal abstract void CleanupIn(ref TUserType to);
        internal abstract void CopyOut(TUserType from, ref TSacsType to);

        internal void InitTypes(ref Type _type, ref int _size)
        {
            _type = typeof(TUserType);
            _size = Marshal.SizeOf(type);
        }

        internal UserMarshaler()
        {
            InitTypes(ref type, ref size);
            userPtr = os.malloc(new IntPtr(size));
        }

        internal UserMarshaler(IntPtr nativePtr, bool cleanupRequired)
        {
            InitTypes(ref type, ref size);
            this.userPtr = nativePtr;
            this.cleanupRequired = cleanupRequired;
        }

        internal IntPtr UserPtr
        {
            get { return userPtr; }
        }

        public void Dispose()
        {
            if (cleanupRequired)
            {
                if (nativeImg != null)
                {
                    CleanupIn(ref nativeImg);
                }
                os.free(userPtr);
            }
        }

        internal DDS.ReturnCode CopyIn(TSacsType from)
        {
            DDS.ReturnCode result;

            nativeImg = new TUserType();
            result = CopyIn(from, ref nativeImg);
            if (result == DDS.ReturnCode.Ok)
            {
                Marshal.StructureToPtr(nativeImg, userPtr, false);
            }
            return result;
        }

        internal void CopyOut(ref TSacsType to)
        {
            nativeImg = (TUserType) Marshal.PtrToStructure(userPtr, type);
            if (to == null) to = new TSacsType();
            CopyOut(nativeImg, ref to);
        }
    }
}
