// The OpenSplice DDS Community Edition project.
//
// Copyright (C) 2006 to 2009 PrismTech Limited and its licensees.
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
    /// <summary>
    /// This class is the abstract base class for all the DCPS objects. It acts as a generic class for Entity objects.
    /// </summary>
    public class Entity : SacsSuperClass, IEntity
    {
        internal Entity(IntPtr gapiPtr)
            : base(gapiPtr)
        {
            // Base class handles everything.
        }

        /// <summary>
        /// This operation enables the Entity on which it is being called when the Entity was created 
        /// with the EntityFactoryQosPolicy set to FALSE.
        /// </summary>
        /// <returns>Return codes are:Ok,Error,AlreadyDeleted,OutOfResources or PreconditionNotMet.</returns>
        public ReturnCode Enable()
        {
            return Gapi.Entity.enable(GapiPeer);
        }

        /// <summary>
        /// This property allows access to the StatusCondition associated with the Entity.
        /// </summary>
        public IStatusCondition StatusCondition
        {
            get
            {
                IntPtr gapiPtr = Gapi.Entity.get_statuscondition(GapiPeer);
                IStatusCondition statusCondition = (IStatusCondition)SacsSuperClass.fromUserData(gapiPtr);
                if (gapiPtr != null && statusCondition == null)
                {
                    statusCondition = new StatusCondition(gapiPtr);
                }
                return statusCondition;
            }
        }

        /// <summary>
        /// This operation returns a mask with the communication statuses in the Entity that are “triggered”.
        /// </summary>
        public StatusKind StatusChanges
        {
            get
            {
                return Gapi.Entity.get_status_changes(GapiPeer);
            }
        }

        /// <summary>
        /// This operation returns the InstanceHandle of the builtin topic sample that represents the specified Entity.
        /// </summary>
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