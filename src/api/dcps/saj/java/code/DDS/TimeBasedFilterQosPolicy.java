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


public final class TimeBasedFilterQosPolicy 
{
  public DDS.Duration_t minimum_separation = null;

  public TimeBasedFilterQosPolicy ()
  {
  } // ctor

  public TimeBasedFilterQosPolicy (DDS.Duration_t _minimum_separation)
  {
    minimum_separation = _minimum_separation;
  } // ctor

} // class TimeBasedFilterQosPolicy
