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
package org.opensplice.common.model.table.qos;

import org.opensplice.cm.CMException;
import org.opensplice.cm.Writer;
import org.opensplice.cm.qos.DeadlinePolicy;
import org.opensplice.cm.qos.DurabilityPolicy;
import org.opensplice.cm.qos.HistoryPolicy;
import org.opensplice.cm.qos.LatencyPolicy;
import org.opensplice.cm.qos.LifespanPolicy;
import org.opensplice.cm.qos.LivelinessPolicy;
import org.opensplice.cm.qos.OrderbyPolicy;
import org.opensplice.cm.qos.ReliabilityPolicy;
import org.opensplice.cm.qos.OwnershipPolicy;
import org.opensplice.cm.qos.ResourcePolicy;
import org.opensplice.cm.qos.StrengthPolicy;
import org.opensplice.cm.qos.TopicQoS;
import org.opensplice.cm.qos.TransportPolicy;
import org.opensplice.cm.qos.UserDataPolicy;
import org.opensplice.cm.qos.WriterLifecyclePolicy;
import org.opensplice.cm.qos.WriterQoS;
import org.opensplice.common.CommonException;

/**
 * Concrete implementation of the EntityQoSTableModel that is capable of
 * resolving and administrating the QoS of a Writer (WriterQoS).
 * 
 * @date Jan 10, 2005 
 */
public class WriterQoSTableModel extends EntityQoSTableModel {
    private TopicQoS topicQos = null;
    private boolean noUpdate = false;
    
    /**
     * Constructs a new table model that holds the QoS of the supplied
     * Writer.
     *
     * @param _entity The Writer, which QoS must be administrated.
     * @throws CommonException Thrown when the Entity is not available (anymore)
     */
    public WriterQoSTableModel(Writer _entity) throws CommonException {
        super(_entity);
    }
    
    public WriterQoSTableModel(TopicQoS topicQos) {
        super(topicQos);
        this.topicQos = topicQos;
        this.update();
    }
    
    public WriterQoSTableModel(WriterQoS writerQos) {
        super(writerQos);
        noUpdate = true;
        this.update();
    }

    protected void init() {
        Object[] data = new Object[3];
        int row;
        
        row = 0;
        data[0] = "DURABILITY";
        data[1] = "kind";
        this.addRow(data);
        nonEditRows.add(new Integer(row));
        
        row++;
        data[0] = "DEADLINE";
        data[1] = "period";
        this.addRow(data);
        
        row++;
        data[0] = "LATENCY_BUDGET";
        data[1] = "duration";
        this.addRow(data);
        
        row++;
        data[0] = "LIVELINESS";
        data[1] = "kind";
        this.addRow(data);
        nonEditRows.add(new Integer(row));
        
        row++;
        data[1] = "lease_duration";
        this.addRow(data);
        nonEditRows.add(new Integer(row));
        
        row++;
        data[0] = "RELIABILITY";
        data[1] = "kind";
        this.addRow(data);
        nonEditRows.add(new Integer(row));
        
        row++;
        data[1] = "max_blocking_time";
        this.addRow(data);
        nonEditRows.add(new Integer(row));
        
        row++;
        data[1] = "synchronous";
        this.addRow(data);
        nonEditRows.add(new Integer(row));
        
        row++;
        data[0] = "DESTINATION_ORDER";
        data[1] = "kind";
        this.addRow(data);
        nonEditRows.add(new Integer(row));
        
        row++;
        data[0] = "HISTORY";
        data[1] = "kind";
        this.addRow(data);
        nonEditRows.add(new Integer(row));
        
        row++;
        data[1] = "depth";
        this.addRow(data);
        nonEditRows.add(new Integer(row));
        
        row++;
        data[0] = "RESOURCE_LIMITS";
        data[1] = "max_samples";
        this.addRow(data);
        nonEditRows.add(new Integer(row));
        
        row++;
        data[1] = "max_instances";
        this.addRow(data);
        nonEditRows.add(new Integer(row));
        
        row++;
        data[1] = "max_samples_per_instance";
        this.addRow(data);
        nonEditRows.add(new Integer(row));
        
        row++;
        data[0] = "TRANSPORT_PRIORITY";
        data[1] = "value";
        this.addRow(data);
        
        row++;
        data[0] = "LIFESPAN";
        data[1] = "duration";
        this.addRow(data);
        
        row++;
        data[0] = "USER_DATA";
        data[1] = "value";
        this.addRow(data);
        
        row++;
        data[0] = "OWNERSHIP";
        data[1] = "kind";
        this.addRow(data);
        nonEditRows.add(new Integer(row));
        
        row++;
        data[0] = "OWNERSHIP_STRENGTH";
        data[1] = "value";
        this.addRow(data);
        
        row++;
        data[0] = "WRITER_DATA_LIFECYCLE";
        data[1] = "autodispose_unregistered_instances";
        this.addRow(data);
        
        row++;
        data[1] = "autopurge_suspended_samples_delay";
        this.addRow(data);
        
        row++;
        data[1] = "autounregister_instance_delay";
        this.addRow(data);
    }

