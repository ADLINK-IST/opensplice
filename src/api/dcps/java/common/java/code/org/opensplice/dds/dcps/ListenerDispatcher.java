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
 *
 */


package org.opensplice.dds.dcps;

import java.util.HashSet;
import java.util.concurrent.locks.*;

public class ListenerDispatcher {

    private final static int STOPPED = 0;
    private final static int STARTING = 1;
    private final static int RUNNING = 2;
    private final static int STOPPING = 3;

    private long uListener = 0;

    private volatile int state = STOPPED;
    private Thread thread = null;

    private final HashSet<EntityImpl> observables = new HashSet<EntityImpl>();
    private DDS.SchedulingQosPolicy scheduling_policy = null;
    private int stack_size = 0;

    /* The synchronized keyword cannot be used because the lock is locked and
       unlocked inside the ListenerDispatcher main function. */
    private final ReentrantLock lock = new ReentrantLock ();
    private final Condition condition = lock.newCondition ();

    private class ListenerRunnable implements Runnable {

        @Override
        public void run()
        {
            int result = DDS.RETCODE_OK.value;
            int length = 0;
            Event event = null;
            EventList events = new EventList();

            lock.lock();
            try {
                if (state == STARTING) {
                    state = RUNNING;
                    condition.signalAll();

                    while (result == DDS.RETCODE_OK.value && state == RUNNING) {
                        lock.unlock();
                        try {
                            ReportStack.start();

                            events.value = null;
                            result = jniWait(uListener, events);
                            if (result == DDS.RETCODE_OK.value) {
                                if (events.value != null) {
                                    length = events.value.length;
                                    for (int i = 0; i < length; i++) {
                                        event = events.value[i];
                                        if (event != null) {
                                            if (event.kind == Event.OBJECT_DESTROYED ||
                                                event.kind == Event.PREPARE_DELETE)
                                            {
                                                /* work around for NPE see OSPL-8205 */
                                                if (event.observable != null) {
                                                    /* Last event so wake-up threads blocking in disable_callbacks. */
                                                    event.observable.notify_listener_disabled();
                                                } else {
                                                    ReportStack.report (DDS.RETCODE_ERROR.value,
                                                            "ListenerDispatcher observable entity was null for kind: "+ event.kind);
                                                }
                                            } else if (event.kind != Event.TRIGGER) {
                                                /* Call entity listener */
                                                if (event.observer != null) {
                                                    event.observer.notify(event);
                                                } else {
                                                    ReportStack.report (DDS.RETCODE_ERROR.value,
                                                            "ListenerDispatcher observer entity was null for kind: "+ event.kind);
                                                }
                                            }
                                        }
                                    }
                                }
                            } else if (result == DDS.RETCODE_TIMEOUT.value){
                                result = DDS.RETCODE_OK.value;
                            }

                            ReportStack.flush(result != DDS.RETCODE_OK.value &&
                                              result != DDS.RETCODE_ALREADY_DELETED.value);
                        } finally {
                            lock.lock();
                        }
                    }
                }

                state = STOPPED;
                condition.signalAll();
            } finally {
                lock.unlock();
            }
        }
    }

    private final int start()
    {
        int result = DDS.RETCODE_OK.value;

        switch (state) {
            /* Create thread and switch state to STARTING. */
            case STOPPED:
                ListenerRunnable runnable = new ListenerRunnable();
                if (stack_size == 0) {
                    thread = new Thread (
                        null, runnable, "ListenerEventThread");
                } else {
                    thread = new Thread (
                        null, runnable, "ListenerEventThread", stack_size);
                }

                thread.setPriority (scheduling_priority (scheduling_policy));
                /* ListenerDispatcher thread is required to be a daemon thread
                   or the shutdown hook will not be called until this thread
                   exists, resulting in a deadlock when the application exits
                   without all entities having been cleaned up. */
                thread.setDaemon(true);
                thread.start ();
                state = STARTING;
                break;
            /* Switch state to RUNNING. Thread main loop should still be active. */
            case STOPPING:
                state = RUNNING;
                break;
            /* No action required, Wait for state to be RUNNING. */
            default:
                break;
        }

        while (result == DDS.RETCODE_OK.value && state == STARTING) {
            /* Wait for main loop to switch state to RUNNING, but timeout
               after a given amount of time in case an event is missed. */
            try {
                condition.await();
            } catch (InterruptedException e) {
                ReportStack.report (
                    DDS.RETCODE_ERROR.value,
                    "ListenerDispatcher wait interrupted.");
            }
        }

        if (result != DDS.RETCODE_OK.value) {
            ReportStack.report(result, "Could not start listener");
        }

        condition.signalAll();

        return result;
    }

