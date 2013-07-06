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
import org.opensplice.cm.Subscriber;
import org.opensplice.cm.qos.EntityFactoryPolicy;
import org.opensplice.cm.qos.GroupDataPolicy;
import org.opensplice.cm.qos.PresentationPolicy;
import org.opensplice.cm.qos.SharePolicy;
import org.opensplice.cm.qos.SubscriberQoS;
import org.opensplice.common.CommonException;

/**
 * Concrete implementation of the EntityQoSTableModel that is capable of
 * resolving and administrating the QoS of a Subscriber (SubscriberQoS). 
 * 
 * @date Jan 10, 2005 
 */
public class SubscriberQoSTableModel extends EntityQoSTableModel {

    private static final long serialVersionUID   = 6432517140961348734L;
    private SubscriberQoS selectedDefaultQos = null;
    /**
     * Constructs a new table model that holds the QoS of the supplied
     * Subscriber.
     *
     * @param _entity The Subscriber, which QoS must be administrated.
     * @throws CommonException Thrown when the Entity is not available (anymore)
     */
    public SubscriberQoSTableModel(Subscriber _entity) throws CommonException {
        super(_entity);
    }

    public SubscriberQoSTableModel(SubscriberQoS sqos) throws CommonException {
        super(sqos);
        this.update();
    }

