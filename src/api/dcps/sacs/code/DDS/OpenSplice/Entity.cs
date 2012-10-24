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
using DDS;

namespace DDS.OpenSplice
{
    public class Entity : SacsSuperClass, IEntity
    {
        internal Entity(IntPtr gapiPtr)
            : base(gapiPtr)
        {
            // Base class handles everything.
        }
        
        public ReturnCode Enable()
        {
            return Gapi.Entity.enable(GapiPeer);
        }
        
        public IStatusCondition StatusCondition
        {
            get
            {
                IntPtr gapiPtr = Gapi.Entity.get_statuscondition(GapiPeer);
                IStatusCondition statusCondition = (IStatusCondition)SacsSuperClass.fromUserData(gapiPtr);
                if (statusCondition == null)
                {
                    statusCondition = new StatusCondition(gapiPtr);
                }
                return statusCondition;
            }
        }
        
        public StatusKind StatusChanges
        {
            get
            {
                return Gapi.Entity.get_status_changes(GapiPeer);
            }
        }
        
        public InstanceHandle InstanceHandle
        {
            get
            {
                InstanceHandle handle = Gapi.Entity.get_instance_handle(GapiPeer);
                return handle;
            }
        }
    }
}
