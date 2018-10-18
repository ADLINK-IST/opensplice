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
using System.Collections.Generic;
using DDS;
using DDS.OpenSplice;
using DDS.OpenSplice.User;

namespace DDS
{
    public abstract class Condition : SacsSuperClass, ICondition
    {
        protected List<WaitSet> waitSetList = new List<WaitSet>();
        protected bool deinitializing = false;

        internal Condition()
        {
            // Base class handles everything.
        }

        internal override ReturnCode init(IntPtr userPtr, bool isWeak)
        {
            return base.init(userPtr, isWeak);
        }

        internal override ReturnCode wlReq_deinit()
        {
            ReturnCode result = DDS.ReturnCode.Ok;

            // Create a temporary representation of the waitSetList, since
            // it is not allowed to remove items from a collection during
            // a walk through that same collection. So we walk through
            // a copy of that list instead.
            WaitSet[] tmpList = waitSetList.ToArray();
            deinitializing = true;

            foreach (WaitSet ws in tmpList)
            {
                /* The ws.DetachCondition() wants access to this condition,
                 * which means we have to unlock it for this small while.
                 * TODO: There should be another solution. */
                result = ws.DetachCondition (this);
                if (result != DDS.ReturnCode.Ok) break;
            }

            if (result == DDS.ReturnCode.Ok)
            {
                waitSetList.Clear();
                waitSetList = null;
                result = base.wlReq_deinit();
            }

            return result;
        }

        internal virtual ReturnCode AttachToWaitSet(WaitSet waitset)
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;

            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    if (!waitSetList.Contains(waitset))
                    {
                        result = waitset.wlReq_AttachGeneralCondition(this, rlReq_UserPeer, rlReq_HandleSelf);
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

            return result;
        }

        internal virtual ReturnCode DetachFromWaitSet (WaitSet waitset)
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;

            lock(this)
            {
                if (this.rlReq_isAlive)
                {
                    if (waitSetList.Remove(waitset)) {
                        result = waitset.wlReq_DetachGeneralCondition(this, rlReq_UserPeer);
                    }
                    else
                    {
                        /* Unable to take the given waitset is not a problem when de-initializing. */
                        if (!deinitializing) {
                            result = DDS.ReturnCode.PreconditionNotMet;
                        }
                    }
                }
            }

            return result;
        }

        public abstract bool GetTriggerValue();

        internal static void wlReq_DummyFunction(IntPtr p, IntPtr arg)
        {
        }

        internal virtual ReturnCode IsAlive()
        {
            ReturnCode result = DDS.ReturnCode.AlreadyDeleted;

            lock(this)
            {
                result = uResultToReturnCode(
                        DDS.OpenSplice.User.Observable.Action(rlReq_UserPeer, wlReq_DummyFunction, IntPtr.Zero));
            }

            return result;
        }

    }
}
