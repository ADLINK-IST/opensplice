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

using DDS;
using DDS.OpenSplice;
using DDS.OpenSplice.Common;
using DDS.OpenSplice.Kernel;
using DDS.OpenSplice.kernelModuleI;
using DDS.OpenSplice.CustomMarshalers;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Diagnostics;
using System.Threading;

namespace DDS.OpenSplice
{
    internal class ListenerDispatcher : SacsSuperClass
    {
        private static Type listenerEventType = typeof(DDS.OpenSplice.kernelModuleI.v_listenerEvent);

        enum ListenerThreadState {
            STOPPED,
            STARTING,
            RUNNING,
            STOPPING
        }

        private Thread ListenerThread = null;
        private SchedulingQosPolicy SchedulingPolicy;
        private int StackSize = 0;
        private List<Entity> Observables = null;
        private List<ListenerEvent> Events = null;

        private static ThreadPriority SchedulingPriority(SchedulingQosPolicy SchedulingPolicy)
        {
            int Priority = (int)ThreadPriority.Normal;

            if (SchedulingPolicy.SchedulingClass.Kind !=
                    DDS.SchedulingClassQosPolicyKind.ScheduleDefault)
            {
                Priority = SchedulingPolicy.SchedulingPriority;
                if (SchedulingPolicy.SchedulingPriorityKind.Kind ==
                        DDS.SchedulingPriorityQosPolicyKind.PriorityRelative)
                {
                    Priority = (int)Thread.CurrentThread.Priority;
                }

                if (Priority < (int)ThreadPriority.Lowest)
                {
                    Priority = (int)ThreadPriority.Lowest;
                }
                else if (Priority > (int)ThreadPriority.Highest)
                {
                    Priority = (int)ThreadPriority.Highest;
                }
            }

            return (ThreadPriority)Priority;
        }

        internal ListenerDispatcher()
        {
            Observables = new List<Entity>();
            Events = new List<ListenerEvent>();
        }

        internal ReturnCode init(IntPtr uParticipant, SchedulingQosPolicy scheduling)
        {
            ReturnCode result = DDS.ReturnCode.Ok;

            Debug.Assert(uParticipant != IntPtr.Zero);

            this.SchedulingPolicy = scheduling;
            this.StackSize = DDS.OpenSplice.Common.ListenerDispatcher.StackSize(uParticipant);

            IntPtr uListener = User.Listener.New(uParticipant);
            if (uListener == IntPtr.Zero)
            {
                result = DDS.ReturnCode.OutOfResources;
            } else {
                base.init(uListener, false);
            }

            if (result != DDS.ReturnCode.Ok)
            {
                deinit();
            }

            return result;
        }

        internal override ReturnCode wlReq_deinit()
        {
            ReturnCode result = DDS.ReturnCode.Ok;

            lock(this)
            {
                result = Stop();
                if (result == DDS.ReturnCode.Ok &&
                    State != ListenerThreadState.STOPPED)
                {
                    result = DDS.ReturnCode.PreconditionNotMet;
                }
                if (result == DDS.ReturnCode.Ok)
                {
                    result = base.wlReq_deinit();
                    if (result == DDS.ReturnCode.Ok)
                    {
                        Observables.Clear();
                    }
                }
            }

            return result;
        }

