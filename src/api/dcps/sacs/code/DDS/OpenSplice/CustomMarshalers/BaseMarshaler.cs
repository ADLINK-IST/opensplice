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
using System.Runtime.InteropServices;
using System.Text;

namespace DDS.OpenSplice.CustomMarshalers
{
    public abstract class BaseMarshaler
    {
        #region Writers
        public static void Write(IntPtr to, int offset, long from)
        {
            Marshal.WriteInt64(to, offset, from);
        }

        public static void Write(IntPtr to, int offset, ulong from)
        {
            Marshal.WriteInt64(to, offset, (long)from);
        }

        public static void Write(IntPtr to, int offset, int from)
        {
            Marshal.WriteInt32(to, offset, from);
        }

        public static void Write(IntPtr to, int offset, uint from)
        {
            Marshal.WriteInt32(to, offset, (int)from);
        }

        public static void Write(IntPtr to, int offset, short from)
        {
            Marshal.WriteInt16(to, offset, from);
        }

        public static void Write(IntPtr to, int offset, ushort from)
        {
            Marshal.WriteInt16(to, offset, (short)from);
        }

        public static void Write(IntPtr to, int offset, bool from)
        {
            Marshal.WriteByte(to, offset, (byte)(from ? 1 : 0));
        }

        public static void Write(IntPtr to, int offset, byte from)
        {
            Marshal.WriteByte(to, offset, from);
        }

        public static void Write(IntPtr to, int offset, char from)
        {
            Marshal.WriteByte(to, offset, (byte)from);
        }

        [StructLayout(LayoutKind.Explicit)]
        private struct DoubleToLong
        {
            [FieldOffset(0)]
            public double theDouble;
            [FieldOffset(0)]
            public long theLong;
        }

        public static void Write(IntPtr to, int offset, double from)
        {
            DoubleToLong dtl = new DoubleToLong();
            dtl.theDouble = from;
            Marshal.WriteInt64(to, offset, dtl.theLong);
        }

        [StructLayout(LayoutKind.Explicit)]
        private struct SingleToInt
        {
            [FieldOffset(0)]
            public float theSingle;
            [FieldOffset(0)]
            public int theInt;
        }

        public static void Write(IntPtr to, int offset, float from)
        {
            SingleToInt sti = new SingleToInt();
            sti.theSingle = from;
            Marshal.WriteInt32(to, offset, sti.theInt);
        }

        public static void Write(IntPtr to, int offset, Duration from)
        {
            Marshal.WriteInt32(to, offset, from.Sec);
            Marshal.WriteInt32(to, offset + 4, (int)from.NanoSec);
        }

        public static void Write(IntPtr to, int offset, Time from)
        {
            Marshal.WriteInt32(to, offset, from.Sec);
            Marshal.WriteInt32(to, offset + 4, (int)from.NanoSec);
        }

        public static void Write(IntPtr to, int offset, InstanceHandle from)
        {
            Marshal.WriteInt64(to, offset, from);
        }

        public static void Write(IntPtr basePtr, IntPtr to, int offset, ref string from)
        {
            IntPtr strPtr = Database.c.stringNew(basePtr, from);
            Marshal.WriteIntPtr(to, offset, strPtr);
        }

        public static void Write(IntPtr basePtr, IntPtr to, int offset, ref object[] from)
        {
            //IntPtr strPtr = Gapi.DDSDatabase.stringNew(basePtr, from);
            //Marshal.WriteIntPtr(to, offset, strPtr);
        }

        public static void Write(IntPtr to, int offset, IntPtr from)
        {
            Marshal.WriteIntPtr(to, offset, from);
        }

        #endregion

        #region Readers
        public static long ReadInt64(IntPtr from, int offset)
        {
            return Marshal.ReadInt64(from, offset);
        }

        public static ulong ReadUInt64(IntPtr from, int offset)
        {
            return (ulong)Marshal.ReadInt64(from, offset);
        }

        public static int ReadInt32(IntPtr from, int offset)
        {
            return Marshal.ReadInt32(from, offset);
        }

        public static uint ReadUInt32(IntPtr from, int offset)
        {
            return (uint)Marshal.ReadInt32(from, offset);
        }

        public static short ReadInt16(IntPtr from, int offset)
        {
            return Marshal.ReadInt16(from, offset);
        }

        public static ushort ReadUInt16(IntPtr from, int offset)
        {
            return (ushort)Marshal.ReadInt16(from, offset);
        }

        public static bool ReadBoolean(IntPtr from, int offset)
        {
            return Marshal.ReadByte(from, offset) != 0;
        }

        public static byte ReadByte(IntPtr from, int offset)
        {
            return Marshal.ReadByte(from, offset);
        }

        public static char ReadChar(IntPtr from, int offset)
        {
            return (char)Marshal.ReadByte(from, offset);
        }

        public static double ReadDouble(IntPtr from, int offset)
        {
            DoubleToLong dtl = new DoubleToLong();
            dtl.theLong = Marshal.ReadInt64(from, offset);
            return dtl.theDouble;
        }

        public static float ReadSingle(IntPtr from, int offset)
        {
            SingleToInt sti = new SingleToInt();
            sti.theInt = Marshal.ReadInt32(from, offset);
            return sti.theSingle;
        }

        public static Duration ReadDuration(IntPtr from, int offset)
        {
            return new Duration(Marshal.ReadInt32(from, offset),
                (uint)Marshal.ReadInt32(from, offset + 4));
        }

        public static Time ReadTime(IntPtr from, int offset)
        {
            return new Time(Marshal.ReadInt32(from, offset),
                (uint)Marshal.ReadInt32(from, offset + 4));
        }

        public static InstanceHandle ReadInstanceHandle(IntPtr from, int offset)
        {
            return new InstanceHandle(Marshal.ReadInt64(from, offset));
        }

        public static string ReadString(IntPtr from, int offset)
        {
            IntPtr stringPtr = Marshal.ReadIntPtr(new IntPtr(from.ToInt64() + offset));
            return stringPtr != IntPtr.Zero ? Marshal.PtrToStringAnsi(stringPtr) : "";
        }

        public static object[] ReadArray(IntPtr basePtr, IntPtr from, int offset)
        {
            throw new NotImplementedException();
        }

        public static IntPtr ReadIntPtr(IntPtr from, int offset)
        {
            return Marshal.ReadIntPtr(from, offset);
        }

        #endregion
    }
}
