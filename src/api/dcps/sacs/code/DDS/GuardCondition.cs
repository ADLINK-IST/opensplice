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

namespace DDS
{
    public class GuardCondition : Condition, IGuardCondition
    {
        public GuardCondition()
        {
            IntPtr ptr = OpenSplice.Gapi.GuardCondition.alloc();
            if (ptr != IntPtr.Zero)
            {
                SetPeer(ptr, true);
            }
            else
            {
                // Gapi already logged that the GuardCondition has not been created 
                // successfully. Now create a deliberate null pointer exception
                // to let the current constructor fail.
                throw new System.NullReferenceException("gapi_guardCondition__alloc returned a NULL pointer.");
            }
        }

        public ReturnCode SetTriggerValue(bool value)
        {
            byte byteValue = value ? (byte)1 : (byte)0;
            return OpenSplice.Gapi.GuardCondition.set_trigger_value(GapiPeer, byteValue);
        }
    }
}
