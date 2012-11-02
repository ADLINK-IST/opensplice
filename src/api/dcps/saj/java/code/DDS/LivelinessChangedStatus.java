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


public final class LivelinessChangedStatus 
{
  public int alive_count = (int)0;
  public int not_alive_count = (int)0;
  public int alive_count_change = (int)0;
  public int not_alive_count_change = (int)0;
  public long last_publication_handle = (long)0L;

  public LivelinessChangedStatus ()
  {
  } // ctor

  public LivelinessChangedStatus (int _alive_count, int _not_alive_count, int _alive_count_change, int _not_alive_count_change, long _last_publication_handle)
  {
    alive_count = _alive_count;
    not_alive_count = _not_alive_count;
    alive_count_change = _alive_count_change;
    not_alive_count_change = _not_alive_count_change;
    last_publication_handle =_last_publication_handle;
  } // ctor

} // class LivelinessChangedStatus
