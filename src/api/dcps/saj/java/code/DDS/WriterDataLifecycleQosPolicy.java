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


public final class WriterDataLifecycleQosPolicy 
{
  public boolean autodispose_unregistered_instances = false;

  public WriterDataLifecycleQosPolicy ()
  {
  } // ctor

  public WriterDataLifecycleQosPolicy (boolean _autodispose_unregistered_instances)
  {
    autodispose_unregistered_instances = _autodispose_unregistered_instances;
  } // ctor

} // class WriterDataLifecycleQosPolicy
