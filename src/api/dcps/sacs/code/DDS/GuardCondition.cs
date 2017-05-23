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
using DDS.OpenSplice;
using DDS;

namespace DDS
{
    public class GuardCondition : Condition, IGuardCondition
    {
        private bool triggerValue = false;
        
        public GuardCondition()
        {
        }

        internal ReturnCode init()
        {
            return base.init(IntPtr.Zero, true);
        }
        
        internal override ReturnCode wlReq_deinit()
        {
            return base.wlReq_deinit();
        }
        
        internal override ReturnCode AttachToWaitSet(WaitSet waitset)
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;

            ReportStack.Start();
            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    if (!waitSetList.Contains(waitset))
                    {
                        result = waitset.wlReq_AttachGuardCondition(this);
                        if (result == DDS.ReturnCode.Ok)
                        {
                            /* The waitset will detach itself when it is destructed. */
                            waitSetList.Add(waitset);
                        }
                    } 
                    else
                    {
                        result = DDS.ReturnCode.Ok;
                    }
                }
            }

//            if (result != DDS.ReturnCode.Ok) {
//                OS_REPORT(OS_ERROR,
//                            "Condition::attach_waitset", 0,
//                            "attach failed with %s",
//                            DDS::OpenSplice::Utils::returnCodeToString(result));
//            }

            return result;
        }

        internal override ReturnCode DetachFromWaitSet (WaitSet waitset)
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;

            ReportStack.Start();
            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    if (waitSetList.Remove(waitset))
                    {
                        result = waitset.wlReq_DetachGuardCondition(this);
                    }
                    else
                    {
                        result = DDS.ReturnCode.PreconditionNotMet;
                    }
                }
            }

//            if (result != DDS.ReturnCode.Ok) {
//                OS_REPORT(OS_ERROR,
//                            "Condition::detach_waitset", 0,
//                            "detach failed with %s",
//                            DDS::OpenSplice::Utils::returnCodeToString(result));
//            }

            return result;
        }

        public override bool GetTriggerValue()
        {
            bool tValue = false;
            bool isAlive;

            ReportStack.Start();
            lock(this)
            {
                isAlive = this.rlReq_isAlive;
                if (isAlive)
                {
                    tValue = triggerValue;
                }
            }
            ReportStack.Flush(this, !isAlive);
            return tValue;
        }

        public ReturnCode SetTriggerValue(bool value)
        {
            WaitSet[] list = null;
            IntPtr context = IntPtr.Zero;
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;

            ReportStack.Start();
            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    triggerValue = value;
                    context = rlReq_HandleSelf;
                    
                    /* A copy of the list of waitsets is made to be triggered after
                     * the condition is released. Triggering a Waitset will lock the
                     * Waitset whereas the Waitset may lock the condition to get the
                     * trigger value. So If the Condition is locked when triggering
                     * a Waitset a deadlock may occur if the Waitset simultaneously
                     * tries to get the conditions trigger value, by first taking a
                     * copy of all waitsets and then releasing the condition before
                     * triggering the Waitsets this situation is avoided.
                     */
                    list = waitSetList.ToArray();
                    result = DDS.ReturnCode.Ok;
                }
            }
            if (result == DDS.ReturnCode.Ok)
            {
                foreach(WaitSet ws in list)
                {
                    ws.trigger(context);
                }
            }
            ReportStack.Flush(this, result != ReturnCode.Ok);
            return result;
        }

        internal override ReturnCode IsAlive()
        {
            return ReturnCode.Ok;
        }
    }
}
