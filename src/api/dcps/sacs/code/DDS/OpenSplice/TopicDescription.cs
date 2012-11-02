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
using System.Runtime.InteropServices;
using DDS;

namespace DDS.OpenSplice
{
    internal class TopicDescription : SacsSuperClass, ITopicDescription
    {
        internal TopicDescription(IntPtr gapiPtr)
            : base(gapiPtr)
        {
            // Base class handles everything.
        }
        
        public string TypeName
        {
            get
            {
                IntPtr ptr = Gapi.TopicDescription.get_type_name(GapiPeer);
                string result = Marshal.PtrToStringAnsi(ptr);
                Gapi.GenericAllocRelease.Free(ptr);

                return result;
            }
        }
        
        public string Name
        {
            get
            {
                IntPtr ptr = Gapi.TopicDescription.get_name(GapiPeer);
                string result = Marshal.PtrToStringAnsi(ptr);
                Gapi.GenericAllocRelease.Free(ptr);

                return result;
            }
        }
        
        public IDomainParticipant Participant
        {
            get
            {
                IntPtr gapiPtr = Gapi.TopicDescription.get_participant(GapiPeer);
                IDomainParticipant domainParticipant = 
                        SacsSuperClass.fromUserData(gapiPtr) as IDomainParticipant;
                return domainParticipant;
            }
        }
    }
}
