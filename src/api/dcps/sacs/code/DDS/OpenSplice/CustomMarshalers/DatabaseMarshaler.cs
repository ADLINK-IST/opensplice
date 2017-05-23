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
using DDS.OpenSplice;
using DDS.OpenSplice.Kernel;

namespace DDS.OpenSplice.CustomMarshalers
{
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate V_COPYIN_RESULT SampleCopyInDelegate(IntPtr basePtr, IntPtr from, IntPtr to);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void SampleCopyOutDelegate(IntPtr from, IntPtr to);

    public abstract class DatabaseMarshaler : BaseMarshaler
    {
        public static Dictionary<KeyValuePair<IDomainParticipant, Type>, DatabaseMarshaler> typeMarshalers =
                new Dictionary<KeyValuePair<IDomainParticipant, Type>, DatabaseMarshaler>();

        public abstract V_COPYIN_RESULT CopyIn(IntPtr basePtr, IntPtr from, IntPtr to);
        public abstract void CopyOut(IntPtr from, IntPtr to);

        private SampleCopyInDelegate copyInDelegate;
        private SampleCopyOutDelegate copyOutDelegate;

        public abstract void InitEmbeddedMarshalers(IDomainParticipant participant);

        public SampleCopyInDelegate CopyInDelegate
        {
            get
            {
                return copyInDelegate;
            }
        }

        public SampleCopyOutDelegate CopyOutDelegate
        {
            get
            {
                return copyOutDelegate;
            }
        }

        public DatabaseMarshaler()
        {
            copyInDelegate = CopyIn;
            copyOutDelegate = CopyOut;
        }

        public static void Add(
                IDomainParticipant participant,
                Type t,
                DatabaseMarshaler marshaler)
        {
            DatabaseMarshaler tmp;

            // Check if a Marshaler for this type already exists, and if not, add it.
            if (!typeMarshalers.TryGetValue(new KeyValuePair<IDomainParticipant, Type>(participant, t), out tmp))
            {
                // Add the new marshaler to the list of known marshalers.
                typeMarshalers.Add(new KeyValuePair<IDomainParticipant, Type>(participant, t), marshaler);
            }
        }

        public static DatabaseMarshaler GetMarshaler(
                IDomainParticipant participant,
                Type t)
        {
            DatabaseMarshaler marshaler;

            // Check if a Marshaler for this type already exists, and if so return it.
            typeMarshalers.TryGetValue(new KeyValuePair<IDomainParticipant, Type>(participant, t), out marshaler);
            return marshaler;
        }
        
        public static void initObjectSeq(object[] src, object[] target)
        {
            if (src != null)
            {
                int nrElements = Math.Min(target.Length, src.Length);
                for (int i = 0; i < nrElements; i++)
                {
                    target[i] = src[i];
                }
            }
        }
    }
}
