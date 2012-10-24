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

    protected void init() {
        Object[] data = new Object[3];
        
        /*row 0*/
        data[0] = "PRESENTATION";
        data[1] = "access_scope";
        data[2] = "N/A";
        this.addRow(data);
        nonEditRows.add(new Integer(0));
        
        /*row 1*/
        data[1] = "coherent_access";
        this.addRow(data);
        nonEditRows.add(new Integer(1));
        /*row 2*/
        data[1] = "ordered_access";
        this.addRow(data);
        nonEditRows.add(new Integer(2));
        
        /*row 3*/
        data[0] = "PARTITION";
        data[1] = "name";
        this.addRow(data);
        
        /*row 4*/
        data[0] = "GROUP_DATA";
        data[1] = "value";
        this.addRow(data);
        
        /*row 5*/
        data[0] = "ENTITY_FACTORY";
        data[1] = "autoenable_created_entities";
        this.addRow(data);
    }

    public boolean update() {
        boolean result;
        
        this.cancelEditing();
        
        try {
            int row = 0;
            String nill = "null";
            
            PublisherQoS qos = (PublisherQoS)entity.getQoS();
            currentQos = qos;
            PresentationPolicy prp = qos.getPresentation();
            
            if(prp != null){
                this.setValueAt(prp.access_scope, row++, 2);
                this.setValueAt(new Boolean(prp.coherent_access), row++, 2);
                this.setValueAt(new Boolean(prp.ordered_access), row++, 2);
            } else {
                this.setValueAt(nill, row++, 2);
                this.setValueAt(nill, row++, 2);
                this.setValueAt(nill, row++, 2);
            }
            String p = qos.getPartition();
            
            if(p != null){
                this.setValueAt(p, row++, 2);
            } else {
                this.setValueAt(nill, row++, 2);
            }
            GroupDataPolicy gdp = qos.getGroupData();
            
            if(gdp != null){
                this.setValueAt(gdp, row++, 2);
            } else {
                this.setValueAt(nill, row++, 2);
            }
            EntityFactoryPolicy efp = qos.getEntityFactory();
            
            if(efp != null){
                this.setValueAt(new Boolean(efp.autoenable_created_entities), row++, 2);
            } else {
                this.setValueAt(Boolean.FALSE, row++, 2);
            }
            assert row == this.getRowCount() : "#rows does not match filled rows.";
            result = true;
        } catch (CMException e) {
            result = false;
        }
        return result;
    }
}
