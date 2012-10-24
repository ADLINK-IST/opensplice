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


public final class QosPolicyCount 
{
  public int policy_id = (int)0;
  public int count = (int)0;

  public QosPolicyCount ()
  {
  } // ctor

  public QosPolicyCount (int _policy_id, int _count)
  {
    policy_id = _policy_id;
    count = _count;
  } // ctor

} // class QosPolicyCount
