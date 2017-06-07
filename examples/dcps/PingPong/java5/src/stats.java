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

class stats {

    public String name;
    public long average;
    public long min;
    public long max;
    public int count;

    public stats (
        String stats_name
        )
    {
	name = stats_name;
        this.init_stats ();
    }

    public void
    add_stats (
        long data
        )
    {
        average = (count * average + data)/(count + 1);
        min     = (count == 0 || data < min) ? data : min;
        max     = (count == 0 || data > max) ? data : max;
        count++;
    }

    public void
    init_stats (
	)
    {
        count   = 0;
        average = 0L;
        min     = 0L;
        max     = 0L; 
    }

}
