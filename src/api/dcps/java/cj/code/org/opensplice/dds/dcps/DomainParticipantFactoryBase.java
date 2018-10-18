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

public class DomainParticipantFactoryBase extends ObjectImpl {

    @Override
    protected int deinit() {
        return super.deinit();
    }

    public static class DomainParticipantFactoryBaseImpl extends
            DDS._DomainParticipantFactoryInterfaceLocalBase {
        private static final long serialVersionUID = 1050867525625383446L;

        /* domainparticipantfactoryinterface operations */
        @Override
        public DDS.DomainParticipant create_participant(int domainId,
                DDS.DomainParticipantQos qos,
                DDS.DomainParticipantListener a_listener, int mask) {
            return null;
        }

        @Override
        public int delete_participant(DDS.DomainParticipant a_participant) {
            return 0;
        }

        @Override
        public DDS.DomainParticipant lookup_participant(int domainId) {
            return null;
        }

        @Override
        public int set_default_participant_qos(DDS.DomainParticipantQos qos) {
            return 0;
        }

        @Override
        public int get_default_participant_qos(
                DDS.DomainParticipantQosHolder qos) {
            return 0;
        }

        @Override
        public int set_qos(DDS.DomainParticipantFactoryQos qos) {
            return 0;
        }

        @Override
        public int get_qos(DDS.DomainParticipantFactoryQosHolder qos) {
            return 0;
        }

        @Override
        public DDS.Domain lookup_domain(int domain_id) {
            return null;
        }

        @Override
        public int delete_domain(DDS.Domain a_domain) {
            return 0;
        }

        @Override
        public int delete_contained_entities() {
            return 0;
        }

        @Override
        public int detach_all_domains(boolean block_operations, boolean delete_entities) {
            return 0;
        }
    }

    private DDS._DomainParticipantFactoryInterfaceLocalBase base;

    public DomainParticipantFactoryBase() {
        base = new DomainParticipantFactoryBaseImpl();
    }

    @Override
    public String[] _ids() {
        return base._ids();
    }

}
