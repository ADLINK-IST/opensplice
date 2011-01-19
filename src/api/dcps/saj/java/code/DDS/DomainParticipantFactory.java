/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2010 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
package DDS;

/**
 * Allow the creation and destruction of {@link DomainParticipant} objects.
 * This class is implemented as a Singleton.
 */
public class DomainParticipantFactory
	extends org.opensplice.dds.dcps.SajSuperClass
	implements DomainParticipantFactoryOperations
{

	/**
     * The one and only instance of the DomainParticipantFactory.
     */
	private static DDS.DomainParticipantFactory theParticipantFactory = null;

	/**
	 * Private constructor to prevent the creation of new instances of the
	 * DomainParticipantFactory. Call the static method get_instance to get a
	 * reference.
	 */
    private DomainParticipantFactory() {
    }

    /**
     * Static method to get a reference to the one DomainParticipantFactory
     * instance.
     * @return a reference to the one and only instance of the
     * DomainParticipantFactory.
     */
    public static synchronized DDS.DomainParticipantFactory get_instance() {
    	if (theParticipantFactory == null) {
            try{
                OSPLShutdown sh;

                System.loadLibrary("dcpssaj");
                theParticipantFactory = jniGetInstance();
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
                System.err.println("DDS.DomainParticipantFactory.get_instance() failed: " + ule.getMessage());
            }

    	}
    	return theParticipantFactory;
    }

    public DDS.DomainParticipant create_participant (String domainId, DDS.DomainParticipantQos qos, DDS.DomainParticipantListener a_listener, int mask) {
        DDS.DomainParticipant dp = jniCreateParticipant(domainId, qos, a_listener,mask);

        if(dp != null){
            boolean success = false;
            int rc;

            ParticipantBuiltinTopicDataTypeSupport participantTypeSupport =
                                new ParticipantBuiltinTopicDataTypeSupport();
            rc = participantTypeSupport.register_type(dp, "DDS::ParticipantBuiltinTopicData");

            if(rc == RETCODE_OK.value){
                TopicBuiltinTopicDataTypeSupport topicTypeSupport =
                                        new TopicBuiltinTopicDataTypeSupport();
                rc = topicTypeSupport.register_type(dp, "DDS::TopicBuiltinTopicData");

                if(rc == RETCODE_OK.value){
                    PublicationBuiltinTopicDataTypeSupport publicationTypeSupport =
                                new PublicationBuiltinTopicDataTypeSupport();
                    rc = publicationTypeSupport.register_type(dp, "DDS::PublicationBuiltinTopicData");

                    if(rc == RETCODE_OK.value){
                        SubscriptionBuiltinTopicDataTypeSupport subscriptionTypeSupport =
                                new SubscriptionBuiltinTopicDataTypeSupport();
                        rc = subscriptionTypeSupport.register_type(dp, "DDS::SubscriptionBuiltinTopicData");

                        if(rc == RETCODE_OK.value){
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

    private native static DomainParticipantFactory jniGetInstance();
    private native DDS.DomainParticipant jniCreateParticipant(String domainId, DDS.DomainParticipantQos qos, DDS.DomainParticipantListener a_listener,int mask);
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

        status = DomainParticipantFactory.get_instance().delete_contained_entities();
        if (status != RETCODE_OK.value)
        {
            System.err.println("Error in DomainParticipantFactory.delete_contained_entities, status = " + status);
        }
    }
}
