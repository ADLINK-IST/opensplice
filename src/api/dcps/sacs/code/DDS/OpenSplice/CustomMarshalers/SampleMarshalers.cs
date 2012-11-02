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
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;
using System.Reflection;

namespace DDS.OpenSplice.CustomMarshalers
{
    internal static class SampleInfoMarshaler
    {

        internal static void CopyIn(OpenSplice.Gapi.gapi_readerInfo from, IntPtr to)
        {
            // No CopyIn needed: SampleInfo is never written into the database.
        }

        internal static void CopyOut(IntPtr from, ref SampleInfo toSampleInfo, int offset)
        {
            if (toSampleInfo == null) toSampleInfo = new SampleInfo();
            Marshal.PtrToStructure(new IntPtr(from.ToInt64() + offset), toSampleInfo);
        }
    }

#if FancySampleMarshaling
    public delegate T GetPropertyDelegate<T>();
    public delegate void SetPropertyDelegate<T>(T t);

    public class PropertyInvoker<T>
    {
        private readonly string name;
        public readonly GetPropertyDelegate<T> Get;
        public readonly SetPropertyDelegate<T> Set;

        public PropertyInvoker(object obj, string propertyName)
        {
            name = propertyName;
            string getter = "get_" + propertyName;
            string setter = "set_" + propertyName;
            Get = (GetPropertyDelegate<T>)Delegate.CreateDelegate(typeof(GetPropertyDelegate<T>), obj, getter);
            Set = (SetPropertyDelegate<T>)Delegate.CreateDelegate(typeof(SetPropertyDelegate<T>), obj, setter);
        }
    }

    public class PropertyContainer
    {
        private readonly Dictionary<int, PropertyInvoker<short>> int16Properties =
            new Dictionary<int, PropertyInvoker<short>>();

        public Dictionary<int, PropertyInvoker<short>> Int16Properties
        {
            get { return int16Properties; }
        }

        private readonly Dictionary<int, PropertyInvoker<ushort>> uint16Properties =
            new Dictionary<int, PropertyInvoker<ushort>>();

        public Dictionary<int, PropertyInvoker<ushort>> UInt16Properties
        {
            get { return uint16Properties; }
        }


        private readonly Dictionary<int, PropertyInvoker<int>> int32Properties = 
            new Dictionary<int, PropertyInvoker<int>>();

        public Dictionary<int, PropertyInvoker<int>> Int32Properties
        {
            get { return int32Properties; }
        }

        private readonly Dictionary<int, PropertyInvoker<uint>> uint32Properties =
            new Dictionary<int, PropertyInvoker<uint>>();

        public Dictionary<int, PropertyInvoker<uint>> UInt32Properties
        {
            get { return uint32Properties; }
        }

        private readonly Dictionary<int, PropertyInvoker<long>> int64Properties =
            new Dictionary<int, PropertyInvoker<long>>();

        public Dictionary<int, PropertyInvoker<long>> Int64Properties
        {
            get { return int64Properties; }
        }

        private readonly Dictionary<int, PropertyInvoker<ulong>> uint64Properties =
            new Dictionary<int, PropertyInvoker<ulong>>();

        public Dictionary<int, PropertyInvoker<ulong>> UInt64Properties
        {
            get { return uint64Properties; }
        }

        private readonly Dictionary<int, PropertyInvoker<float>> singleProperties =
            new Dictionary<int, PropertyInvoker<float>>();

        public Dictionary<int, PropertyInvoker<float>> SingleProperties
        {
            get { return singleProperties; }
        }

        private readonly Dictionary<int, PropertyInvoker<double>> doubleProperties =
            new Dictionary<int, PropertyInvoker<double>>();

        public Dictionary<int, PropertyInvoker<double>> DoubleProperties
        {
            get { return doubleProperties; }
        }

        private readonly Dictionary<int, PropertyInvoker<byte>> byteProperties =
            new Dictionary<int, PropertyInvoker<byte>>();

        public Dictionary<int, PropertyInvoker<byte>> ByteProperties
        {
            get { return byteProperties; }
        }

        private readonly Dictionary<int, PropertyInvoker<char>> charProperties =
            new Dictionary<int, PropertyInvoker<char>>();

        public Dictionary<int, PropertyInvoker<char>> CharProperties
        {
            get { return charProperties; }
        }

