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

package org.opensplice.common.model.table;

import java.util.Date;

import javax.swing.table.DefaultTableModel;

import org.opensplice.cm.CMException;
import org.opensplice.cm.Entity;
import org.opensplice.cm.Time;
import org.opensplice.cm.statistics.AbstractValue;
import org.opensplice.cm.statistics.AvgValue;
import org.opensplice.cm.statistics.FullCounter;
import org.opensplice.cm.statistics.Statistics;
import org.opensplice.cm.statistics.StringValue;
import org.opensplice.cm.statistics.TimedValue;
import org.opensplice.cm.statistics.Value;
import org.opensplice.common.CommonException;

/**
 * Represents a TableModel that is capable of keeping track of the Statistics
 * of a specific Entity.
 * 
 * @date May 12, 2005 
 */
public class StatisticsTableModel extends DefaultTableModel{
    /**
     * The Entity, where to resolve the Statistics from.
     */
    private Entity entity = null;
    
    /**
     * The current Statistics of the model. 
     */
    private Statistics statistics = null;
    
    /**
     * Whether the entity has any statistics.
     */
    private boolean entityHasStatistics = true;
    
    /**
     * Used to update the model.
     */
    private int counterCount = 0;
    
    /**
     * Constructs a new model that holds the Statistics of the supplied Entity.
     *
     * @param entity The Entity where to resolve the Statistics of.
     * @throws CommonException Thrown when:
     *                          - C&M API not initialized.
     *                          - Supplied Entity not valid.
     *                          - Communication with SPLICE-DDS failed.
     *                          - Supplied Entity not available.
     */
    public StatisticsTableModel(Entity entity) throws CommonException {
        super();
        
        if(entity == null){
            throw new CommonException("StatisticsTableModel: supplied entity not valid.");
        }
        this.entity = entity;
        this.init();
    }
    
    /**
     * Resets the Statistics field located at the supplied row.
     * 
     * @param row The row that holds the field that must be resetted.
     * @param update Whether to update the Statistics in the model after resetting.
     * @return true if reset succeeded, false otherwise.
     */
    public boolean reset(int row, boolean update){
        boolean result = true;
        
        if(entityHasStatistics){
            if(this.getRowCount() > row){
                String row0 = (String)getValueAt(row, 0);
                String row1 = (String)getValueAt(row, 1);
                String value;
                
                if(("".equals(row0)) && ("".equals(row1))){
                    value = "";
                } else if("".equals(row0)){
                    value = (String)getValueAt(row, 1);
                } else if("".equals(row1)){
                    value = (String)getValueAt(row, 0);
                } else {
                    value = (String)this.getValueAt(row, 0) + "." + 
                            (String)this.getValueAt(row, 1);
                }
                
                try {
                    
                    if((value.endsWith("Update")) || (value.endsWith("Reset"))){
                        entity.resetStatistics(value + ".seconds");
                        entity.resetStatistics(value + ".nanoseconds");
                    } else {
                        entity.resetStatistics(value);
                    }
                
                    if(update){
                        result = this.update();
                    }
                } catch (CMException e) {
                    result = false;
                }
            } else {
                result = false;
            }
        }
        return result;
    }
    
    /**
     * Resets the complete Statistics of the Entity of this model.
     * 
     * @param update Whether to update the Statistics in the model after resetting.
     * @return true if reset succeeded, false otherwise.
     */
    public boolean reset(boolean update){
        boolean result = true;
        
        if(entityHasStatistics){
            if(this.getRowCount() > 0){
                try {
                    entity.resetStatistics(null);
                    
                    if(update){
                        result = this.update();
                    }
                } catch (CMException e) {
                    result = false;
                }
            } else {
                result = false;
            }
        }
        return result;
    }
    
