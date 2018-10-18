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

import DDS.Property;
import DDS.PropertyHolder;

/**
 * Implementation of the {@link DDS.Entity} interface.
 */
public abstract class EntityImpl extends EntityBase implements DDS.Entity {

    private static final long serialVersionUID = 8667249067325179655L;
    StatusConditionImpl statusCondition = null;
    private ListenerDispatcher dispatcher = null;
    private boolean wait = false;

    protected abstract int notify(Event e);

    protected int set_dispatcher(ListenerDispatcher d)
    {
        dispatcher = d;
        return DDS.RETCODE_OK.value;
    }

    protected ListenerDispatcher get_dispatcher()
    {
        return dispatcher;
    }

    protected int set_listener(long uListener, int mask)
    {
        int result = DDS.RETCODE_OK.value;
        long uEntity = this.get_user_object();
        result = jniSetListener(uEntity, uListener, mask);
        return result;
    }

    protected synchronized int set_listener_interest(int mask)
    {
        int result = DDS.RETCODE_OK.value;

        if (dispatcher != null) {
            if (mask != 0) {
                result = dispatcher.add(this, mask);
                if (result == DDS.RETCODE_OK.value) {
                    wait = true;
                }
            } else {
                result = this.set_listener (0, mask);
                if (result == DDS.RETCODE_OK.value) {
                    this.wait_listener_disabled();
                    dispatcher.remove(this);
                }
            }
        }

        return result;
    }

    @Override
    protected int deinit ()
    {
        synchronized (this)
        {
            dispatcher = null;
            if (statusCondition != null) {
                statusCondition.deinit();
                statusCondition = null;
            }
        }
        return super.deinit();
    }

    /* see DDS.EntityOperations for javadoc */
    @Override
    public int enable ()
    {
        int result = DDS.RETCODE_OK.value;
        long uEntity = 0;
        ReportStack.start();

        uEntity = this.get_user_object();
        if (uEntity != 0) {
            result = jniEnable(uEntity);
        } else {
            result = DDS.RETCODE_ALREADY_DELETED.value;
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    protected boolean is_enabled ()
    {
        long uEntity = 0;
        boolean result = false;

        uEntity = this.get_user_object();
        if (uEntity != 0) {
            result = jniIsEnable(uEntity);
        }

        return result;
    }

    /* see DDS.EntityOperations for javadoc */
    @Override
    public DDS.StatusCondition get_statuscondition ()
    {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        synchronized (this)
        {
            if (this.statusCondition == null) {
                this.statusCondition = new StatusConditionImpl();
                result = statusCondition.init(this);
                if (result != DDS.RETCODE_OK.value) {
                    this.statusCondition = null;
                }
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return this.statusCondition;
    }

    public int detach_statuscondition()
    {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        synchronized (this)
        {
            if (this.statusCondition != null) {
                result = statusCondition.deinit();
                if (result == DDS.RETCODE_OK.value) {
                    statusCondition = null;
                }
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.EntityOperations for javadoc */
    @Override
    public int get_status_changes ()
    {
        int result = DDS.RETCODE_OK.value;
        int status = 0;
        long uEntity = 0;
        ReportStack.start();

        uEntity = this.get_user_object();
        if (uEntity != 0) {
            status = jniGetStatusChanges(uEntity);
        } else {
            result = DDS.RETCODE_ALREADY_DELETED.value;
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return status;
    }

    @Override
    public long get_instance_handle ()
    {
        int result = DDS.RETCODE_OK.value;
        long handle = DDS.HANDLE_NIL.value;
        long uEntity = 0;
        ReportStack.start();

        uEntity = this.get_user_object();
        if (uEntity != 0) {
            handle = jniGetInstanceHandle(uEntity);
        } else {
            result = DDS.RETCODE_ALREADY_DELETED.value;
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return handle;
    }

    protected int checkProperty(Property a_property) {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if ((result == DDS.RETCODE_OK.value) && (a_property == null)) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "Supplied Property is null.");
        }
        if ((result == DDS.RETCODE_OK.value) && (a_property.name == null)) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "Supplied Property.name is null.");
        }
        if ((result == DDS.RETCODE_OK.value) && (a_property.value == null)) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "Supplied Property.value is null.");
        }
        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;

    }

    @Override
    public int set_property(Property a_property) {
        int result = DDS.RETCODE_OK.value;
        long uEntity;

        ReportStack.start();

        uEntity = this.get_user_object();

        if (uEntity == 0) {
            result = DDS.RETCODE_ALREADY_DELETED.value;
        }
        result = checkProperty(a_property);
        if (result == DDS.RETCODE_OK.value) {
            result = DDS.RETCODE_UNSUPPORTED.value;
            ReportStack.report(result, "Method not implemented yet.");
        }
        ReportStack.flush(this, result != DDS.RETCODE_OK.value);

        return result;
    }

    protected int checkPropertyHolder(PropertyHolder a_property) {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if ((result == DDS.RETCODE_OK.value) && (a_property == null)) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "Supplied PropertyHolder is null.");
        }
        if ((result == DDS.RETCODE_OK.value) && (a_property.value == null)) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "Supplied PropertyHolder.value is null.");
        }
        if ((result == DDS.RETCODE_OK.value) && (a_property.value.name == null)) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,"Supplied PropertyHolder.value.name is null.");
        }
        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;

    }

    @Override
    public int get_property(PropertyHolder a_property) {
        int result = DDS.RETCODE_OK.value;
        long uEntity;

        ReportStack.start();

        uEntity = this.get_user_object();

        if (uEntity == 0) {
            result = DDS.RETCODE_ALREADY_DELETED.value;
        }
        result = checkPropertyHolder(a_property);
        if (result == DDS.RETCODE_OK.value){
            result = DDS.RETCODE_UNSUPPORTED.value;
            ReportStack.report(result, "Method not implemented yet.");
        }
        ReportStack.flush(this, result != DDS.RETCODE_OK.value);

        return result;
    }

    synchronized protected void notify_listener_disabled()
    {
        assert (wait == true);

        wait = false;
        this.notifyAll();
    }

    synchronized protected void wait_listener_disabled() {
        try {
            int wakeCount = 0;
            while (wait && wakeCount < 4) {
                /* OSPL-6164 Made wait use a timeout in order to be able to detect
                 * a race where sometimes the wait() doesn't return (missed event?). */
                this.wait(2500);
                if(wait){
                    wakeCount++;
                    /* This is either a spurious wake-up or a timeout. Unfortunately
                     * we can't distinguish between them. */
                    ReportStack.deprecated(this + ": timeout or spurious wake-up happened " + wakeCount + " times. Will " + (wakeCount < 4 ? "" : "not") + " wait again.");
                }
            }
        } catch (InterruptedException e1) { }
    }

    synchronized protected int disable_callbacks() {
        int result = DDS.RETCODE_OK.value;
        long uEntity;

        ReportStack.start();

        uEntity = this.get_user_object();
        if (uEntity != 0) {
            if (jniDisableCallbacks(uEntity) != 0) {
                wait = true;
                wait_listener_disabled();
            }
        } else {
            result = DDS.RETCODE_ALREADY_DELETED.value;
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    private native int jniDisableCallbacks(long uEntity);
    private native int jniEnable(long uEntity);
    private native boolean jniIsEnable(long uEntity);
    private native int jniGetStatusChanges(long uEntity);
    private native long jniGetInstanceHandle(long uEntity);
    private native int jniSetListener(long uEntity, long uListener, int mask);
}
