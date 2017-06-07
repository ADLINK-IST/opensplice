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

public abstract class FooDataReaderImpl extends org.opensplice.dds.dcps.DataReaderImpl
{
    private static final long serialVersionUID = -4401621125050984663L;

    @Override
    protected int deinit () { return super.deinit(); }

    public int readWCondition (
        long copyCache,
        Object data_values,
        DDS.SampleInfoSeqHolder info_seq,
        int max_samples,
        Object condition)
    {
        int result = DDS.RETCODE_ALREADY_DELETED.value;
        int sample_state;
        int view_state;
        int instance_state;
        long uReader;
        long uQuery;

        uReader = this.get_user_object();
        if (uReader != 0) {
            if (condition instanceof ReadConditionImpl) {
                if (condition instanceof QueryConditionImpl) {
                    uQuery = ((QueryConditionImpl)condition).get_user_object();
                    if (uQuery != 0) {
                        result = jniReadWCondition(this,
                                         uReader,
                                         copyCache,
                                         data_values,
                                         info_seq,
                                         max_samples,
                                         uQuery);
                    } else {
                        result = DDS.RETCODE_BAD_PARAMETER.value;
                        ReportStack.report(result, "Condition already deleted.");
                    }
                } else {
                    sample_state = ((ReadConditionImpl)condition).get_sample_state_mask();
                    view_state =((ReadConditionImpl)condition).get_view_state_mask();
                    instance_state =((ReadConditionImpl)condition).get_instance_state_mask();
                    result = jniRead(this,
                                     uReader,
                                     copyCache,
                                     data_values,
                                     info_seq,
                                     max_samples,
                                     sample_state,
                                     view_state,
                                     instance_state);
                }
            } else {
                result = DDS.RETCODE_BAD_PARAMETER.value;
                ReportStack.report(result, "Invalid condition.");

            }
        }
        return result;
    }

    public int takeWCondition (
        long copyCache,
        Object data_values,
        DDS.SampleInfoSeqHolder info_seq,
        int max_samples,
        Object condition)
    {
        int result = DDS.RETCODE_ALREADY_DELETED.value;
        int sample_state;
        int view_state;
        int instance_state;
        long uReader;
        long uQuery;

        uReader = this.get_user_object();
        if (uReader != 0) {
            if (condition instanceof ReadConditionImpl) {
                if (condition instanceof QueryConditionImpl) {
                    uQuery = ((QueryConditionImpl)condition).get_user_object();
                    if (uQuery != 0) {
                        result = jniTakeWCondition(this,
                                         uReader,
                                         copyCache,
                                         data_values,
                                         info_seq,
                                         max_samples,
                                         uQuery);
                    } else {
                        result = DDS.RETCODE_BAD_PARAMETER.value;
                        ReportStack.report(result, "Condition already deleted.");
                    }
                } else {
                    sample_state = ((ReadConditionImpl)condition).get_sample_state_mask();
                    view_state =((ReadConditionImpl)condition).get_view_state_mask();
                    instance_state =((ReadConditionImpl)condition).get_instance_state_mask();
                    result = jniTake(this,
                                     uReader,
                                     copyCache,
                                     data_values,
                                     info_seq,
                                     max_samples,
                                     sample_state,
                                     view_state,
                                     instance_state);
                }
            } else {
                result = DDS.RETCODE_BAD_PARAMETER.value;
                ReportStack.report(result, "Invalid condition.");
            }
        }
        return result;
    }

    public int readNextInstanceWCondition (
        long copyCache,
        Object data_values,
        DDS.SampleInfoSeqHolder info_seq,
        int max_samples,
        long handle,
        Object condition)
    {
        int result = DDS.RETCODE_ALREADY_DELETED.value;
        int sample_state;
        int view_state;
        int instance_state;
        long uReader;
        long uQuery;

        uReader = this.get_user_object();
        if (uReader != 0) {
            if (condition instanceof ReadConditionImpl) {
                if (condition instanceof QueryConditionImpl) {
                    uQuery = ((QueryConditionImpl)condition).get_user_object();
                    if (uQuery != 0) {
                        result = jniReadNextInstanceWCondition(this,
                                         uReader,
                                         copyCache,
                                         data_values,
                                         info_seq,
                                         max_samples,
                                         handle,
                                         uQuery);
                    } else {
                        result = DDS.RETCODE_BAD_PARAMETER.value;
                        ReportStack.report(result, "Condition already deleted.");
                    }
                } else {
                    sample_state = ((ReadConditionImpl)condition).get_sample_state_mask();
                    view_state =((ReadConditionImpl)condition).get_view_state_mask();
                    instance_state =((ReadConditionImpl)condition).get_instance_state_mask();
                    result = jniReadNextInstance(this,
                                     uReader,
                                     copyCache,
                                     data_values,
                                     info_seq,
                                     max_samples,
                                     handle,
                                     sample_state,
                                     view_state,
                                     instance_state);
                }
            } else {
                result = DDS.RETCODE_BAD_PARAMETER.value;
                ReportStack.report(result, "Invalid condition.");
            }
        }
        return result;
    }

