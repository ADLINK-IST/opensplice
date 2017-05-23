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
using DDS.OpenSplice;
using DDS.OpenSplice.OS;
using System.Runtime.InteropServices;

namespace DDS.OpenSplice.CustomMarshalers
{
    /** 
     * Common base class for all Marshalers that marshal to
     * and from gapi datatypes.
     */ 
    internal abstract class UserMarshaler<TUserType, TSacsType> : BaseMarshaler, IDisposable
            where TUserType : class, new()
            where TSacsType : class, new()
    {
        protected TUserType nativeImg = null;
        protected IntPtr userPtr;

        private bool cleanupRequired = true;
        private readonly Type type;
        private readonly int size;

        internal abstract DDS.ReturnCode CopyIn(TSacsType from, ref TUserType to);
        internal abstract void CleanupIn(ref TUserType to);
        internal abstract void CopyOut(TUserType from, ref TSacsType to);

        internal void InitTypes(ref Type _type, ref int _size)
        {
            _type = typeof(TUserType);
            _size = Marshal.SizeOf(type);
        }

        internal UserMarshaler()
        {
            InitTypes(ref type, ref size);
            userPtr = os.malloc(new IntPtr(size));
        }

        internal UserMarshaler(IntPtr nativePtr, bool cleanupRequired)
        {
            InitTypes(ref type, ref size);
            this.userPtr = nativePtr;
            this.cleanupRequired = cleanupRequired;
        }

        internal IntPtr UserPtr
        {
            get { return userPtr; }
        }

        public void Dispose()
        {
            if (cleanupRequired) 
            {
                if (nativeImg != null)
                {
                    CleanupIn(ref nativeImg);
                }
                os.free(userPtr);
            }
        }

        internal DDS.ReturnCode CopyIn(TSacsType from)
        {
            DDS.ReturnCode result;
            
            nativeImg = new TUserType();
            result = CopyIn(from, ref nativeImg);
            if (result == DDS.ReturnCode.Ok)
            {
                Marshal.StructureToPtr(nativeImg, userPtr, false);
            }
            return result;
        }
        
        internal void CopyOut(ref TSacsType to)
        {
            nativeImg = (TUserType) Marshal.PtrToStructure(userPtr, type);
            if (to == null) to = new TSacsType();
            CopyOut(nativeImg, ref to);
        }
    }
}