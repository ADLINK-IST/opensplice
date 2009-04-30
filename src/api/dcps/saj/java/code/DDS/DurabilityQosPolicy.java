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


public final class DurabilityQosPolicy 
{
  public DDS.DurabilityQosPolicyKind kind = null;

  public DurabilityQosPolicy ()
  {
  } // ctor

  public DurabilityQosPolicy (DDS.DurabilityQosPolicyKind _kind)
  {
    kind = _kind;
  } // ctor

} // class DurabilityQosPolicy
