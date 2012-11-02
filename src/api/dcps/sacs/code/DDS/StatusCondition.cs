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
using DDS.OpenSplice;

namespace DDS
{
    /// <summary>
    /// Specialized class of Condition, and indicates the condition that may be attached to
    /// a WaitSet.
    /// Entity objects that have status attributes also have a StatusCondition, access is 
    /// provided to the application by the GetStatusCondition operation.
    /// The communication statuses whose changes can be communicated to the application 
    /// depend on the Entity.
    /// The trigger_value of the StatusCondition depends on the communication
    /// statuses of that Entity (e.g., missed deadline) and also depends on the value of the
    /// StatusCondition attribute mask (enabled_statuses mask). A
    /// StatusCondition can be attached to a WaitSet in order to allow an application
    /// to suspend until the trigger_value has become TRUE.
    /// The trigger_value of a StatusCondition will be TRUE if one of the enabled
    /// StatusChangedFlags is set. That is, trigger_value==FALSE only if all the
    /// values of the StatusChangedFlags are FALSE.
    /// The sensitivity of the StatusCondition to a particular communication status is
    /// controlled by the list of enabled_statuses set on the condition by means of the
    /// set_enabled_statuses operation.
    /// When the enabled_statuses are not changed by the SetEnabledStatuses
    /// operation, all statuses are enabled by default.
    /// </summary>
    internal class StatusCondition : Condition, IStatusCondition
    {
        internal StatusCondition(IntPtr gapiPtr)
            : base(gapiPtr)
        {
            // Base class handles everything.
        }
        
        /// <summary>
        /// This operation returns the list of enabled communication statuses of the StatusCondition.
        /// </summary>
        /// <returns>StatusKind - a bit mask in which each bit shows which status is taken into account 
        /// for the StatusCondition</returns>
        public StatusKind GetEnabledStatuses()
        {
            return OpenSplice.Gapi.StatusCondition.get_enabled_statuses(GapiPeer);
        }
        /// <summary>
        /// This operation sets the list of communication statuses that are taken into account to 
        /// determine the trigger_value of the StatusCondition.
        /// </summary>
        /// <param name="mask">A bit mask in which each bit sets the status which is taken into
        /// account to determine the trigger_value of the StatusCondition</param>
        /// <returns>ReturnCode - Possible return codes of the operation are: Ok, Error or AlreadyDeleted.</returns>
        public ReturnCode SetEnabledStatuses(StatusKind mask)
        {
            return OpenSplice.Gapi.StatusCondition.set_enabled_statuses(
                GapiPeer,
                mask);
        }
        /// <summary>
        /// This operation returns the Entity associated with the StatusCondition or a
        /// null Entity.
        /// </summary>
        /// <returns>IEntity - a pointer to the Entity associated with the StatusCondition.</returns>
        public IEntity GetEntity()
        {
            IntPtr gapiPtr = OpenSplice.Gapi.StatusCondition.get_entity(GapiPeer);
            IEntity entity = SacsSuperClass.fromUserData(gapiPtr) as IEntity;
            return entity;
        }
    }
}
