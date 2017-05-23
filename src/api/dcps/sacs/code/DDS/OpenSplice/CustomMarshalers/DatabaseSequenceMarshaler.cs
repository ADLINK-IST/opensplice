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
﻿//using System;
//using System.Collections.Generic;
//using System.Text;
//
//namespace DDS.OpenSplice.CustomMarshalers
//{
//    internal class DatabaseSequenceOctetMarshaler : BaseMarshaler
//    {
//        internal static void CopyOut(IntPtr from, out byte[] to, int offset)
//        {
//            to = null;
//            IntPtr arrayPtr = ReadIntPtr(from, offset);
//            int length = Database.c.arraySize(arrayPtr);
//            if (length > 0)
//            {
//                to = new byte[length];  // Initialize managed array to the correct size.
//                for (int index = 0; index < length; index++)
//                {
//                    to[index] = ReadByte(arrayPtr, index);
//                }
//            }
//        }
//    }
//}
// 