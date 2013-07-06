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
package org.opensplice.common.controller;

import java.awt.Color;
import java.awt.Component;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.util.ArrayList;
import java.util.StringTokenizer;

import javax.swing.AbstractCellEditor;
import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JTable;
import javax.swing.JTextField;
import javax.swing.table.TableCellEditor;

import org.opensplice.cm.Time;
import org.opensplice.cm.qos.DurabilityKind;
import org.opensplice.cm.qos.GroupDataPolicy;
import org.opensplice.cm.qos.HistoryQosKind;
import org.opensplice.cm.qos.LivelinessKind;
import org.opensplice.cm.qos.OrderbyKind;
import org.opensplice.cm.qos.OwnershipKind;
import org.opensplice.cm.qos.ParticipantQoS;
import org.opensplice.cm.qos.PresentationKind;
import org.opensplice.cm.qos.PublisherQoS;
import org.opensplice.cm.qos.QoS;
import org.opensplice.cm.qos.ReaderQoS;
import org.opensplice.cm.qos.ReliabilityKind;
import org.opensplice.cm.qos.ScheduleKind;
import org.opensplice.cm.qos.SchedulePriorityKind;
import org.opensplice.cm.qos.SubscriberQoS;
import org.opensplice.cm.qos.TopicDataPolicy;
import org.opensplice.cm.qos.TopicQoS;
import org.opensplice.cm.qos.UserDataPolicy;
import org.opensplice.cm.qos.WriterQoS;
import org.opensplice.common.model.table.qos.EntityQoSTableModel;
import org.opensplice.common.util.Config;
import org.opensplice.common.view.StatusPanel;
import org.opensplice.common.view.table.QoSTable;

/**
 * Editor for a QoSTable component. It allows a user to edit the QoS within
 * the QoSTable. 
 * 
 * @date Feb 2, 2005 
 */
public class QoSTableEditor extends AbstractCellEditor implements TableCellEditor, ActionListener, KeyListener {
    private Object curValue = null;
    private final Color editColor = Config.getInputColor();
    private final Color errorColor = Config.getIncorrectColor();
    private QoSTable view = null;
    private EntityQoSTableModel model = null;
    private StatusPanel status = null;
    private int editRow, editColumn;
    private Component curEditor = null;
    
    /**
     * Constructs a new QoSTableEditor.
     *
     * @param view The QoSTable where the editor is attached to.
     */
    public QoSTableEditor(QoSTable view){
        super();
        this.view = view;
        this.model = (EntityQoSTableModel)view.getModel();
    }
    
    /**
     * Stores the supplied object that will receive validation and assigment
     * information from now.
     * 
     * @param _status The component that will receive the information.
     */
    public void setStatusListener(StatusPanel  _status){
        status = _status;
    }
    
    @Override
    public Component getTableCellEditorComponent(JTable table, Object value, boolean isSelected, int row, int column) {
        Component result = null;
        
        editRow = row;
        editColumn = column;
        curValue = value;
        if (table.getModel().getColumnCount() > 3 && column == 0) {
            result = this.getCheckboxEditor(value);
        } else if (value instanceof Integer) {
            result = this.getTextFieldEditor(value);
        } else if(value instanceof Boolean){
            result = this.getBooleanEditor(value);
        } else if(value instanceof String){
            result = this.getTextFieldEditor(value);
        } else if(value instanceof Time){
            result = this.getTimeEditor(value);
        } else if(value instanceof DurabilityKind){
            result = this.getDurabilityKindEditor(value);
        } else if(value instanceof PresentationKind){
            result = this.getPresentationKindEditor(value);
        } else if(value instanceof LivelinessKind){
            result = this.getLivelinessKindEditor(value);
        } else if(value instanceof ReliabilityKind){
            result = this.getReliabilityKindEditor(value);
        } else if(value instanceof OwnershipKind){
            result = this.getOwnershipKindEditor(value);
        } else if(value instanceof HistoryQosKind){
            result = this.getHistoryQosKindEditor(value);
        } else if(value instanceof OrderbyKind){
            result = this.getOrderbyKindEditor(value);
        } else if(value instanceof ScheduleKind){
            result = this.getScheduleKindEditor(value);
        } else if(value instanceof SchedulePriorityKind){
            result = this.getSchedulePriorityKindEditor(value);
        } else {
            result = this.getTextFieldEditor(value);
        }
        curEditor = result;
        curEditor.setBackground(editColor);
        curEditor.addKeyListener(this);
        
        return result;
    }

