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

public abstract class TopicBase extends EntityImpl {
    private static final long serialVersionUID = -6607431759357099911L;

    @Override
    protected int deinit() {
        return super.deinit();
    }

    public class TopicBaseImpl extends DDS._TopicLocalBase {
        private static final long serialVersionUID = -2209751554333737867L;

        /* topic operations */
        @Override
        public int get_inconsistent_topic_status(
                DDS.InconsistentTopicStatusHolder a_status) {
            return 0;
        }

        @Override
        public int get_all_data_disposed_topic_status(
                DDS.AllDataDisposedTopicStatusHolder status) {
            return 0;
        }

        @Override
        public int get_qos(DDS.TopicQosHolder qos) {
            return 0;
        }

        @Override
        public int set_qos(DDS.TopicQos qos) {
            return 0;
        }

        @Override
        public DDS.TopicListener get_listener() {
            return null;
        }

        @Override
        public int set_listener(DDS.TopicListener a_listener, int mask) {
            return 0;
        }

        @Override
        public int dispose_all_data() {
            return 0;
        }

        /* topicdescription operations */
        @Override
        public java.lang.String get_type_name() {
            return null;
        }

        @Override
        public java.lang.String get_name() {
            return null;
        }

        @Override
        public DDS.DomainParticipant get_participant() {
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

    private DDS._TopicLocalBase base;

    public TopicBase() {
        base = new TopicBaseImpl();
    }

    @Override
    public String[] _ids() {
        return base._ids();
    }

}
