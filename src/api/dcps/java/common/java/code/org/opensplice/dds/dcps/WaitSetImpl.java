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

import org.opensplice.dds.dcps.ReportStack;

import java.util.Arrays;
import java.util.Collections;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;

import DDS.Condition;
import DDS.DomainParticipantFactory;

/**
 * Implementation of the {@link DDS.WaitSetInterface} interface.
 */
public class WaitSetImpl extends WaitSetBase implements DDS.WaitSetInterface {

    private static final long serialVersionUID = -6919928181265050955L;
    private final Set<DDS.Condition> conditions = Collections.synchronizedSet(new HashSet<DDS.Condition>());
    private final Set<GuardConditionImpl> guards = Collections.synchronizedSet(new HashSet<GuardConditionImpl>());

    /* The following static code exists to guarantee that the native dcps library is loaded.
     * This because WaitSets as Guard conditions can be created before the
     * DomainParticipantFactory.get_instance() is called which normally loads the native library.
     */
    static {
        DomainParticipantFactory.get_instance();
    }

    protected int denit () { return super.deinit(); }

    public WaitSetImpl(){
        long uWaitset = 0;
        synchronized(this)
        {
            uWaitset = jniWaitSetNew();
        }
        this.set_user_object(uWaitset);
    }

    private void finalizeArray(DDS.ConditionSeqHolder activeCond, int nrTriggeredConditions)
    {
        assert(activeCond.value != null);
        if (nrTriggeredConditions != activeCond.value.length)
        {
            DDS.Condition[] matchingArr = Arrays.copyOf(activeCond.value, nrTriggeredConditions);
            activeCond.value = matchingArr;
        }
    }

    private int collectDetachedConditions(DDS.ConditionSeqHolder activeCond)
    {
        int nr = 0;

        synchronized (conditions) {
            /* allocate worst-case array */
            assert(activeCond.value != null);
            if (activeCond.value.length < conditions.size()) {
                activeCond.value = new Condition[conditions.size()];
            }

            Iterator<DDS.Condition> i = conditions.iterator();
            while (i.hasNext()) {
                ConditionImpl ci = (ConditionImpl) i.next();
                if (ci.is_alive() == DDS.RETCODE_ALREADY_DELETED.value) {
                    activeCond.value[nr++] = ci;
                }
            }
        }
        return nr;
    }


