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

public final class DurabilityServiceQosPolicy {

    public DDS.Duration_t service_cleanup_delay = new DDS.Duration_t();
    public DDS.HistoryQosPolicyKind history_kind = DDS.HistoryQosPolicyKind.from_int(0);
    public int history_depth;
    public int max_samples;
    public int max_instances;
    public int max_samples_per_instance;

    public DurabilityServiceQosPolicy() {
    }

    public DurabilityServiceQosPolicy(
        DDS.Duration_t _service_cleanup_delay,
        DDS.HistoryQosPolicyKind _history_kind,
        int _history_depth,
        int _max_samples,
        int _max_instances,
        int _max_samples_per_instance)
    {
        service_cleanup_delay = _service_cleanup_delay;
        history_kind = _history_kind;
        history_depth = _history_depth;
        max_samples = _max_samples;
        max_instances = _max_instances;
        max_samples_per_instance = _max_samples_per_instance;
    }

}
