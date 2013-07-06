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

package org.opensplice.cm.statistics;

import java.util.LinkedHashMap;

import org.opensplice.cm.Entity;
import org.opensplice.cm.Time;

/**
 * Represents a snapshot of the statistics of a specific Entity. It consists
 * of a set of Counter objects. The combination of these Counter object form
 * the statistics at a given time of a specific Entity.
 * 
 * @date May 12, 2005 
 */
public class Statistics {
    /**
     * The Entity, where the statistics belong to.
     */
    private final Entity entity;
    
    private Time lastReset;
    
    /**
     * Set of Counter objects. &lt;String countername, Counter counter&gt; 
     */
    private final LinkedHashMap counters;
    
    /**
     * Constructs a new Statistics object.
     *
     * @param entity The Entity where the Statistics belong to.
     */
    public Statistics(Entity entity, Time lastReset){
        assert (entity != null) : "Statistics constructor: supplied entity not valid";
        this.entity = entity;
        this.lastReset = lastReset;
        counters = new LinkedHashMap();
    }
    
    /**
     * Resolves a Counter that matches the supplied name.
     * 
     * @param name The name of the Counter.
     * @return The Counter that matches the supplied name or null if no
     *         Counter was found.
     */
    public AbstractValue getCounter(String name){
        return (AbstractValue)counters.get(name);
    }
    
    /**
     * Adds the supplied counter to the Statistics.
     * 
     * @param counter The Counter to add to the Statistics.
     */
    public void addCounter(AbstractValue counter, String prefix){
    	String name = prefix + counter.getName();
    	counter.setName(name);
        counters.put(name, counter);
    }

    public void addString(StringValue name, String fieldName) {
        name.setName(fieldName);
        counters.put(fieldName, name);
    }

    /**
     * Removes the supplied Counter from the Statistics. 
     * 
     * @param counter The Counter to remove.
     * @return true if succeeded, false if the supplied Counter was not 
     *         available.
     */
    public boolean removeCounter(AbstractValue counter){
        Object obj = counters.remove(counter.getName());
        
        if(obj != null){
            return true;
        }
        return false;
    }
    
    /**
     * Provides access to all Counter objects.
     * 
     * @return The array of all Counter objects within this Statistics instance.
     */
    public AbstractValue[] getCounters(){
        return (AbstractValue[])counters.values().toArray(new AbstractValue[counters.size()]);
    }

    public Time getLastReset() {
        return lastReset;
    }

    public void setLastReset(Time lastReset) {
        this.lastReset = lastReset;
    }    
}
