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

package org.opensplice.cm.statistics;

import java.util.LinkedHashMap;

import org.opensplice.cm.Entity;
import org.opensplice.cm.Time;

/**
 * Represents a snapshot of the statistics of a specific Entity. It consists of
 * a set of Counter objects. The combination of these Counter object form the
 * statistics at a given time of a specific Entity.
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
    private final LinkedHashMap<String, AbstractValue> counters;

    /**
     * Constructs a new Statistics object.
     *
     * @param entity The Entity where the Statistics belong to.
     */
    public Statistics(Entity entity, Time lastReset){
        assert (entity != null) : "Statistics constructor: supplied entity not valid";
        this.entity = entity;
        this.lastReset = lastReset;
        counters = new LinkedHashMap<String, AbstractValue>();
    }

    /**
     * Resolves a Counter that matches the supplied name.
     * 
     * @param name
     *            The name of the Counter.
     * @return The Counter that matches the supplied name or null if no Counter
     *         was found.
     */
    public AbstractValue getCounter(String name){
        return counters.get(name);
    }

    /**
     * Adds the supplied counter to the Statistics.
     * 
     * @param counter
     *            The Counter to add to the Statistics.
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
     * @param counter
     *            The Counter to remove.
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
        return counters.values().toArray(new AbstractValue[counters.size()]);
    }

    public Time getLastReset() {
        return lastReset;
    }

    public void setLastReset(Time lastReset) {
        this.lastReset = lastReset;
    }
}
