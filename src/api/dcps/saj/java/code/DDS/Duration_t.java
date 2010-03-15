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

public final class Duration_t {

    public int sec;
    public int nanosec;

    public Duration_t() {
    }

    public Duration_t(
        int _sec,
        int _nanosec)
    {
        sec = _sec;
        nanosec = _nanosec;
    }

}