    @Override
    public Object getCellEditorValue() {
        return curValue;
    }

    @Override
    public void actionPerformed(ActionEvent e) {
        if(e.getSource().equals(curEditor)){
            this.assign();
         }
    }

    /**
     * Called when the user releases a key on the keyboard. When currently 
     * editing a field, the input is validated. When a status listener is
     * attached status information is provided to that listener.
     * 
     * @param e The key event that occurred.
     */
    @Override
    public void keyReleased(KeyEvent e) {
        if(curEditor != null){
            if(e.getSource() instanceof JTextField){
                AssignmentResult test = this.testAssignment();
                
                if(test.isValid()){
                    curEditor.setBackground(editColor);
                    
                    if(status != null){
                        status.setStatus("Current input valid.", false, false);
                    }
                } else {
                    curEditor.setBackground(errorColor);
                    
                    if(status != null){
                        status.setStatus(test.getErrorMessage(), false, false);
                    }
                }
            } else if(e.getSource() instanceof JComboBox){
                /*Do nothing.*/
            }
        }       
    }
    
    /**
     * Validates whether the current value in the editor is valid.
     * 
     * @return Whether or not the value in the editor can be assigned.
     */
    public AssignmentResult testAssignment(){
        AssignmentResult result = new AssignmentResult(true, null);
        
        if(curEditor != null){
            try{
                if(curValue instanceof Integer){
                    JTextField source = (JTextField)curEditor;
                    int test = Integer.parseInt(source.getText());
                    source.setText(Integer.toString(test));
                } else if(curValue instanceof Time){
                    JTextField source = (JTextField)curEditor;
                    Time.fromString(source.getText());
                } else if(curValue instanceof TopicDataPolicy){
                    JTextField source = (JTextField)curEditor;
                    this.getByteArray(source.getText());
                } else if(curValue instanceof GroupDataPolicy){
                    JTextField source = (JTextField)curEditor;
                    this.getByteArray(source.getText());
                } else if(curValue instanceof UserDataPolicy){
                    JTextField source = (JTextField)curEditor;
                    this.getByteArray(source.getText());
                }
            } catch(NumberFormatException ne){
                result = new AssignmentResult(false, "Parsing failed: " + (ne.getMessage()).toLowerCase());
            }
        }
        return result;
    }
    
    /**
     * Called when editing has been cancelled.
     */
    @Override
    public void cancelCellEditing(){
        super.cancelCellEditing();
        curEditor = null;
        
        if(status != null){
            status.setStatus("Editing cancelled", false, false);
        }
    }
    
    /**
     * Called when editing has been stopped.
     */
    @Override
    public boolean stopCellEditing(){
        boolean result = true;
        
        if(curEditor != null){
            result = this.assign().isValid();
        }
        if(result){
            result = super.stopCellEditing();
            curEditor = null;
        }
        return result;
    }
    
    /**
     * Does nothing.
     */
    @Override
    public void keyTyped(KeyEvent e) {}
    
    /**
     * Does nothing.
     */
    @Override
    public void keyPressed(KeyEvent e) {}

    public boolean isEditing(){
        return (curEditor != null);
    }
    
