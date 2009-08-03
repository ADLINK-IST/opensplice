using System;

namespace DDS.OpenSplice
{
    public interface IMarshalerTypeGenerator
    {
        BaseMarshaler CreateMarshaler(IntPtr participant, Type dataType, string typeName, string[] attNames, int[] attOffsets);
    }
}
        
    