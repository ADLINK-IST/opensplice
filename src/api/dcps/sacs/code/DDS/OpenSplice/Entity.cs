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
using System.Runtime.InteropServices;
using System.Diagnostics;
using System.Threading;
using DDS;
using DDS.OpenSplice.OS;
using DDS.OpenSplice.Database;
using DDS.OpenSplice.Kernel;
using DDS.OpenSplice.kernelModuleI;
using DDS.OpenSplice.CustomMarshalers;

namespace DDS.OpenSplice
{
    internal struct vMaskHolder
    {
        internal StatusKind vMask;
    }

    public abstract class Entity : SacsSuperClass, IEntity
    {
        private InstanceHandle myHandle = DDS.InstanceHandle.Nil;
        private StatusCondition statusCondition = null;
        private ListenerDispatcher listenerDispatcher = null;
        protected IListener listener = null;
        protected StatusKind listenerMask = 0;
        private volatile bool wait = false;

        internal Entity()
        {
        }

        internal ReturnCode checkProperty(ref Property prop)
        {
            ReturnCode result = DDS.ReturnCode.Ok;
            if (prop.Name == null) {
                result = DDS.ReturnCode.BadParameter;
            } else if (prop.Value == null) {
                result = DDS.ReturnCode.BadParameter;
            }
            return result;
        }

        internal ReturnCode init(IntPtr userPtr)
        {
            ReturnCode result = base.init(userPtr, false);
            if (result == DDS.ReturnCode.Ok)
            {
                myHandle = User.Entity.GetInstanceHandle(userPtr);
                if (User.Observable.SetUserData(userPtr, rlReq_HandleSelf) != IntPtr.Zero)
                {
                    ReportStack.Report(result, "Could not initialize Entity.");
                    result = DDS.ReturnCode.Error;
                }
            }
            return result;
        }

        internal override ReturnCode wlReq_deinit()
        {
            ReturnCode result = DDS.ReturnCode.Ok;

            if (statusCondition != null)
            {
                result = statusCondition.deinit();
            }

            if (result == DDS.ReturnCode.Ok)
            {
                result = base.wlReq_deinit();
            }
            return result;
        }

        internal static void GetStatusMask(IntPtr p, IntPtr arg)
        {
            // Extract the maskHolder from the pointer.
            GCHandle tmpGCHandle = GCHandle.FromIntPtr(arg);
            vMaskHolder holder = (vMaskHolder) tmpGCHandle.Target;

            // Obtain the Status object from the v_entity that is passed by its v_public handle.
            IntPtr vStatus = Kernel.Entity.Status(p);
            uint vMask = Kernel.Status.GetMask(vStatus);

            // Grab the handle kind by marshalling its v_object representation into C#.
            v_object vObject = (v_object) Marshal.PtrToStructure(p, typeof(v_object));

            // Set the mask value.
            holder.vMask = vEventMarshaler.vEventMaskToStatusMask(vMask, vObject.kind);
            c.free(vStatus);

            // Write the modified mask back into the pointer.
            tmpGCHandle.Target = holder;
        }

        internal static void wlReq_ResetDataAvailable(IntPtr p, IntPtr arg)
        {
            // Grab the status by marshalling its v_entity representation into C#.
            IntPtr status  = Kernel.Entity.Status(p);
            Kernel.Status.Reset(status, V_EVENT.DATA_AVAILABLE);
            c.free(status);
        }

        internal ReturnCode ResetDataAvailableStatus()
        {
            lock(this)
            {
                return uResultToReturnCode(
                        User.Observable.Action(rlReq_UserPeer, wlReq_ResetDataAvailable, IntPtr.Zero));
            }
        }

        internal abstract void NotifyListener(Entity source, V_EVENT triggerMask, DDS.OpenSplice.Common.EntityStatus status);

        internal ListenerDispatcher wlReq_ListenerDispatcher
        {
            get
            {
                return listenerDispatcher;
            }

            set
            {
                listenerDispatcher = value;
            }
        }

        protected IListener rlReq_Listener
        {
            get
            {
                return listener;
            }
        }

