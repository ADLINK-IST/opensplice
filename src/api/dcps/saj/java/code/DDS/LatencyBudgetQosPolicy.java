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

public final class LatencyBudgetQosPolicy {

    public DDS.Duration_t duration = new DDS.Duration_t();

    public LatencyBudgetQosPolicy() {
    }

    public LatencyBudgetQosPolicy(
        DDS.Duration_t _duration)
    {
        duration = _duration;
    }

}