    private final int stop ()
    {
        int result = DDS.RETCODE_OK.value;

        switch (state) {
            /* Instruct thread to terminate and wait for state to be STOPPED. */
            case RUNNING:
                result = jniListenerInterrupt (uListener);
                if (result == DDS.RETCODE_OK.value) {
                    state = STOPPING;
                }
                break;
            case STARTING:
                state = STOPPING;
                break;
            /* No action required. Wait for state to be STOPPED. */
            default:
                break;
        }

        while (result == DDS.RETCODE_OK.value && state == STOPPING) {
            try {
                condition.await();
            } catch (InterruptedException e) {
                ReportStack.report (
                    DDS.RETCODE_ERROR.value,
                    "ListenerDispatcher wait interrupted.");
            }
        }

        if (thread != null && state == STOPPED) {
            try {
                thread.join();
                thread = null;
            } catch (InterruptedException e) {
                ReportStack.report(
                    DDS.RETCODE_ERROR.value,
                    "ListenerDispatcher wait interrupted.");
            }
        }

        if (result != DDS.RETCODE_OK.value) {
            ReportStack.report (result, "Could not stop listener");
        }

        condition.signalAll();

        return result;
    }

    private static int scheduling_priority (
        DDS.SchedulingQosPolicy scheduling_policy)
    {
        int priority;

        if (scheduling_policy.scheduling_class.kind ==
            DDS.SchedulingClassQosPolicyKind.SCHEDULE_DEFAULT)
        {
            priority = Thread.NORM_PRIORITY;
        } else {
            priority = scheduling_policy.scheduling_priority;
            if (scheduling_policy.scheduling_priority_kind.kind ==
                    DDS.SchedulingPriorityQosPolicyKind.PRIORITY_RELATIVE)
            {
                priority += Thread.currentThread().getPriority();
            }

            if (priority < Thread.MIN_PRIORITY) {
                priority = Thread.MIN_PRIORITY;
            } else if (priority > Thread.MAX_PRIORITY) {
                priority = Thread.MAX_PRIORITY;
            }
        }

        return priority;
    }

    public ListenerDispatcher(
        long uParticipant,
        DDS.SchedulingQosPolicy scheduling_policy)
    {
        uListener = jniListenerNew(uParticipant);
        if (uListener != 0) {
            /* Instead of figuring out the default stack size, which depends on
               the operating system, the constructor without a stack size
               argument is invoked. */
            this.stack_size = jniStackSize (uParticipant);
            this.scheduling_policy = scheduling_policy;
        }
    }

    protected int deinit()
    {
        int result = DDS.RETCODE_OK.value;

        try {
            lock.lock ();

            result = stop ();
            if (result == DDS.RETCODE_OK.value && state != STOPPED) {
                result = DDS.RETCODE_PRECONDITION_NOT_MET.value;
            }
            if (result == DDS.RETCODE_OK.value) {
                result = jniListenerFree (uListener);
                if (result == DDS.RETCODE_OK.value) {
                    uListener = 0;
                    observables.clear ();
                }
            }
        } finally {
            lock.unlock ();
        }

        return result;
    }

    protected int add (
        EntityImpl observable, int mask)
    {
        int result = DDS.RETCODE_OK.value;

        try {
            lock.lock ();

            result = observable.set_listener (uListener, mask);
            if (result == DDS.RETCODE_OK.value) {
                observables.add (observable);
                result = start ();
                /* result = start ();
                   TODO: OSPL-6104. Restore original listener if thread fails to start.
                   See cmn_listenerDispatcher_add for more information. */
            }
        } finally {
            lock.unlock ();
        }

        return result;
    }

    protected int remove (
        EntityImpl observable)
    {
        int result = DDS.RETCODE_OK.value;

        try {
            lock.lock ();

            if (observables.remove (observable)) {
                if (observables.size () == 0) {
                    result = stop ();
                }
            }
        } finally {
            lock.unlock ();
        }

        return result;
    }

    protected int get_scheduling (
        DDS.SchedulingQosPolicyHolder scheduling_policy)
    {
        int result = DDS.RETCODE_BAD_PARAMETER.value;

        try {
            lock.lock ();

            if (scheduling_policy != null) {
                result = DDS.RETCODE_OK.value;
                scheduling_policy.value = Utilities.deepCopy(
                    this.scheduling_policy);
            }
        } finally {
            lock.unlock ();
        }

        return result;
    }

    protected int set_scheduling (
        DDS.SchedulingQosPolicy scheduling_policy)
    {
        int result = DDS.RETCODE_BAD_PARAMETER.value;
        int priority, old_priority;
        DDS.SchedulingQosPolicy old_scheduling_policy;

        try {
            lock.lock ();

            if (scheduling_policy != null) {
                result = DDS.RETCODE_OK.value;

                priority = scheduling_priority (scheduling_policy);
                old_priority = scheduling_priority (this.scheduling_policy);
                old_scheduling_policy = this.scheduling_policy;

                this.scheduling_policy = Utilities.deepCopy (scheduling_policy);
                if (priority != old_priority) {
                    result = stop ();
                    if (result == DDS.RETCODE_OK.value && state != STOPPED) {
                        result = DDS.RETCODE_PRECONDITION_NOT_MET.value;
                    } else {
                        if (observables.size() > 0) {
                            result = start ();
                        }
                    }
                    if (result != DDS.RETCODE_OK.value) {
                        this.scheduling_policy = old_scheduling_policy;
                    }
                }
            }
        } finally {
            lock.unlock ();
        }

        return result;
    }

    private native long jniListenerNew(long uParticipant);
    private native int jniListenerFree(long uListener);
    private native int jniListenerInterrupt(long uListener);
    private native int jniWait(long uListener, EventList eventList);
    private native int jniSetListener(long uListener, int mask);
    private native int jniStackSize(long uParticipant);
}
