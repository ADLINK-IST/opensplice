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


public final class TopicDataQosPolicy 
{
  public byte value[] = null;

  public TopicDataQosPolicy ()
  {
  } // ctor

  public TopicDataQosPolicy (byte[] _value)
  {
    value = _value;
  } // ctor

} // class TopicDataQosPolicy
