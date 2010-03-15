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

public final class DeadlineQosPolicy {

    public DDS.Duration_t period = new DDS.Duration_t();

    public DeadlineQosPolicy() {
    }

    public DeadlineQosPolicy(
        DDS.Duration_t _period)
    {
        period = _period;
    }

}