    public boolean update() {
        WriterQoS qos;
        boolean result;
        Object nill = "null";
        
        this.cancelEditing();
        
        try {
            if(noUpdate){
                qos = (WriterQoS)currentQos;
            } else if(topicQos == null){
                qos = (WriterQoS)entity.getQoS();
            } else {
                qos = WriterQoS.copyFromTopicQoS(topicQos);
            }
            currentQos = qos;
            int row = 0;
            
            DurabilityPolicy dbp = qos.getDurability();
            
            if(dbp != null){
                this.setValueAt(dbp.kind, row++, 2);
            } else {
                this.setValueAt(nill, row++, 2);
            }
            DeadlinePolicy dlp = qos.getDeadline();
            
            if(dlp != null){
                this.setValueAt(dlp.period, row++, 2);
            } else {
                this.setValueAt(nill, row++, 2);
            }
            LatencyPolicy ltp = qos.getLatency();
            
            if(ltp != null){
                this.setValueAt(ltp.duration, row++, 2);
            } else {
                this.setValueAt(nill, row++, 2);
            }
            LivelinessPolicy llp = qos.getLiveliness();
            
            if(llp != null){
                this.setValueAt(llp.kind, row++, 2);
                this.setValueAt(llp.lease_duration, row++, 2);
            } else {
                this.setValueAt(nill, row++, 2);
                this.setValueAt(nill, row++, 2);
            }
            ReliabilityPolicy rlp = qos.getReliability();
            
            if(rlp != null){
                this.setValueAt(rlp.kind, row++, 2);
                this.setValueAt(rlp.max_blocking_time, row++, 2);
                this.setValueAt(rlp.synchronous, row++, 2);
            } else {
                this.setValueAt(nill, row++, 2);
                this.setValueAt(nill, row++, 2);
                this.setValueAt(nill, row++, 2);
            }
            OrderbyPolicy obp = qos.getOrderby();
            
            if(obp != null){
                this.setValueAt(obp.kind, row++, 2);
            } else {
                this.setValueAt(nill, row++, 2);
            }
            HistoryPolicy hsp = qos.getHistory();
            
            if(hsp != null){
                this.setValueAt(hsp.kind, row++, 2);
                this.setValueAt(new Integer(hsp.depth), row++, 2);
            } else {
                this.setValueAt(nill, row++, 2);
                this.setValueAt(nill, row++, 2);
            }
            ResourcePolicy rsp = qos.getResource();
            
            if(rsp != null){
                this.setValueAt(new Integer(rsp.max_samples), row++, 2);
                this.setValueAt(new Integer(rsp.max_instances), row++, 2);
                this.setValueAt(new Integer(rsp.max_samples_per_instance), row++, 2);
            } else {
                this.setValueAt(nill, row++, 2);
                this.setValueAt(nill, row++, 2);
                this.setValueAt(nill, row++, 2);
            }
            TransportPolicy tpp = qos.getTransport();
            
            if(tpp != null){
                this.setValueAt(new Integer(tpp.value), row++, 2);
            } else {
                this.setValueAt(nill, row++, 2);
            }
            LifespanPolicy lsp = qos.getLifespan();
            
            if(lsp != null){
                this.setValueAt(lsp.duration, row++, 2);
            } else {
                this.setValueAt(nill, row++, 2);
            }
            UserDataPolicy udp = qos.getUserData();
            
            if(udp != null){
                this.setValueAt(udp, row++, 2);
            } else {
                this.setValueAt(nill, row++, 2);
            }
            OwnershipPolicy osp = qos.getOwnership();
            
            if(osp != null){
                this.setValueAt(osp.kind, row++, 2);
            } else {
                this.setValueAt(nill, row++, 2);
            }
            StrengthPolicy stp = qos.getStrength();
            
            if(stp != null){
                this.setValueAt(new Integer(stp.value), row++, 2);
            } else {
                this.setValueAt(nill, row++, 2);
            }
            WriterLifecyclePolicy wlp = qos.getLifecycle();
            
            if(wlp != null){
                this.setValueAt(new Boolean(wlp.autodispose_unregistered_instances), row++, 2);
                this.setValueAt(wlp.autopurge_suspended_samples_delay, row++, 2);
                this.setValueAt(wlp.autounregister_instance_delay, row++, 2);
            } else {
                this.setValueAt(nill, row++, 2);
                this.setValueAt(nill, row++, 2);
                this.setValueAt(nill, row++, 2);
            }
            
            assert row == this.getRowCount(): "#rows does not match filled rows.";
            
            result = true;
        } catch (CMException e) {
            result = false;
        }
        return result;
    }

}
