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
using System.Diagnostics;
using DDS;
using DDS.OpenSplice;
using DDS.OpenSplice.OS;
using DDS.OpenSplice.Kernel;
using DDS.OpenSplice.User;
using DDS.OpenSplice.Database;
using DDS.OpenSplice.CustomMarshalers;

namespace DDS
{
    internal struct WaitActionArg
    {
        internal int nrTriggeredConditions;
        internal int maxConditions;
        internal ICondition[] triggeredConditions;
        internal IGuardCondition[] attachedGuards;

        internal WaitActionArg(ICondition[] preAllocated)
        {
            nrTriggeredConditions = 0;
            maxConditions = 0;
            triggeredConditions = preAllocated;
            attachedGuards = null;
        }

        internal void Add(ICondition cond)
        {
            /*
             * Try to keep the amount of allocations minimal, so assume the preallocated
             * array is big enough. If that is not the case, then allocate the worst-case
             * size, so that there is never more than 1 reallocation.
             */
            ++nrTriggeredConditions;
            if ((triggeredConditions == null) || (triggeredConditions.Length < nrTriggeredConditions))
            {
                ICondition[] tmpCondArr = new ICondition[maxConditions];
                if (triggeredConditions != null) {
                    for (int i = 0; i < triggeredConditions.Length; i++)
                    {
                        tmpCondArr[i] = triggeredConditions[i];
                    }
                }
                triggeredConditions = tmpCondArr;
            }
            triggeredConditions[nrTriggeredConditions - 1] = cond;
        }

        internal ICondition[] Finalize()
        {
            /*
             * Make sure the array that is to be returned is not too big. Replace with a smaller
             * array when needed.
             */
            if (triggeredConditions != null && nrTriggeredConditions != triggeredConditions.Length)
            {
                ICondition[] tmpCondArr = new ICondition[nrTriggeredConditions];
                for(int i = 0; i < nrTriggeredConditions; i++)
                {
                    tmpCondArr[i] = triggeredConditions[i];
                }
                triggeredConditions = tmpCondArr;
            }
            return triggeredConditions;
        }
    }

    /// <summary>
    /// A WaitSet object allows an application to wait until one or more of the attached
    /// Condition objects evaluates to true or until the timeout expires.
    /// The WaitSet has no factory and must be created by the application. It is directly
    /// created as an object by using WaitSet constructors.
    /// </summary>
    public class WaitSet : SacsSuperClass, IWaitSet
    {
        private List<ICondition> conditionList = new List<ICondition>();
        private List<IGuardCondition> guardList = new List<IGuardCondition>();

        /// <summary>
        /// The default WaitSet constructor. Creates a WaitSet object.
        /// </summary>
        public WaitSet()
        {
            init();
        }

        internal ReturnCode init()
        {
            ReturnCode result;
            IntPtr uWaitSet = DDS.OpenSplice.User.WaitSet.New2();

            if (uWaitSet != IntPtr.Zero)
            {
                result = base.init(uWaitSet, true);
            }
            else
            {
                result = DDS.ReturnCode.OutOfResources;
                ReportStack.Report(result, "u_waitsetNew returned NULL");
                Environment.Exit(-1);
            }
            return result;
        }

        internal override ReturnCode wlReq_deinit()
        {
            ReturnCode result = DDS.ReturnCode.Ok;

            // In normal circumstances, no more Conditions can be attached to the WaitSet at
            // this point in time, since each attached condition has a ref to the WaitSet,
            // avoiding it from becoming garbage collected.
            // However, when an exit handler terminates the application or the application
            // runs out of its main, then both the waitset and its conditions may still be
            // alive, and so both their destructors may be called in random order.
            // Because of that scenario, we walk over the condition list and remove all
            // atached conditions.
            DDS.OpenSplice.User.WaitSet.AnnounceDestruction(rlReq_UserPeer);
            ICondition[] condArr = conditionList.ToArray();
            foreach(ICondition cond in condArr)
            {
                Condition condImpl = cond as Condition;
                result = condImpl.DetachFromWaitSet(this);
                if (result != DDS.ReturnCode.Ok) break;
            }
            if (result == DDS.ReturnCode.Ok)
            {
                IGuardCondition[] guardArr = guardList.ToArray();
                foreach(IGuardCondition guard in guardArr)
                {
                    GuardCondition guardImpl = guard as GuardCondition;
                    result = guardImpl.DetachFromWaitSet(this);
                    if (result != DDS.ReturnCode.Ok) break;
                }
            }
            if (result == DDS.ReturnCode.Ok)
            {
                result = base.wlReq_deinit();
            }
            return result;
        }

