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
package org.opensplice.dds.sub;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.omg.dds.sub.QueryCondition;
import org.opensplice.dds.core.Condition;
import org.opensplice.dds.core.IllegalArgumentExceptionImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;
import org.opensplice.dds.core.Utilities;

public class QueryConditionImpl<TYPE> extends ReadConditionImpl<TYPE> implements
        QueryCondition<TYPE>, Condition<DDS.ReadCondition> {
    public QueryConditionImpl(OsplServiceEnvironment environment,
            AbstractDataReader<TYPE> parent, DataStateImpl state,
            String queryExpression, List<String> queryParameters) {
        super(environment, parent, state, false);

        this.initCondition(queryExpression, queryParameters);
    }

    public QueryConditionImpl(OsplServiceEnvironment environment,
            AbstractDataReader<TYPE> parent,
            String queryExpression, List<String> queryParameters) {
        super(environment, parent, false);

        this.initCondition(queryExpression, queryParameters);
    }

    private void initCondition(String queryExpression, List<String> queryParameters){
        if (queryParameters != null) {
            for (String param : queryParameters) {
                if (param == null) {
                    throw new IllegalArgumentExceptionImpl(this.environment,
                            "Invalid query parameter (null) provided.");
                }
            }
            this.old = parent.getOld()
                    .create_querycondition(
                            this.state.getOldSampleState(),
                            this.state.getOldViewState(),
                            this.state.getOldInstanceState(),
                            queryExpression,
                            queryParameters.toArray(new String[queryParameters
                                    .size()]));
        } else {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Illegal List of query parameters (null) provided.");
        }
        if (this.old == null) {
            Utilities.throwLastErrorException(this.environment);
        }
    }

    @Override
    public String getQueryExpression() {
        return ((DDS.QueryCondition) this.old).get_query_expression();
    }

    @Override
    public List<String> getQueryParameters() {
        DDS.StringSeqHolder holder = new DDS.StringSeqHolder();
        int rc = ((DDS.QueryCondition) this.old).get_query_parameters(holder);

        Utilities.checkReturnCode(rc, this.environment,
                "QueryCondition.getQueryParameters() failed");
        ArrayList<String> queryParams = new ArrayList<String>();

        for (String param : holder.value) {
            queryParams.add(param);
        }
        return queryParams;
    }

    @Override
    public void setQueryParameters(List<String> queryParameters) {
        if (queryParameters == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Invalid queryParameters parameter (null) provided.");
        }
        for (String param : queryParameters) {
            if (param == null) {
                throw new IllegalArgumentExceptionImpl(this.environment,
                        "Invalid query parameter (null) provided.");
            }
        }
        ((DDS.QueryCondition) this.old).set_query_parameters(queryParameters
                .toArray(new String[queryParameters.size()]));
    }

    @Override
    public void setQueryParameters(String... queryParameters) {
        if (queryParameters == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Invalid queryParameters parameter (null) provided.");
        }
        for (String param : queryParameters) {
            if (param == null) {
                throw new IllegalArgumentExceptionImpl(this.environment,
                        "Invalid query parameter (null) provided.");
            }
        }
        this.setQueryParameters(Arrays.asList(queryParameters));
    }

    @Override
    public DDS.ReadCondition getOldCondition() {
        return this.old;
    }

}
