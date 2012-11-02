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


public final class OfferedDeadlineMissedStatus 
{
  public int total_count = (int)0;
  public int total_count_change = (int)0;
  public long last_instance_handle = (long)0L;

  public OfferedDeadlineMissedStatus ()
  {
  } // ctor

  public OfferedDeadlineMissedStatus (int _total_count, int _total_count_change, long _last_instance_handle)
  {
    total_count = _total_count;
    total_count_change = _total_count_change;
    last_instance_handle = _last_instance_handle;
  } // ctor

} // class OfferedDeadlineMissedStatus
