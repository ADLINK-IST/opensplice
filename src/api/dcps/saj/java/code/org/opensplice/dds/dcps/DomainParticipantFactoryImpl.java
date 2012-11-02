/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
package org.opensplice.dds.dcps;

/**
 * Allow the creation and destruction of {@link DomainParticipant} objects.
 * This class is implemented as a Singleton.
 */
public class DomainParticipantFactoryImpl extends SajSuperClass implements DDS.DomainParticipantFactoryOperations
{

	/**
	 * Private constructor to prevent the creation of new instances of the
	 * DomainParticipantFactory. Call the static method get_instance to get a
	 * reference.
	 */
    public DomainParticipantFactoryImpl() {}

    public static DomainParticipantFactoryImpl get_instance() {
    	DomainParticipantFactoryImpl theDomainParticipantFactory = null;

    	try{
            OSPLShutdown sh;

            System.loadLibrary("dcpssaj");
            theDomainParticipantFactory = jniGetInstance();
           
            /* ES: dds2025: 04/13/2010: Install a shutdown hook to ensure
             * all entities are cleaned up when the JVM terminates as the
             * normal exit handlers are executed at a point where the JVM
             * has already terminated and JNI callbacks to detach threads
             * (managed by several entities) will fail with a lock up.
             */
            sh = new OSPLShutdown();
            Runtime.getRuntime().addShutdownHook(sh);
        } catch(UnsatisfiedLinkError ule){
            /*Library could not be loaded.*/
            System.err.println("org.opensplice.dds.dcps.DomainParticipantFactoryImpl.get_instance() failed: " + ule.getMessage());
        }
    	return theDomainParticipantFactory;
    }


    public DDS.DomainParticipant create_participant (String domainId, DDS.DomainParticipantQos qos, DDS.DomainParticipantListener a_listener, int mask) {
    	org.opensplice.dds.dcps.DomainParticipantImpl dp = jniCreateParticipant(domainId, qos, a_listener,mask);

        if(dp != null){
            boolean success = false;
            int rc;

            DDS.ParticipantBuiltinTopicDataTypeSupport participantTypeSupport =
                                new DDS.ParticipantBuiltinTopicDataTypeSupport();
            rc = participantTypeSupport.register_type(dp, "DDS::ParticipantBuiltinTopicData");
            dp.setParticipantDataCopyCache(participantTypeSupport.get_copyCache());

            if(rc == DDS.RETCODE_OK.value){
            	DDS.TopicBuiltinTopicDataTypeSupport topicTypeSupport =
                                        new DDS.TopicBuiltinTopicDataTypeSupport();
                rc = topicTypeSupport.register_type(dp, "DDS::TopicBuiltinTopicData");
                dp.setTopicBuiltinTopicDataCopyCache(topicTypeSupport.get_copyCache());

                if(rc == DDS.RETCODE_OK.value){
                	DDS.PublicationBuiltinTopicDataTypeSupport publicationTypeSupport =
                                new DDS.PublicationBuiltinTopicDataTypeSupport();
                    rc = publicationTypeSupport.register_type(dp, "DDS::PublicationBuiltinTopicData");

                    if(rc == DDS.RETCODE_OK.value){
                    	DDS.SubscriptionBuiltinTopicDataTypeSupport subscriptionTypeSupport =
                                new DDS.SubscriptionBuiltinTopicDataTypeSupport();
                        rc = subscriptionTypeSupport.register_type(dp, "DDS::SubscriptionBuiltinTopicData");

                        if(rc == DDS.RETCODE_OK.value){
                            success = true;
                        }
                    }
                }
            }
            if(!success ){
                this.delete_participant(dp);
                dp = null;
            }
        }
        return dp;
    }

    public int delete_participant (DDS.DomainParticipant a_participant) {
        return jniDeleteParticipant(a_participant);
    }

    public DDS.DomainParticipant lookup_participant (String domainId) {
        return jniLookupParticipant(domainId);
    }

    public int set_default_participant_qos (DDS.DomainParticipantQos qos) {
        return jniSetDefaultParticipantQos(qos);
    }

    public int get_default_participant_qos (DDS.DomainParticipantQosHolder qos) {
        return jniGetDefaultParticipantQos(qos);
    }

    public int set_qos (DDS.DomainParticipantFactoryQos qos){
		return jniSetQos(qos);
    	}
    public int get_qos (DDS.DomainParticipantFactoryQosHolder qos){
		return jniGetQos(qos);
    	}

    public DDS.Domain lookup_domain (String domain_id) {
        return jniLookupDomain(domain_id);
    }

    public int delete_domain (DDS.Domain a_domain) {
        return jniDeleteDomain(a_domain);
    }

    public int delete_contained_entities () {
        return jniDeleteContainedEntities();
    }

    private native static DomainParticipantFactoryImpl jniGetInstance();
    private native DomainParticipantImpl jniCreateParticipant(String domainId, DDS.DomainParticipantQos qos, DDS.DomainParticipantListener a_listener,int mask);
    private native int jniDeleteParticipant(DDS.DomainParticipant a_participant);
    private native DDS.DomainParticipant jniLookupParticipant(String domainId);
    private native int jniSetDefaultParticipantQos(DDS.DomainParticipantQos qos);
    private native int jniGetDefaultParticipantQos(DDS.DomainParticipantQosHolder qos);

    private native int jniSetQos(DDS.DomainParticipantFactoryQos qos);
    private native int jniGetQos(DDS.DomainParticipantFactoryQosHolder qos);

    private native DDS.Domain jniLookupDomain(String domain_id);
    private native int jniDeleteDomain(DDS.Domain a_domain);
    private native int jniDeleteContainedEntities();
} // DomainParticipantFactory

class OSPLShutdown extends Thread
{

    public OSPLShutdown()
    {
    }

    public void run()
    {
        int status;

        status = DDS.DomainParticipantFactory.get_instance().delete_contained_entities();
        if (status != DDS.RETCODE_OK.value)
        {
            System.err.println("Error in DomainParticipantFactory.delete_contained_entities, status = " + status);
        }
    }
}
