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
using DDS.OpenSplice.CustomMarshalers;

namespace DDS
{
    public class WaitSet : SacsSuperClass, IWaitSet
    {
        public WaitSet()
            : base(OpenSplice.Gapi.WaitSet.alloc(), true)
        { }

        public ReturnCode Wait(ref ICondition[] activeConditions, Duration timeout)
        {
            ReturnCode result = ReturnCode.Error;

            using (SequenceMarshaler<ICondition, Condition> marshaler = new SequenceMarshaler<ICondition, Condition>(activeConditions))
            {
                result = OpenSplice.Gapi.WaitSet.wait(
                    GapiPeer,
                    marshaler.GapiPtr,
                    ref timeout);

                if (result == ReturnCode.Ok || result == ReturnCode.Timeout)
                {
                    marshaler.CopyOut(ref activeConditions);
                }
            }

            return result;
        }

        public ReturnCode AttachCondition(ICondition condition)
        {
            Condition conditionObj = (Condition)condition;
            return OpenSplice.Gapi.WaitSet.attach_condition(
                GapiPeer,
                conditionObj.GapiPeer);
        }

        public ReturnCode DetachCondition(ICondition condition)
        {
            Condition conditionObj = (Condition)condition;
            return OpenSplice.Gapi.WaitSet.detach_condition(
                GapiPeer,
                conditionObj.GapiPeer);
        }

        public ReturnCode GetConditions(ref ICondition[] attachedConditions)
        {
            ReturnCode result = ReturnCode.Error;

            using (SequenceMarshaler<ICondition, Condition> marshaler = new SequenceMarshaler<ICondition, Condition>())
            {
                result = OpenSplice.Gapi.WaitSet.get_conditions(
                    GapiPeer,
                    marshaler.GapiPtr);

                if (result == ReturnCode.Ok)
                {
                    marshaler.CopyOut(ref attachedConditions);
                }
            }

            return result;
        }
    }
}
