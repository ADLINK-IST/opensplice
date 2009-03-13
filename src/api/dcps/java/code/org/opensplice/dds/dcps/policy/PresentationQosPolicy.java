package org.opensplice.dds.dcps.policy;

/**@brief Specifies how the samples representation changes in dat isntances are presented to
 * the subscribing application.
 * 
 * - Concerns:    Publisher/Subscriber
 * - RxO:         Yes
 * - Changable:   No
 */
public class PresentationQosPolicy {
    private PresentationQosAccessScopeKind access_scope;
    private boolean cohorent_access;
    private boolean ordered_access;
    
    /**@brief Creates new PresentationQosPolicy
     * 
     * @param _access_scope Access scope of the data.
     * @param _cohorent_access Is cohorent access supported.
     * @param _ordered_access Is ordered access supported.
     */
    public PresentationQosPolicy(PresentationQosAccessScopeKind _access_scope,
                                    boolean _cohorent_access,
                                    boolean _ordered_access){
        access_scope    = _access_scope;
        cohorent_access = _cohorent_access;
        ordered_access  = _ordered_access;                                    
    }
}

