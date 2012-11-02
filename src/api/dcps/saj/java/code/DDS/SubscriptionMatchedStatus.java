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


public final class SubscriptionMatchedStatus 
{
  public int total_count = (int)0;
  public int total_count_change = (int)0;
  public int current_count = (int)0;
  public int current_count_change = (int)0;
  public long last_publication_handle = (long)0L;

  public SubscriptionMatchedStatus ()
  {
  } // ctor

  public SubscriptionMatchedStatus (int _total_count, int _total_count_change,int _current_count, int _current_count_change, long _last_publication_handle)
  {
    total_count = _total_count;
    total_count_change = _total_count_change;
    current_count = _current_count;
    current_count_change = _current_count_change;
    last_publication_handle = _last_publication_handle;
  } // ctor

} // class SubscriptionMatchStatus
