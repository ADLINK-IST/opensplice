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
 * Implementation of the {@link DDS.QueryCondition} interface. 
 */ 
public class QueryConditionImpl extends QueryConditionBase implements DDS.QueryCondition {

    private static final long serialVersionUID = 6576466365353996218L;
    private String expression = null;
    private String parameters[] = null;
    protected QueryConditionImpl() { }

    protected int init (
        DataReaderImpl reader,
        int sample_states,
        int view_states,
        int instance_states,
        String query_expression,
        String[] query_parameters)
    {
        int result = DDS.RETCODE_OK.value;

        assert (reader != null);
        assert (query_expression != null);

        long uReader = reader.get_user_object();
        if (uReader != 0) {
            expression = new String(query_expression);
            if (query_parameters != null) {
                int len = query_parameters.length;
                parameters = new String[len];
                for (int i=0; i<len; i++) {
                    parameters[i] = new String(query_parameters[i]);
                }
            }
            long uQuery = jniQueryConditionNew(uReader, this, sample_states,
                                               view_states, instance_states,
                                               query_expression, query_parameters);

            if (uQuery != 0) {
                result = init(reader, sample_states, view_states, instance_states, uQuery);
            } else {
                result = DDS.RETCODE_ERROR.value;
            }
        } else {
            result = DDS.RETCODE_ALREADY_DELETED.value;
        }

        return result;
    }

    protected int init (
        DataReaderViewImpl view,
        int sample_states,
        int view_states,
        int instance_states,
        String query_expression,
        String[] query_parameters)
    {
        int result = DDS.RETCODE_OK.value;

        assert (view != null);
        assert (query_expression != null);

        long uView = view.get_user_object();
        if (uView != 0) {
            expression = new String(query_expression);
            if (query_parameters != null) {
                int len = query_parameters.length;
                parameters = new String[len];
                for (int i=0; i<len; i++) {
                    parameters[i] = new String(query_parameters[i]);
                }
            }
            long uQuery = jniQueryConditionNew(uView, this, sample_states,
                                               view_states, instance_states,
                                               query_expression, query_parameters);
            if (uQuery != 0) {
                result = init(view, sample_states, view_states, instance_states, uQuery);
            } else {
                result = DDS.RETCODE_ERROR.value;
            }
        } else {
            result = DDS.RETCODE_ALREADY_DELETED.value;
        }

        return result;
    }

    /* see DDS.QueryConditionOperations for javadoc */
    @Override
    public String get_query_expression ()
    {
        return expression;
    }

    /* see DDS.QueryConditionOperations for javadoc */
    @Override
    public int get_query_parameters (DDS.StringSeqHolder query_parameters)
    {
        int len = 0;
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (query_parameters == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "query_parameters 'null' is invalid.");
        } else {
            synchronized (this)
            {
                if (parameters != null) {
                    len = parameters.length;
                }
                query_parameters.value = new String[len];
                for (int i=0; i<len; i++) {
                    query_parameters.value[i] = new String(parameters[i]);
                }
            }
        }

        ReportStack.flush(this, result != DDS.RETCODE_OK.value);
        return result;
    }

    /* see DDS.QueryConditionOperations for javadoc */
    @Override
    public int set_query_parameters (String[] query_parameters)
    {
        long uQuery = 0;
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        synchronized (this)
        {
            uQuery = this.get_user_object();
            if (uQuery != 0) {
                if (query_parameters != null) {
                    int len = query_parameters.length;
                    parameters = new String[len];
                    for (int i=0; i<len; i++) {
                        parameters[i] = new String(query_parameters[i]);
                    }
                    result = jniSetQueryParameters(uQuery, query_parameters);
                } else {
                    result = DDS.RETCODE_BAD_PARAMETER.value;
                    ReportStack.report (result, "query_parameters 'null' is invalid.");
                }
            } else {
                result = DDS.RETCODE_ALREADY_DELETED.value;
            }
        }

        ReportStack.flush (this, result != DDS.RETCODE_OK.value);
        return result;
    }

    private native long jniQueryConditionNew(long uReader,
                                             Object condition,
                                             int sample_mask,
                                             int view_mask,
                                             int instance_mask,
                                             String query_expression,
                                             String[] query_parameters);

    private native int jniQueryConditionFree(long uQuery);
    private native int jniSetQueryParameters(long uQuery, String[] query_parameters);
}
