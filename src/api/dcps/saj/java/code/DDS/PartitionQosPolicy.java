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


public final class PartitionQosPolicy 
{
  public String name[] = null;

  public PartitionQosPolicy ()
  {
  } // ctor

  public PartitionQosPolicy (String[] _name)
  {
    name = _name;
  } // ctor

} // class PartitionQosPolicy