        internal int WaitAction(IntPtr userData, IntPtr arg)
        {
            int proceed = 1;

            // Extract the condition list from the pointer and add the condition to it.
            GCHandle argGCHandle = GCHandle.FromIntPtr(arg);
            WaitActionArg actionArg = (WaitActionArg) argGCHandle.Target;

            if (userData == IntPtr.Zero)
            {
                foreach (IGuardCondition guardCond in actionArg.attachedGuards)
                {
                    if (guardCond.GetTriggerValue())
                    {
                        actionArg.Add(guardCond);
                    }
                }
                proceed = (actionArg.nrTriggeredConditions == 0) ? 1 : 0;
            }
            else
            {
                GCHandle condGCHandle = GCHandle.FromIntPtr(userData);
                ICondition cond = condGCHandle.Target as ICondition;
                actionArg.Add(cond);
            }
            argGCHandle.Target = actionArg;
            return proceed;
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
            ReturnCode result = DDS.ReturnCode.Ok;

            ReportStack.Start();
            WaitActionArg arg = new WaitActionArg(activeConditions);

            if (QosManager.countErrors(timeout) > 0)
            {
                result = DDS.ReturnCode.BadParameter;
                ReportStack.Report (result, "Duration timeout incorrect");
            }

            while (result == DDS.ReturnCode.Ok && arg.nrTriggeredConditions == 0)
            {
                lock(this)
                {
                    arg.maxConditions = conditionList.Count + guardList.Count;
                    arg.attachedGuards = guardList.ToArray();
                    if (activeConditions == null) activeConditions = new ICondition[arg.maxConditions];
                }

                GCHandle argGCHandle = GCHandle.Alloc(arg, GCHandleType.Normal);

                V_RESULT uResult = DDS.OpenSplice.User.WaitSet.WaitAction2(
                                       rlReq_UserPeer,
                                       WaitAction,
                                       GCHandle.ToIntPtr(argGCHandle),
                                       timeout.OsDuration);

                if (uResult == V_RESULT.DETACHING)
                {
                    arg = (WaitActionArg) argGCHandle.Target;
                    lock(this)
                    {
                        foreach(ICondition cond in conditionList)
                        {
                            Condition condImpl = cond as Condition;
                            if (condImpl.IsAlive() == DDS.ReturnCode.AlreadyDeleted)
                            {
                                arg.Add(cond);
                            }
                        }
                    }
                    result = DDS.ReturnCode.Ok;
                } else {
                    result = SacsSuperClass.uResultToReturnCode(uResult);
                    arg = (WaitActionArg) argGCHandle.Target;
                }

                argGCHandle.Free();

                activeConditions = arg.Finalize();
            }
            ReportStack.Flush(this, (result != ReturnCode.Ok ) && (result != ReturnCode.Timeout));

            return result;
        }

        /**
         * Condition specific callback from non-GuardConditions to WaitSet.
         */
        internal ReturnCode wlReq_AttachGeneralCondition (ICondition condition, IntPtr uCondition, IntPtr condGCHandle)
        {
            ReturnCode result;

            Debug.Assert (!conditionList.Contains (condition));
            result = SacsSuperClass.uResultToReturnCode (
                    DDS.OpenSplice.User.WaitSet.Attach (
                            rlReq_UserPeer, uCondition, condGCHandle));
            if (result == DDS.ReturnCode.Ok) {
                conditionList.Add (condition);
                MyDomainId = DDS.OpenSplice.User.WaitSet.GetDomainId(rlReq_UserPeer);
            } else {
                ReportStack.Report (result, "Could not attach Condition to WaitSet.");
            }


            return result;
        }

        /**
         * Condition specific callback from GuardConditions to WaitSet.
         */
        internal ReturnCode wlReq_AttachGuardCondition(IGuardCondition guardCond)
        {
            ReturnCode result;

            Debug.Assert(!guardList.Contains(guardCond));
            result = SacsSuperClass.uResultToReturnCode(
                    DDS.OpenSplice.User.WaitSet.Notify(rlReq_UserPeer, IntPtr.Zero));
            if (result == DDS.ReturnCode.Ok)
            {
                guardList.Add(guardCond);
            } else {
                ReportStack.Report (result, "Could not attach GuardCondition to WaitSet.");
            }
            return result;
        }

