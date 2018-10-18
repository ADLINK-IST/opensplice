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
import java.util.Iterator;
import java.util.Set;

/**
 * Implementation of the {@link DDS.Condition} interface.
 */
public abstract class ConditionImpl extends ConditionBase implements DDS.Condition {
    private static final long serialVersionUID = 1490993411385828631L;
    protected final Set<WaitSetImpl> waitsets = new HashSet<WaitSetImpl>();

    @Override
    protected int deinit ()
    {
        int result = DDS.RETCODE_OK.value;

        synchronized (this)
        {
            // Copy the waitsets Set because the ws.detach_condition()
            // call can change that waitsets Set in a roundabout way.
            Set<WaitSetImpl> copy = new HashSet<WaitSetImpl>(this.waitsets);
            Iterator<WaitSetImpl> it = copy.iterator();
            while (it.hasNext()) {
                WaitSetImpl ws = it.next();
                ws.detach_condition(this);
            }

            copy.clear();
            this.waitsets.clear();
            result = super.deinit();
        }
        return result;
    }

    protected int attach_waitset(WaitSetImpl ws)
    {
        int result;

        synchronized (this)
        {
            if (!this.waitsets.contains(ws)) {
                long uCondition = get_user_object();
                if (uCondition != 0) {
                    result = ws.attachGeneralCondition(this, uCondition);
                    if (result == DDS.RETCODE_OK.value) {
                        boolean insertOK;

                        insertOK = this.waitsets.add(ws);
                        assert(insertOK);
                    }
                } else {
                    result = DDS.RETCODE_BAD_PARAMETER.value;
                }
            } else {
                result = DDS.RETCODE_OK.value;
            }
        }
        return result;
    }

    /* see DDS.WaitSetOperations for javadoc */
    protected int detach_waitset(WaitSetImpl ws)
    {
        int result;

        synchronized (this)
        {
            long uCondition = get_user_object();
            if (uCondition != 0) {
                if (this.waitsets.remove(ws)) {
                    result = ws.detachGeneralCondition(this, uCondition);
                } else {
                    result = DDS.RETCODE_PRECONDITION_NOT_MET.value;
                }
            } else {
                result = DDS.RETCODE_BAD_PARAMETER.value;
            }
        }
        return result;
    }

    protected int trigger()
    {
        int result = DDS.RETCODE_OK.value;
        int end_result = DDS.RETCODE_OK.value;
        synchronized (this)
        {
            Iterator<WaitSetImpl> it = this.waitsets.iterator();
            while (it.hasNext()) {
                WaitSetImpl ws = it.next();
                result = ws.trigger();
                if (result == DDS.RETCODE_ALREADY_DELETED.value) {
                    result = DDS.RETCODE_OK.value;
                }
                if (result != DDS.RETCODE_OK.value) {
                    end_result = result;
                }
            }
        }
        return end_result;
    }

    protected int is_alive()
    {
        int result = DDS.RETCODE_OK.value;
        long uCondition = get_user_object();

        if (uCondition != 0) {
            result = jniConditionIsAlive(uCondition);
        }

        return result;
    }

    private native int jniConditionIsAlive(long uCondition);
}
