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


public final class SampleInfo 
{
  public int sample_state = (int)0;
  public int view_state = (int)0;
  public int instance_state = (int)0;
  public boolean valid_data = false;
  public DDS.Time_t source_timestamp = null;
  public long instance_handle = (long)0L;
  public long publication_handle = (long)0L;
  public int disposed_generation_count = (int)0;
  public int no_writers_generation_count = (int)0;
  public int sample_rank = (int)0;
  public int generation_rank = (int)0;
  public int absolute_generation_rank = (int)0;

  public SampleInfo ()
  {
  } // ctor

  public SampleInfo (int _sample_state, int _view_state, int _instance_state, boolean _valid_data, DDS.Time_t _source_timestamp, long
          _instance_handle, long _publication_handle, int _disposed_generation_count, int _no_writers_generation_count, int _sample_rank, int _generation_rank, int _absolute_generation_rank)
  {
    sample_state = _sample_state;
    view_state = _view_state;
    instance_state = _instance_state;
    valid_data = _valid_data;
    source_timestamp = _source_timestamp;
    instance_handle = _instance_handle;
    publication_handle = _publication_handle;
    disposed_generation_count = _disposed_generation_count;
    no_writers_generation_count = _no_writers_generation_count;
    sample_rank = _sample_rank;
    generation_rank = _generation_rank;
    absolute_generation_rank = _absolute_generation_rank;
  } // ctor

} // class SampleInfo