    public int takeNextInstanceWCondition (
        long copyCache,
        Object data_values,
        DDS.SampleInfoSeqHolder info_seq,
        int max_samples,
        long handle,
        Object condition)
    {
        int result = DDS.RETCODE_ALREADY_DELETED.value;
        int sample_state;
        int view_state;
        int instance_state;
        long uReader;
        long uQuery;

        uReader = this.get_user_object();
        if (uReader != 0) {
            if (condition instanceof ReadConditionImpl) {
                if (condition instanceof QueryConditionImpl) {
                    uQuery = ((QueryConditionImpl)condition).get_user_object();
                    if (uQuery != 0) {
                        result = jniTakeNextInstanceWCondition(this,
                                         uReader,
                                         copyCache,
                                         data_values,
                                         info_seq,
                                         max_samples,
                                         handle,
                                         uQuery);
                    } else {
                        result = DDS.RETCODE_BAD_PARAMETER.value;
                        ReportStack.report(result, "Condition already deleted.");
                    }
                } else {
                    sample_state = ((ReadConditionImpl)condition).get_sample_state_mask();
                    view_state =((ReadConditionImpl)condition).get_view_state_mask();
                    instance_state =((ReadConditionImpl)condition).get_instance_state_mask();
                    result = jniTakeNextInstance(this,
                                     uReader,
                                     copyCache,
                                     data_values,
                                     info_seq,
                                     max_samples,
                                     handle,
                                     sample_state,
                                     view_state,
                                     instance_state);
                }
            } else {
                result = DDS.RETCODE_BAD_PARAMETER.value;
                ReportStack.report(result, "Invalid condition.");
            }
        }
        return result;
    }

    public native static int jniRead (
        Object DataReader,
        long uReader,
        long copyCache,
        Object data_values,
        DDS.SampleInfoSeqHolder info_seq,
        int max_samples,
        int sample_states,
        int view_states,
        int instance_states);

    public native static int jniReadWCondition (
        Object DataReader,
        long uReader,
        long copyCache,
        Object data_values,
        DDS.SampleInfoSeqHolder info_seq,
        int instance_states,
        long uQuery);

    public native static int jniTake (
        Object DataReader,
        long uReader,
        long copyCache,
        Object data_values,
        DDS.SampleInfoSeqHolder info_seq,
        int max_samples,
        int sample_states,
        int view_states,
        int instance_states);

    public native static int jniTakeWCondition (
        Object DataReader,
        long uReader,
        long copyCache,
        Object data_values,
        DDS.SampleInfoSeqHolder info_seq,
        int max_samples,
        long uQuery);

    public native static int jniReadNextSample (
        Object DataReader,
        long uReader,
        long copyCache,
        Object data_values,
        DDS.SampleInfoHolder sample_info);

    public native static int jniTakeNextSample (
        Object DataReader,
        long uReader,
        long copyCache,
        Object data_values,
        DDS.SampleInfoHolder sample_info);

    public native static int jniReadInstance (
        Object DataReader,
        long uReader,
        long copyCache,
        Object data_values,
        DDS.SampleInfoSeqHolder info_seq,
        int max_samples,
        long a_handle,
        int sample_states,
        int view_states,
        int instance_states);

    public native static int jniTakeInstance (
        Object DataReader,
        long uReader,
        long copyCache,
        Object data_values,
        DDS.SampleInfoSeqHolder info_seq,
        int max_samples,
        long a_handle,
        int sample_states,
        int view_states,
        int instance_states);

    public native static int jniReadNextInstance (
        Object DataReader,
        long uReader,
        long copyCache,
        Object data_values,
        DDS.SampleInfoSeqHolder info_seq,
        int max_samples,
        long a_handle,
        int sample_states,
        int view_states,
        int instance_states);

    public native static int jniTakeNextInstance (
        Object DataReader,
        long uReader,
        long copyCache,
        Object data_values,
        DDS.SampleInfoSeqHolder info_seq,
        int max_samples,
        long a_handle,
        int sample_states,
        int view_states,
        int instance_states);

    public native static int jniReadNextInstanceWCondition (
        Object DataReader,
        long uReader,
        long copyCache,
        Object data_values,
        DDS.SampleInfoSeqHolder info_seq,
        int max_samples,
        long a_handle,
        long uQuery);

    public native static int jniTakeNextInstanceWCondition (
        Object DataReader,
        long uReader,
        long copyCache,
        Object data_values,
        DDS.SampleInfoSeqHolder info_seq,
        int max_samples,
        long a_handle,
        long uQuery);

    public native static int jniGetKeyValue (
        long uReader,
        long copyCache,
        Object key_holder,
        long handle);

    public native static long jniLookupInstance (
        long uReader,
        long copyCache,
        Object instance);
}
