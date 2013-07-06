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
package org.opensplice.common.model.table.qos;

import javax.swing.event.TableModelEvent;
import javax.swing.event.TableModelListener;
import javax.swing.table.TableModel;

import org.opensplice.cm.CMException;
import org.opensplice.cm.Reader;
import org.opensplice.cm.qos.DeadlinePolicy;
import org.opensplice.cm.qos.DurabilityPolicy;
import org.opensplice.cm.qos.HistoryPolicy;
import org.opensplice.cm.qos.LatencyPolicy;
import org.opensplice.cm.qos.LivelinessPolicy;
import org.opensplice.cm.qos.OrderbyPolicy;
import org.opensplice.cm.qos.OwnershipPolicy;
import org.opensplice.cm.qos.PacingPolicy;
import org.opensplice.cm.qos.ReaderLifecyclePolicy;
import org.opensplice.cm.qos.ReaderLifespanPolicy;
import org.opensplice.cm.qos.ReaderQoS;
import org.opensplice.cm.qos.ReliabilityPolicy;
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

    private static final long serialVersionUID   = -7920765116635148718L;
    private TopicQoS topicQos = null;
    private boolean noUpdate = false;
    private ReaderQoS selectedDefaultQos = null;
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
    
    public ReaderQoSTableModel(ReaderQoS readerQos, boolean editable) {
        super(readerQos, editable);
        selectedDefaultQos = readerQos;
        noUpdate = true;
        this.addTableModelListener(new TableModelListener() {

            @Override
            public void tableChanged(TableModelEvent e) {
                if (e.getColumn() == 0) {
                    TableModel source = (TableModel) e.getSource();
                    if (((Boolean) ((ReaderQoSTableModel) source).getValueAt(e.getFirstRow(), e.getColumn())) == true) {
                        ((ReaderQoSTableModel) source).updateDefaultValues(selectedDefaultQos);
                    } else {
                        ((ReaderQoSTableModel) source).updateDefaultValues((ReaderQoS) currentQos);
                    }
                }
            }
        });
        this.update();
    }

    @Override
    protected void init() {
        Object[] data = new Object[3];
        int row;
        
        if (this.getColumnCount() > 3) {
            row = 0;
            data[0] = true;
            data[1] = "DURABILITY";
            data[2] = "kind";
            this.addRow(data);
            nonEditRows.add(new Integer(row));
                    
            row++;
            data[1] = "DEADLINE";
            data[2] = "period";
            this.addRow(data);
            
            row++;
            data[1] = "LATENCY_BUDGET";
            data[2] = "duration";
            this.addRow(data);
            
            row++;
            data[1] = "LIVELINESS";
            data[2] = "kind";
            this.addRow(data);
            nonEditRows.add(new Integer(row));
            
            row++;
            data[2] = "lease_duration";
            this.addRow(data);
            nonEditRows.add(new Integer(row));
            
            row++;
            data[1] = "RELIABILITY";
            data[2] = "kind";
            this.addRow(data);
            nonEditRows.add(new Integer(row));
            
            row++;
            data[2] = "max_blocking_time";
            this.addRow(data);
            nonEditRows.add(new Integer(row));
            
            row++;
            data[2] = "synchronous";
            this.addRow(data);
            nonEditRows.add(new Integer(row));
            
            row++;
            data[1] = "DESTINATION_ORDER";
            data[2] = "kind";
            this.addRow(data);
            nonEditRows.add(new Integer(row));
            
            row++;
            data[1] = "HISTORY";
            data[2] = "kind";
            this.addRow(data);
            nonEditRows.add(new Integer(row));
            
            row++;
            data[2] = "depth";
            this.addRow(data);
            nonEditRows.add(new Integer(row));
            
            row++;
            data[1] = "RESOURCE_LIMITS";
            data[2] = "max_samples";
            this.addRow(data);
            nonEditRows.add(new Integer(row));
            
            row++;
            data[2] = "max_instances";
            this.addRow(data);
            nonEditRows.add(new Integer(row));
            
            row++;
            data[2] = "max_samples_per_instance";
            this.addRow(data);
            nonEditRows.add(new Integer(row));
                    
            row++;
            data[1] = "USER_DATA";
            data[2] = "value";
            this.addRow(data);
            
            row++;
            data[1] = "OWNERSHIP";
            data[2] = "kind";
            this.addRow(data);
            nonEditRows.add(new Integer(row));
            
            row++;
            data[1] = "TIME_BASED_FILTER";
            data[2] = "minimum_separation";
            this.addRow(data);
            
            row++;
            data[1] = "READER_DATA_LIFECYCLE";
            data[2] = "autopurge_nowriter_samples_delay";
            this.addRow(data);
            
            row++;
            data[2] = "autopurge_disposed_samples_delay";
            this.addRow(data);
            
            row++;
            data[2] = "enable_invalid_samples";
            this.addRow(data);
            nonEditRows.add(new Integer(row));
            
            row++;
            data[1] = "READER_DATA_LIFESPAN";
            data[2] = "used";
            this.addRow(data);
            
            row++;
            data[2] = "duration";
            this.addRow(data);
            
            row++;
            data[1] = "SHARE";
            data[2] = "name";
            this.addRow(data);
            nonEditRows.add(new Integer(row));
            
            row++;
            data[2] = "enable";
            this.addRow(data);
            nonEditRows.add(new Integer(row));
            
            row++;
            data[1] = "USER_KEY";
            data[2] = "enable";
            this.addRow(data);
            nonEditRows.add(new Integer(row));
            
            row++;
            data[2] = "expression";
            this.addRow(data);
            nonEditRows.add(new Integer(row));
        } else {
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
    }

    @Override
    public boolean update() {
        boolean result;
        ReaderQoS qos;
        Object nill = "null";
        int valueColumn = this.getColumnCount() - 1;
        
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
                this.setValueAt(dbp.kind, row++, valueColumn);
            } else {
                this.setValueAt(nill, row++, valueColumn);
            }
            DeadlinePolicy dlp = qos.getDeadline();
            
            if(dlp != null){
                this.setValueAt(dlp.period, row++, valueColumn);
            } else {
                this.setValueAt(nill, row++, valueColumn);
            }
            LatencyPolicy ltp = qos.getLatency();
            
            if(ltp != null){
                this.setValueAt(ltp.duration, row++, valueColumn);
            } else {
                this.setValueAt(nill, row++, valueColumn);
            }
            LivelinessPolicy llp = qos.getLiveliness();
            
            if(llp != null){
                this.setValueAt(llp.kind, row++, valueColumn);
                this.setValueAt(llp.lease_duration, row++, valueColumn);
            } else {
                this.setValueAt(nill, row++, valueColumn);
                this.setValueAt(nill, row++, valueColumn);
            }
            ReliabilityPolicy rlp = qos.getReliability();
            
            if(rlp != null){
                this.setValueAt(rlp.kind, row++, valueColumn);
                this.setValueAt(rlp.max_blocking_time, row++, valueColumn);
                this.setValueAt(rlp.synchronous, row++, valueColumn);
            } else {
                this.setValueAt(nill, row++, valueColumn);
                this.setValueAt(nill, row++, valueColumn);
                this.setValueAt(nill, row++, valueColumn);
            }
            OrderbyPolicy obp = qos.getOrderby();
            
            if(obp != null){
                this.setValueAt(obp.kind, row++, valueColumn);
            } else {
                this.setValueAt(nill, row++, valueColumn);
            }
            HistoryPolicy hsp = qos.getHistory();
            
            if(hsp != null){
                this.setValueAt(hsp.kind, row++, valueColumn);
                this.setValueAt(new Integer(hsp.depth), row++, valueColumn);
            } else {
                this.setValueAt(nill, row++, valueColumn);
                this.setValueAt(nill, row++, valueColumn);
            }
            ResourcePolicy rsp = qos.getResource();
            
            if(rsp != null){
                this.setValueAt(new Integer(rsp.max_samples), row++, valueColumn);
                this.setValueAt(new Integer(rsp.max_instances), row++, valueColumn);
                this.setValueAt(new Integer(rsp.max_samples_per_instance), row++, valueColumn);
            } else {
                this.setValueAt(nill, row++, valueColumn);
                this.setValueAt(nill, row++, valueColumn);
                this.setValueAt(nill, row++, valueColumn);
            }
            UserDataPolicy udp = qos.getUserData();
            
            if(udp != null){
                this.setValueAt(udp, row++, valueColumn);
            } else {
                this.setValueAt(nill, row++, valueColumn);
            }
            OwnershipPolicy osp = qos.getOwnership();
            
            if(osp != null){
                this.setValueAt(osp.kind, row++, valueColumn);
            } else {
                this.setValueAt(nill, row++, valueColumn);
            }
            PacingPolicy pcp = qos.getPacing();
            
            if(pcp != null){
                this.setValueAt(pcp.minSeperation, row++, valueColumn);
            } else {
                this.setValueAt(nill, row++, valueColumn);
            }
            ReaderLifecyclePolicy rcp = qos.getLifecycle();
            
            if(rcp != null){
                this.setValueAt(rcp.autopurge_nowriter_samples_delay, row++, valueColumn);
                this.setValueAt(rcp.autopurge_disposed_samples_delay, row++, valueColumn);
                this.setValueAt(new Boolean(rcp.enable_invalid_samples), row++, valueColumn);
            } else {
                this.setValueAt(nill, row++, valueColumn);
                this.setValueAt(nill, row++, valueColumn);
                this.setValueAt(Boolean.FALSE, row++, valueColumn);
            }
            ReaderLifespanPolicy rlsp = qos.getLifespan();
            
            if(rcp != null){
                this.setValueAt(new Boolean(rlsp.used), row++, valueColumn);
                this.setValueAt(rlsp.duration, row++, valueColumn);
            } else {
                this.setValueAt(Boolean.FALSE, row++, valueColumn);
                this.setValueAt(nill, row++, valueColumn);
            }
            SharePolicy sh = qos.getShare();
            
            if(sh != null){
                if(sh.name == null){
                    this.setValueAt(nill, row++, valueColumn);
                } else {
                    this.setValueAt(sh.name, row++, valueColumn);
                }
                this.setValueAt(new Boolean(sh.enable), row++, valueColumn);
            } else {
                this.setValueAt(nill, row++, valueColumn);
                this.setValueAt(Boolean.FALSE, row++, valueColumn);
            }
            UserKeyPolicy kp = qos.getUserKey();
            
            if(kp != null){
                this.setValueAt(new Boolean(kp.enable), row++, valueColumn);
                
                if(kp.expression == null){
                    this.setValueAt(nill, row++, valueColumn);
                } else {
                    this.setValueAt(kp.expression, row++, valueColumn);
                }
                
            } else {
                this.setValueAt(Boolean.FALSE, row++, valueColumn);
                this.setValueAt(nill, row++, valueColumn);
            }
            
            assert row == this.getRowCount(): "#rows does not match filled rows.";
            
            result = true;
        } catch (CMException e) {
            result = false;
        }
        return result;        
    }
    
    public void changeDefaultQos(ReaderQoS qos) {
        selectedDefaultQos = qos;
        updateDefaultValues(qos);
    }
    
    public boolean updateDefaultValues(ReaderQoS qos) {
        boolean result;
        Object nill = "null";
        int valueColumn = this.getColumnCount() - 1;
        int row = 0;
        int checkBoxColumn = 0;
        
        DurabilityPolicy dbp = qos.getDurability();
        if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
            if(dbp != null){
                this.setValueAt(dbp.kind, row++, valueColumn);
            } else {
                this.setValueAt(nill, row++, valueColumn);
            }
        } else {
            row++;
        }
        DeadlinePolicy dlp = qos.getDeadline();
        if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
            if(dlp != null){
                this.setValueAt(dlp.period, row++, valueColumn);
            } else {
                this.setValueAt(nill, row++, valueColumn);
            }
        } else {
            row++;
        }
        LatencyPolicy ltp = qos.getLatency();
        if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
            if(ltp != null){
                this.setValueAt(ltp.duration, row++, valueColumn);
            } else {
                this.setValueAt(nill, row++, valueColumn);
            }
        } else {
            row++;
        }
        LivelinessPolicy llp = qos.getLiveliness();
        if(llp != null){
            if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
                this.setValueAt(llp.kind, row++, valueColumn);
            } else {
                row++;
            }
            if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
                this.setValueAt(llp.lease_duration, row++, valueColumn);
            } else {
                row++;
            }
        } else {
            if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
                this.setValueAt(nill, row++, valueColumn);
            } else {
                row++;
            }
            if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
                this.setValueAt(nill, row++, valueColumn);
            } else {
                row++;
            }
        }
        ReliabilityPolicy rlp = qos.getReliability();
        
        if(rlp != null){
            if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
                this.setValueAt(rlp.kind, row++, valueColumn);
            } else {
                row++;
            }
            if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
                this.setValueAt(rlp.max_blocking_time, row++, valueColumn);
            } else {
                row++;
            }
            if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
                this.setValueAt(rlp.synchronous, row++, valueColumn);
            } else {
                row++;
            }
        } else {
            if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
                this.setValueAt(nill, row++, valueColumn);
            } else {
                row++;
            }
            if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
                this.setValueAt(nill, row++, valueColumn);
            } else {
                row++;
            }
            if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
                this.setValueAt(nill, row++, valueColumn);
            } else {
                row++;
            }
        }
        OrderbyPolicy obp = qos.getOrderby();
        if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
            if(obp != null){
                this.setValueAt(obp.kind, row++, valueColumn);
            } else {
                this.setValueAt(nill, row++, valueColumn);
            }
        } else {
            row++;
        }
        HistoryPolicy hsp = qos.getHistory();
        
        if(hsp != null){
            if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
                this.setValueAt(hsp.kind, row++, valueColumn);
            } else {
                row++;
            }
            if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
                this.setValueAt(new Integer(hsp.depth), row++, valueColumn);
            } else {
                row++;
            }
        } else {
            if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
                this.setValueAt(nill, row++, valueColumn);
            } else {
                row++;
            }
            if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
                this.setValueAt(nill, row++, valueColumn);
            } else {
                row++;
            }
        }
        ResourcePolicy rsp = qos.getResource();
        
        if(rsp != null){
            if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
                this.setValueAt(new Integer(rsp.max_samples), row++, valueColumn);
            } else {
                row++;
            }
            if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
                this.setValueAt(new Integer(rsp.max_instances), row++, valueColumn);
            } else {
                row++;
            }
            if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
                this.setValueAt(new Integer(rsp.max_samples_per_instance), row++, valueColumn);
            } else {
                row++;
            }
        } else {
            if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
                this.setValueAt(nill, row++, valueColumn);
            } else {
                row++;
            }
            if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
                this.setValueAt(nill, row++, valueColumn);
            } else {
                row++;
            }
            if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
                this.setValueAt(nill, row++, valueColumn);
            } else {
                row++;
            }
        }
        
        UserDataPolicy udp = qos.getUserData();
        if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
            if(udp != null){
                this.setValueAt(udp, row++, valueColumn);
            } else {
                this.setValueAt(nill, row++, valueColumn);
            }
        } else {
            row++;
        }    
        OwnershipPolicy osp = qos.getOwnership();
        if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
            if(osp != null){
                this.setValueAt(osp.kind, row++, valueColumn);
            } else {
                this.setValueAt(nill, row++, valueColumn);
            }
        } else {
            row++;
        }
        PacingPolicy pcp = qos.getPacing();
        if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
            if(pcp != null){
                this.setValueAt(pcp.minSeperation, row++, valueColumn);
            } else {
                this.setValueAt(nill, row++, valueColumn);
            }
        } else {
            row++;
        }
        ReaderLifecyclePolicy rcp = qos.getLifecycle();
        
        if(rcp != null){
            if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
                this.setValueAt(rcp.autopurge_nowriter_samples_delay, row++, valueColumn);
            } else {
                row++;
            }
            if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
                this.setValueAt(rcp.autopurge_disposed_samples_delay, row++, valueColumn);
            } else {
                row++;
            }
            if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
                this.setValueAt(new Boolean(rcp.enable_invalid_samples), row++, valueColumn);
            } else {
                row++;
            }
        } else {
            if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
                this.setValueAt(nill, row++, valueColumn);
            } else {
                row++;
            }
            if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
                this.setValueAt(nill, row++, valueColumn);
            } else {
                row++;
            }
            if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
                this.setValueAt(Boolean.FALSE, row++, valueColumn);
            } else {
                row++;
            }
        }
        ReaderLifespanPolicy rlsp = qos.getLifespan();
        
        if(rcp != null){
            if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
                this.setValueAt(new Boolean(rlsp.used), row++, valueColumn);
            } else {
                row++;
            }
            if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
                this.setValueAt(rlsp.duration, row++, valueColumn);
            } else {
                row++;
            }
        } else {
            if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
                this.setValueAt(Boolean.FALSE, row++, valueColumn);
            } else {
                row++;
            }
            if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
                this.setValueAt(nill, row++, valueColumn);
            } else {
                row++;
            }
        }
        SharePolicy sh = qos.getShare();
        
        if(sh != null){
            if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
                if(sh.name == null){
                    this.setValueAt(nill, row++, valueColumn);
                } else {
                    this.setValueAt(sh.name, row++, valueColumn);
                }
            } else {
                row++;
            }  
            if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
                this.setValueAt(new Boolean(sh.enable), row++, valueColumn);
            } else {
                row++;
            }
        } else {
            if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
                this.setValueAt(nill, row++, valueColumn);
            } else {
                row++;
            }
            if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
                this.setValueAt(Boolean.FALSE, row++, valueColumn);
            } else {
                row++;
            }
        }
        UserKeyPolicy kp = qos.getUserKey();
        
        if(kp != null){
            if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
                this.setValueAt(new Boolean(kp.enable), row++, valueColumn);
            } else {
                row++;
            }
            if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
                if(kp.expression == null){
                    this.setValueAt(nill, row++, valueColumn);
                } else {
                    this.setValueAt(kp.expression, row++, valueColumn);
                }
            } else {
                row++;
            }
            
        } else {
            if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
                this.setValueAt(Boolean.FALSE, row++, valueColumn);
            } else {
                row++;
            }
            if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
                this.setValueAt(nill, row++, valueColumn);
            } else {
                row++;
            }
        }
        
        if (row == this.getRowCount()) {
            result = true;
            currentQos = qos.copy();
        } else {
            result = false;
        }
        return result;
    }
        

}
