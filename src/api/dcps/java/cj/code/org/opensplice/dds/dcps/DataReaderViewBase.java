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

public abstract class DataReaderViewBase extends EntityImpl {

    private static final long serialVersionUID = -233503129020203452L;

    @Override
    protected int deinit() {
        return super.deinit();
    }

    public static class DataReaderViewBaseImpl extends DDS._DataReaderViewLocalBase {
        private static final long serialVersionUID = 3443210233114376160L;

        /* datareaderview operations */
        @Override
        public DDS.ReadCondition create_readcondition(int sample_states,
                int view_states, int instance_states) {
            return null;
        }

        @Override
        public DDS.QueryCondition create_querycondition(int sample_states,
                int view_states, int instance_states,
                java.lang.String query_expression,
                java.lang.String[] query_parameters) {
            return null;
        }

        @Override
        public int delete_readcondition(DDS.ReadCondition a_condition) {
            return 0;
        }

        @Override
        public int delete_contained_entities() {
            return 0;
        }

        @Override
        public int set_qos(DDS.DataReaderViewQos qos) {
            return 0;
        }

        @Override
        public int get_qos(DDS.DataReaderViewQosHolder qos) {
            return 0;
        }

        @Override
        public DDS.DataReader get_datareader() {
            return null;
        }

        /* entity operations */
        @Override
        public int enable() {
            return 0;
        }

        @Override
        public DDS.StatusCondition get_statuscondition() {
            return null;
        }

        @Override
        public int get_status_changes() {
            return 0;
        }

        @Override
        public long get_instance_handle() {
            return 0;
        }

        /* DDS.PropertyInterfaceOperations */
        @Override
        public int set_property(DDS.Property a_property) {
            return 0;
        }

        @Override
        public int get_property(DDS.PropertyHolder a_property) {
            return 0;
        }
    }

    private DDS._DataReaderViewLocalBase base;

    public DataReaderViewBase() {
        base = new DataReaderViewBaseImpl();
    }

    @Override
    public String[] _ids() {
        return base._ids();
    }

}
