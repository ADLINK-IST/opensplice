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

public final class ReliabilityQosPolicy {

    public DDS.ReliabilityQosPolicyKind kind = DDS.ReliabilityQosPolicyKind.from_int(0);
    public DDS.Duration_t max_blocking_time = new DDS.Duration_t();
    public boolean synchronous;

    public ReliabilityQosPolicy() {
    }

    public ReliabilityQosPolicy(
        DDS.ReliabilityQosPolicyKind _kind,
        DDS.Duration_t _max_blocking_time,
        boolean _synchronous)
    {
        kind = _kind;
        max_blocking_time = _max_blocking_time;
        synchronous = _synchronous;
    }

}
