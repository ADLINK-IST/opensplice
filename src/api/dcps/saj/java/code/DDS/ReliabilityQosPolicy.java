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


public final class ReliabilityQosPolicy 
{
  public DDS.ReliabilityQosPolicyKind kind = null;
  public DDS.Duration_t max_blocking_time = null;
  public boolean synchronous = false;

  public ReliabilityQosPolicy ()
  {
  } // ctor

  public ReliabilityQosPolicy (DDS.ReliabilityQosPolicyKind _kind, DDS.Duration_t _max_blocking_time, boolean _synchronous)
  {
    kind = _kind;
    max_blocking_time = _max_blocking_time;
    synchronous = _synchronous;
  } // ctor

} // class ReliabilityQosPolicy