    /* see DDS.WaitSetOperations for javadoc */
    @Override
    public int _wait (DDS.ConditionSeqHolder active_conditions, DDS.Duration_t timeout)
    {
        int result = DDS.RETCODE_OK.value;
        int nrTriggeredConditions = 0;
        int maxConditions = 0;
        long resultCombo;
        long uObject;
        Object[] attachedGuards;
        ReportStack.start();

        if (active_conditions == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "active_conditions 'null' is invalid");
        } else {
            result = Utilities.checkDuration(timeout);

            while (result == DDS.RETCODE_OK.value && nrTriggeredConditions == 0)
            {
                synchronized (guards) {
                    maxConditions = conditions.size() + guards.size() +1;
                    attachedGuards = guards.toArray();
                    if (active_conditions.value == null) {
                        active_conditions.value = new Condition[maxConditions];
                    }
                }
                uObject = this.get_user_object();
                if (uObject != 0) {
                    resultCombo = jniWait(uObject, active_conditions,
                            attachedGuards, maxConditions, timeout.sec, timeout.nanosec);

                    // resultCombo is a 64-bit combination of the returncode and
                    // the nrTriggeredConditions variable.
                    // It is demangled in its constituent parts below.
                    result = (int) (resultCombo & 0xffffffff);
                    if (result == -1) {
                        nrTriggeredConditions = collectDetachedConditions(active_conditions);
                        result = DDS.RETCODE_OK.value;
                    } else {
                        nrTriggeredConditions = (int) (resultCombo >> 32);
                    }
                    // reallocate array to exact fit.
                    finalizeArray(active_conditions, nrTriggeredConditions);
                } else {
                    result = DDS.RETCODE_ALREADY_DELETED.value;
                }
            }
        }
        ReportStack.flush(this, result != DDS.RETCODE_OK.value &&
                                result != DDS.RETCODE_TIMEOUT.value);
        return result;
    }

    /**
     * Condition specific callback from non-GuardConditions to WaitSet.
     */
    protected int attachGeneralCondition(DDS.Condition condition, long uCondition)
    {
        int result;

        synchronized(conditions)
        {
            assert(!conditions.contains(condition));

            // uCondition parameter contains a backRef to the Java object. JNI will
            // use this backRef for the userData in the UserLayer.
            result = jniAttachCondition(get_user_object(), uCondition, uCondition);
            if (result == DDS.RETCODE_OK.value)
            {
                boolean insertOK;

                insertOK = conditions.add(condition);
                assert(insertOK);
            }
        }
        return result;
    }

    /**
     * Condition specific callback from GuardConditions to WaitSet.
     */
    protected int attachGuardCondition(GuardConditionImpl guardCond)
    {
        int result;

        synchronized(guards)
        {
            assert(!guards.contains(guardCond));
            result = this.trigger();
            if (result == DDS.RETCODE_OK.value)
            {
                boolean insertOK;

                insertOK = guards.add(guardCond);
                assert(insertOK);
            }
        }
        return result;
    }

    /**
     * Condition specific callback from non-GuardConditions to WaitSet.
     */
    protected int detachGeneralCondition(DDS.Condition condition, long uCondition)
    {
        int result;

        synchronized(conditions)
        {
            assert(conditions.contains(condition));
            result = jniDetachCondition(get_user_object(), uCondition);
            if ((result == DDS.RETCODE_OK.value) || (result == DDS.RETCODE_ALREADY_DELETED.value))
            {
                boolean removeOK;

                removeOK = conditions.remove(condition);
                assert(removeOK);
            }
        }
        return result;
    }

    /**
     * Condition specific callback from GuardConditions to WaitSet.
     */
    protected int detachGuardCondition(GuardConditionImpl guardCond)
    {
        int result;

        synchronized(guards)
        {
            assert(guards.contains(guardCond));
            result = this.trigger();
            if (result == DDS.RETCODE_OK.value)
            {
                boolean removeOK;

                removeOK = guards.remove(guardCond);
                assert(removeOK);
            }
            return result;
        }
    }


    /* see DDS.WaitSetOperations for javadoc */
    @Override
    public int attach_condition (DDS.Condition cond)
    {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (cond instanceof GuardConditionImpl) {
            GuardConditionImpl guard = (GuardConditionImpl)cond;
            result = guard.attach_waitset(this);
        } else if (cond instanceof ConditionImpl) {
            ConditionImpl ci = (ConditionImpl) cond;
            result = ci.attach_waitset(this);
        } else {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "cond is invalid.");
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.WaitSetOperations for javadoc */
    @Override
    public int detach_condition (DDS.Condition cond)
    {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (cond instanceof GuardConditionImpl) {
            GuardConditionImpl guard = (GuardConditionImpl)cond;
            result = guard.detach_waitset(this);
        } else if (cond instanceof ConditionImpl) {
            ConditionImpl ci = (ConditionImpl) cond;
            result = ci.detach_waitset(this);
        } else {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "cond is invalid.");
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.WaitSetOperations for javadoc */
    @Override
    public int get_conditions (DDS.ConditionSeqHolder seq)
    {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (seq == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "attached_conditions 'null' is invalid.");
        } else {
            Set<DDS.Condition> allConditions = new HashSet<DDS.Condition>();
            synchronized (this.conditions) {
                synchronized (this.guards) {
                    allConditions.addAll(conditions);
                    allConditions.addAll(guards);
                    int nrOfCond = allConditions.size();
                    int n;

                    if (seq.value == null || seq.value.length != nrOfCond) {
                        seq.value = new DDS.Condition[nrOfCond];
                    }
                    Iterator<Condition> i = allConditions.iterator();
                    n = 0;
                    while (i.hasNext()) {
                        seq.value[n++] = i.next();
                    }
                    return DDS.RETCODE_OK.value;
                }
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    protected int trigger()
    {
        long uWaitset = 0;
        int result = DDS.RETCODE_OK.value;
        uWaitset = this.get_user_object();
        if (uWaitset != 0) {
            result = jniTrigger(uWaitset, 0);
        }

        return result;
    }

    @Override
    protected void finalize(){
        long uWaitset = this.get_user_object();
        if (uWaitset != 0) {
            jniWaitSetFree(uWaitset);
        }
    }

    @Override
    protected int getDomainId() {
        int domainId = -1;
        long uWaitset = this.get_user_object();

        if (uWaitset != 0) {
            domainId = jniWaitSetGetDomainId(uWaitset);
        }
        return domainId;
    }

    private native int jniTrigger(long uWaitset, long userData);
    private static native long jniWaitSetNew();
    private native void jniWaitSetFree(long uWaitset);
    private native long jniWait(
            long uWaitset,
            DDS.ConditionSeqHolder active_conditions,
            Object[] attachedGuards,
            int maxConditions,
            int durationSec,
            int durationNanoSec);
    private native int jniAttachCondition(long uWaitset, long uCondition, long userData);
    private native int jniDetachCondition(long uWaitset, long uCondition);
    private native int jniWaitSetGetDomainId(long uWaitset);
}