        /**
         * Condition specific callback from non-GuardConditions to WaitSet.
         */
        internal ReturnCode wlReq_DetachGeneralCondition(ICondition condition, IntPtr uCondition)
        {
            ReturnCode result;

            Debug.Assert(conditionList.Contains(condition));
            result = SacsSuperClass.uResultToReturnCode(
                    DDS.OpenSplice.User.WaitSet.Detach(
                            rlReq_UserPeer, uCondition));
            if (result == DDS.ReturnCode.Ok)
            {
                conditionList.Remove(condition);
                MyDomainId = DDS.OpenSplice.User.WaitSet.GetDomainId(rlReq_UserPeer);
            } else {
                ReportStack.Report (result, "Could not detach Condition from WaitSet.");
            }
            return result;
        }

        /**
         * Condition specific callback from GuardConditions to WaitSet.
         */
        internal ReturnCode wlReq_DetachGuardCondition(IGuardCondition guardCond)
        {
            ReturnCode result;

            Debug.Assert(guardList.Contains(guardCond));
            result = SacsSuperClass.uResultToReturnCode(
                    DDS.OpenSplice.User.WaitSet.Notify(rlReq_UserPeer, IntPtr.Zero));
            if (result == DDS.ReturnCode.Ok)
            {
               guardList.Remove(guardCond);
            } else {
                ReportStack.Report (result, "Could not detach GuardCondition from WaitSet.");
            }
            return result;
        }

        /// <summary>
        /// This operation attaches a condition to the WaitSet
        /// </summary>
        /// <param name="condition">The condition to be attached to the WaitSet.
        /// The parameter must be either a ReadCondition, QueryCondition, StatusCondition or GuardCondition</param>
        /// <returns>Possible return codes are: Ok,Error,BadParameter or OutOfResources.</returns>
        public ReturnCode AttachCondition (ICondition condition)
        {
            ReturnCode result = DDS.ReturnCode.BadParameter;
            Condition condImpl = condition as Condition;
            ReportStack.Start ();

            if (condImpl == null) {
                ReportStack.Report (result, "cond is invalid (null), or not of type " +
                                            "DDS::OpenSplice::Condition");
            } else {
                lock(this)
                {
                    result = condImpl.AttachToWaitSet (this);

                    // ALREADY_DELETED may only apply to the WaitSet in this context,
                    // so for a deleted condition use BAD_PARAMETER instead.
                    if (result == DDS.ReturnCode.AlreadyDeleted) {
                        result = DDS.ReturnCode.BadParameter;
                    }
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return result;
        }

        /// <summary>
        /// This operation detaches a Condition from the WaitSet. If the Condition was not attached
        /// to this WaitSet, the operation returns PreconditionNotMet
        /// </summary>
        /// <param name="condition">The attached condition in the WaitSet which is to be detached.</param>
        /// <returns>Possible return codes are: Ok, Error, BadParameter, OutOfResources or PreconditionNotMet.</returns>
        public ReturnCode DetachCondition (ICondition condition)
        {
            ReturnCode result = DDS.ReturnCode.BadParameter;
            Condition condImpl = condition as Condition;
            ReportStack.Start ();

            if (condImpl == null) {
                ReportStack.Report (result, "cond is invalid (null), or not of type " +
                                    "DDS::OpenSplice::Condition");
            } else {
                lock(this)
                {
                    result = condImpl.DetachFromWaitSet (this);

                    // ALREADY_DELETED may only apply to the WaitSet in this context,
                    // so for a deleted condition use BAD_PARAMETER instead.
                    if (result == DDS.ReturnCode.AlreadyDeleted) {
                        result = DDS.ReturnCode.BadParameter;
                    }
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);
            return result;
        }

        /// <summary>
        /// This operation retrieves the list of attached conditions.
        /// </summary>
        /// <param name="attachedConditions">A reference to a sequence that will hold all the attached conditions
        /// on the WaitSet</param>
        /// <returns>Possible return codes are: Ok, Error or OutOfResources.</returns>
        public ReturnCode GetConditions(ref ICondition[] attachedConditions)
        {
            lock(this)
            {
                int i = 0;
                int total = conditionList.Count + guardList.Count;
                if (attachedConditions == null || attachedConditions.Length != total)
                {
                    attachedConditions = new ICondition[total];
                }
                foreach(ICondition cond in conditionList)
                {
                    attachedConditions[i++] = cond;
                }
                foreach(ICondition cond in guardList)
                {
                    attachedConditions[i++] = cond;
                }
            }

            return DDS.ReturnCode.Ok;
        }

        internal ReturnCode trigger (IntPtr context)
        {
            ReturnCode result;

            lock(this)
            {
                result = SacsSuperClass.uResultToReturnCode (
                        DDS.OpenSplice.User.WaitSet.Notify (rlReq_UserPeer, context));
                if (result != ReturnCode.Ok) {
                    ReportStack.Report (result, "WaitSet notify failed ");
                }
            }
            return result;
        }
    }
}
