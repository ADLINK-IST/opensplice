/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */

package DDS;

public final class PresentationQosPolicy {

    public DDS.PresentationQosPolicyAccessScopeKind access_scope = DDS.PresentationQosPolicyAccessScopeKind.from_int(0);
    public boolean coherent_access;
    public boolean ordered_access;

    public PresentationQosPolicy() {
    }

    public PresentationQosPolicy(
        DDS.PresentationQosPolicyAccessScopeKind _access_scope,
        boolean _coherent_access,
        boolean _ordered_access)
    {
        access_scope = _access_scope;
        coherent_access = _coherent_access;
        ordered_access = _ordered_access;
    }

}