    public SubscriberQoSTableModel(SubscriberQoS sqos, boolean editable) {
        super(sqos, editable);
        selectedDefaultQos = sqos;
        this.addTableModelListener(new TableModelListener() {

            @Override
            public void tableChanged(TableModelEvent e) {
                if (e.getColumn() == 0) {
                    TableModel source = (TableModel) e.getSource();
                    if (source instanceof SubscriberQoSTableModel) {
                        if (((Boolean) ((SubscriberQoSTableModel) source).getValueAt(e.getFirstRow(), e.getColumn())) == true) {
                            ((SubscriberQoSTableModel) source).updateDefaultValues(selectedDefaultQos);
                        } else {
                            ((SubscriberQoSTableModel) source).updateDefaultValues((SubscriberQoS) currentQos);
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

            /* row 0 */
            data[0] = true;
            data[1] = "PRESENTATION";
            data[2] = "access_scope";
            data[3] = "N/A";
            this.addRow(data);
            nonEditRows.add(new Integer(0));

            /* row 1 */
            data[2] = "coherent_access";
            this.addRow(data);
            nonEditRows.add(new Integer(1));

            /* row 2 */
            data[2] = "ordered_access";
            this.addRow(data);
            nonEditRows.add(new Integer(2));

            /* row 3 */
            data[1] = "PARTITION";
            data[2] = "name";
            data[3] = "partition";
            this.addRow(data);

            /* row 4 */
            data[1] = "GROUP_DATA";
            data[2] = "value";
            this.addRow(data);

            /* row 5 */
            data[1] = "ENTITY_FACTORY";
            data[2] = "autoenable_created_entities";
            this.addRow(data);

            /* row 6 */
            data[1] = "SHARE";
            data[2] = "name";
            this.addRow(data);
            nonEditRows.add(new Integer(6));

            /* row 7 */
            data[2] = "enable";
            this.addRow(data);
            nonEditRows.add(new Integer(7));
        } else {
            Object[] data = new Object[3];

            /* row 0 */
            data[0] = "PRESENTATION";
            data[1] = "access_scope";
            data[2] = "N/A";
            this.addRow(data);
            nonEditRows.add(new Integer(0));

            /* row 1 */
            data[1] = "coherent_access";
            this.addRow(data);
            nonEditRows.add(new Integer(1));

            /* row 2 */
            data[1] = "ordered_access";
            this.addRow(data);
            nonEditRows.add(new Integer(2));

            /* row 3 */
            data[0] = "PARTITION";
            data[1] = "name";
            data[2] = "partition";
            this.addRow(data);

            /* row 4 */
            data[0] = "GROUP_DATA";
            data[1] = "value";
            this.addRow(data);

            /* row 5 */
            data[0] = "ENTITY_FACTORY";
            data[1] = "autoenable_created_entities";
            this.addRow(data);

            /* row 6 */
            data[0] = "SHARE";
            data[1] = "name";
            this.addRow(data);
            nonEditRows.add(new Integer(6));

            /* row 7 */
            data[1] = "enable";
            this.addRow(data);
            nonEditRows.add(new Integer(7));
        }
    }

    public void changeDefaultQos(SubscriberQoS qos) {
        selectedDefaultQos = qos;
        updateDefaultValues(qos);
    }

    public boolean updateDefaultValues(SubscriberQoS qos) {
        boolean result;
        
        int row = 0;
        String nill = "null";
        int valueColumn = this.getColumnCount() - 1;
        int checkBoxColumn = 0;
        
        PresentationPolicy prp = qos.getPresentation();
        if (prp != null) {
            if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
                this.setValueAt(prp.access_scope, row++, valueColumn);
            } else {
                row++;
            }
            if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
                this.setValueAt(new Boolean(prp.coherent_access), row++, valueColumn);
            } else {
                row++;
            }
            if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
                this.setValueAt(new Boolean(prp.ordered_access), row++, valueColumn);
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
            String p = qos.getPartition();
            if (p != null) {
                this.setValueAt(p, row++, valueColumn);
            } else {
                this.setValueAt(nill, row++, valueColumn);
            }
        } else {
            row++;
        }
        if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
            GroupDataPolicy gdp = qos.getGroupData();
            if (gdp != null) {
                this.setValueAt(gdp, row++, valueColumn);
            } else {
                this.setValueAt(nill, row++, valueColumn);
            }
        } else {
            row++;
        }
        if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
            EntityFactoryPolicy efp = qos.getEntityFactory();
            if (efp != null) {
                this.setValueAt(new Boolean(efp.autoenable_created_entities), row++, valueColumn);
            } else {
                this.setValueAt(Boolean.FALSE, row++, valueColumn);
            }
        } else {
            row++;
        }

        SharePolicy sh = qos.getShare();
        if (sh != null) {
            if ((Boolean) this.getValueAt(row, checkBoxColumn)) {
                if (sh.name == null) {
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
        boolean result;
        
        this.cancelEditing();
        int valueColumn = this.getColumnCount() - 1;
        try {
            int row = 0;
            String nill = "null";
            SubscriberQoS qos = null;
            if (entity != null) {
                qos = (SubscriberQoS) entity.getQoS();
                currentQos = qos;
            } else {
                qos = (SubscriberQoS) currentQos;
            }
            PresentationPolicy prp = qos.getPresentation();
            
            if(prp != null){
                this.setValueAt(prp.access_scope, row++, valueColumn);
                this.setValueAt(new Boolean(prp.coherent_access), row++, valueColumn);
                this.setValueAt(new Boolean(prp.ordered_access), row++, valueColumn);
            } else {
                this.setValueAt(nill, row++, valueColumn);
                this.setValueAt(nill, row++, valueColumn);
                this.setValueAt(nill, row++, valueColumn);
            }
            String p = qos.getPartition();
            
            if(p != null){
                this.setValueAt(p, row++, valueColumn);
            } else {
                this.setValueAt(nill, row++, valueColumn);
            }
            GroupDataPolicy gdp = qos.getGroupData();
            
            if(gdp != null){
                this.setValueAt(gdp, row++, valueColumn);
            } else {
                this.setValueAt(nill, row++, valueColumn);
            }
            EntityFactoryPolicy efp = qos.getEntityFactory();
            
            if(efp != null){
                this.setValueAt(new Boolean(efp.autoenable_created_entities), row++, valueColumn);
            } else {
                this.setValueAt(Boolean.FALSE, row++, valueColumn);
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
            assert row == this.getRowCount() : "#rows does not match filled rows.";
            result = true;
        } catch (CMException e) {
            result = false;
        }
        return result;
    }

}
