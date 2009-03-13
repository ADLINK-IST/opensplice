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
                System.loadLibrary("dcpssaj");
                theParticipantFactory = jniGetInstance();
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
	

    private native static DomainParticipantFactory jniGetInstance();
    private native DDS.DomainParticipant jniCreateParticipant(String domainId, DDS.DomainParticipantQos qos, DDS.DomainParticipantListener a_listener,int mask);
    private native int jniDeleteParticipant(DDS.DomainParticipant a_participant);
    private native DDS.DomainParticipant jniLookupParticipant(String domainId);
    private native int jniSetDefaultParticipantQos(DDS.DomainParticipantQos qos);
    private native int jniGetDefaultParticipantQos(DDS.DomainParticipantQosHolder qos);

    private native int jniSetQos(DDS.DomainParticipantFactoryQos qos);
    private native int jniGetQos(DDS.DomainParticipantFactoryQosHolder qos);
	
    
} // DomainParticipantFactory
