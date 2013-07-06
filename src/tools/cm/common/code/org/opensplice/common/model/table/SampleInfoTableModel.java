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

import org.opensplice.cm.data.GID;
import org.opensplice.cm.data.Message;
import org.opensplice.cm.data.Sample;
import org.opensplice.cm.data.State;

/**
 * Holds the Sample info of one specific Sample. The following information
 * is currently administrated:
 * 
 * - node_state
 * - sample_state
 * - insert_time
 * - write_time
 * - write_insert_latency
 * - writerGID
 * - instanceGID
 * - sampleSequenceNumber
 * 
 * @todo TODO: Add QoS information.
 * @date Oct 21, 2004 
 */
public class SampleInfoTableModel extends DefaultTableModel{
    private Sample currentValue = null;    
    private boolean enabled = true;
    /**
     * Constructs new model that is capable of holding Sample info.
     */
    public SampleInfoTableModel(){
        super(0,0);
        this.initialize();
    }
    
    public boolean isCellValueReliableInSnapshot(int row, int col){
        boolean result;
        
        if(col == 0){
            result = true;
        } else if(col == 1){
            if(row >= 0 && row < 6){
                result = false;
            } else {
                result = true;
            }
        } else {
            result = false;
        }
        return result;
    }
    
    private void initialize(){
        Object[] data = new Object[2];
        
        this.addColumn("attribute");
        this.addColumn("value");
        data[1] = "N/A";
        
        /* 0 */
        data[0] = "sample_state";
        this.addRow(data);
        
        /* 1 */
        data[0] = "view_state";
        this.addRow(data);
        
        /* 2 */
        data[0] = "instance_state";
        this.addRow(data);
        
        /* 3 */
        data[0] = "valid_data";
        this.addRow(data);
        
        /* 4 */
        data[0] = "disposed_generation_count";
        this.addRow(data);
        
        /* 5 */
        data[0] = "no_writers_generation_count";
        this.addRow(data);
        
        /* 6 */
        data[0] = "insert_timestamp";
        this.addRow(data);
        
        /* 7 */
        data[0] = "source_timestamp";
        this.addRow(data);
        
        /* 8 */
        data[0] = "write_insert_latency";
        this.addRow(data);    
        
        /* 9 */
        data[0] = "writerGID.localId";
        this.addRow(data);
        
        /* 10 */
        data[0] = "writerGID.systemId";
        this.addRow(data);
        
        /* 11 */
        data[0] = "instanceGID.localId";
        this.addRow(data);
        
        /* 12 */
        data[0] = "instanceGID.systemId";
        this.addRow(data);
        
        /* 13 */
        data[0] = "sampleSequenceNumber";
        this.addRow(data);
        
        /* 14 */
        data[0] = "qos.reliability.kind";
        this.addRow(data);
        
        /* 15 */
        data[0] = "qos.reliability.max_blocking_time";
        this.addRow(data);
    }
    
    public void setEnabled(boolean enabled){
        this.enabled = enabled;
        
        if(enabled){
            this.setData(this.currentValue);
        }
    }
    
