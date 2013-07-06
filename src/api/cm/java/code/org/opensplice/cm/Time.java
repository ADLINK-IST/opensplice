/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
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
    
    public static final Time infinite = new Time(2147483647,2147483647);
    public static final Time zero = new Time(0,0);
    
    /**
     * Constructs a new Time instance.
     *  
     *
     * @param _sec The seconds.
     * @param _nsec The nanoseconds.
     */
    public Time(int _sec, int _nsec){
        sec = _sec;
        nsec = _nsec;
    }
    
    /**
     * Creates a String representation of the Time.
     * 
     * @return The String representation of the Time.
     */
    @Override
    public String toString(){
        String nanos;
        
        
        if(nsec != infinite.nsec){
            nanos = Double.toString(nsec / 1000000000.0);
            
            if(nanos.indexOf('.') != -1){
                nanos = nanos.substring(2);
            }
        } else {
            nanos = Integer.toString(nsec);
        }
        return sec + "." + nanos;
    }
        
    public static Time fromString(String str){
        int sec, nsec, index;
        String nanos;
        
        index = str.indexOf('.');
        
        if(index != -1){
            sec = Integer.parseInt(str.substring(0, index));
            nanos = str.substring(index+1);
            nsec = Integer.parseInt(nanos);
            
            if(nsec != infinite.nsec){
                nanos = "0." + nanos;
                nsec = (int)(Double.parseDouble(nanos) * 1000000000.0);
                
                while(nsec >= 1000000000){
                    nsec -= 1000000000;
                    sec += 1;
                }
            }
        } else {
            sec = Integer.parseInt(str);
            nsec = 0;
        }
        return new Time(sec, nsec);
    }
    
    @Override
    public boolean equals(Object obj){
        if(obj instanceof Time){
            Time t = (Time)obj;
            
            if((t.nsec == this.nsec) && (t.sec == this.sec)){
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
        return new Time(this.sec, this.nsec);
    }
}