    private AssignmentResult assign(){
        String name, field;
        Object value = null;
        AssignmentResult test = this.testAssignment();
        
        if(test.isValid()){
            if(status != null){
                status.setStatus("Input valid.", false, false);
            }
            if(curEditor instanceof JTextField){
                value = ((JTextField)curEditor).getText();
            } else if(curEditor instanceof JComboBox){
                value = ((JComboBox)curEditor).getSelectedItem();
            }
            QoS qos = model.getQoS();
            name = (String)model.getValueAt(editRow, editColumn-2);
            field = (String)model.getValueAt(editRow, editColumn-1);
            
            //model.setUserDataField( (String)model.getValueAt(editRow, editColumn-1), (String)curValue);
            
            if(qos instanceof ParticipantQoS){
                curValue = this.assignParticipant(name, field, value);
            } else if(qos instanceof PublisherQoS){
                curValue = this.assignPublisher(name, field, value);
            } else if(qos instanceof SubscriberQoS){
                curValue = this.assignSubscriber(name, field, value);
            } else if(qos instanceof TopicQoS){
                curValue = this.assignTopic(name, field, value);
            } else if(qos instanceof ReaderQoS){
                curValue = this.assignReader(name, field, value);
            } else if(qos instanceof WriterQoS){
                curValue = this.assignWriter(name, field, value);
            }
            model.setValueAt(curValue, editRow, editColumn);
            curEditor.removeKeyListener(this);
            curEditor = null;
            fireEditingStopped();
        } else if(status != null){
            status.setStatus("Error: Invalid input " + test.getErrorMessage().toLowerCase(), false, false);
        }
        return test;
    }
    
    private Component getTimeEditor(Object value){
        return this.getTextFieldEditor(value);
    }
    
    private Component getCheckboxEditor(Object value) {
        JCheckBox result = new JCheckBox();
        result.setSelected((Boolean) value);
        return result;
    }

    private Component getBooleanEditor(Object value){
        Object bool[] = {Boolean.TRUE, Boolean.FALSE};
        
        return this.getEnumEditor(value, bool);
    }
    
    private Component getTextFieldEditor(Object value){
        JTextField result = null;
        result = new JTextField();
        result.setText(value.toString());
        
        return result;
    }
    
    private Component getEnumEditor(Object defaultValue, Object[] values){
        JComboBox result = new JComboBox(values);
        result.setSelectedItem(defaultValue);
        
        return result;
    }
    
    private Component getDurabilityKindEditor(Object value){
        Object[] values = { DurabilityKind.VOLATILE, 
                            DurabilityKind.TRANSIENT_LOCAL,
                            DurabilityKind.TRANSIENT, 
                            DurabilityKind.PERSISTENT};
        return this.getEnumEditor(value, values);
    }
    
    private Component getPresentationKindEditor(Object value){
        Object[] values = { PresentationKind.INSTANCE, 
                            PresentationKind.TOPIC,
                            PresentationKind.GROUP};
        return this.getEnumEditor(value, values);
    }
    
    private Component getLivelinessKindEditor(Object value){
        Object[] values = { LivelinessKind.AUTOMATIC, 
                            LivelinessKind.PARTICIPANT,
                            LivelinessKind.TOPIC};
        return this.getEnumEditor(value, values);
    }
    
    private Component getReliabilityKindEditor(Object value){
        Object[] values = { ReliabilityKind.BESTEFFORT,
                            ReliabilityKind.RELIABLE};
        return this.getEnumEditor(value, values);
    }
    
    private Component getOwnershipKindEditor(Object value){
        Object[] values = { OwnershipKind.SHARED,
                            OwnershipKind.EXCLUSIVE};
        return this.getEnumEditor(value, values);
    }
    
    private Component getHistoryQosKindEditor(Object value){
        Object[] values = { HistoryQosKind.KEEPLAST,
                            HistoryQosKind.KEEPALL};
        return this.getEnumEditor(value, values);
    }
    
    private Component getOrderbyKindEditor(Object value){
        Object[] values = { OrderbyKind.BY_RECEPTION_TIMESTAMP,
                            OrderbyKind.BY_SOURCE_TIMESTAMP};
        return this.getEnumEditor(value, values);
    }
    
    private Component getScheduleKindEditor(Object value){
        Object[] values = {ScheduleKind.DEFAULT,
                           ScheduleKind.TIMESHARING,
                           ScheduleKind.REALTIME };
        return this.getEnumEditor(value, values);
    }
    
