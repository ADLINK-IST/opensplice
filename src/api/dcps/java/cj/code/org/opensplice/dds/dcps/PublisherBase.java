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

public abstract class PublisherBase extends EntityImpl {
    private static final long serialVersionUID = 4011256343775172067L;

    @Override
    protected int deinit() {
        return super.deinit();
    }

    public static class PublisherBaseImpl extends DDS._PublisherLocalBase {
        private static final long serialVersionUID = 6695494379254094125L;

        /* publisher operations */
        @Override
        public DDS.DataWriter create_datawriter(DDS.Topic a_topic,
                DDS.DataWriterQos qos, DDS.DataWriterListener a_listener,
                int mask) {
            return null;
        }

        @Override
        public int delete_datawriter(DDS.DataWriter a_datawriter) {
            return 0;
        }

        @Override
        public DDS.DataWriter lookup_datawriter(java.lang.String topic_name) {
            return null;
        }

        @Override
        public int delete_contained_entities() {
            return 0;
        }

        @Override
        public int set_qos(DDS.PublisherQos qos) {
            return 0;
        }

        @Override
        public int get_qos(DDS.PublisherQosHolder qos) {
            return 0;
        }

        @Override
        public int set_listener(DDS.PublisherListener a_listener, int mask) {
            return 0;
        }

        @Override
        public DDS.PublisherListener get_listener() {
            return null;
        }

        @Override
        public int suspend_publications() {
            return 0;
        }

        @Override
        public int resume_publications() {
            return 0;
        }

        @Override
        public int begin_coherent_changes() {
            return 0;
        }

        @Override
        public int end_coherent_changes() {
            return 0;
        }

        @Override
        public int wait_for_acknowledgments(DDS.Duration_t max_wait) {
            return 0;
        }

        @Override
        public DDS.DomainParticipant get_participant() {
            return null;
        }

        @Override
        public int set_default_datawriter_qos(DDS.DataWriterQos qos) {
            return 0;
        }

        @Override
        public int get_default_datawriter_qos(DDS.DataWriterQosHolder qos) {
            return 0;
        }

        @Override
        public int copy_from_topic_qos(
                DDS.DataWriterQosHolder a_datawriter_qos,
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

    private DDS._PublisherLocalBase base;

    public PublisherBase() {
        base = new PublisherBaseImpl();
    }

    @Override
    public String[] _ids() {
        return base._ids();
    }

}
