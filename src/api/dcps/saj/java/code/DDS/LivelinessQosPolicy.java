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

public final class LivelinessQosPolicy {

    public DDS.LivelinessQosPolicyKind kind = DDS.LivelinessQosPolicyKind.from_int(0);
    public DDS.Duration_t lease_duration = new DDS.Duration_t();

    public LivelinessQosPolicy() {
    }

    public LivelinessQosPolicy(
        DDS.LivelinessQosPolicyKind _kind,
        DDS.Duration_t _lease_duration)
    {
        kind = _kind;
        lease_duration = _lease_duration;
    }

}
