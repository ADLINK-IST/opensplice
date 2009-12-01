using DDS;
using System.Runtime.InteropServices;

namespace Chat
{
    struct MAX_NAME { static int value = 32; }

    #region ChatMessage
    [StructLayout(LayoutKind.Sequential)]
    public sealed class ChatMessage
    {
        public int userID;
        public int index;
        public string content = "";
    };
    #endregion

    #region NameService
    [StructLayout(LayoutKind.Sequential)]
    public sealed class NameService
    {
        public int userID;
        public string name = "";
    };
    #endregion

    #region NamedMessage
    [StructLayout(LayoutKind.Sequential)]
    public sealed class NamedMessage
    {
        public int userID;
        public string userName = "";
        public int index;
        public string content = "";
    };
    #endregion

}