        protected ReturnCode wlReq_SetListener(IListener aListener, StatusKind mask)
        {
            ReturnCode result = ReturnCode.Ok;

            Debug.Assert(listenerDispatcher != null);
            if (mask != 0 && aListener != null)
            {
                result = listenerDispatcher.Add(this, mask);
                if (result == DDS.ReturnCode.Ok)
                {
                    wait = true;
                }
            }
            else
            {
                V_RESULT uResult = User.Entity.SetListener (
                    rlReq_UserPeer, IntPtr.Zero, IntPtr.Zero, 0);
                result = uResultToReturnCode (uResult);
                if (result == DDS.ReturnCode.Ok)
                {
                    WaitListenerRemoved();
                    listenerDispatcher.Remove(this);
                }
            }

            if (result == DDS.ReturnCode.Ok)
            {
                listener = aListener;
                listenerMask = mask;
            }

            return result;
        }

        public ReturnCode Enable()
        {
            ReportStack.Start();

            DDS.ReturnCode result = uResultToReturnCode (
                    User.Entity.Enable(rlReq_UserPeer));

            ReportStack.Flush(this, result != ReturnCode.Ok);
            return result;
        }

        public bool IsEnabled()
        {
           byte state = User.Entity.Enabled(rlReq_UserPeer);
           if (state != 0) {
              return true;
           } else {
              return false;
           }
        }

        public IStatusCondition StatusCondition
        {
            get
            {
                ReturnCode result = DDS.ReturnCode.AlreadyDeleted;
                StatusCondition condition = null;

                ReportStack.Start();
                lock(this)
                {
                    if (this.rlReq_isAlive) {
                        if (statusCondition == null) {
                            statusCondition = new StatusCondition();
                            result = statusCondition.init(this);
                            if (result != DDS.ReturnCode.Ok) {
                                ReportStack.Report(result, "Could not initialize new StatusCondition.");
                                statusCondition = null;
                            }
                        }
                        condition = statusCondition;
                    }
                }
                ReportStack.Flush(this, result != ReturnCode.Ok);
                return condition;
            }
        }

        public StatusKind StatusChanges
        {
            get
            {
                // Create a holder to the mask, and obtain a pointer to that holder.
                ReportStack.Start();
                vMaskHolder holder = new vMaskHolder();
                GCHandle maskGCHandle = GCHandle.Alloc(holder, GCHandleType.Normal);
                DDS.ReturnCode result = uResultToReturnCode (
                        User.Observable.Action(rlReq_UserPeer, GetStatusMask, GCHandle.ToIntPtr(maskGCHandle)));
                holder = (vMaskHolder) maskGCHandle.Target;
                maskGCHandle.Free();
                ReportStack.Flush(this, result != ReturnCode.Ok);
                return holder.vMask;
            }
        }

        public InstanceHandle InstanceHandle
        {
            get
            {
                bool isAlive;
                InstanceHandle handle = DDS.InstanceHandle.Nil;

                ReportStack.Start();
                lock(this)
                {
                    isAlive = this.rlReq_isAlive;
                    if (isAlive)
                    {
	                    handle = myHandle;
                    }
                }
                ReportStack.Flush(this, !isAlive);

                return handle;
            }
        }

        internal void NotifyListenerRemoved()
        {
            lock (this)
            {
                Debug.Assert(wait == true);
                wait = false;
                Monitor.PulseAll(this);
            }
        }

        internal void WaitListenerRemoved()
        {
            ReportStack.Start();
            int wakeCount = 0;
            lock (this)
            {
                while (wait && wakeCount < 4)
                {
                    /* Made wait use a timeout in order to be able to detect
                     * a race where sometimes the wait() doesn't return (missed event?). */
                    Monitor.Wait(this, 250);
                    if (wait)
                    {
                        wakeCount++;
                        /* This is either a spurious wake-up or a timeout. Unfortunately
                         * we can't distinguish between them. */
                        ReportStack.Deprecated(this + ": timeout or spurious wake-up happened " + wakeCount + " times. Will " + (wakeCount < 4 ? "" : "not") + " wait again. ");
                    }
                }
            }
            ReportStack.Flush(this, wakeCount > 0);
        }

        protected ReturnCode DisableCallbacks()
        {
            DDS.ReturnCode result = DDS.ReturnCode.Ok;
            ReportStack.Start();
            lock (this)
            {
                if (this.rlReq_isAlive)
                {
                    byte triggered = User.Entity.DisableCallbacks(rlReq_UserPeer);
                    if (triggered != 0)
                    {
                        wait = true;
                        WaitListenerRemoved();
                    }
                }
                else
                {
                    result = DDS.ReturnCode.AlreadyDeleted;
                }
            }
            ReportStack.Flush(this, result != DDS.ReturnCode.Ok);
            return result;
        }
    }
}