        private readonly Dictionary<int, PropertyInvoker<string>> stringProperties =
            new Dictionary<int, PropertyInvoker<string>>();

        public Dictionary<int, PropertyInvoker<string>> StringProperties
        {
            get { return stringProperties; }
        }

        private readonly Dictionary<int, PropertyInvoker<bool>> boolProperties = 
            new Dictionary<int, PropertyInvoker<bool>>();

        public Dictionary<int, PropertyInvoker<bool>> BoolProperties
        {
            get { return boolProperties; }
        }

        // need seq, arrays, unions???
    }

    static public class SampleMarshalerFactory
    {
        static public SampleMarshaler CreateMarshaler(object theData)
        {
            PropertyContainer propertyContainer = new PropertyContainer();

            Type type = theData.GetType();

            PropertyInfo[] properties = type.GetProperties();

            FieldInfo[] fields = type.GetFields(BindingFlags.Instance | BindingFlags.NonPublic);

            foreach (PropertyInfo property in properties)
            {
                try
                {
                    int offset = (int)Marshal.OffsetOf(type, GetFieldName(property.Name));

                    switch (property.PropertyType.Name)
                    {
                        case "Int32":
                            propertyContainer.Int32Properties.Add(offset,
                                new PropertyInvoker<int>(theData, property.Name));
                            break;
                        case "Boolean":
                            propertyContainer.BoolProperties.Add(offset,
                                new PropertyInvoker<bool>(theData, property.Name));
                            break;
                        case "String":
                            propertyContainer.StringProperties.Add(offset,
                                new PropertyInvoker<string>(theData, property.Name));
                            break;
                        case "Double":
                            propertyContainer.DoubleProperties.Add(offset,
                                new PropertyInvoker<double>(theData, property.Name));
                            break;
                        case "Single":
                            propertyContainer.SingleProperties.Add(offset,
                                new PropertyInvoker<float>(theData, property.Name));
                            break;
                        default:
                            Console.WriteLine("Unsupported field type in data for SampleMarshaler.");
                            break;
                    }
                }
                catch (Exception e)
                {
                    Console.WriteLine(e);
                }
            }

            return new SampleMarshaler(propertyContainer, Marshal.SizeOf(type));
        }

        private static string GetFieldName(string p)
        {
            return p.Substring(0,1).ToLower() + p.Substring(1);
        }
    }

    public class SampleMarshalHelper : IDisposable
    {
        private readonly SampleMarshaler marshaler;
        private readonly IntPtr gapiPtr;
        public IntPtr GapiPtr
        {
            get { return gapiPtr; }
        }

        private bool cleanupRequired;

        public SampleMarshalHelper(SampleMarshaler marshaler)
            : this(marshaler.Size)
        {
            this.marshaler = marshaler;
            this.marshaler.CopyIn(gapiPtr, 0);
            cleanupRequired = true;
        }

        public SampleMarshalHelper(int size)
        {
            gapiPtr = OpenSplice.Gapi.GenericAllocRelease.Alloc(size);
            //gapiPtr = Marshal.AllocHGlobal(size);
        }

        public void Dispose()
        {
            if (cleanupRequired)
            {
                marshaler.CleanupIn(gapiPtr, 0);
            }

            OpenSplice.Gapi.GenericAllocRelease.Free(gapiPtr);
            //Marshal.FreeHGlobal(gapiPtr);
        }
    }

    public class SampleMarshaler
    {
        [StructLayout(LayoutKind.Explicit)]
        private struct DoubleToLong
        {
            [FieldOffset(0)]
            public double theDouble;
            [FieldOffset(0)]
            public long theLong;
        }

        [StructLayout(LayoutKind.Explicit)]
        private struct SingleToInt
        {
            [FieldOffset(0)]
            public float theSingle;
            [FieldOffset(0)]
            public int theInt;
        }

        private readonly PropertyContainer propertyContainer;
        public readonly int Size;

        public SampleMarshaler(PropertyContainer propertyContainer, int size)
        {
            this.propertyContainer = propertyContainer;
            Size = size;
        }

