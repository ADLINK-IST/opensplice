using System;

namespace DDS.OpenSplice
{
    public interface IMarshalerTypeGenerator
    {
        BaseMarshaler CreateMarshaler(IntPtr participant, IntPtr metaData, Type dataType);
    }
}
        
    