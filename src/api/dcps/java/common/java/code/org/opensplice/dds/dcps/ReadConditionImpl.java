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

/**
 * Implementation of the {@link DDS.ReadCondition} interface.
 */
public class ReadConditionImpl extends ReadConditionBase implements DDS.ReadCondition {

    private static final long serialVersionUID = -8237808904293010400L;
    private DataReaderImpl reader;
    private DataReaderViewImpl view;
    private int sample_states;
    private int view_states;
    private int instance_states;

    protected ReadConditionImpl() { }

    protected int init (
        DataReaderImpl reader,
        int sample_states,
        int view_states,
        int instance_states,
        long uQuery)
    {
        int result = DDS.RETCODE_BAD_PARAMETER.value;
        if (reader != null) {
            result = DDS.RETCODE_OK.value;
            if (uQuery == 0) {
                long uReader = reader.get_user_object();
                if (uReader != 0) {
                    uQuery = jniReadConditionNew(uReader,
                                                 this,
                                                 sample_states,
                                                 view_states,
                                                 instance_states);
                } else {
                    result = DDS.RETCODE_ALREADY_DELETED.value;
                }
            }
            this.set_user_object(uQuery);
            this.reader = reader;
            this.view = null;
            this.sample_states = sample_states;
            this.view_states = view_states;
            this.instance_states = instance_states;
            this.setDomainId(reader.getDomainId());
        }

        return result;
    }

    protected int init (
        DataReaderViewImpl view,
        int sample_states,
        int view_states,
        int instance_states,
        long uQuery)
    {
        int result = DDS.RETCODE_BAD_PARAMETER.value;
        if (view != null) {
            result = DDS.RETCODE_OK.value;
            if (uQuery == 0) {
                long uView = view.get_user_object();
                if (uView != 0) {
                    uQuery = jniReadConditionNew(uView,
                                                 this,
                                                 sample_states,
                                                 view_states,
                                                 instance_states);
                } else {
                    result = DDS.RETCODE_ALREADY_DELETED.value;
                }
            }
            this.set_user_object(uQuery);
            this.reader = null;
            this.view = view;
            this.sample_states = sample_states;
            this.view_states = view_states;
            this.instance_states = instance_states;
        }
        return result;
    }

    @Override
    protected int deinit ()
    {
        int result = DDS.RETCODE_ALREADY_DELETED.value;
        synchronized (this)
        {
            long uQuery = this.get_user_object();
            if (uQuery != 0) {
                this.reader = null;
                this.view = null;
                result = super.deinit(); /* first detach condition from waitset */
                if (result == DDS.RETCODE_OK.value) {
                    result = jniReadConditionFree(uQuery);
                }
            }
        }
        return result;
    }

    @Override
    public boolean get_trigger_value ()
    {
        long uQuery = 0;
        boolean result = false;
        ReportStack.start();

        uQuery = this.get_user_object();
        if (uQuery != 0) {
            result = jniGetTriggerValue(uQuery);
        }
        /* temporary disable the flush until the jniGetTriggerValue returns a retcode
         * which we can check for the flush */
        ReportStack.flush(this, false);
        return result;
    }

    private native boolean jniGetTriggerValue(long uQuery);

    /* see DDS.ReadConditionOperations for javadoc */
    @Override
    public int get_sample_state_mask () {
        return sample_states;
    }

    /* see DDS.ReadConditionOperations for javadoc */
    @Override
    public int get_view_state_mask () {
        return view_states;
    }

    /* see DDS.ReadConditionOperations for javadoc */
    @Override
    public int get_instance_state_mask () {
        return instance_states;
    }

    /* see DDS.ReadConditionOperations for javadoc */
    @Override
    public DDS.DataReader get_datareader () {
        return reader;
    }

    @Override
    public DDS.DataReaderView get_datareaderview(){
        return view;
    }

    private native long jniReadConditionNew(
                            long uReader,
                            Object condition,
                            int sample_mask,
                            int view_mask,
                            int instance_mask);

    private native int jniReadConditionFree(
                            long uQuery);
}
