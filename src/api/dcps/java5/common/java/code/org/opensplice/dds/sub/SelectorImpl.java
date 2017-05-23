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
import java.util.Collections;
import java.util.List;

import org.omg.dds.core.InstanceHandle;
import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.sub.DataReader;
import org.omg.dds.sub.DataReader.Selector;
import org.omg.dds.sub.ReadCondition;
import org.omg.dds.sub.Sample;
import org.omg.dds.sub.Sample.Iterator;
import org.omg.dds.sub.Subscriber.DataState;
import org.opensplice.dds.core.IllegalArgumentExceptionImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;

public class SelectorImpl<TYPE> implements DataReader.Selector<TYPE> {
    private final OsplServiceEnvironment environment;
    private final AbstractDataReader<TYPE> reader;
    private final int maxSamples;
    private final InstanceHandle instance;
    private final boolean retrieveNextInstance;
    private final DataState dataState;
    private final ReadConditionImpl<TYPE> condition;
    private final String queryExpression;
    private final List<String> queryParameters;

    public SelectorImpl(OsplServiceEnvironment environment,
            AbstractDataReader<TYPE> reader) {
        this.environment = environment;
        this.reader = reader;
        this.maxSamples = DDS.LENGTH_UNLIMITED.value;
        this.condition = null;
        this.dataState = DataStateImpl.any(environment);
        this.queryExpression = null;
        this.queryParameters = new ArrayList<String>();
        this.instance = this.environment.getSPI().nilHandle();
        this.retrieveNextInstance = false;
    }

    @SuppressWarnings("unchecked")
    private SelectorImpl(SelectorImpl<TYPE> s, DataState dataState,
            String queryExpression, List<String> queryParameters,
            int maxSamples, InstanceHandle instance,
            boolean retrieveNextInstance) {
        this.environment = s.environment;
        this.reader = s.reader;
        this.maxSamples = maxSamples;
        this.instance = instance;
        this.retrieveNextInstance = retrieveNextInstance;
        this.dataState = dataState.clone();
        this.queryExpression = queryExpression;

        if (queryParameters == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Illegal List of query arguments (null) provided.");
        }
        this.queryParameters = new ArrayList<String>(queryParameters);

        if (this.queryExpression != null) {
            this.condition = (ReadConditionImpl<TYPE>) this.reader
                    .createQueryCondition(this.dataState, this.queryExpression,
                            this.queryParameters);
        } else {
            this.condition = null;
        }
    }

    private SelectorImpl(SelectorImpl<TYPE> s, String queryExpression,
            List<String> queryParameters) {
        this(s, s.dataState, queryExpression, queryParameters, s.maxSamples,
                s.instance, s.retrieveNextInstance);
    }

    private SelectorImpl(SelectorImpl<TYPE> s, DataState dataState) {
        this(s, dataState, s.queryExpression, s.queryParameters, s.maxSamples,
                s.instance, s.retrieveNextInstance);
    }

    private SelectorImpl(SelectorImpl<TYPE> s, int maxSamples) {
        this(s, s.dataState, s.queryExpression, s.queryParameters, maxSamples,
                s.instance, s.retrieveNextInstance);
    }

    private SelectorImpl(SelectorImpl<TYPE> s, InstanceHandle instance) {
        this(s, s.dataState, s.queryExpression, s.queryParameters,
                s.maxSamples, instance, s.retrieveNextInstance);
    }

    private SelectorImpl(SelectorImpl<TYPE> s, boolean retrieveNextInstance) {
        this(s, s.dataState, s.queryExpression, s.queryParameters,
                s.maxSamples, s.instance, retrieveNextInstance);
    }

    @Override
    public ServiceEnvironment getEnvironment() {
        return this.environment;
    }