        public void CopyIn(IntPtr gapiPtr, int offset)
        {
            // short to gapi int
            foreach (KeyValuePair<int, PropertyInvoker<short>> keyVal in propertyContainer.Int16Properties)
            {
                Marshal.WriteInt16(gapiPtr, offset + keyVal.Key, keyVal.Value.Get());
            }

            // ushort to gapi unsigned int
            foreach (KeyValuePair<int, PropertyInvoker<ushort>> keyVal in propertyContainer.UInt16Properties)
            {
                Marshal.WriteInt16(gapiPtr, offset + keyVal.Key, (short)keyVal.Value.Get());
            }

            // int to gapi long
            foreach (KeyValuePair<int, PropertyInvoker<int>> keyVal in propertyContainer.Int32Properties)
            {
                Marshal.WriteInt32(gapiPtr, offset + keyVal.Key, keyVal.Value.Get());
            }

            // uint to gapi unsigned long
            foreach (KeyValuePair<int, PropertyInvoker<uint>> keyVal in propertyContainer.UInt32Properties)
            {
                Marshal.WriteInt32(gapiPtr, offset + keyVal.Key, (int)keyVal.Value.Get());
            }


            // long to gapi long long
            foreach (KeyValuePair<int, PropertyInvoker<long>> keyVal in propertyContainer.Int64Properties)
            {
                Marshal.WriteInt64(gapiPtr, offset + keyVal.Key, keyVal.Value.Get());
            }

            // ulong to gapi unsigned long long
            foreach (KeyValuePair<int, PropertyInvoker<ulong>> keyVal in propertyContainer.UInt64Properties)
            {
                Marshal.WriteInt64(gapiPtr, offset + keyVal.Key, (long)keyVal.Value.Get());
            }

            // float
            foreach (KeyValuePair<int, PropertyInvoker<float>> keyVal in propertyContainer.SingleProperties)
            {
                // floats are actually Struct Single
                // IntPtr doesn't allow pointer arithmetic, so convert to long and add
                //IntPtr newPtr = new IntPtr(gapiPtr.ToInt64() + offset + keyVal.Key);
                //Marshal.StructureToPtr(keyVal.Value.Get(), newPtr, true);

                SingleToInt sti = new SingleToInt();
                sti.theSingle = keyVal.Value.Get();
                Marshal.WriteInt32(gapiPtr, offset + keyVal.Key, sti.theInt);
            }

            // double
            foreach (KeyValuePair<int, PropertyInvoker<double>> keyVal in propertyContainer.DoubleProperties)
            {
                // doubles are actually Struct Double
                // IntPtr doesn't allow pointer arithmetic, so convert to long and add
                //IntPtr newPtr = new IntPtr(gapiPtr.ToInt64() + offset + keyVal.Key);
                //Marshal.StructureToPtr(keyVal.Value.Get(), newPtr, true);

                DoubleToLong dtl = new DoubleToLong();
                dtl.theDouble = keyVal.Value.Get();
                Marshal.WriteInt64(gapiPtr, offset + keyVal.Key, dtl.theLong);
            }

            // byte to gapi octet
            foreach (KeyValuePair<int, PropertyInvoker<byte>> keyVal in propertyContainer.ByteProperties)
            {
                Marshal.WriteByte(gapiPtr, offset + keyVal.Key, keyVal.Value.Get());
            }

            // char to gapi char (only 1 byte)
            foreach (KeyValuePair<int, PropertyInvoker<char>> keyVal in propertyContainer.CharProperties)
            {
                // Only use the lower 8 bits or lower byte.
                Marshal.WriteByte(gapiPtr, offset + keyVal.Key, (byte)keyVal.Value.Get());
            }

            // string to gapi string
            foreach (KeyValuePair<int, PropertyInvoker<string>> keyVal in propertyContainer.StringProperties)
            {
                IntPtr stringPtr = Marshal.StringToHGlobalAnsi(keyVal.Value.Get());
                Marshal.WriteIntPtr(gapiPtr, offset + keyVal.Key, stringPtr);
            }

            // bool
            foreach (KeyValuePair<int, PropertyInvoker<bool>> keyVal in propertyContainer.BoolProperties)
            {
                Marshal.WriteByte(gapiPtr, offset + keyVal.Key, keyVal.Value.Get() ? (byte)1 : (byte)0);
            }

            // array type[] to gapi sequence<type>


            // jagged array type[][] to gapi sequence<sequence<type>>


            // fixed array type[] to gapi type[n]


            // multi-dim array type[,] to gapi type[][]


            // union

        }

