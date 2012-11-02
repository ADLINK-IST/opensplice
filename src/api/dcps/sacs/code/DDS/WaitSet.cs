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
using DDS.OpenSplice.CustomMarshalers;

namespace DDS
{
    /// <summary>
    /// A WaitSet object allows an application to wait until one or more of the attached 
    /// Condition objects evaluates to true or until the timeout expires. 
    /// The WaitSet has no factory and must be created by the application. It is directly 
    /// created as an object by using WaitSet constructors.
    /// </summary>
    public class WaitSet : SacsSuperClass, IWaitSet
    {
        /// <summary>
        /// The default WaitSet constructor. Creates a WaitSet object.
        /// </summary>
        public WaitSet()
        {
            IntPtr ptr = OpenSplice.Gapi.WaitSet.alloc();
            if (ptr != IntPtr.Zero)
            {
                SetPeer(ptr, true);
            }
            else
            {
                // Gapi already logged that the WaitSet has not been created 
                // successfully. Now create a deliberate null pointer exception
                // to let the current constructor fail.
                throw new System.NullReferenceException("gapi_waitSet__alloc returned a NULL pointer.");
            }
        }

        /// <summary>
        /// This operation allows an application thread to wait for the occurrence of at least one 
        /// of the conditions that is attached to the WaitSet.
        /// </summary>
        /// <param name="activeConditions">a sequence which is used to pass the list of all the attached 
        /// conditions that have a trigger_value of true.</param>
        /// <param name="timeout">the maximum duration to block for the wait, after which the application thread 
        /// is unblocked. The special constant Infinite can be used when the maximum waiting time does not 
        /// need to be bounded.</param>
        /// <returns>Possible return codes for the operation are: 
        /// Ok, Error, OutOfResources, Timeout or PreconditionNotMet</returns>
        public ReturnCode Wait(ref ICondition[] activeConditions, Duration timeout)
        {
            ReturnCode result = ReturnCode.Error;

            using (SequenceMarshaler<ICondition, Condition> marshaler = 
                    new SequenceMarshaler<ICondition, Condition>(activeConditions))
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

        /// <summary>
        /// This operation attaches a condition to the WaitSet
        /// </summary>
        /// <param name="condition">The condition to be attached to the WaitSet. 
        /// The parameter must be either a ReadCondition, QueryCondition, StatusCondition or GuardCondition</param>
        /// <returns>Possible return codes are: Ok,Error,BadParameter or OutOfResources.</returns>
        public ReturnCode AttachCondition(ICondition condition)
        {
            Condition conditionObj = (Condition)condition;
            return OpenSplice.Gapi.WaitSet.attach_condition(
                    GapiPeer,
                    conditionObj.GapiPeer);
        }

        /// <summary>
        /// This operation detaches a Condition from the WaitSet. If the Condition was not attached 
        /// to this WaitSet, the operation returns PreconditionNotMet
        /// </summary>
        /// <param name="condition">The attached condition in the WaitSet which is to be detached.</param>
        /// <returns>Possible return codes are: Ok, Error, BadParameter, OutOfResources or PreconditionNotMet.</returns>
        public ReturnCode DetachCondition(ICondition condition)
        {
            Condition conditionObj = (Condition)condition;
            return OpenSplice.Gapi.WaitSet.detach_condition(
                    GapiPeer,
                    conditionObj.GapiPeer);
        }

        /// <summary>
        /// This operation retrieves the list of attached conditions.
        /// </summary>
        /// <param name="attachedConditions">A reference to a sequence that will hold all the attached conditions
        /// on the WaitSet</param>
        /// <returns>Possible return codes are: Ok, Error or OutOfResources.</returns>
        public ReturnCode GetConditions(ref ICondition[] attachedConditions)
        {
            ReturnCode result = ReturnCode.Error;

            using (SequenceMarshaler<ICondition, Condition> marshaler = 
                    new SequenceMarshaler<ICondition, Condition>())
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
