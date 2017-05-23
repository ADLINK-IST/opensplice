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

public abstract class SubscriberBase extends EntityImpl {
    private static final long serialVersionUID = 5481341365093555559L;

    @Override
    protected int deinit() {
        return super.deinit();
    }

    public class SubscriberBaseImpl extends DDS._SubscriberLocalBase {
        private static final long serialVersionUID = 3494757228723920438L;

        /* subscriber operations */
        @Override
        public DDS.DataReader create_datareader(DDS.TopicDescription a_topic,
                DDS.DataReaderQos qos, DDS.DataReaderListener a_listener,
                int mask) {
            return null;
        }

        @Override
        public int delete_datareader(DDS.DataReader a_datareader) {
            return 0;
        }

        @Override
        public int delete_contained_entities() {
            return 0;
        }

        @Override
        public DDS.DataReader lookup_datareader(java.lang.String topic_name) {
            return null;
        }

        @Override
        public int get_datareaders(DDS.DataReaderSeqHolder readers,
                int sample_states, int view_states, int instance_states) {
            return 0;
        }

        @Override
        public int notify_datareaders() {
            return 0;
        }

        @Override
        public int set_qos(DDS.SubscriberQos qos) {
            return 0;
        }

        @Override
        public int get_qos(DDS.SubscriberQosHolder qos) {
            return 0;
        }

        @Override
        public int set_listener(DDS.SubscriberListener a_listener, int mask) {
            return 0;
        }

        @Override
        public DDS.SubscriberListener get_listener() {
            return null;
        }

        @Override
        public int begin_access() {
            return 0;
        }

        @Override
        public int end_access() {
            return 0;
        }

        @Override
        public DDS.DomainParticipant get_participant() {
            return null;
        }

        @Override
        public int set_default_datareader_qos(DDS.DataReaderQos qos) {
            return 0;
        }

        @Override
        public int get_default_datareader_qos(DDS.DataReaderQosHolder qos) {
            return 0;
        }

        @Override
        public int copy_from_topic_qos(
                DDS.DataReaderQosHolder a_datareader_qos,
                DDS.TopicQos a_topic_qos) {
            return 0;
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

    private DDS._SubscriberLocalBase base;

    public SubscriberBase() {
        base = new SubscriberBaseImpl();
    }

    @Override
    public String[] _ids() {
        return base._ids();
    }

}