    /**
     * Administrates the Sample info oof the supplied Sample. It replaces the
     * possible previous info.
     * 
     * @param data The Sample, which info must be administrated.
     * @return true if the supplied Sample and its Message are not null, false
     *              otherwise.
     */
    public boolean setData(Sample data){
        currentValue = data;
        boolean success = false;
        
        if(enabled){
            Message msg = data.getMessage();
            int row = 0;
            
            if(msg != null){
                State state = data.getState();
                
                this.setValueAt(this.getSampleState(state), row++, 1);
                this.setValueAt(this.getViewState(state), row++, 1);
                this.setValueAt(this.getInstanceState(state), row++, 1);
                this.setValueAt(this.getValidDataState(state), row++, 1);
                this.setValueAt(Long.toString(data.getDisposeCount()), row++, 1);
                this.setValueAt(Long.toString(data.getNoWritersCount()), row++, 1);
                
                String date = "(" + new Date(data.getInsertTimeSec() * 1000) + ")";
                this.setValueAt(Long.toString(data.getInsertTimeSec()) + "s. " +
                                Long.toString(data.getInsertTimeNanoSec()) + "ns. " +
                                date, 
                                row++, 1);
                
                date = "(" + new Date(msg.getWriteTimeSec() * 1000) + ")";
                this.setValueAt(Long.toString(msg.getWriteTimeSec()) + "s. " +
                                Long.toString(msg.getWriteTimeNanoSec()) + "ns. " +
                                date, 
                                row++, 1);
                
                long sec = data.getInsertTimeSec() - msg.getWriteTimeSec();
                long nsec = data.getInsertTimeNanoSec() - msg.getWriteTimeNanoSec();
                
                if(nsec < 0){
                    sec--;
                    
                    String strNsec = Long.toString(nsec);
                    StringBuffer buf = new StringBuffer();
                    buf.append("1");
                    for (int i = 0; i < (strNsec.length() - 1); i++) {
                        buf.append("0");
                    }
                    String strOne = buf.toString();

                    long one = Long.parseLong(strOne);
                    
                    /*Add up, because nsec is negative.*/
                    nsec = one + nsec;
                }
                
                this.setValueAt(Long.toString(sec) + "s. " + Long.toString(nsec) 
                                + "ns.", row++, 1);
                
                GID gid = msg.getWriterGid();
                
                if(gid != null){
                    this.setValueAt(Long.toString(gid.getLocalId()), row++, 1);
                    this.setValueAt(Long.toString(gid.getSystemId()), row++, 1);
                } else {
                    this.setValueAt("N/A", row++, 1);
                    this.setValueAt("N/A", row++, 1);
                    this.setValueAt("N/A", row++, 1);
                }
                gid = msg.getInstanceGid();
                
                if(gid != null){
                    this.setValueAt(Long.toString(gid.getLocalId()), row++, 1);
                    this.setValueAt(Long.toString(gid.getSystemId()), row++, 1);
                } else {
                    this.setValueAt("N/A", row++, 1);
                    this.setValueAt("N/A", row++, 1);
                    this.setValueAt("N/A", row++, 1);
                }
                this.setValueAt(Long.toString(msg.getSampleSequenceNumber()), row++, 1);

                if(msg.getQos() != null){
                    this.setValueAt(msg.getQos().getReliabilityKind().toString(), row++, 1);
                } else {
                    this.setValueAt("N/A", row++, 1);
                }
                
                success = true;
            }
        } else {
            success = true;
        }
        return success;
    }
    
    public String getStringValueForRow(String name){
        String result = null;
        int rowCount = this.getRowCount();
        
        for(int i=0; (i<rowCount) && (result == null); i++){
            if(this.getValueAt(i,0).equals(name)){
                result = this.getValueAt(i,1).toString(); 
            }
        }
        return result;
    }
    
    /**
     * Makes sure the model cannot be edited.
     * 
     * @param row The row that wants to be edited.
     * @param column The column that wants to be edited.
     */
    @Override
    public boolean isCellEditable(int row, int column){
        return false;
    }
    
    /**
     * Clears the model. The data is removed.
     */
    public synchronized void clear(){
        int rowCount = this.getRowCount();
        
        for(int i=0; i<rowCount; i++){
            this.setValueAt("N/A", i, 1);
        }
    }
    
    protected String getSampleState(State state){
        String result = null;
        
        if(state != null){
            int value = state.getValue();
            
            if((value & (State.READ)) == State.READ){
                result = "READ";
            } else {
                result = "NOT_READ";
            }
        } else {
            result = "N/A";
        }
        return result;
    }
    
    protected String getViewState(State state){
        String result = null;
        
        if(state != null){
            int value = state.getValue();
            
            if((value & (State.NEW)) == State.NEW){
                result = "NEW";
            } else {
                result = "NOT_NEW";
            }
        } else {
            result = "N/A";
        }
        return result;
    }
    
    protected String getInstanceState(State state){
        String result = null;
        
        if(state != null){
            int value = state.getValue();
            
            if((value & (State.DISPOSED)) == State.DISPOSED){
                result = "NOT_ALIVE_DISPOSED";
            } else if((value & (State.NOWRITERS)) == State.NOWRITERS){
                result = "NOT_ALIVE_NO_WRITERS";
            } else {
                result = "ALIVE";
            }
        } else {
            result = "N/A";
        }
        return result;
    }
    
    protected String getValidDataState(State state){
        String result = null;
        
        if(state != null){
            int value = state.getValue();
            
            if((value & (State.VALIDDATA)) == State.VALIDDATA){
                result = "TRUE";
            } else {
                result = "FALSE";
            }
        } else {
            result = "N/A";
        }
        return result;
    }
}
