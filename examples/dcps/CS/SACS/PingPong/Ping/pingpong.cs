using DDS;
using System.Runtime.InteropServices;

namespace pingpong
{
    #region PP_min_msg
    [StructLayout(LayoutKind.Sequential)]
    public sealed class PP_min_msg
    {
        public uint block;
        public uint count;
    };
    #endregion

    #region PP_seq_msg
    [StructLayout(LayoutKind.Sequential)]
    public sealed class PP_seq_msg
    {
        public uint block;
        public uint count;
        public char[] payload = new char[0];
    };
    #endregion

    #region PP_string_msg
    [StructLayout(LayoutKind.Sequential)]
    public sealed class PP_string_msg
    {
        public uint block;
        public uint count;
        public string a_string = "";
    };
    #endregion

    #region PP_fixed_msg
    [StructLayout(LayoutKind.Sequential)]
    public sealed class PP_fixed_msg
    {
        public uint block;
        public uint count;
        public char a_char;
        public byte a_octet;
        public short a_short;
        public ushort a_ushort;
        public int a_long;
        public uint a_ulong;
        public long a_longlong;
        public ulong a_ulonglong;
        public float a_float;
        public double a_double;
        public bool a_boolean;
        public string a_bstring = "";
    };
    #endregion

    #region PP_array_msg
    [StructLayout(LayoutKind.Sequential)]
    public sealed class PP_array_msg
    {
        public uint block;
        public uint count;
        public char[] str_arr_char = new char[10];
        public byte[] str_arr_octet = new byte[10];
        public short[] str_arr_short = new short[10];
        public ushort[] str_arr_ushort = new ushort[10];
        public int[] str_arr_long = new int[10];
        public uint[] str_arr_ulong = new uint[10];
        public long[] str_arr_longlong = new long[10];
        public ulong[] str_arr_ulonglong = new ulong[10];
        public float[] str_arr_float = new float[10];
        public double[] str_arr_double = new double[10];
        public bool[] str_arr_boolean = new bool[11];
    };
    #endregion

    #region PP_quit_msg
    [StructLayout(LayoutKind.Sequential)]
    public sealed class PP_quit_msg
    {
        public bool quit;
    };
    #endregion

}

