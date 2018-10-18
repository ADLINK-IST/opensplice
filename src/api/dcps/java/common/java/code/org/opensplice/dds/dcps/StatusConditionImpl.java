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

import DDS.RETCODE_OK;

/**
 * Implementation of the {@link DDS.StatusCondition} interface.
 */
public class StatusConditionImpl extends StatusConditionBase implements DDS.StatusCondition {

    private static final long serialVersionUID = -4875040149672344445L;
    private int enabledStatuses = DDS.STATUS_MASK_ANY.value;
    private EntityImpl entity = null;

    protected int init (
        EntityImpl entity)
    {
        int result = DDS.RETCODE_OK.value;

        assert (entity != null);

        long uEntity = entity.get_user_object();
        if (uEntity != 0) {
            long uCondition = jniStatusConditionNew(uEntity);
            if (uCondition != 0) {
                this.set_user_object(uCondition);
                this.entity = entity;
                this.setDomainId(entity.getDomainId());
                result = DDS.RETCODE_OK.value;
            } else {
                result = DDS.RETCODE_ERROR.value;
            }
        } else {
            result = DDS.RETCODE_ALREADY_DELETED.value;
        }

        return result;
    }

    @Override
    protected int deinit ()
    {
        int result = DDS.RETCODE_OK.value;

        synchronized (this)
        {
            long uCondition = this.get_user_object();

            if (uCondition != 0) {
                entity = null;
                result = super.deinit();/* first detach from waitset */
                if (result == RETCODE_OK.value) {
                    result = jniStatusConditionFree(uCondition);
                }
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        return result;
    }

    /* see DDS.StatusConditionOperations for javadoc */
    @Override
    public int get_enabled_statuses ()
    {
        return enabledStatuses;
    }

    /* see DDS.StatusConditionOperations for javadoc */
    @Override
    public int set_enabled_statuses (int mask)
    {
        long uStatusCondition = 0;
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        uStatusCondition = this.get_user_object();
        if (uStatusCondition != 0) {
            result = jniSetEnabledStatuses(uStatusCondition, mask);
            if (result == DDS.RETCODE_OK.value) {
                enabledStatuses = mask;
            }
        } else {
            result = DDS.RETCODE_ALREADY_DELETED.value;
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.StatusConditionOperations for javadoc */
    @Override
    public DDS.Entity get_entity ()
    {
        return entity;
    }

    @Override
    public boolean get_trigger_value ()
    {
        long uStatusCondition = 0;
        boolean result = false;
        ReportStack.start();

        uStatusCondition = this.get_user_object();
        if (uStatusCondition != 0) {
            result = jniGetTriggerValue(uStatusCondition);
        }
        /* temporary disable the flush until the jniGetTriggerValue returns a retcode
         * which we can check for the flush */
        ReportStack.flush(this, false);
        return result;
    }

    private native long jniStatusConditionNew(long uEntity);
    private native int jniStatusConditionFree(long uStatusCondition);
    private native int jniSetEnabledStatuses(long uStatusCondition, int mask);
    private native boolean jniGetTriggerValue(long uStatusCondition);
}