    private Component getSchedulePriorityKindEditor(Object value){
        Object[] values = {SchedulePriorityKind.RELATIVE,
                           SchedulePriorityKind.ABSOLUTE };
        return this.getEnumEditor(value, values);
    }
    
    private byte[] getByteArray(String value) throws NumberFormatException {
        byte[] result = null;
        
        if (value == null || value.length() < 3) {
            throw new NumberFormatException("Value not valid.");
        } else if("NULL".equalsIgnoreCase(value)){
            /*ok; do nothing.*/
        } else if(!(value.startsWith("["))){
            throw new NumberFormatException("Value must start with '['");
        } else if(!(value.endsWith("]"))){
            throw new NumberFormatException("Value must end with ']'");
        } else {
            value = value.replaceAll(" ", "");
            String token;
            boolean prevComma = true;
            String val = value.substring(1, value.length()-1);
            StringTokenizer tokenizer = new StringTokenizer(val, ",", true);
            ArrayList list = new ArrayList();
            
            
            while(tokenizer.hasMoreTokens()){
                token = tokenizer.nextToken();
                
                if(prevComma){
                    Byte.parseByte(token);
                    list.add(token);
                    prevComma = false;
                } else {
                    if(!(",".equals(token))){
                        throw new NumberFormatException("Comma's not valid");
                    }
                    prevComma = true;
                }
            }
            
            if(prevComma){
                throw new NumberFormatException("Comma's not valid");
            }
            result = new byte[list.size()];
            
            for(int i=0; i<result.length; i++){
                result[i] = Byte.parseByte((String)list.get(i));
            }
        }
        return result;
    }
    
    private Time getTime(String value){
        Time t = null;
        if (value != null) {
            t = Time.fromString(value);
        }
        return t;
    }
    
    private Object assignParticipant(String name, String field, Object value){
        Object result = null;
        ParticipantQoS qos = (ParticipantQoS)model.getQoS();
        
        if("ENTITY_FACTORY".equals(name)){
            if("autoenable_created_entities".equals(field)){
                if (value != null) {
                    if (value.equals(Boolean.TRUE)) {
                        qos.getEntityFactory().autoenable_created_entities = true;
                        result = Boolean.TRUE;
                    } else {
                        qos.getEntityFactory().autoenable_created_entities = false;
                        result = Boolean.FALSE;
                    }
                }
            }
        } else if("USER_DATA".equals(name)){
            if("value".equals(field)){
                byte[] ba = this.getByteArray((String)value);
                qos.getUserData().setValue(ba);
                result = qos.getUserData();
            }
        } else if("WATCHDOG_SCHEDULING".equals(name)){
            if("kind".equals(field)){
                qos.getWatchdogScheduling().kind = (ScheduleKind)value;
            } else if("priorityKind".equals(field)){
                qos.getWatchdogScheduling().priorityKind = (SchedulePriorityKind)value;
            } else if("priority".equals(field)){
                int i = Integer.parseInt((String)value);
                result = new Integer(i);
                qos.getWatchdogScheduling().priority = i; 
            }
        }
        return result;
    }
    