        internal void EventHandler(IntPtr EventPtr, IntPtr arg)
        {
            const uint topicTrigger = (uint)(V_EVENT.INCONSISTENT_TOPIC | V_EVENT.ALL_DATA_DISPOSED);
            const uint writerTrigger = (uint)(V_EVENT.OFFERED_DEADLINE_MISSED | V_EVENT.LIVELINESS_LOST |
                                              V_EVENT.OFFERED_INCOMPATIBLE_QOS | V_EVENT.PUBLICATION_MATCHED);
            const uint readerTrigger = (uint)(V_EVENT.SAMPLE_REJECTED | V_EVENT.LIVELINESS_CHANGED |
                                              V_EVENT.SAMPLE_LOST | V_EVENT.REQUESTED_DEADLINE_MISSED |
                                              V_EVENT.REQUESTED_INCOMPATIBLE_QOS | V_EVENT.SUBSCRIPTION_MATCHED);
           // The DATA_AVAILABLE and DATA_ON_READERS events are left out of the readerTrigger because
           // they don't have a v_readerStatus that has to be copied.

            Debug.Assert(EventPtr != IntPtr.Zero);
            v_listenerEvent listenerEvent = Marshal.PtrToStructure(EventPtr, listenerEventType) as v_listenerEvent;

            if (listenerEvent.kind == (uint)V_EVENT.TRIGGER)
            {
                // Nothing to deliver, so ignore.
                return;
            }

            ListenerEvent ev = new ListenerEvent(listenerEvent.kind);
            ev.Source = SacsSuperClass.fromUserData(listenerEvent.source) as Entity;
            if (ev.Source == null) {
               // Apparently the Source Entity has already been deleted.
               return;
            }
            if ((listenerEvent.kind & (uint)(V_EVENT.OBJECT_DESTROYED | V_EVENT.PREPARE_DELETE)) == 0)
            {
                ev.Target = SacsSuperClass.fromUserData(listenerEvent.userData) as Entity;
                if (listenerEvent.eventData != IntPtr.Zero)
                {
                    if ((listenerEvent.kind & topicTrigger) != 0)
                    {
                        v_topicStatus vTopicStatus = (v_topicStatus)Marshal.PtrToStructure(listenerEvent.eventData, typeof(v_topicStatus)) as v_topicStatus;
                        TopicStatus topicStatus = new TopicStatus();
                        vTopicStatusMarshaler.CopyOut(ref vTopicStatus, topicStatus);
                        ev.Status = topicStatus;
                    }
                    else if ((listenerEvent.kind & writerTrigger) != 0)
                    {
                        v_writerStatus vWriterStatus = (v_writerStatus)Marshal.PtrToStructure(listenerEvent.eventData, typeof(v_writerStatus)) as v_writerStatus;
                        WriterStatus writerStatus = new WriterStatus();
                        vWriterStatusMarshaler.CopyOut(ref vWriterStatus, writerStatus);
                        ev.Status = writerStatus;
                    }
                    else if ((listenerEvent.kind & readerTrigger) != 0)
                    {
                        v_readerStatus vReaderStatus = (v_readerStatus)Marshal.PtrToStructure(listenerEvent. eventData, typeof(v_readerStatus)) as v_readerStatus;
                        ReaderStatus readerStatus = new ReaderStatus();
                        vReaderStatusMarshaler.CopyOut(ref vReaderStatus, readerStatus);
                        ev.Status = readerStatus;
                    }
                    else
                    {
                        v_status vStatus = (v_status)Marshal.PtrToStructure(listenerEvent.eventData, typeof(v_status)) as v_status;
                        EntityStatus status = new EntityStatus();
                        vStatusMarshaler.CopyOut(ref vStatus, status);
                        ev.Status = status;
                    }
                }
                else
                {
                   ev.Status = null;
                }
            }

            Events.Add(ev);
        }

        internal void ProcessEvents()
        {
            foreach (ListenerEvent ev in Events)
            {
                if ((ev.Kind & (uint)(V_EVENT.OBJECT_DESTROYED | V_EVENT.PREPARE_DELETE)) != 0)
                {
                    /* The last event so wake-up threads blocking in disable_callbacks. */
                    ev.Source.NotifyListenerRemoved();
                }
                else
                {
                    Debug.Assert(ev.Target != null);
                    if (ev.Target != null)
                    {
                        ev.Target.NotifyListener(ev.Source, (V_EVENT)ev.Kind, ev.Status);
                    }
                }
            }
            Events.Clear();
        }


        private ListenerThreadState State = ListenerThreadState.STOPPED;

        internal void Run()
        {
            ReturnCode result = DDS.ReturnCode.Ok;
            V_RESULT uResult;

            Monitor.Enter (this);
            try
            {
                if (State == ListenerThreadState.STARTING)
                {
                    State = ListenerThreadState.RUNNING;
                    Monitor.PulseAll (this);

                    while (result == DDS.ReturnCode.Ok &&
                           State == ListenerThreadState.RUNNING)
                    {
                        Monitor.Exit (this);
                        uResult = User.Listener.Wait (
                            rlReq_UserPeer,
                            EventHandler,
                            IntPtr.Zero,
                            Duration.Infinite.OsDuration);
                        /* Result can't be timeout due to Duration being
                           infinite. */
                        result = uResultToReturnCode (uResult);
                        if (result == DDS.ReturnCode.Ok)
                        {
                            ProcessEvents();
                        }
                        Monitor.Enter (this);
                    }
                }
            }
            finally
            {
                State = ListenerThreadState.STOPPED;
                Monitor.PulseAll (this);
                Monitor.Exit (this);
            }
        }

