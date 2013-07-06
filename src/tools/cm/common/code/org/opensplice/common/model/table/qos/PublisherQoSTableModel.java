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
import org.opensplice.cm.Publisher;
import org.opensplice.cm.qos.EntityFactoryPolicy;
import org.opensplice.cm.qos.GroupDataPolicy;
import org.opensplice.cm.qos.PresentationPolicy;
import org.opensplice.cm.qos.PublisherQoS;
import org.opensplice.common.CommonException;

/**
 * Concrete implementation of the EntityQoSTableModel that is capable of
 * resolving and administrating the QoS of a Publisher (PublisherQoS).
 * 
 * @date Jan 10, 2005 
 */
public class PublisherQoSTableModel extends EntityQoSTableModel {

    private static final long serialVersionUID   = -4953452240478853549L;
    private PublisherQoS selectedDefaultQos = null;
    /**
     * Constructs a new table model that holds the QoS of the supplied
     * Publisher.
     *
     * @param _entity The Publisher, which QoS must be administrated.
     * @throws CommonException Thrown when the Entity is not available (anymore)
     */
    public PublisherQoSTableModel(Publisher _entity) throws CommonException {
        super(_entity);
    }

    public PublisherQoSTableModel(PublisherQoS pqos) throws CommonException {
        super(pqos);
        this.update();
    }

    public PublisherQoSTableModel(PublisherQoS pqos, boolean editable) {
        super(pqos, editable);
        selectedDefaultQos = pqos;
        this.addTableModelListener(new TableModelListener() {

            @Override
            public void tableChanged(TableModelEvent e) {
                if (e.getColumn() == 0) {
                    TableModel source = (TableModel) e.getSource();
                    if (source instanceof PublisherQoSTableModel) {
                        if (((Boolean) ((PublisherQoSTableModel) source).getValueAt(e.getFirstRow(), e.getColumn())) == true) {
                            ((PublisherQoSTableModel) source).updateDefaultValues(selectedDefaultQos);
                        } else {
                            ((PublisherQoSTableModel) source).updateDefaultValues((PublisherQoS) currentQos);
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
            this.addRow(data);

            /* row 4 */
            data[1] = "GROUP_DATA";
            data[2] = "value";
            this.addRow(data);

            /* row 5 */
            data[1] = "ENTITY_FACTORY";
            data[2] = "autoenable_created_entities";
            this.addRow(data);
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
            this.addRow(data);

            /* row 4 */
            data[0] = "GROUP_DATA";
            data[1] = "value";
            this.addRow(data);

            /* row 5 */
            data[0] = "ENTITY_FACTORY";
            data[1] = "autoenable_created_entities";
            this.addRow(data);
        }
    }

    public void changeDefaultQos(PublisherQoS qos) {
        selectedDefaultQos = qos;
        updateDefaultValues(qos);
    }

    public boolean updateDefaultValues(PublisherQoS qos) {
        boolean result;
        Object nill = "null";
        int valueColumn = this.getColumnCount() - 1;
        int row = 0;
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
        int valueColumn = this.getColumnCount() - 1;
        
        this.cancelEditing();
        
        try {
            int row = 0;
            String nill = "null";
            
            PublisherQoS qos = null;
            if (entity != null) {
                qos = (PublisherQoS) entity.getQoS();
                currentQos = qos;
            } else {
                qos = (PublisherQoS) currentQos;
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
            assert row == this.getRowCount() : "#rows does not match filled rows.";
            result = true;
        } catch (CMException e) {
            result = false;
        }
        return result;
    }
}