    private Object assignSubscriber(String name, String field, Object value){
        Object result = null;
        SubscriberQoS qos = (SubscriberQoS)model.getQoS();
        result = value;
        
        if("ENTITY_FACTORY".equals(name)){
            if("autoenable_created_entities".equals(field)){
                if (value != null) {
                    if (value.equals(Boolean.TRUE)) {
                        qos.getEntityFactory().autoenable_created_entities = true;
                    } else {
                        qos.getEntityFactory().autoenable_created_entities = false;
                    }
                }
            }
        } else if("GROUP_DATA".equals(name)){
            if("value".equals(field)){
                byte[] ba = this.getByteArray((String)value);
                qos.getGroupData().setValue(ba);
                result = qos.getGroupData();
            }
        } else if("PRESENTATION".equals(name)){
            if("access_scope".equals(field)){
                qos.getPresentation().access_scope = (PresentationKind)value;
            } else if("coherent_access".equals(field)){
                if (value != null) {
                    if (value.equals(Boolean.TRUE)) {
                        qos.getPresentation().coherent_access = true;
                    } else {
                        qos.getPresentation().coherent_access = false;
                    }
                }
            } else if("ordered_access".equals(field)){
                if (value != null) {
                    if (value.equals(Boolean.TRUE)) {
                        qos.getPresentation().ordered_access = true;
                    } else {
                        qos.getPresentation().ordered_access = false;
                    }
                }
            }
        } else if("PARTITION".equals(name)){
            if("name".equals(field)){
                qos.setPartition((String)value);
            }
        } else if("SHARE".equals(name)){
            if("enable".equals(field)){
                if (value != null) {
                    if (value.equals(Boolean.TRUE)) {
                        qos.getShare().enable = true;
                        result = Boolean.TRUE;
                    } else {
                        qos.getShare().enable = false;
                        result = Boolean.FALSE;
                    }
                }
            } else if("name".equals(field)){
                qos.getShare().name = (String)value;
                result = value;
            }
        }
        return result;
    }
    
    private Object assignPublisher(String name, String field, Object value){
        Object result = null;
        PublisherQoS qos = (PublisherQoS)model.getQoS();
        result = value;
        
        if("ENTITY_FACTORY".equals(name)){
            if("autoenable_created_entities".equals(field)){
                if (value != null) {
                    if (value.equals(Boolean.TRUE)) {
                        qos.getEntityFactory().autoenable_created_entities = true;
                    } else {
                        qos.getEntityFactory().autoenable_created_entities = false;
                    }
                }
            }
        } else if("GROUP_DATA".equals(name)){
            if("value".equals(field)){
                byte[] ba = this.getByteArray((String)value);
                qos.getGroupData().setValue(ba);
                result = qos.getGroupData();
            }
        } else if("PRESENTATION".equals(name)){
            if("access_scope".equals(field)){
                qos.getPresentation().access_scope = (PresentationKind)value;
            } else if("coherent_access".equals(field)){
                if (value != null) {
                    if (value.equals(Boolean.TRUE)) {
                        qos.getPresentation().coherent_access = true;
                    } else {
                        qos.getPresentation().coherent_access = false;
                    }
                }
            } else if("ordered_access".equals(field)){
                if (value != null) {
                    if (value.equals(Boolean.TRUE)) {
                        qos.getPresentation().ordered_access = true;
                    } else {
                        qos.getPresentation().ordered_access = false;
                    }
                }
            }
        } else if("PARTITION".equals(name)){
            if("name".equals(field)){
                qos.setPartition((String)value);
            }
        }
        return result;
    }
    
