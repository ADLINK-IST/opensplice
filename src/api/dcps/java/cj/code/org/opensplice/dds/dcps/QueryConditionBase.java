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

public class QueryConditionBase extends ReadConditionImpl {

    private static final long serialVersionUID = 2206958401480415134L;

    @Override
    protected int deinit() {
        return super.deinit();
    }

    public static class QueryConditionBaseImpl extends DDS._QueryConditionLocalBase {
        private static final long serialVersionUID = 4539410966516244165L;

        /* querycondition operations */
        @Override
        public java.lang.String get_query_expression() {
            return null;
        }

        @Override
        public int get_query_parameters(DDS.StringSeqHolder query_parameters) {
            return 0;
        }

        @Override
        public int set_query_parameters(java.lang.String[] query_parameters) {
            return 0;
        }

        /* readcondition operations */
        @Override
        public int get_sample_state_mask() {
            return 0;
        }

        @Override
        public int get_view_state_mask() {
            return 0;
        }

        @Override
        public int get_instance_state_mask() {
            return 0;
        }

        @Override
        public DDS.DataReader get_datareader() {
            return null;
        }

        @Override
        public DDS.DataReaderView get_datareaderview() {
            return null;
        }

        /* condition operations */
        @Override
        public boolean get_trigger_value() {
            return false;
        }
    }

    private DDS._QueryConditionLocalBase base;

    public QueryConditionBase() {
        base = new QueryConditionBaseImpl();
    }

    @Override
    public String[] _ids() {
        return base._ids();
    }

}
