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