    private Object assignTopic(String name, String field, Object value){
        Object result = null;
        TopicQoS qos = (TopicQoS)model.getQoS();
        result = value;
        
        if("DURABILITY".equals(name)){
            if("kind".equals(field)){
                qos.getDurability().kind = (DurabilityKind)value;
            }
        } else if("DURABILITY_SERVICE".equals(name)){
            if("history_kind".equals(field)){
                qos.getDurabilityService().history_kind = (HistoryQosKind)value;
            } else if("service_cleanup_delay".equals(field)){
                result = this.getTime((String)value);
                qos.getDurabilityService().service_cleanup_delay = (Time)result; 
            } else if("history_depth".equals(field)){
                int i = Integer.parseInt((String)value);
                result = new Integer(i);
                qos.getDurabilityService().history_depth = i; 
            } else if("max_samples".equals(field)){
                int i = Integer.parseInt((String)value);
                result = new Integer(i);
                qos.getDurabilityService().max_samples = i; 
            } else if("max_instances".equals(field)){
                int i = Integer.parseInt((String)value);
                result = new Integer(i);
                qos.getDurabilityService().max_instances = i; 
            } else if("max_samples_per_instance".equals(field)){
                int i = Integer.parseInt((String)value);
                result = new Integer(i);
                qos.getDurabilityService().max_samples_per_instance = i; 
            }
        } else if("TOPIC_DATA".equals(name)){
            if("value".equals(field)){
                byte[] ba = this.getByteArray((String)value);
                qos.getTopicData().setValue(ba);
                result = qos.getTopicData();
            }
        } else if("DEADLINE".equals(name)){
            if("period".equals(field)){
                result = this.getTime((String)value);
                qos.getDeadline().period = (Time)result;
            }
        } else if("LATENCY_BUDGET".equals(name)){
            if("duration".equals(field)){
                result = this.getTime((String)value);
                qos.getLatency().duration = (Time)result;
            }
        } else if("LIVELINESS".equals(name)){
            if("kind".equals(field)){
                qos.getLiveliness().kind = (LivelinessKind)value;
            } else if("lease_duration".equals(field)){
                result = this.getTime((String)value);
                qos.getLiveliness().lease_duration = (Time)result; 
            }
        } else if("RELIABILITY".equals(name)){
            if("kind".equals(field)){
                qos.getReliability().kind = (ReliabilityKind)value;
            } else if("max_blocking_time".equals(field)){
                result = this.getTime((String)value);
                qos.getReliability().max_blocking_time = (Time)result; 
            }
        } else if("DESTINATION_ORDER".equals(name)){
            if("kind".equals(field)){
                qos.getOrderby().kind = (OrderbyKind)value;
            }
        } else if("HISTORY".equals(name)){
            if("kind".equals(field)){
                qos.getHistory().kind = (HistoryQosKind)value;
            } else if("depth".equals(field)){
                int i = Integer.parseInt((String)value);
                result = new Integer(i);
                qos.getHistory().depth = i; 
            }
        } else if("RESOURCE_LIMITS".equals(name)){
            if("max_samples".equals(field)){
                int i = Integer.parseInt((String)value);
                result = new Integer(i);
                qos.getResource().max_samples = i; 
            } else if("max_instances".equals(field)){
                int i = Integer.parseInt((String)value);
                result = new Integer(i);
                qos.getResource().max_instances = i; 
            } else if("max_samples_per_instance".equals(field)){
                int i = Integer.parseInt((String)value);
                result = new Integer(i);
                qos.getResource().max_samples_per_instance = i; 
            }
        } else if("TRANSPORT_PRIORITY".equals(name)){
            if("value".equals(field)){
                int i = Integer.parseInt((String)value);
                result = new Integer(i);
                qos.getTransport().value = i; 
            }
        } else if("LIFESPAN".equals(name)){
            if("duration".equals(field)){
                result = this.getTime((String)value);
                qos.getLifespan().duration = (Time)result;
            }
        } else if("OWNERSHIP".equals(name)){
            if("kind".equals(field)){
                qos.getOwnership().kind = (OwnershipKind)value;
            }
        }
        return result;
    }
    
