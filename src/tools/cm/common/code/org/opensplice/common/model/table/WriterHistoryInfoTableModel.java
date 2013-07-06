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

import org.opensplice.cm.data.GID;
import org.opensplice.cm.data.Message;
import org.opensplice.cm.data.Sample;


/**
 * Model that holds the history of one specific Sample.
 * 
 * @date Nov 17, 2004 
 */
public class WriterHistoryInfoTableModel extends SampleInfoTableModel {
    /**
     * Constructs a new HistoryTableModel.
     *  
     */
    public WriterHistoryInfoTableModel(){
        super();
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
        Message msg = data.getMessage();
        boolean success = false;
        int row = 0;
        
        if(msg != null){
            /*
            State state = data.getState();
            this.setValueAt(this.getSampleState(state), row++, 1);
            this.setValueAt(this.getViewState(state), row++, 1);
            this.setValueAt(this.getInstanceState(state), row++, 1);
            */
            this.setValueAt("N/A", row++, 1);
            this.setValueAt("N/A", row++, 1);
            this.setValueAt("N/A", row++, 1);
            
            this.setValueAt("N/A", row++, 1);
            this.setValueAt("N/A", row++, 1);
            
            this.setValueAt("N/A", row++, 1);
            
            String date = "(" + new Date(msg.getWriteTimeSec() * 1000) + ")";
            this.setValueAt(Long.toString(msg.getWriteTimeSec()) + "s. " +
                            Long.toString(msg.getWriteTimeNanoSec()) + "ns. " +
                            date, 
                            row++, 1);
            this.setValueAt("N/A", row++, 1);
            
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
            this.setValueAt(msg.getQos().getReliabilityKind().toString(), row++, 1);
            
            success = true;
        }
        return success;
    }
}
