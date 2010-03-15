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

public final class ResourceLimitsQosPolicy {

    public int max_samples;
    public int max_instances;
    public int max_samples_per_instance;

    public ResourceLimitsQosPolicy() {
    }

    public ResourceLimitsQosPolicy(
        int _max_samples,
        int _max_instances,
        int _max_samples_per_instance)
    {
        max_samples = _max_samples;
        max_instances = _max_instances;
        max_samples_per_instance = _max_samples_per_instance;
    }

}