    /**
     * Updates the Statistics in the model by resolving the Statistics of the
     * Entity again.
     * 
     * @return true if update succeeded, false otherwise.
     */
    public boolean update(){
        boolean result;
        AbstractValue counter;
        
        if(entityHasStatistics){
            try {
                statistics = entity.getStatistics();
                int index = 0;
                if (statistics != null) {
                    this.setValueAt(this.getTimeString(statistics.getLastReset()), index++, 2);

                    for (int i = 0; i < counterCount; i++) {
                        counter = statistics.getCounter((String) this.getValueAt(index, 0));

                        if (counter instanceof Value) {
                            this.setValueAt(Long.toString(((Value) counter).getValue()), index++, 2);

                            if (counter instanceof TimedValue) {
                                this.setValueAt(this.getTimeString(((TimedValue) counter).getLastUpdate()), index++, 2);
                            } else if (counter instanceof FullCounter) {
                                TimedValue min = ((FullCounter) counter).getMin();
                                this.setValueAt(Long.toString(min.getValue()), index++, 2);
                                this.setValueAt(this.getTimeString(min.getLastUpdate()), index++, 2);

                                TimedValue max = ((FullCounter) counter).getMax();
                                this.setValueAt(Long.toString(max.getValue()), index++, 2);
                                this.setValueAt(this.getTimeString(max.getLastUpdate()), index++, 2);

                                AvgValue avg = ((FullCounter) counter).getAvg();
                                this.setValueAt(Float.toString(avg.getValue()), index++, 2);
                                this.setValueAt(Long.toString(avg.getCount()), index++, 2);
                            }
                        } else if (counter instanceof AvgValue) {
                            this.setValueAt(Float.toString(((AvgValue) counter).getValue()), index++, 2);
                            this.setValueAt(Long.toString(((AvgValue) counter).getCount()), index++, 2);
                        } else if (counter instanceof StringValue) {
                            this.setValueAt(((StringValue) counter).getValue(), index++, 2);
                        }
                    }
                    result = true;
                } else {
                    result = false;
                }
                
                /*
                for(int i=0; i<counterCount; i++){
                    counter = statistics.getCounter((String)this.getValueAt(index, 0));
                    
                    this.setValueAt(Long.toString(counter.getValue()), index++, 2);
                    this.setValueAt(this.getTimeString(counter.getLastTime()), index++, 2);
                    
                    if(counter instanceof MaxCounter){
                        this.setValueAt(Long.toString(((MaxCounter)counter).getMax()), index++, 2);
                        this.setValueAt(this.getTimeString(((MaxCounter)counter).getMaxTime()), index++, 2);
                        
                        if(counter instanceof FullCounter){
                            this.setValueAt(Long.toString(((FullCounter)counter).getMin()), index++, 2);
                            this.setValueAt(this.getTimeString(((FullCounter)counter).getMinTime()), index++, 2);
                            
                            this.setValueAt(Long.toString(((FullCounter)counter).getAvg()), index++, 2);
                            this.setValueAt(Long.toString(((FullCounter)counter).getCount()), index++, 2);
                        }
                    }
                }
                */
            } catch (CMException e) {
                result = false;
            }
        } else {
            result = true;
        }
        return result;
    }
    
    private void init() throws CommonException {
        try {
            statistics = entity.getStatistics();
        } catch (CMException ce) {
            throw new CommonException("Entity statistics could not be resolved.");
        }
        if(statistics == null){
            entityHasStatistics = false;
            
            this.addColumn("No statistics available");
        } else {
            this.addColumn("Name");
            this.addColumn("Field");
            this.addColumn("Value");
            
            Object[] data = new Object[3];
            
            data[0] = "";
            data[1] = "lastReset";
            data[2] = this.getTimeString(statistics.getLastReset());
            this.addRow(data);
            
            AbstractValue[] counters = statistics.getCounters();
            counterCount = counters.length;
            
            for(int i=0; i<counters.length; i++){
                data[0] = counters[i].getName();
                
                if(counters[i] instanceof TimedValue){
                    data[1] = "value";
                    data[2] = Long.toString(((Value)counters[i]).getValue());
                    this.addRow(data);
                    
                    data[1] = "lastUpdate";
                    data[2] = this.getTimeString(((TimedValue)counters[i]).getLastUpdate());
                    this.addRow(data);
                } else if(counters[i] instanceof FullCounter){
                    data[1] = "value";
                    data[2] = Long.toString(((Value)counters[i]).getValue());
                    this.addRow(data);
                    
                    TimedValue min = ((FullCounter)counters[i]).getMin();
                    data[1] = "min.value";
                    data[2] = Long.toString(min.getValue());
                    this.addRow(data);
                    
                    data[1] = "min.lastUpdate";
                    data[2] = this.getTimeString(min.getLastUpdate());
                    this.addRow(data);
                    
                    TimedValue max = ((FullCounter)counters[i]).getMax();
                    data[1] = "max.value";
                    data[2] = Long.toString(max.getValue());
                    this.addRow(data);
                    
                    data[1] = "max.lastUpdate";
                    data[2] = this.getTimeString(max.getLastUpdate());
                    this.addRow(data);
                    
                    AvgValue avg = ((FullCounter)counters[i]).getAvg();
                    data[1] = "avg.value";
                    data[2] = Float.toString(avg.getValue());
                    this.addRow(data);
                    
                    data[1] = "avg.count";
                    data[2] = Long.toString(avg.getCount());
                    this.addRow(data);
                } else if(counters[i] instanceof AvgValue){
                    data[1] = "value";
                    data[2] = Float.toString(((AvgValue)counters[i]).getValue());
                    this.addRow(data);
                    
                    data[1] = "count";
                    data[2] = Long.toString(((AvgValue)counters[i]).getCount());
                    this.addRow(data);
                } else if(counters[i] instanceof Value){
                    data[1] = "";
                    data[2] = Long.toString(((Value)counters[i]).getValue());
                    this.addRow(data);
                    
                } else if(counters[i] instanceof StringValue){
                    data[1] = "";
                    data[2] = ((StringValue)counters[i]).getValue();
                    this.addRow(data);
                }
            }
        }
    }
        
    private String getTimeString(Time time){
        String date = "(" + new Date(((long) (time.sec * 1000)) + ((long) (time.nsec / 1000000000))) + ")";
        String result = time.sec + "s. " + time.nsec + " ns." + date;
        
        return result;
    }
}
