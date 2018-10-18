/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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
using DDS;
using DDS.OpenSplice;
using DDS.OpenSplice.CustomMarshalers;

namespace DDS.OpenSplice
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
        private Entity entity;
        private StatusKind enabledStatusMask;

        internal StatusCondition()
        {
            // Base class handles everything.
        }

        internal ReturnCode init(Entity entity)
        {
            ReturnCode result;

            ReportStack.Start();
            IntPtr userPtr = User.StatusCondition.New(entity.rlReq_UserPeer);
            if (userPtr != IntPtr.Zero)
            {
                this.entity = entity;
                enabledStatusMask = StatusKind.Any;
                result = base.init(userPtr, false);
            }
            else
            {
                result = DDS.ReturnCode.Error;
                ReportStack.Report(result, "Could not create StatusCondition.");

            }
            ReportStack.Flush(this, result != ReturnCode.Ok);
            return result;
        }

        internal override ReturnCode wlReq_deinit()
        {
            return base.wlReq_deinit();
        }

        /// <summary>
        /// This operation returns the list of enabled communication statuses of the StatusCondition.
        /// </summary>
        /// <returns>StatusKind - a bit mask in which each bit shows which status is taken into account
        /// for the StatusCondition</returns>
        public StatusKind GetEnabledStatuses()
        {
            StatusKind mask = 0;
            bool isAlive;

            ReportStack.Start();
            lock(this)
            {
                isAlive = this.rlReq_isAlive;
                if (isAlive)
                {
                    mask = enabledStatusMask;
                }
            }
            ReportStack.Flush(this, !isAlive);

            return mask;
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
            uint vMask;
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;

            ReportStack.Start();
            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    vMask = vEventMarshaler.vEventMaskFromStatusMask(mask);
                    result = SacsSuperClass.uResultToReturnCode(
                            User.StatusCondition.SetMask(rlReq_UserPeer, vMask));
                    if (result == DDS.ReturnCode.Ok) {
                        enabledStatusMask = mask;
                    }
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);

            return result;
        }
        /// <summary>
        /// This operation returns the Entity associated with the StatusCondition or a
        /// null Entity.
        /// </summary>
        /// <returns>IEntity - a pointer to the Entity associated with the StatusCondition.</returns>
        public IEntity GetEntity()
        {
            Entity e = null;
            bool isAlive;

            ReportStack.Start();
            lock(this)
            {
                isAlive = this.rlReq_isAlive;
                if (isAlive)
                {
                    e = entity;
                }
            }
            ReportStack.Flush(this, !isAlive);

            return e;
        }

        public override bool GetTriggerValue()
        {
            uint triggerValue = 0;
            bool isAlive;

            ReportStack.Start();
            lock(this)
            {
                isAlive = this.rlReq_isAlive;
                if (isAlive)
                {
                    User.StatusCondition.GetTriggerValue(rlReq_UserPeer, ref triggerValue);
                }
            }
            ReportStack.Flush(this, !isAlive);

            return (triggerValue > 0);
        }
    }
}
