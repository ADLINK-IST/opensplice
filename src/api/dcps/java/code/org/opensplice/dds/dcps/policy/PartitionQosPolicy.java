package org.opensplice.dds.dcps.policy;

/**@brief A list of strings that introduce a logical partition among the topics visible by
 * a Publisher and Subscriber.
 *
 * -Concerns:    Publisher/Subscriber
 * -RxO:         No
 * -Changable:   Yes
 */
public class PartitionQosPolicy {
    private String name;
    
    public PartitionQosPolicy(String _name){
        name = _name;
    }
    
    /**
     * @return The partitions.
     */
    public String getName() {
        return name;
    }

    /**
     * @param string Set the partitions.
     */
    public void setName(String string) {
        name = string;
    }
}