    private Object assignReader(String name, String field, Object value){
        Object result = null;
        ReaderQoS qos = (ReaderQoS)model.getQoS();
        result = value;
        
        if("DURABILITY".equals(name)){
            if("kind".equals(field)){
                qos.getDurability().kind = (DurabilityKind)value;
            }
        } else if("DEADLINE".equals(name)){
            if("period".equals(field)){
                result = this.getTime((String)value);
                qos.getDeadline().period = (Time)result;
            }
        } else if("LATENCY_BUDGET".equals(name)){
            if("duration".equals(field)){
                result = this.getTime((String)value);
                qos.getLatency().duration = (Time)result;
            }
        } else if("LIVELINESS".equals(name)){
            if("kind".equals(field)){
                qos.getLiveliness().kind = (LivelinessKind)value;
            } else if("lease_duration".equals(field)){
                result = this.getTime((String)value);
                qos.getLiveliness().lease_duration = (Time)result; 
            }
        } else if("RELIABILITY".equals(name)){
            if("kind".equals(field)){
                qos.getReliability().kind = (ReliabilityKind)value;
            } else if("max_blocking_time".equals(field)){
                result = this.getTime((String)value);
                qos.getReliability().max_blocking_time = (Time)result; 
            }
        } else if("DESTINATION_ORDER".equals(name)){
            if("kind".equals(field)){
                qos.getOrderby().kind = (OrderbyKind)value;
            }
        } else if("HISTORY".equals(name)){
            if("kind".equals(field)){
                qos.getHistory().kind = (HistoryQosKind)value;
            } else if("depth".equals(field)){
                int i = Integer.parseInt((String)value);
                result = new Integer(i);
                qos.getHistory().depth = i; 
            }
        } else if("RESOURCE_LIMITS".equals(name)){
            if("max_samples".equals(field)){
                int i = Integer.parseInt((String)value);
                result = new Integer(i);
                qos.getResource().max_samples = i; 
            } else if("max_instances".equals(field)){
                int i = Integer.parseInt((String)value);
                result = new Integer(i);
                qos.getResource().max_instances = i; 
            } else if("max_samples_per_instance".equals(field)){
                int i = Integer.parseInt((String)value);
                result = new Integer(i);
                qos.getResource().max_samples_per_instance = i; 
            }
        } else if("USER_DATA".equals(name)){
            if("value".equals(field)){
                byte[] ba = this.getByteArray((String)value);
                qos.getUserData().setValue(ba);
                result = qos.getUserData();
            }
        } else if("OWNERSHIP".equals(name)){
            if("kind".equals(field)){
                qos.getOwnership().kind = (OwnershipKind)value;
            }
        } else if("TIME_BASED_FILTER".equals(name)){
            if("minimum_separation".equals(field)){
                result = this.getTime((String)value);
                qos.getPacing().minSeperation = (Time)result;
            }
        } else if("READER_DATA_LIFECYCLE".equals(name)){
            if("autopurge_nowriter_samples_delay".equals(field)){
                result = this.getTime((String)value);
                qos.getLifecycle().autopurge_nowriter_samples_delay = (Time)result;
            } else if("autopurge_disposed_samples_delay".equals(field)){
                result = this.getTime((String)value);
                qos.getLifecycle().autopurge_disposed_samples_delay = (Time)result;
            } else if("enable_invalid_samples".equals(field)){
                if (value != null) {
                    if (value.equals(Boolean.TRUE)) {
                        qos.getLifecycle().enable_invalid_samples = true;
                        result = Boolean.TRUE;
                    } else {
                        qos.getLifecycle().enable_invalid_samples = false;
                        result = Boolean.FALSE;
                    }
                }
            }
        } else if("READER_DATA_LIFESPAN".equals(name)){
            if("used".equals(field)){
                if (value != null) {
                    if (value.equals(Boolean.TRUE)) {
                        qos.getLifespan().used = true;
                        result = Boolean.TRUE;
                    } else {
                        qos.getLifespan().used = false;
                        result = Boolean.FALSE;
                    }
                }
            } else if("duration".equals(field)){
                result = this.getTime((String) value);
                qos.getLifespan().duration = (Time)result;
            }
        } else if("SHARE".equals(name)){
            if("enable".equals(field)){
                if (value != null) {
                    if (value.equals(Boolean.TRUE)) {
                        qos.getShare().enable = true;
                        result = Boolean.TRUE;
                    } else {
                        qos.getShare().enable = false;
                        result = Boolean.FALSE;
                    }
                }
            } else if("name".equals(field)){
                qos.getShare().name = (String)value;
                result = value;
            }
        } else if("USER_KEY".equals(name)){
            if("enable".equals(field)){
                if (value != null) {
                    if (value.equals(Boolean.TRUE)) {
                        qos.getUserKey().enable = true;
                        result = Boolean.TRUE;
                    } else {
                        qos.getUserKey().enable = false;
                        result = Boolean.FALSE;
                    }
                }
            } else if("expression".equals(field)){
                qos.getUserKey().expression = (String)value;
                result = value;
            }
        }
        return result;
    }
    
