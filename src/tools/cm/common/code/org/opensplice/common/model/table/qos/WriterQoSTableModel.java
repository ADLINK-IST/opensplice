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
import org.opensplice.cm.Writer;
import org.opensplice.cm.qos.DeadlinePolicy;
import org.opensplice.cm.qos.DurabilityPolicy;
import org.opensplice.cm.qos.HistoryPolicy;
import org.opensplice.cm.qos.LatencyPolicy;
import org.opensplice.cm.qos.LifespanPolicy;
import org.opensplice.cm.qos.LivelinessPolicy;
import org.opensplice.cm.qos.OrderbyPolicy;
import org.opensplice.cm.qos.OwnershipPolicy;
import org.opensplice.cm.qos.ReliabilityPolicy;
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

    private static final long serialVersionUID   = -4016075833458925954L;
    private TopicQoS topicQos = null;
    private boolean noUpdate = false;
    private WriterQoS selectedDefaultQos = null;
    
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

    public WriterQoSTableModel(WriterQoS writerQos, boolean editable) {
        super(writerQos, editable);
        selectedDefaultQos = writerQos;
        noUpdate = true;
        this.addTableModelListener(new TableModelListener() {

            @Override
            public void tableChanged(TableModelEvent e) {
                if (e.getColumn() == 0) {
                    TableModel source = (TableModel) e.getSource();
                    if (source instanceof WriterQoSTableModel) {
                        if (((Boolean) ((WriterQoSTableModel) source).getValueAt(e.getFirstRow(), e.getColumn())) == true) {
                            ((WriterQoSTableModel) source).updateDefaultValues(selectedDefaultQos);
                        } else {
                            ((WriterQoSTableModel) source).updateDefaultValues((WriterQoS) currentQos);
                        }
                    }
                }
            }
        });
        this.update();
    }

    @Override
    protected void init() {
        if (this.getColumnCount() > 3) {
            Object[] data = new Object[4];
            int row;

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
            data[1] = "TRANSPORT_PRIORITY";
            data[2] = "value";
            this.addRow(data);

            row++;
            data[1] = "LIFESPAN";
            data[2] = "duration";
            this.addRow(data);

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
            data[1] = "OWNERSHIP_STRENGTH";
            data[2] = "value";
            this.addRow(data);

            row++;
            data[1] = "WRITER_DATA_LIFECYCLE";
            data[2] = "autodispose_unregistered_instances";
            this.addRow(data);

            row++;
            data[2] = "autopurge_suspended_samples_delay";
            this.addRow(data);

            row++;
            data[2] = "autounregister_instance_delay";
            this.addRow(data);
        } else {
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
    }
    
    public void changeDefaultQos(WriterQoS qos) {
        selectedDefaultQos = qos;
        updateDefaultValues(qos);
    }

    public boolean updateDefaultValues(WriterQoS qos) {
        boolean result;
        Object nill = "null";
        int valueColumn = this.getColumnCount() - 1;
        int row = 0;
        int checkBoxColumn = 0;

        DurabilityPolicy dbp = qos.getDurability();
        if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
            if (dbp != null) {
                this.setValueAt(dbp.kind, row++, valueColumn);
            } else {
                this.setValueAt(nill, row++, valueColumn);
            }
        } else {
            row++;
        }

        if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
            DeadlinePolicy dlp = qos.getDeadline();
            if (dlp != null) {
                this.setValueAt(dlp.period, row++, valueColumn);
            } else {
                this.setValueAt(nill, row++, valueColumn);
            }
        } else {
            row++;
        }

        if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
            LatencyPolicy ltp = qos.getLatency();
            if (ltp != null) {
                this.setValueAt(ltp.duration, row++, valueColumn);
            } else {
                this.setValueAt(nill, row++, valueColumn);
            }
        } else {
            row++;
        }

        LivelinessPolicy llp = qos.getLiveliness();
        if (llp != null) {
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
        if (rlp != null) {
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

        if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
            OrderbyPolicy obp = qos.getOrderby();
            if (obp != null) {
                this.setValueAt(obp.kind, row++, valueColumn);
            } else {
                this.setValueAt(nill, row++, valueColumn);
            }
        } else {
            row++;
        }

        HistoryPolicy hsp = qos.getHistory();
        if (hsp != null) {
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
        if (rsp != null) {
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

        if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
            TransportPolicy tpp = qos.getTransport();
            if(tpp != null){
                this.setValueAt(new Integer(tpp.value), row++, valueColumn);
            } else {
                this.setValueAt(nill, row++, valueColumn);
            }
        } else {
            row++;
        }

        if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
            LifespanPolicy lsp = qos.getLifespan();
            if(lsp != null){
                this.setValueAt(lsp.duration, row++, valueColumn);
            } else {
                this.setValueAt(nill, row++, valueColumn);
            }
        } else {
            row++;
        }

        if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
            UserDataPolicy udp = qos.getUserData();
            if(udp != null){
                this.setValueAt(udp, row++, valueColumn);
            } else {
                this.setValueAt(nill, row++, valueColumn);
            }
        } else {
            row++;
        }

        if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
            OwnershipPolicy osp = qos.getOwnership();
            if(osp != null){
                this.setValueAt(osp.kind, row++, valueColumn);
            } else {
                this.setValueAt(nill, row++, valueColumn);
            }
        } else {
            row++;
        }

        if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
            StrengthPolicy stp = qos.getStrength();
            if(stp != null){
                this.setValueAt(new Integer(stp.value), row++, valueColumn);
            } else {
                this.setValueAt(nill, row++, valueColumn);
            }
        } else {
            row++;
        }

        WriterLifecyclePolicy wlp = qos.getLifecycle();
        if (wlp != null) {
            if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
                this.setValueAt(new Boolean(wlp.autodispose_unregistered_instances), row++, valueColumn);
            } else {
                row++;
            }
            if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
                this.setValueAt(wlp.autopurge_suspended_samples_delay, row++, valueColumn);
            } else {
                row++;
            }
            if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
                this.setValueAt(wlp.autounregister_instance_delay, row++, valueColumn);
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

        if (row == this.getRowCount()) {
            result = true;
            currentQos = qos.copy();
        } else {
            result = false;
        }
        return result;
    }

    @Override
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
            int valueColumn = this.getColumnCount() - 1;
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
            TransportPolicy tpp = qos.getTransport();
            
            if(tpp != null){
                this.setValueAt(new Integer(tpp.value), row++, valueColumn);
            } else {
                this.setValueAt(nill, row++, valueColumn);
            }
            LifespanPolicy lsp = qos.getLifespan();
            
            if(lsp != null){
                this.setValueAt(lsp.duration, row++, valueColumn);
            } else {
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
            StrengthPolicy stp = qos.getStrength();
            
            if(stp != null){
                this.setValueAt(new Integer(stp.value), row++, valueColumn);
            } else {
                this.setValueAt(nill, row++, valueColumn);
            }
            WriterLifecyclePolicy wlp = qos.getLifecycle();
            
            if(wlp != null){
                this.setValueAt(new Boolean(wlp.autodispose_unregistered_instances), row++, valueColumn);
                this.setValueAt(wlp.autopurge_suspended_samples_delay, row++, valueColumn);
                this.setValueAt(wlp.autounregister_instance_delay, row++, valueColumn);
            } else {
                this.setValueAt(nill, row++, valueColumn);
                this.setValueAt(nill, row++, valueColumn);
                this.setValueAt(nill, row++, valueColumn);
            }
            
            assert row == this.getRowCount(): "#rows does not match filled rows.";
            
            result = true;
        } catch (CMException e) {
            result = false;
        }
        return result;
    }

}
