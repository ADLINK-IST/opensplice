/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

package DDS;


public final class WriterDataLifecycleQosPolicy 
{
  public boolean autodispose_unregistered_instances = false;
  public Duration_t autopurge_suspended_samples_delay;
  public Duration_t autounregister_instance_delay;

  public WriterDataLifecycleQosPolicy ()
  {
  } // ctor

  public WriterDataLifecycleQosPolicy (boolean _autodispose_unregistered_instances, Duration_t _autopurge_suspended_samples_delay, Duration_t _autounregister_instance_delay)
  {
    autodispose_unregistered_instances = _autodispose_unregistered_instances;
    autopurge_suspended_samples_delay = _autopurge_suspended_samples_delay;
    autounregister_instance_delay = _autounregister_instance_delay;
  } // ctor

} // class WriterDataLifecycleQosPolicy
