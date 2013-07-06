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

import org.opensplice.cm.CMException;
import org.opensplice.cm.Participant;
import org.opensplice.cm.qos.EntityFactoryPolicy;
import org.opensplice.cm.qos.ParticipantQoS;
import org.opensplice.cm.qos.SchedulePolicy;
import org.opensplice.cm.qos.UserDataPolicy;
import org.opensplice.common.CommonException;

/**
 * Concrete implementation of the EntityQoSTableModel that is capable of
 * resolving and administrating the QoS of a Participant (ParticipantQoS).
 * 
 * @date Jan 10, 2005 
 */
public class ParticipantQoSTableModel extends EntityQoSTableModel {

    private static final long serialVersionUID = 2956805238026540335L;

    /**
     * Constructs a new table model that holds the QoS of the supplied
     * Participant.
     *
     * @param _entity The Participant, which QoS must be administrated.
     * @throws CommonException Thrown when the Entity is not available (anymore)
     */
    public ParticipantQoSTableModel(Participant _entity) throws CommonException {
        super(_entity);
    }

    @Override
    protected void init() {
        Object[] data = new Object[3];
        
        /*row 0*/
        data[0] = "ENTITY_FACTORY";
        data[1] = "autoenable_created_entities";
        data[2] = "N/A";
        this.addRow(data);
        
        /*row 1*/
        data[0] = "USER_DATA";
        data[1] = "value";
        this.addRow(data);
        
        /*row 2*/
        data[0] = "WATCHDOG_SCHEDULING";
        data[1] = "kind";
        this.addRow(data);
        nonEditRows.add(new Integer(2));
        
        /*row 3*/
        data[1] = "priorityKind";
        this.addRow(data);
        nonEditRows.add(new Integer(3));
        
        /*row 4*/
        data[1] = "priority";
        this.addRow(data);
        nonEditRows.add(new Integer(4));
    }

    @Override
    public boolean update() {
        boolean result;
        
        this.cancelEditing();
        
        try {
            int row = 0;
            String nill = "null";
            
            ParticipantQoS qos = (ParticipantQoS)entity.getQoS();
            currentQos = qos;
            EntityFactoryPolicy efp = qos.getEntityFactory();
            
            if(efp != null){
                this.setValueAt(new Boolean(efp.autoenable_created_entities), row++, 2);
            } else {
                this.setValueAt(Boolean.FALSE, row++, 2);
            }
            UserDataPolicy udp = qos.getUserData();
            
            if(udp != null){
                this.setValueAt(udp, row++, 2);
            } else {
                this.setValueAt(nill, row++, 2);
            }
            SchedulePolicy wsp = qos.getWatchdogScheduling();
            
            if(wsp != null){
                this.setValueAt(wsp.kind, row++, 2);
                this.setValueAt(wsp.priorityKind, row++, 2);
                this.setValueAt(new Integer(wsp.priority), row++, 2);
            } else {
                this.setValueAt(nill, row++, 2);
                this.setValueAt(nill, row++, 2);
                this.setValueAt(nill, row++, 2);
            }
            
            assert row == this.getRowCount() : "#rows does not match filled rows.";
            result = true;
        } catch (CMException e) {
            result = false;
        }
        return result;
    }
}
