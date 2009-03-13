package org.opensplice.dds.dcps.policy;

/**@brief The Qos policies for a Publisher.
 */
public class PublisherQos extends QosPolicy {
    private PresentationQosPolicy presentation;
    private PartitionQosPolicy partition;
    
    public PublisherQos(PresentationQosPolicy _presentation,
                        PartitionQosPolicy _partition){
        presentation = _presentation;
        partition = _partition;                        
    }
    
    public static QosPolicy getDefault(){
        PresentationQosPolicy presentation = new PresentationQosPolicy(
                    PresentationQosAccessScopeKind.INSTANCE, true, true);
        PartitionQosPolicy partition = new PartitionQosPolicy("");
        
        return new PublisherQos(presentation, partition); 
    }
    
    /**@brief Returns the attached Partition.
     * 
     * @return The attached Partition.
     */
    public PartitionQosPolicy getPartition() {
        return partition;
    }

    /**@brief Returns the attached presentation.
     * 
     * @return The attached presentation.
     */
    public PresentationQosPolicy getPresentation() {
        return presentation;
    }

    /**@brief Changes the partition policy.
     * 
     * @param policy The policy to set.
     */
    public void setPartition(PartitionQosPolicy policy) {
        partition = policy;
    }

    /**@brief Changes the presentation.
     * 
     * @param policy The policy to set.
     */
    public void setPresentation(PresentationQosPolicy policy) {
        presentation = policy;
    }
}

