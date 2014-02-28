using System;

namespace DDS.OpenSplice.CustomMarshalers
{
    /** 
     * Common base class for all Marshalers that marshal to
     * and from gapi datatypes.
     */ 
    public abstract class GapiMarshaler : BaseMarshaler, IDisposable
    {
        protected bool cleanupRequired = false;
        private readonly IntPtr gapiPtr;

        public GapiMarshaler(IntPtr nativePtr)
        {
            gapiPtr = nativePtr;
        }

        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        public abstract void Dispose();
    }

}