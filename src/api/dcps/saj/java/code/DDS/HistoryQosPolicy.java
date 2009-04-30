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


public final class HistoryQosPolicy 
{
  public DDS.HistoryQosPolicyKind kind = null;
  public int depth = (int)0;

  public HistoryQosPolicy ()
  {
  } // ctor

  public HistoryQosPolicy (DDS.HistoryQosPolicyKind _kind, int _depth)
  {
    kind = _kind;
    depth = _depth;
  } // ctor

} // class HistoryQosPolicy