    private Object assignWriter(String name, String field, Object value){
        Object result = null;
        WriterQoS qos = (WriterQoS)model.getQoS();
        result = value;
        
        if("DURABILITY".equals(name)){
            if("kind".equals(field)){
                qos.getDurability().kind = (DurabilityKind)value;
            }
        } else if("DEADLINE".equals(name)){
            if("period".equals(field)){
                result = this.getTime((String)value);
                qos.getDeadline().period = (Time)result;
            }
        } else if("LATENCY_BUDGET".equals(name)){
            if("duration".equals(field)){
                result = this.getTime((String)value);
                qos.getLatency().duration = (Time)result;
            }
        } else if("LIVELINESS".equals(name)){
            if("kind".equals(field)){
                qos.getLiveliness().kind = (LivelinessKind)value;
            } else if("lease_duration".equals(field)){
                result = this.getTime((String)value);
                qos.getLiveliness().lease_duration = (Time)result; 
            }
        } else if("RELIABILITY".equals(name)){
            if("kind".equals(field)){
                qos.getReliability().kind = (ReliabilityKind)value;
            } else if("max_blocking_time".equals(field)){
                result = this.getTime((String)value);
                qos.getReliability().max_blocking_time = (Time)result; 
            }
        } else if("DESTINATION_ORDER".equals(name)){
            if("kind".equals(field)){
                qos.getOrderby().kind = (OrderbyKind)value;
            }
        } else if("HISTORY".equals(name)){
            if("kind".equals(field)){
                qos.getHistory().kind = (HistoryQosKind)value;
            } else if("depth".equals(field)){
                int i = Integer.parseInt((String)value);
                result = new Integer(i);
                qos.getHistory().depth = i; 
            }
        } else if("RESOURCE_LIMITS".equals(name)){
            if("max_samples".equals(field)){
                int i = Integer.parseInt((String)value);
                result = new Integer(i);
                qos.getResource().max_samples = i; 
            } else if("max_instances".equals(field)){
                int i = Integer.parseInt((String)value);
                result = new Integer(i);
                qos.getResource().max_instances = i; 
            } else if("max_samples_per_instance".equals(field)){
                int i = Integer.parseInt((String)value);
                result = new Integer(i);
                qos.getResource().max_samples_per_instance = i; 
            }
        } else if("TRANSPORT_PRIORITY".equals(name)){
            if("value".equals(field)){
                int i = Integer.parseInt((String)value);
                result = new Integer(i);
                qos.getTransport().value = i;
            }
        } else if("LIFESPAN".equals(name)){
            if("duration".equals(field)){
                result = this.getTime((String)value);
                qos.getLifespan().duration = (Time)result;
            }
        } else if("USER_DATA".equals(name)){
            if("value".equals(field)){
                byte[] ba = this.getByteArray((String)value);
                qos.getUserData().setValue(ba);
                result = qos.getUserData();
            }
        } else if("OWNERSHIP".equals(name)){
            if("kind".equals(field)){
                qos.getOwnership().kind = (OwnershipKind)value;
            }
        } else if("OWNERSHIP_STRENGTH".equals(name)){
            if("value".equals(field)){
                int i = Integer.parseInt((String)value);
                result = new Integer(i);
                qos.getStrength().value = i;
            }
        } else if("WRITER_DATA_LIFECYCLE".equals(name)){
            if("autodispose_unregistered_instances".equals(field)){
                if (value != null) {
                    if (value.equals(Boolean.TRUE)) {
                        qos.getLifecycle().autodispose_unregistered_instances = true;
                    } else {
                        qos.getLifecycle().autodispose_unregistered_instances = false;
                    }
                }
            } else if("autopurge_suspended_samples_delay".equals(field)){
                result = this.getTime((String)value);
                qos.getLifecycle().autopurge_suspended_samples_delay = (Time)result;
            } else if("autounregister_instance_delay".equals(field)){
                result = this.getTime((String)value);
                qos.getLifecycle().autounregister_instance_delay = (Time)result;
            }
        }
        return result;
    }
}
