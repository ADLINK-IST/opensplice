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
 *
 */
package org.opensplice.dds.dcps;

import java.util.concurrent.atomic.AtomicBoolean;

import DDS.DomainParticipantFactory;

public class GuardConditionImpl extends org.opensplice.dds.dcps.GuardConditionBase implements
        DDS.GuardConditionInterface
{
    private static final long serialVersionUID = 451094059159292905L;
    private AtomicBoolean triggerValue = new AtomicBoolean(false);

    /* The following static code exists to guarantee that the native dcps library is loaded.
     * This because WaitSets as Guard conditions can be created before the
     * DomainParticipantFactory.get_instance() is called which normally loads the native library.
     */
    static {
        DomainParticipantFactory.get_instance();
    }

    public GuardConditionImpl(){ }

    @Override
    protected int deinit ()
    {
        return super.deinit();
    }

    /* see DDS.GuardConditionOperations for javadoc */
    @Override
    public int set_trigger_value (boolean value) {
        /* only trigger when the state changes */
        if (triggerValue.compareAndSet(!value, value)) {
            this.trigger();
        }
        return DDS.RETCODE_OK.value;
    }

    @Override
    public boolean get_trigger_value ()
    {
        return triggerValue.get();
    }

    @Override
    protected int attach_waitset(WaitSetImpl ws)
    {
        int result = DDS.RETCODE_OK.value;
        
        synchronized (this.waitsets)
        {
            if (!this.waitsets.contains(ws)) {
                result = ws.attachGuardCondition(this);
                if (result == DDS.RETCODE_OK.value) {
                    boolean inserted = this.waitsets.add(ws);
                    assert(inserted);
                }
            }
        }

        return result;
    }

    /* see DDS.WaitSetOperations for javadoc */
    @Override
    protected int detach_waitset(WaitSetImpl ws)
    {
        int result = DDS.RETCODE_OK.value;

        synchronized (this.waitsets)
        {
            if (this.waitsets.remove(ws)) {
                result = ws.detachGuardCondition(this);
            } else {
                result = DDS.RETCODE_PRECONDITION_NOT_MET.value;
            }
        }

        return result;
    }
}
