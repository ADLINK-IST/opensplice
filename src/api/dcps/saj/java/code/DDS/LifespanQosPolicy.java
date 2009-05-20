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


public final class LifespanQosPolicy 
{
  public DDS.Duration_t duration = null;

  public LifespanQosPolicy ()
  {
  } // ctor

  public LifespanQosPolicy (DDS.Duration_t _duration)
  {
    duration = _duration;
  } // ctor

} // class LifespanQosPolicy
