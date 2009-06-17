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
using DDS.OpenSplice;

namespace DDS
{
    internal class StatusCondition : Condition, IStatusCondition
    {
        internal StatusCondition(IntPtr gapiPtr)
            : base(gapiPtr)
        {
            // Base class handles everything.
        }

        public StatusKind GetEnabledStatuses()
        {
            return OpenSplice.Gapi.StatusCondition.get_enabled_statuses(GapiPeer);
        }

        public ReturnCode SetEnabledStatuses(StatusKind mask)
        {
            return OpenSplice.Gapi.StatusCondition.set_enabled_statuses(
                GapiPeer,
                mask);
        }

        public IEntity GetEntity()
        {
            IntPtr gapiPtr = OpenSplice.Gapi.StatusCondition.get_entity(GapiPeer);
            IEntity entity = SacsSuperClass.fromUserData(gapiPtr) as IEntity;
            return entity;
        }
    }
}
