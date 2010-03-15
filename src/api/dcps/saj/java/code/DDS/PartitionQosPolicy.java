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

public final class PartitionQosPolicy {

    public java.lang.String[] name = new java.lang.String[0];

    public PartitionQosPolicy() {
    }

    public PartitionQosPolicy(
        java.lang.String[] _name)
    {
        name = _name;
    }

}
