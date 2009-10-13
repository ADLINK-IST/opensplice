using System;
using System.Collections.Generic;
using System.Text;

namespace DDS.OpenSplice.CustomMarshalers
{
    internal class DatabaseSequenceOctetMarshaler : BaseMarshaler
    {
        internal static void CopyOut(IntPtr from, out byte[] to, int offset)
        {
            to = null;
            IntPtr arrayPtr = ReadIntPtr(from, offset);
            int length = Database.c.arraySize(arrayPtr);
            if (length > 0)
            {
                to = new byte[length];  // Initialize managed array to the correct size.
                for (int index = 0; index < length; index++)
                {
                    to[index] = ReadByte(arrayPtr, index);
                }
            }
        }
    }
}
 