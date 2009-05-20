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


public final class LivelinessQosPolicy 
{
  public DDS.LivelinessQosPolicyKind kind = null;
  public DDS.Duration_t lease_duration = null;

  public LivelinessQosPolicy ()
  {
  } // ctor

  public LivelinessQosPolicy (DDS.LivelinessQosPolicyKind _kind, DDS.Duration_t _lease_duration)
  {
    kind = _kind;
    lease_duration = _lease_duration;
  } // ctor

} // class LivelinessQosPolicy
