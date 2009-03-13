package org.opensplice.dds.dcps.policy;

/**@brief Specifies the behaviour of the Service in the case where the value of a sample
 * changes.
 * 
 * - Concerns:    DataWriter/DataReader/Topic
 * - RxO:         NO
 * - Changable:   No
 */
public class HistoryQosPolicy {
    private HistoryQosKind kind;
    private long depth;
    
    /**@brief Creates new HistoryQosPolicy.
     * 
     * @param _kind The kind.
     * @param _depth The depth of the history.
     */
    public HistoryQosPolicy(HistoryQosKind _kind, long _depth){
        kind    = _kind;
        depth   = _depth;
    }
}

