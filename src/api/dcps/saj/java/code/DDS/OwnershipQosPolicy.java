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

public final class OwnershipQosPolicy {

    public DDS.OwnershipQosPolicyKind kind = DDS.OwnershipQosPolicyKind.from_int(0);

    public OwnershipQosPolicy() {
    }

    public OwnershipQosPolicy(
        DDS.OwnershipQosPolicyKind _kind)
    {
        kind = _kind;
    }

}
