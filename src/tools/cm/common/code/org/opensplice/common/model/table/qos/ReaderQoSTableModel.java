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
import org.opensplice.cm.Reader;
import org.opensplice.cm.qos.DeadlinePolicy;
import org.opensplice.cm.qos.DurabilityPolicy;
import org.opensplice.cm.qos.HistoryPolicy;
import org.opensplice.cm.qos.LatencyPolicy;
import org.opensplice.cm.qos.LivelinessPolicy;
import org.opensplice.cm.qos.OrderbyPolicy;
import org.opensplice.cm.qos.PacingPolicy;
import org.opensplice.cm.qos.ReaderLifecyclePolicy;
import org.opensplice.cm.qos.ReaderLifespanPolicy;
import org.opensplice.cm.qos.ReaderQoS;
import org.opensplice.cm.qos.ReliabilityPolicy;
import org.opensplice.cm.qos.OwnershipPolicy;
import org.opensplice.cm.qos.ResourcePolicy;
import org.opensplice.cm.qos.SharePolicy;
import org.opensplice.cm.qos.TopicQoS;
import org.opensplice.cm.qos.UserDataPolicy;
import org.opensplice.cm.qos.UserKeyPolicy;
import org.opensplice.common.CommonException;

/**
 * Concrete implementation of the EntityQoSTableModel that is capable of
 * resolving and administrating the QoS of a Reader (ReaderQoS). 
 * 
 * @date Jan 10, 2005 
 */
public class ReaderQoSTableModel extends EntityQoSTableModel {
    private TopicQoS topicQos = null;
    private boolean noUpdate = false;
    /**
     * Constructs a new table model that holds the QoS of the supplied
     * Reader.
     *
     * @param _entity The Reader, which QoS must be administrated.
     * @throws CommonException Thrown when the Entity is not available (anymore)
     */
    public ReaderQoSTableModel(Reader _entity) throws CommonException {
        super(_entity);
    }
    
    public ReaderQoSTableModel(TopicQoS topicQos) {
        super(topicQos);
        this.topicQos = topicQos;
        this.update();
    }
    
    public ReaderQoSTableModel(ReaderQoS readerQos) {
        super(readerQos);
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
        data[0] = "USER_DATA";
        data[1] = "value";
        this.addRow(data);
        
        row++;
        data[0] = "OWNERSHIP";
        data[1] = "kind";
        this.addRow(data);
        nonEditRows.add(new Integer(row));
        
        row++;
        data[0] = "TIME_BASED_FILTER";
        data[1] = "minimum_separation";
        this.addRow(data);
        
        row++;
        data[0] = "READER_DATA_LIFECYCLE";
        data[1] = "autopurge_nowriter_samples_delay";
        this.addRow(data);
        
        row++;
        data[1] = "autopurge_disposed_samples_delay";
        this.addRow(data);
        
        row++;
        data[1] = "enable_invalid_samples";
        this.addRow(data);
        nonEditRows.add(new Integer(row));
        
        row++;
        data[0] = "READER_DATA_LIFESPAN";
        data[1] = "used";
        this.addRow(data);
        
        row++;
        data[1] = "duration";
        this.addRow(data);
        
        row++;
        data[0] = "SHARE";
        data[1] = "name";
        this.addRow(data);
        nonEditRows.add(new Integer(row));
        
        row++;
        data[1] = "enable";
        this.addRow(data);
        nonEditRows.add(new Integer(row));
        
        row++;
        data[0] = "USER_KEY";
        data[1] = "enable";
        this.addRow(data);
        nonEditRows.add(new Integer(row));
        
        row++;
        data[1] = "expression";
        this.addRow(data);
        nonEditRows.add(new Integer(row));
    }

    public boolean update() {
        boolean result;
        ReaderQoS qos;
        Object nill = "null";
        
        this.cancelEditing();
        
        try {
            if(noUpdate){
                qos = (ReaderQoS)currentQos;
            } else if(topicQos == null){
                qos = (ReaderQoS)entity.getQoS();
            } else {
                qos = ReaderQoS.copyFromTopicQoS(topicQos);
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
            PacingPolicy pcp = qos.getPacing();
            
            if(pcp != null){
                this.setValueAt(pcp.minSeperation, row++, 2);
            } else {
                this.setValueAt(nill, row++, 2);
            }
            ReaderLifecyclePolicy rcp = qos.getLifecycle();
            
            if(rcp != null){
                this.setValueAt(rcp.autopurge_nowriter_samples_delay, row++, 2);
                this.setValueAt(rcp.autopurge_disposed_samples_delay, row++, 2);
                this.setValueAt(new Boolean(rcp.enable_invalid_samples), row++, 2);
            } else {
                this.setValueAt(nill, row++, 2);
                this.setValueAt(nill, row++, 2);
                this.setValueAt(Boolean.FALSE, row++, 2);
            }
            ReaderLifespanPolicy rlsp = qos.getLifespan();
            
            if(rcp != null){
                this.setValueAt(new Boolean(rlsp.used), row++, 2);
                this.setValueAt(rlsp.duration, row++, 2);
            } else {
                this.setValueAt(Boolean.FALSE, row++, 2);
                this.setValueAt(nill, row++, 2);
            }
            SharePolicy sh = qos.getShare();
            
            if(sh != null){
                if(sh.name == null){
                    this.setValueAt(nill, row++, 2);
                } else {
                    this.setValueAt(sh.name, row++, 2);
                }
                this.setValueAt(new Boolean(sh.enable), row++, 2);
            } else {
                this.setValueAt(nill, row++, 2);
                this.setValueAt(Boolean.FALSE, row++, 2);
            }
            UserKeyPolicy kp = qos.getUserKey();
            
            if(kp != null){
                this.setValueAt(new Boolean(kp.enable), row++, 2);
                
                if(kp.expression == null){
                    this.setValueAt(nill, row++, 2);
                } else {
                    this.setValueAt(kp.expression, row++, 2);
                }
                
            } else {
                this.setValueAt(Boolean.FALSE, row++, 2);
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
