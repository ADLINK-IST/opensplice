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

import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;

/**
 * Allow the creation and destruction of {@link DomainParticipant} objects.
 * This class is implemented as a Singleton.
 */
public class DomainParticipantFactoryImpl extends DomainParticipantFactoryBase
                                          implements DDS.DomainParticipantFactoryInterface
{

    private static final long serialVersionUID = 574757968947606395L;
    private static final DomainParticipantFactoryImpl TheFactory = new DomainParticipantFactoryImpl();
    private DDS.DomainParticipantFactoryQos factoryQos = Utilities.defaultDomainParticipantFactoryQos;
    private DDS.DomainParticipantQos defaultParticipantQos = Utilities.defaultDomainParticipantQos;
    private Set<DomainParticipantImpl> domainParticipants = new HashSet<DomainParticipantImpl>();
    private Set<DDS.Domain> domains = new HashSet<DDS.Domain>();

    @Override
    protected int deinit () { return super.deinit(); }

    /**
     * Private constructor to prevent the creation of new instances of the
     * DomainParticipantFactory. Call the static method get_instance to get a
     * reference.
     */
    private DomainParticipantFactoryImpl() {
        try{
            OSPLShutdown sh;

            System.loadLibrary("dcpssaj");
            int result = jniUserInitialize();

            if(result == DDS.RETCODE_OK.value){
                /* ES: dds2025: 04/13/2010: Install a shutdown hook to ensure
                 * all entities are cleaned up when the JVM terminates as the
                 * normal exit handlers are executed at a point where the JVM
                 * has already terminated and JNI callbacks to detach threads
                 * (managed by several entities) will fail with a lock up.
                 */
                String DisableShutdownHook = System.getProperty("osplNoDcpsShutdownHook", "false");
                if (DisableShutdownHook.equals("")) {
                    /* If osplNoDcpsShutdownHook is not set to a value, which implies true */
                    DisableShutdownHook = "true";
                }
                if (!Boolean.parseBoolean(DisableShutdownHook)) {
                    sh = new OSPLShutdown();
                    Runtime.getRuntime().addShutdownHook(sh);
                }
            } else {
                /* TODO: raise an exception */
            }
        } catch(UnsatisfiedLinkError ule){
            System.err.println("Library \"dcpssaj\" could not be loaded: " + ule.getMessage());
            /* TODO: raise an exception */
        }
    }

    public static DomainParticipantFactoryImpl get_instance() {
        return TheFactory;
    }

    @Override
    public synchronized DDS.DomainParticipant create_participant (
        int domainId,
        DDS.DomainParticipantQos qos,
        DDS.DomainParticipantListener a_listener,
        int mask)
    {
        ReportStack.start();
        DomainParticipantImpl dp = null;
        DDS.ParticipantBuiltinTopicDataTypeSupport participantTypeSupport = null;
        DDS.TopicBuiltinTopicDataTypeSupport topicTypeSupport = null;
        DDS.PublicationBuiltinTopicDataTypeSupport publicationTypeSupport = null;
        DDS.SubscriptionBuiltinTopicDataTypeSupport subscriptionTypeSupport = null;
        DDS.CMParticipantBuiltinTopicDataTypeSupport cmParticipantTypeSupport = null;
        DDS.CMPublisherBuiltinTopicDataTypeSupport cmPublisherTypeSupport = null;
        DDS.CMSubscriberBuiltinTopicDataTypeSupport cmSubscriberTypeSupport = null;
        DDS.CMDataWriterBuiltinTopicDataTypeSupport cmDataWriterTypeSupport = null;
        DDS.CMDataReaderBuiltinTopicDataTypeSupport cmDataReaderTypeSupport = null;
        DDS.TypeBuiltinTopicDataTypeSupport typeTypeSupport = null;
        DDS.MainClassName name = new DDS.MainClassName();
        int result;

        if (domainId != DDS.DOMAIN_ID_INVALID.value) {
            if (qos == DDS.PARTICIPANT_QOS_DEFAULT.value) {
                qos = this.defaultParticipantQos;
            }
            dp = new DomainParticipantImpl();
            result = dp.init(name.getMainClassName(), domainId, qos);
            if (result == DDS.RETCODE_OK.value) {
                this.domainParticipants.add(dp);
                participantTypeSupport = new DDS.ParticipantBuiltinTopicDataTypeSupport();
                result = participantTypeSupport.register_type(dp, "DDS::ParticipantBuiltinTopicData");
            } else {
                dp = null;
            }
            if (result == DDS.RETCODE_OK.value) {
                dp.setParticipantDataCopyCache(participantTypeSupport.get_copyCache());
                topicTypeSupport = new DDS.TopicBuiltinTopicDataTypeSupport();
                result = topicTypeSupport.register_type(dp, "DDS::TopicBuiltinTopicData");
            }
            if (result == DDS.RETCODE_OK.value) {
                dp.setTopicBuiltinTopicDataCopyCache(topicTypeSupport.get_copyCache());
                publicationTypeSupport = new DDS.PublicationBuiltinTopicDataTypeSupport();
                result = publicationTypeSupport.register_type(dp, "DDS::PublicationBuiltinTopicData");
            }
            if (result == DDS.RETCODE_OK.value) {
                subscriptionTypeSupport = new DDS.SubscriptionBuiltinTopicDataTypeSupport();
                result = subscriptionTypeSupport.register_type(dp, "DDS::SubscriptionBuiltinTopicData");
            }
            if (result == DDS.RETCODE_OK.value) {
                cmParticipantTypeSupport = new DDS.CMParticipantBuiltinTopicDataTypeSupport();
                result = cmParticipantTypeSupport.register_type(dp, "DDS::CMParticipantBuiltinTopicData");
            }
            if (result == DDS.RETCODE_OK.value) {
                cmPublisherTypeSupport = new DDS.CMPublisherBuiltinTopicDataTypeSupport();
                result = cmPublisherTypeSupport.register_type(dp, "DDS::CMPublisherBuiltinTopicData");
            }
            if (result == DDS.RETCODE_OK.value) {
                cmSubscriberTypeSupport = new DDS.CMSubscriberBuiltinTopicDataTypeSupport();
                result = cmSubscriberTypeSupport.register_type(dp, "DDS::CMSubscriberBuiltinTopicData");
            }
            if (result == DDS.RETCODE_OK.value) {
                cmDataWriterTypeSupport = new DDS.CMDataWriterBuiltinTopicDataTypeSupport();
                result = cmDataWriterTypeSupport.register_type(dp, "DDS::CMDataWriterBuiltinTopicData");
            }
            if (result == DDS.RETCODE_OK.value) {
                cmDataReaderTypeSupport = new DDS.CMDataReaderBuiltinTopicDataTypeSupport();
                result = cmDataReaderTypeSupport.register_type(dp, "DDS::CMDataReaderBuiltinTopicData");
            }
            if (result == DDS.RETCODE_OK.value) {
                typeTypeSupport = new DDS.TypeBuiltinTopicDataTypeSupport();
                result = typeTypeSupport.register_type(dp, "DDS::TypeBuiltinTopicData");
            }
            if (result == DDS.RETCODE_OK.value) {
                result = dp.set_listener(a_listener, mask);
                if (this.factoryQos.entity_factory.autoenable_created_entities) {
                    result = dp.enable();
                }
            }
        } else {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result,
                    "DomainParticipant is using an invalid domain identifier (" + domainId + ").");
        }
        if (result != DDS.RETCODE_OK.value && dp != null) {
            this.delete_participant(dp);
            dp = null;
        }

        ReportStack.flush(result != DDS.RETCODE_OK.value);
        return dp;
    }

    @Override
    public synchronized int delete_participant (DDS.DomainParticipant a_participant)
    {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (a_participant == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "a_participant 'null' is invalid.");
        } else {
            DomainParticipantImpl dp = (DomainParticipantImpl)a_participant;
            if (this.domainParticipants.remove(dp)) {
                // TODO [EH]: Somewhere in here the list of Domains needs to be updated.....
                result = dp.deinit();
                if (result == DDS.RETCODE_PRECONDITION_NOT_MET.value) {
                    this.domainParticipants.add(dp);
                }
            } else {
                result = DDS.RETCODE_BAD_PARAMETER.value;
                ReportStack.report(
                    result, "DomainParticipant not created by DomainParticipantFactory.");
            }
        }

        ReportStack.flush(result != DDS.RETCODE_OK.value);
        return result;
    }

    @Override
    public synchronized DDS.DomainParticipant lookup_participant (int domainId)
    {
        ReportStack.start();
        DDS.DomainParticipant found = null;

        Iterator<DomainParticipantImpl> i = this.domainParticipants.iterator();
        if (domainId == DDS.DOMAIN_ID_DEFAULT.value) {
            domainId = jniGetDomainIdFromEnvUri();
        }
        while ((found == null) && (i.hasNext())) {
            found = i.next();
            if (found.get_domain_id() != domainId) {
                found = null;
            }
        }

        ReportStack.flush(false); // Not finding a participant is no error. Error conditions may be added in the future.
        return found;
    }

    @Override
    public int set_default_participant_qos (DDS.DomainParticipantQos qos)
    {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        result = Utilities.checkQos(qos);
        if (result == DDS.RETCODE_OK.value) {
            if (qos == DDS.PARTICIPANT_QOS_DEFAULT.value) {
                qos = Utilities.defaultDomainParticipantQos;
            }
            this.defaultParticipantQos = Utilities.deepCopy(qos);
        }

        ReportStack.flush(result != DDS.RETCODE_OK.value);
        return result;
    }

    @Override
    public int get_default_participant_qos (DDS.DomainParticipantQosHolder qos)
    {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (qos == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "qos 'null' is invalid.");
        } else {
            qos.value = Utilities.deepCopy(this.defaultParticipantQos);
        }

        ReportStack.flush(result != DDS.RETCODE_OK.value);
        return result;
    }

    @Override
    public int set_qos (DDS.DomainParticipantFactoryQos qos)
    {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        result = Utilities.checkQos(qos);
        if (result == DDS.RETCODE_OK.value) {
            if (qos == DDS.PARTICIPANTFACTORY_QOS_DEFAULT.value) {
                qos = Utilities.defaultDomainParticipantFactoryQos;
            }
            this.factoryQos = Utilities.deepCopy(qos);
        }

        ReportStack.flush(result != DDS.RETCODE_OK.value);
        return result;
    }

    @Override
    public int get_qos (DDS.DomainParticipantFactoryQosHolder qos)
    {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (qos == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "qos 'null' is invalid.");
        } else {
            qos.value = Utilities.deepCopy(this.factoryQos);
        }

        ReportStack.flush(result != DDS.RETCODE_OK.value);
        return DDS.RETCODE_OK.value;
    }

    @Override
    public synchronized DDS.Domain lookup_domain (int domain_id)
    {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        DomainImpl found = null;
        Iterator<DDS.Domain> i = this.domains.iterator();
        while ((found == null) && (i.hasNext())) {
            found = (DomainImpl)i.next();
            if (found.get_domain_id() != domain_id) {
                found = null;
            }
        }
        if (found == null) {
            found = new DomainImpl();
            result = found.init(domain_id);
            if (result == DDS.RETCODE_OK.value) {
                this.domains.add(found);
            } else {
                found = null;
            }
        }

        ReportStack.flush(found == null);
        return found;
    }

    @Override
    public synchronized int delete_domain (DDS.Domain a_domain)
    {
        int result = DDS.RETCODE_OK.value;
        ReportStack.start();

        if (a_domain == null) {
            result = DDS.RETCODE_BAD_PARAMETER.value;
            ReportStack.report(result, "a_domain 'null' is invalid.");
        } else {
            if (this.domains.remove(a_domain)) {
                result = ((DomainImpl)a_domain).deinit();
                if (result == DDS.RETCODE_PRECONDITION_NOT_MET.value) {
                    this.domains.add(a_domain);
                }
            } else {
                result = DDS.RETCODE_BAD_PARAMETER.value;
                ReportStack.report(
                    result, "Domain not registered to DomainParticipantFactory.");
            }
        }

        ReportStack.flush(result != DDS.RETCODE_OK.value);
        return result;
    }

    @Override
    public synchronized int delete_contained_entities ()
    {
        ReportStack.start();
        int result, endResult = DDS.RETCODE_OK.value;

        Iterator<DomainParticipantImpl> ip = this.domainParticipants.iterator();
        while (ip.hasNext()) {
            DomainParticipantImpl dp = ip.next();
            result = dp.delete_contained_entities();
            if (result == DDS.RETCODE_OK.value) {
                result = dp.deinit();
                if (result == DDS.RETCODE_OK.value || result == DDS.RETCODE_ALREADY_DELETED.value) {
                    /* Use iterator.remove() to remove from the set */
                    ip.remove();
                }
                if (result != DDS.RETCODE_OK.value) {
                    ReportStack.report (result, "Deletion of DomainParticipant contained in DomainParticipantFactory failed.");
                }
            } else {
                if (result == DDS.RETCODE_ALREADY_DELETED.value) {
                    /* Use iterator.remove() to remove from the set */
                    ip.remove();
                }
                ReportStack.report (result, "delete_contained_entities failed on DomainParticipant contained in DomainParticipantFactory.");
            }
            if (endResult == DDS.RETCODE_OK.value ) {
                /* Store first encountered error. */
                endResult = result;
            }
        }

        ReportStack.flush(endResult != DDS.RETCODE_OK.value);
        return endResult;
    }

    @Override
    public int detach_all_domains(boolean block_operations, boolean delete_entities)
    {
        return jniUserDetach(block_operations, delete_entities);
    }

    private native int jniUserInitialize();
    private native String jniGetProcessName();
    private native int jniGetDomainIdFromEnvUri();
    protected static native int jniUserDetach(boolean blockOperations, boolean deleteEntities);

} // DomainParticipantFactory

class OSPLShutdown extends Thread
{
    public OSPLShutdown()
    {
    }

    @Override
    public void run()
    {
        DomainParticipantFactoryImpl.jniUserDetach(false, true);
    }
}
