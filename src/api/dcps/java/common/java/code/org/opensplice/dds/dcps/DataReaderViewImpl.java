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

import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;

/**
 * Implementation of the {@link DDS.DataReader} interface.
 */
public class DataReaderViewImpl extends DataReaderViewBase implements DDS.DataReaderView {

    private static final long serialVersionUID = 8794005356602301039L;
    private DataReaderImpl reader = null;
    private final Set<ReadConditionImpl> conditions = new HashSet<ReadConditionImpl>();

    protected DataReaderViewImpl() { }

    protected int init (
        DataReaderImpl reader,
        String name,
        DDS.DataReaderViewQos qos)
    {
        int result = DDS.RETCODE_OK.value;

        long uReader = reader.get_user_object();
        if (uReader != 0) {
            if (result == DDS.RETCODE_OK.value) {
                long uView = jniDataReaderViewNew(uReader, name, qos);
                if (uView != 0) {
                    this.set_user_object(uView);
                    this.reader = reader;
                    this.setDomainId(reader.getDomainId());
                }
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
        long uView = 0;

        synchronized (this)
        {
            uView = this.get_user_object();
            if (uView != 0) {
                if (this.conditions.size() == 0) {
                    result = ((EntityImpl) this).detach_statuscondition();
                    if (result == DDS.RETCODE_OK.value) {
                        reader = null;
                        result = jniDataReaderViewFree(uView);
                        if (result == DDS.RETCODE_OK.value) {
                            result = super.deinit();
                        }
                    }
                } else {
                    result = DDS.RETCODE_PRECONDITION_NOT_MET.value;
                    ReportStack.report(result,
                        "DataReaderView still contains '" +
                        this.conditions.size() + "' Condition entities.");
                }
            } else {
                result = DDS.RETCODE_PRECONDITION_NOT_MET.value;
            }
        }

        return result;
    }

    @Override
    public DDS.ReadCondition create_readcondition (
        int sample_states,
        int view_states,
        int instance_states)
    {
        int result = DDS.RETCODE_OK.value;
        ReadConditionImpl readCondition = null;
        ReportStack.start();

        if ((result = Utilities.checkSampleStateMask (sample_states))
                == DDS.RETCODE_OK.value &&
            (result = Utilities.checkViewStateMask (view_states))
                == DDS.RETCODE_OK.value &&
            (result = Utilities.checkInstanceStateMask (instance_states))
                == DDS.RETCODE_OK.value)
        {
            readCondition = new ReadConditionImpl();
            synchronized (this)
            {
                result = readCondition.init(
                    this, sample_states, view_states, instance_states, 0);
                if (result == DDS.RETCODE_OK.value) {
                    this.conditions.add(readCondition);
                } else {
                    readCondition = null;
                }
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return readCondition;
    }

    @Override
    public DDS.QueryCondition create_querycondition (
        int sample_states,
        int view_states,
        int instance_states,
        String query_expression,
        String[] query_parameters)
    {
        QueryConditionImpl queryCondition = null;
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (query_expression == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "query_expression 'null' is invalid.");
        } else if ((result = Utilities.checkSampleStateMask (sample_states))
                        == DDS.RETCODE_OK.value &&
                   (result = Utilities.checkViewStateMask (view_states))
                        == DDS.RETCODE_OK.value &&
                   (result = Utilities.checkInstanceStateMask (instance_states))
                        == DDS.RETCODE_OK.value)
        {
            queryCondition = new QueryConditionImpl();
            synchronized (this)
            {
                result = queryCondition.init(
                    this,
                    sample_states,
                    view_states,
                    instance_states,
                    query_expression,
                    query_parameters);

                if (result == DDS.RETCODE_OK.value) {
                    this.conditions.add(queryCondition);
                } else {
                    queryCondition = null;
                }
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return queryCondition;
    }

    @Override
    public int delete_readcondition (
        DDS.ReadCondition a_condition)
    {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (a_condition == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "a_condition 'null' is invalid.");
        } else {
            synchronized (this)
            {
                ReadConditionImpl rc = (ReadConditionImpl)a_condition;
                if (this.conditions.remove(rc)) {
                    result = rc.deinit();
                    if (result != DDS.RETCODE_OK.value && result != DDS.RETCODE_ALREADY_DELETED.value) {
                        this.conditions.add(rc);
                    }
                } else {
                    if (rc.get_user_object() == 0) {
                        result = DDS.RETCODE_ALREADY_DELETED.value;
                    } else {
                        result = DDS.RETCODE_PRECONDITION_NOT_MET.value;
                        ReportStack.report(
                            result, "Condition not created by DataReaderView.");
                    }
                }
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    @Override
    public int delete_contained_entities ()
    {
        int result, endResult = DDS.RETCODE_OK.value;
        ReportStack.start();

        synchronized (this)
        {
            if (this.get_user_object() != 0) {
                Iterator<ReadConditionImpl> it = this.conditions.iterator();
                while (it.hasNext()) {
                    result = it.next().deinit();
                    if (result == DDS.RETCODE_OK.value || result == DDS.RETCODE_ALREADY_DELETED.value) {
                        /* Use iterator.remove() to remove from the set */
                        it.remove();
                    }
                    if( result != DDS.RETCODE_OK.value){
                        ReportStack.report (result, "Deletion of ReadCondition contained in DataReaderView failed.");
                        if(endResult == DDS.RETCODE_OK.value) {
                            /* Store first encountered error. */
                            endResult = result;
                        }
                    }
                }
            } else {
                endResult = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush(this, endResult != DDS.RETCODE_OK.value);
        return endResult;
    }

    @Override
    public int set_qos (
        DDS.DataReaderViewQos qos)
    {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (qos == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "qos 'null' is invalid.");
        } else if (qos == DDS.DATAREADERVIEW_QOS_DEFAULT.value) {
            DDS.DataReaderViewQosHolder holder = new DDS.DataReaderViewQosHolder();
            this.reader.get_default_datareaderview_qos(holder);
            qos = holder.value;
        } else {
            result = Utilities.checkQos(qos);
        }

        if (result == DDS.RETCODE_OK.value) {
            long uView = this.get_user_object();
            if (uView != 0) {
                result = jniSetQos(uView, qos);
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    @Override
    public int get_qos (
        DDS.DataReaderViewQosHolder qos)
    {
        long uView = 0;
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (qos == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "qos 'null' is invalid.");
        } else {
            uView = this.get_user_object();
            if (uView != 0) {
                result = jniGetQos(uView, qos);
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    @Override
    public DDS.DataReader get_datareader ()
    {
        return reader;
    }

    @Override
    public DDS.StatusCondition get_statuscondition()
    {
        return null;
    }

    @Override
    public int get_status_changes()
    {
        int status;
        ReportStack.start();
        status = reader.get_status_changes();
        ReportStack.flush(this, status == 0);
        return status;
    }

    @Override
    protected int notify(Event e)
    {
        return DDS.RETCODE_OK.value;
    }

    private native long jniDataReaderViewNew(long uReader, String name, DDS.DataReaderViewQos qos);
    private native int jniDataReaderViewFree(long uView);

    private native int jniSetQos(long uView, DDS.DataReaderViewQos qos);
    private native int jniGetQos(long uView, DDS.DataReaderViewQosHolder qos);
}
