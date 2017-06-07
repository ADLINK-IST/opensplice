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
package org.opensplice.cm;

/**
 * Represents an instance of time.
 *
 * @date Jan 10, 2005
 */
public class Time {
    /**
     * The seconds.
     */
    public int sec;

    /**
     * The nanoseconds.
     */
    public int nsec;

    /**
     * The seconds in 64 bit
     */
    private long sec64;

    public static final Time infinite = new Time(Integer.MAX_VALUE, Integer.MAX_VALUE);
    public static final Time zero = new Time(0, 0);

    public static final long NANOSECONDS = 1000000000L;

    /**
     * Constructs a new Time instance.
     *
     *
     * @param _sec The seconds.
     * @param _nsec The nanoseconds.
     */
    public Time(long _sec, int _nsec){
        sec64 = _sec;
        sec = Long.valueOf(sec64).intValue();
        nsec = _nsec;
    }

    /**
     * Constructs a new Time instance.
     *
     *
     * @param _nsec The time in nanoseconds.
     */
    public Time(long _value){
        if (_value < Long.MAX_VALUE) {
            sec64 = _value / NANOSECONDS;
            sec = Long.valueOf(sec64).intValue();
            nsec = Long.valueOf(_value % NANOSECONDS).intValue();
            if (nsec == Integer.MAX_VALUE) {
                sec = Integer.MAX_VALUE;
                sec64 = Integer.MAX_VALUE;
            }
        } else {
            sec64 = Integer.MAX_VALUE;
            sec = Integer.MAX_VALUE;
            nsec = Integer.MAX_VALUE;
        }
    }

    /**
     * Creates a String representation of the Time.
     *
     * @return The String representation of the Time.
     */
    @Override
    public String toString(){
        String nanos;

        if (nsec != infinite.nsec) {
            nanos = Double.toString(nsec / 1000000000.0);

            if(nanos.indexOf('.') != -1){
                nanos = nanos.substring(2);
            }
        } else {
            nanos = Integer.toString(nsec);
        }

        return "" + sec64 + "." + nanos;
    }

    public static Time fromString(String str){
        long value;

        int index = str.indexOf('.');

        if(index != -1){
            long sec = Long.parseLong(str.substring(0, index));
            String nanos = str.substring(index+1);
            int nsec = Integer.parseInt(nanos);

            if(nsec != Integer.MAX_VALUE){
                nanos = "0." + nanos;
                nsec = (int)(Double.parseDouble(nanos) * 1000000000.0);

                while(nsec >= 1000000000){
                    nsec -= 1000000000;
                    sec += 1;
                }
                value = sec * NANOSECONDS + nsec;
            } else {
               value = Long.MAX_VALUE;
            }
        } else {
            value = Long.parseLong(str) * NANOSECONDS;
        }

        return new Time(value);
    }

    @Override
    public boolean equals(Object obj){
        if(obj instanceof Time){
            Time t = (Time)obj;

            if((t.nsec == this.nsec) && (t.sec64 == this.sec64)){
                return true;
            }
        }
        return false;
    }

    @Override
    public int hashCode() {
        return 0;
    }

    public Time copy(){
        return new Time(this.getValue());
    }

    public long getSeconds() {
        return sec64;
    }

    public int getNanoSeconds() {
        return nsec;
    }

    public long getValue() {
        long value;
        if (nsec != infinite.nsec) {
            value = sec64 * NANOSECONDS + nsec;
        } else {
            value = Long.MAX_VALUE;
        }
        return value;
    }

}
