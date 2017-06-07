/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

namespace DDS.OpenSplice.CustomMarshalers
{
    public abstract class BaseMarshaler
    {
        #region Writers
        public static bool Write(IntPtr to, int offset, long from)
        {
            Marshal.WriteInt64(to, offset, from);
            return true;
        }

        public static bool Write(IntPtr to, int offset, ulong from)
        {
            Marshal.WriteInt64(to, offset, (long)from);
            return true;
        }

        public static bool Write(IntPtr to, int offset, int from)
        {
            Marshal.WriteInt32(to, offset, from);
            return true;
        }

        public static bool Write(IntPtr to, int offset, uint from)
        {
            Marshal.WriteInt32(to, offset, (int)from);
            return true;
        }

        public static bool Write(IntPtr to, int offset, short from)
        {
            Marshal.WriteInt16(to, offset, from);
            return true;
        }

        public static bool Write(IntPtr to, int offset, ushort from)
        {
            Marshal.WriteInt16(to, offset, (short)from);
            return true;
        }

        public static bool Write(IntPtr to, int offset, bool from)
        {
            Marshal.WriteByte(to, offset, (byte)(from ? 1 : 0));
            return true;
        }

        public static bool Write(IntPtr to, int offset, byte from)
        {
            Marshal.WriteByte(to, offset, from);
            return true;
        }

        public static bool Write(IntPtr to, int offset, char from)
        {
            Marshal.WriteByte(to, offset, (byte)from);
            return true;
        }

        [StructLayout(LayoutKind.Explicit)]
        private struct DoubleToLong
        {
            [FieldOffset(0)]
            public double theDouble;
            [FieldOffset(0)]
            public long theLong;
        }

        public static bool Write(IntPtr to, int offset, double from)
        {
            DoubleToLong dtl = new DoubleToLong();
            dtl.theDouble = from;
            Marshal.WriteInt64(to, offset, dtl.theLong);
            return true;
        }

        [StructLayout(LayoutKind.Explicit)]
        private struct SingleToInt
        {
            [FieldOffset(0)]
            public float theSingle;
            [FieldOffset(0)]
            public int theInt;
        }

        public static bool Write(IntPtr to, int offset, float from)
        {
            SingleToInt sti = new SingleToInt();
            sti.theSingle = from;
            Marshal.WriteInt32(to, offset, sti.theInt);
            return true;
        }

        public static bool Write(IntPtr to, int offset, Duration from)
        {
            Marshal.WriteInt32(to, offset, from.Sec);
            Marshal.WriteInt32(to, offset + 4, (int)from.NanoSec);
            return true;
        }

        public static bool Write(ref Duration to, Duration from)
        {
            to.Sec = from.Sec;
            to.NanoSec = from.NanoSec;
            return true;
        }

        public static bool Write(IntPtr to, int offset, Time from)
        {
            Marshal.WriteInt32(to, offset, from.Sec);
            Marshal.WriteInt32(to, offset + 4, (int)from.NanoSec);
            return true;
        }

        public static bool Write(ref Time to, Time from)
        {
            to.Sec = from.Sec;
            to.NanoSec = from.NanoSec;
            return true;
        }

        public static bool Write(IntPtr to, int offset, InstanceHandle from)
        {
            Marshal.WriteInt64(to, offset, from);
            return true;
        }

        public static bool Write(IntPtr basePtr, IntPtr to, int offset, ref string from)
        {
            IntPtr strPtr = Database.c.stringNew(basePtr, from);
	    if (strPtr == IntPtr.Zero) return false;
            Marshal.WriteIntPtr(to, offset, strPtr);
            return true;
        }

        public static bool Write(IntPtr basePtr, ref IntPtr to, string from)
        {
            to = Database.c.stringNew(basePtr, from);
            if (to == IntPtr.Zero) return false;
            return true;
        }

        public static bool Write(IntPtr to, int offset, IntPtr from)
        {
            Marshal.WriteIntPtr(to, offset, from);
            return true;
        }

        public static void WriteString(ref IntPtr to, string from)
        {
            if (from != null) {
#if COMPACT_FRAMEWORK
                int strLength = from.Length;
                to = os.malloc(strLength + 1);
                Marshal.Copy(Encoding.ASCII.GetBytes(from), 0, to, strLength);
                Write(to, strLength, '\0');
#else
                to = Marshal.StringToHGlobalAnsi(from);
#endif
            } else {
                to = IntPtr.Zero;
            }
        }
        
        public static void ReleaseString(ref IntPtr to)
        {
            if (to != IntPtr.Zero) {
#if COMPACT_FRAMEWORK
                os.free(to);
#else
                Marshal.FreeHGlobal(to);
#endif
                to = IntPtr.Zero;
            }
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
        
        public static string ReadString(IntPtr from)
        {
            string result;
            
            if (from != IntPtr.Zero)
            {
#if COMPACT_FRAMEWORK
                int strLen = Gapi.GenericAllocRelease.string_len(from);
                byte[] strBuf = new byte[strLen];
                Marshal.Copy(from, strBuf, 0, strLen);
                result = Encoding.ASCII.GetString(strBuf, 0, strLen);
#else
                result = Marshal.PtrToStringAnsi(from);
#endif
            } else {
               result = null;
            }
            
            return result;
        }

        #endregion
    }
}
