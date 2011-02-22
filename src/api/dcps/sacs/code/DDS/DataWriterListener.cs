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
using System.Collections.Generic;
using DDS.OpenSplice.CustomMarshalers;

namespace DDS
{
    public abstract class DataWriterListener : IDataWriterListener
    {
        public virtual void OnOfferedDeadlineMissed(IDataWriter entityInterface, OfferedDeadlineMissedStatus status)
        {
        }
        public virtual void OnOfferedIncompatibleQos(IDataWriter entityInterface, OfferedIncompatibleQosStatus status)
        {
        }
        public virtual void OnLivelinessLost(IDataWriter entityInterface, LivelinessLostStatus status)
        {
        }
        public virtual void OnPublicationMatched(IDataWriter entityInterface, PublicationMatchedStatus status)
        {
        }


    }
}