        internal void CleanupIn(IntPtr gapiPtr, int offset)
        {
            foreach (KeyValuePair<int, PropertyInvoker<string>> keyVal in propertyContainer.StringProperties)
            {
                // Read the string pointer containing the string
                IntPtr stringPtr = Marshal.ReadIntPtr(gapiPtr, offset + keyVal.Key);
                Marshal.FreeHGlobal(stringPtr);
            }
        }

        public void CopyOut(IntPtr gapiPtr, int offset)
        {
            foreach (KeyValuePair<int, PropertyInvoker<short>> keyVal in propertyContainer.Int16Properties)
            {
                keyVal.Value.Set(Marshal.ReadInt16(gapiPtr, offset + keyVal.Key));
            }

            foreach (KeyValuePair<int, PropertyInvoker<ushort>> keyVal in propertyContainer.UInt16Properties)
            {
                keyVal.Value.Set((ushort)Marshal.ReadInt16(gapiPtr, offset + keyVal.Key));
            }

            foreach (KeyValuePair<int, PropertyInvoker<int>> keyVal in propertyContainer.Int32Properties)
            {
                keyVal.Value.Set(Marshal.ReadInt32(gapiPtr, offset + keyVal.Key));
            }

            foreach (KeyValuePair<int, PropertyInvoker<uint>> keyVal in propertyContainer.UInt32Properties)
            {
                keyVal.Value.Set((uint)Marshal.ReadInt32(gapiPtr, offset + keyVal.Key));
            }

            foreach (KeyValuePair<int, PropertyInvoker<long>> keyVal in propertyContainer.Int64Properties)
            {
                keyVal.Value.Set(Marshal.ReadInt64(gapiPtr, offset + keyVal.Key));
            }

            foreach (KeyValuePair<int, PropertyInvoker<ulong>> keyVal in propertyContainer.UInt64Properties)
            {
                keyVal.Value.Set((ulong)Marshal.ReadInt64(gapiPtr, offset + keyVal.Key));
            }

            foreach (KeyValuePair<int, PropertyInvoker<float>> keyVal in propertyContainer.SingleProperties)
            {
                //IntPtr newPtr = new IntPtr(gapiPtr.ToInt64() + offset + keyVal.Key);
                //keyVal.Value.Set((float)Marshal.PtrToStructure(newPtr, typeof(float)));
                SingleToInt sti = new SingleToInt();
                sti.theInt = Marshal.ReadInt32(gapiPtr, offset + keyVal.Key);
                keyVal.Value.Set(sti.theSingle);
            }

            foreach (KeyValuePair<int, PropertyInvoker<double>> keyVal in propertyContainer.DoubleProperties)
            {
                //IntPtr newPtr = new IntPtr(gapiPtr.ToInt64() + offset + keyVal.Key);
                //keyVal.Value.Set((double)Marshal.PtrToStructure(newPtr, typeof(double)));
                DoubleToLong dtl = new DoubleToLong();
                dtl.theLong = Marshal.ReadInt64(gapiPtr, offset + keyVal.Key);
                keyVal.Value.Set(dtl.theDouble);
            }

            foreach (KeyValuePair<int, PropertyInvoker<byte>> keyVal in propertyContainer.ByteProperties)
            {
                keyVal.Value.Set(Marshal.ReadByte(gapiPtr, offset + keyVal.Key));
            }

            foreach (KeyValuePair<int, PropertyInvoker<char>> keyVal in propertyContainer.CharProperties)
            {
                keyVal.Value.Set((char)Marshal.ReadByte(gapiPtr, offset + keyVal.Key));
            }

            foreach (KeyValuePair<int, PropertyInvoker<string>> keyVal in propertyContainer.StringProperties)
            {
                IntPtr stringPtr = Marshal.ReadIntPtr(gapiPtr, offset + keyVal.Key);
                keyVal.Value.Set(Marshal.PtrToStringAnsi(stringPtr));
            }

            foreach (KeyValuePair<int, PropertyInvoker<bool>> keyVal in propertyContainer.BoolProperties)
            {
                keyVal.Value.Set(Marshal.ReadByte(gapiPtr, offset + keyVal.Key) != 0);
            }

            // array type[] to gapi sequence<type>


            // jagged array type[][] to gapi sequence<sequence<type>>


            // fixed array type[] to gapi type[n]


            // multi-dim array type[,] to gapi type[][]


            // union
        }
    }
#endif
}
