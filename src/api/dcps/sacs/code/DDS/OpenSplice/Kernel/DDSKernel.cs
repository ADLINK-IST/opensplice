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
using System.Runtime.InteropServices;

namespace DDS.OpenSplice.Kernel
{
    static public class DataReaderInstance
    {
        // from v_dataReaderInstance.h

        //OS_API c_ulong
        //v_dataReaderInstanceGetNotEmptyInstanceCount (
        //    v_dataReaderInstance _this);
        [DllImport("ddskernel", EntryPoint = "v_dataReaderInstanceGetNotEmptyInstanceCount")]
        public static extern uint GetNotEmptyInstanceCount(IntPtr _this);
    }
    
    static public class InstanceHandle
    {
        // from u_instanceHandle.h

        //OS_API u_instanceHandle
        //u_instanceHandleNew (
        //    v_public object);
        [DllImport("ddsuser", EntryPoint = "u_instanceHandleNew")]
        public static extern long New(IntPtr obj);
    }
}