    @Override
    public Selector<TYPE> instance(InstanceHandle handle) {
        if (handle == null) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Illegal InstanceHandle (null) provided.");
        }
        if (!handle.equals(this.instance)) {
            return new SelectorImpl<TYPE>(this, handle);
        }
        return this;
    }

    @Override
    public Selector<TYPE> nextInstance(boolean retrieveNextInstance) {
        if (this.retrieveNextInstance != retrieveNextInstance) {
            return new SelectorImpl<TYPE>(this, retrieveNextInstance);
        }
        return this;
    }

    private void checkDataStateValidity(DataState state) {
        if (state.getInstanceStates().size() == 0) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Illegal instance state.");
        }
        if (state.getSampleStates().size() == 0) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Illegal sample state.");
        }
        if (state.getViewStates().size() == 0) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Illegal view state.");
        }
    }

    @Override
    public Selector<TYPE> dataState(DataState state) {
        if (state == null) {
            throw new IllegalArgumentExceptionImpl(environment,
                    "Illegal DataState (null) provided.");
        }
        checkDataStateValidity(state);

        if (!state.equals(this.dataState)) {
            return new SelectorImpl<TYPE>(this, state);
        }
        return this;
    }

    private boolean isQueryEqualToCurrent(String qe, List<String> qp) {
        if (qe == null) {
            if (qp != null) {
                if (qp.size() > 0) {
                    throw new IllegalArgumentException(
                            "Query parameters for null query expression provided.");
                }
            }
            if (this.queryExpression == null) {
                if (qp == null) {
                    return false;
                }
                return true;
            }
            return false;
        }

        if (qe.equals(this.queryExpression)) {
            if (qp == null) {
                if (this.queryParameters.size() == 0) {
                    return true;
                }
                return false;
            } else if (qp.size() == 0 && this.queryParameters.size() == 0) {
                return true;
            } else if (qp.size() != this.queryParameters.size()) {
                return false;
            } else {
                for (int i = 0; i < qp.size(); i++) {
                    if (!qp.get(i).equals(this.queryParameters.get(i))) {
                        return false;
                    }
                }
                return true;
            }
        }
        return false;
    }

    @Override
    public Selector<TYPE> Content(String queryExpression,
            List<String> queryParameters) {

        if (!isQueryEqualToCurrent(queryExpression, queryParameters)) {
            return new SelectorImpl<TYPE>(this, queryExpression,
                    queryParameters);
        }
        return this;
    }

    @Override
    public Selector<TYPE> Content(String queryExpression,
            String... queryParameters) {
        if (queryParameters == null) {
            return this.Content(queryExpression, new ArrayList<String>());
        }
        return this.Content(queryExpression, Arrays.asList(queryParameters));
    }

    @Override
    public Selector<TYPE> maxSamples(int max) {
        if (max < 1) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "maxSamples < 1 not supported.");
        }
        if (this.maxSamples != max) {
            return new SelectorImpl<TYPE>(this, max);
        }
        return this;
    }

    @Override
    public InstanceHandle getInstance() {
        return this.instance;
    }

    @Override
    public boolean retrieveNextInstance() {
        return this.retrieveNextInstance;
    }

    @Override
    public DataState getDataState() {
        return this.dataState.clone();

    }

    @Override
    public String getQueryExpression() {
        return this.queryExpression;

    }

    @Override
    public List<String> getQueryParameters() {
        return Collections.unmodifiableList(this.queryParameters);
    }

    @Override
    public int getMaxSamples() {
        return this.maxSamples;
    }

    @Override
    public ReadCondition<TYPE> getCondition() {
        if (this.condition == null) {
            return this.reader.createReadCondition(this.dataState);
        }
        return this.condition;
    }

    @Override
    public Iterator<TYPE> read() {
        return this.reader.read(this);
    }

    @Override
    public List<Sample<TYPE>> read(List<Sample<TYPE>> samples) {
        return this.reader.read(samples, this);
    }

    @Override
    public Iterator<TYPE> take() {
        return this.reader.take(this);
    }

    @Override
    public List<Sample<TYPE>> take(List<Sample<TYPE>> samples) {
        return this.reader.take(samples, this);
    }

    @Override
    protected void finalize() {
        if (this.condition != null) {
            try {
                this.condition.close();
            } catch (Exception e) {
                // Ignore any exception here.
            }
        }
    }
}
