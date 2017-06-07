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

public abstract class DataWriterBase extends EntityImpl {

    private static final long serialVersionUID = 452622993551555456L;

    @Override
    protected int deinit() {
        return super.deinit();
    }

    public class DataWriterBaseImpl extends DDS._DataWriterLocalBase {
        private static final long serialVersionUID = 5816716490542465343L;

        /* datawriter operations */
        @Override
        public int set_qos(DDS.DataWriterQos qos) {
            return 0;
        }

        @Override
        public int get_qos(DDS.DataWriterQosHolder qos) {
            return 0;
        }

        @Override
        public int set_listener(DDS.DataWriterListener a_listener, int mask) {
            return 0;
        }

        @Override
        public DDS.DataWriterListener get_listener() {
            return null;
        }

        @Override
        public DDS.Topic get_topic() {
            return null;
        }

        @Override
        public DDS.Publisher get_publisher() {
            return null;
        }

        @Override
        public int wait_for_acknowledgments(DDS.Duration_t max_wait) {
            return 0;
        }

        @Override
        public int get_liveliness_lost_status(
                DDS.LivelinessLostStatusHolder status) {
            return 0;
        }

        @Override
        public int get_offered_deadline_missed_status(
                DDS.OfferedDeadlineMissedStatusHolder status) {
            return 0;
        }

        @Override
        public int get_offered_incompatible_qos_status(
                DDS.OfferedIncompatibleQosStatusHolder status) {
            return 0;
        }

        @Override
        public int get_publication_matched_status(
                DDS.PublicationMatchedStatusHolder status) {
            return 0;
        }

        @Override
        public int assert_liveliness() {
            return 0;
        }

        @Override
        public int get_matched_subscriptions(
                DDS.InstanceHandleSeqHolder subscription_handles) {
            return 0;
        }

        @Override
        public int get_matched_subscription_data(
                DDS.SubscriptionBuiltinTopicDataHolder subscription_data,
                long subscription_handle) {
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

    private DDS._DataWriterLocalBase base;

    public DataWriterBase() {
        base = new DataWriterBaseImpl();
    }

    @Override
    public String[] _ids() {
        return base._ids();
    }

}