        private ReturnCode Start()
        {
            ReturnCode result = DDS.ReturnCode.Ok;

            switch (State)
            {
                case ListenerThreadState.STOPPED:
                    if (StackSize == 0)
                    {
                        ListenerThread = new Thread (this.Run);
                    }
                    else
                    {
                        ListenerThread = new Thread (this.Run, StackSize);
                    }
                    ListenerThread.Priority = SchedulingPriority(SchedulingPolicy);
                    State = ListenerThreadState.STARTING;
                    ListenerThread.Start();
                    break;
                case ListenerThreadState.STOPPING:
                    State = ListenerThreadState.RUNNING;
                    break;
                default:
                    break;
            }

            while (result == DDS.ReturnCode.Ok &&
                   State == ListenerThreadState.STOPPING)
            {
                Monitor.Wait(this);
            }

            if (result != DDS.ReturnCode.Ok)
            {
                ReportStack.Report (result, "Could not start listener dispatcher.");
            }

            Monitor.PulseAll (this);

            return result;
        }

        private ReturnCode Stop()
        {
            ReturnCode result = DDS.ReturnCode.Ok;
            V_RESULT uResult;

            switch (State) {
                case ListenerThreadState.RUNNING:
                    uResult = User.Listener.Trigger(rlReq_UserPeer);
                    result = uResultToReturnCode (uResult);
                    if (result == DDS.ReturnCode.Ok)
                    {
                        State = ListenerThreadState.STOPPING;
                    }
                    break;
                case ListenerThreadState.STARTING:
                    State = ListenerThreadState.STOPPING;
                    break;
                default:
                    break;
            }

            while (result == DDS.ReturnCode.Ok &&
                   State == ListenerThreadState.STOPPING)
            {
                Monitor.Wait(this);
            }

            if (ListenerThread != null &&
                State == ListenerThreadState.STOPPED)
            {
                ListenerThread.Join();
                ListenerThread = null;
            }

            if (result != DDS.ReturnCode.Ok)
            {
                ReportStack.Report (result, "Could not stop listener dispatcher.");
            }

            Monitor.PulseAll (this);

            return result;
        }

        internal ReturnCode Add(Entity observable, StatusKind mask)
        {
            ReturnCode result = DDS.ReturnCode.Ok;
            V_RESULT uResult;

            uint vMask = vEventMarshaler.vEventMaskFromStatusMask(mask);

            lock(this)
            {
                uResult = User.Entity.SetListener (
                    observable.rlReq_UserPeer, rlReq_UserPeer, IntPtr.Zero, vMask);
                result = uResultToReturnCode (uResult);
                if (result == DDS.ReturnCode.Ok)
                {
                    if (!Observables.Contains(observable))
                    {
                        Observables.Add(observable);
                    }
                    Start();
                    /* result = Start();
                       TODO: Restore original listener if thread fails to start.
                       See cmn_listenerDispatcher_add for more information. */
                }
            }

            return result;
        }

        internal ReturnCode Remove(Entity observable)
        {
            ReturnCode result = DDS.ReturnCode.Ok;

            lock(this)
            {
                if (Observables.Contains(observable))
                {
                    Observables.Remove(observable);
                    if (Observables.Count == 0)
                    {
                        Stop();
                    }
                }
            }

            return result;
        }

        internal ReturnCode GetScheduling(ref SchedulingQosPolicy SchedulingPolicy)
        {
            lock(this)
            {
                SchedulingPolicy = this.SchedulingPolicy;
            }

            return DDS.ReturnCode.Ok;
        }

        internal ReturnCode SetScheduling(SchedulingQosPolicy SchedulingPolicy)
        {
            ReturnCode result = DDS.ReturnCode.Ok;
            ThreadPriority Priority, _Priority;
            SchedulingQosPolicy _SchedulingPolicy;

            lock(this)
            {
                Priority = SchedulingPriority (SchedulingPolicy);
                _Priority = SchedulingPriority (this.SchedulingPolicy);
                _SchedulingPolicy = this.SchedulingPolicy;
                this.SchedulingPolicy = SchedulingPolicy;
                if (Priority != _Priority)
                {
                    result = Stop();
                    if (result == DDS.ReturnCode.Ok &&
                        State != ListenerThreadState.STOPPED)
                    {
                        result = DDS.ReturnCode.PreconditionNotMet;
                    } else if (Observables.Count > 0) {
                        result = Start();
                    }
                    if (result != DDS.ReturnCode.Ok)
                    {
                        this.SchedulingPolicy = _SchedulingPolicy;
                    }
                }
            }

            return result;
        }
    }
}
