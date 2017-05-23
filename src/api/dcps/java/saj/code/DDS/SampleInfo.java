/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

package DDS;


public final class SampleInfo
{
  public int sample_state = 0;
  public int view_state = 0;
  public int instance_state = 0;
  public int disposed_generation_count = 0;
  public int no_writers_generation_count = 0;
  public int sample_rank = 0;
  public int generation_rank = 0;
  public int absolute_generation_rank = 0;
  public DDS.Time_t source_timestamp = null;
  public long instance_handle = 0L;
  public long publication_handle = 0L;
  public boolean valid_data = false;
  public DDS.Time_t reception_timestamp = null;

  public SampleInfo ()
  {
  } // ctor

  public SampleInfo (int _sample_state, int _view_state, int _instance_state, int _disposed_generation_count,
          int _no_writers_generation_count, int _sample_rank, int _generation_rank, int _absolute_generation_rank,
          DDS.Time_t _source_timestamp, long _instance_handle, long _publication_handle, boolean _valid_data,
          DDS.Time_t _reception_timestamp)
  {
    sample_state = _sample_state;
    view_state = _view_state;
    instance_state = _instance_state;
    disposed_generation_count = _disposed_generation_count;
    no_writers_generation_count = _no_writers_generation_count;
    sample_rank = _sample_rank;
    generation_rank = _generation_rank;
    absolute_generation_rank = _absolute_generation_rank;
    source_timestamp = _source_timestamp;
    instance_handle = _instance_handle;
    publication_handle = _publication_handle;
    valid_data = _valid_data;
    reception_timestamp = _reception_timestamp;
  } // ctor

} // class SampleInfo
