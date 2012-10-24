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
package org.opensplice.common.view.table;

import javax.swing.JTable;
import javax.swing.table.TableColumnModel;
import javax.swing.table.TableModel;

import org.opensplice.cm.Entity;
import org.opensplice.cm.Participant;
import org.opensplice.cm.Publisher;
import org.opensplice.cm.Reader;
import org.opensplice.cm.Subscriber;
import org.opensplice.cm.Topic;
import org.opensplice.cm.Writer;
import org.opensplice.common.CommonException;
import org.opensplice.common.controller.QoSTableCellRenderer;
import org.opensplice.common.controller.QoSTableEditor;
import org.opensplice.common.model.table.qos.EntityQoSTableModel;
import org.opensplice.common.model.table.qos.ParticipantQoSTableModel;
import org.opensplice.common.model.table.qos.PublisherQoSTableModel;
import org.opensplice.common.model.table.qos.ReaderQoSTableModel;
import org.opensplice.common.model.table.qos.SubscriberQoSTableModel;
import org.opensplice.common.model.table.qos.TopicQoSTableModel;
import org.opensplice.common.model.table.qos.WriterQoSTableModel;
import org.opensplice.common.view.StatusPanel;

/**
 * Descendant of a default table that displays the QoS of a specific Entity. Not
 * all Entity types are supported, because not all Entity types have a QoS. 
 * Types that are supported are:
 * - Participant
 * - Topic
 * - Publisher
 * - Subscriber
 * - Reader
 * - Writer
 * 
 * @date Jan 11, 2005 
 */
public class QoSTable extends JTable {
    private QoSTableEditor editor = null;
    
    /**
     * Constructs a new QoSTable. 
     *  
     * @param entity The Entity, which QoS must be resolved and displayed.
     * @throws CommonException Thrown when:
     *                         - The Entity is not available anymore.
     *                         - The supplied Entity type is not supported.
     */
    public QoSTable(Entity entity, StatusPanel status) throws CommonException{
        super();
        this.setAutoResizeMode(JTable.AUTO_RESIZE_LAST_COLUMN);
        
        if(entity instanceof Participant){
            this.init((Participant)entity);
        } else if(entity instanceof Publisher){
            this.init((Publisher)entity);
        } else if(entity instanceof Subscriber){
            this.init((Subscriber)entity);
        }  else if(entity instanceof Topic){
            this.init((Topic)entity);
        } else if(entity instanceof Reader){
            this.init((Reader)entity);
        } else if(entity instanceof Writer){
            this.init((Writer)entity);
        } else {
            throw new CommonException("Supplied entity type not supported.");
        }
        this.doTableLayout(status);
    }
    
    public QoSTable(EntityQoSTableModel model, StatusPanel status) {
        super();
        this.setAutoResizeMode(JTable.AUTO_RESIZE_LAST_COLUMN);
        this.setModel(model);
        this.doTableLayout(status);
    }
    
    private void doTableLayout(StatusPanel status){
        TableModel model = this.getModel();
        TableColumnModel tcm = this.getColumnModel();
        
        if(model instanceof ParticipantQoSTableModel){
            tcm.getColumn(0).setPreferredWidth(165);
            tcm.getColumn(0).setMaxWidth(165);
            tcm.getColumn(1).setPreferredWidth(165);
            tcm.getColumn(1).setMaxWidth(165);
        } else if(model instanceof PublisherQoSTableModel){
            tcm.getColumn(0).setPreferredWidth(110);
            tcm.getColumn(0).setMaxWidth(110);
            tcm.getColumn(1).setPreferredWidth(165);
            tcm.getColumn(1).setMaxWidth(165);
        } else if(model instanceof SubscriberQoSTableModel){
            tcm.getColumn(0).setPreferredWidth(110);
            tcm.getColumn(0).setMaxWidth(110);
            tcm.getColumn(1).setPreferredWidth(165);
            tcm.getColumn(1).setMaxWidth(165);
        } else if(model instanceof TopicQoSTableModel){
            tcm.getColumn(0).setPreferredWidth(145);
            tcm.getColumn(0).setMaxWidth(145);
            tcm.getColumn(1).setPreferredWidth(170);
            tcm.getColumn(1).setMaxWidth(170);
        } else if(model instanceof ReaderQoSTableModel){
            tcm.getColumn(0).setPreferredWidth(165);
            tcm.getColumn(0).setMaxWidth(165);
            tcm.getColumn(1).setPreferredWidth(210);
            tcm.getColumn(1).setMaxWidth(210);
            tcm.getColumn(2).setPreferredWidth(170);
        } else if(model instanceof WriterQoSTableModel){
            tcm.getColumn(0).setPreferredWidth(160);
            tcm.getColumn(0).setMaxWidth(160);
            tcm.getColumn(1).setPreferredWidth(225);
            tcm.getColumn(1).setMaxWidth(225);
            tcm.getColumn(2).setPreferredWidth(170);
        }
        
        this.setSelectionMode(javax.swing.ListSelectionModel.SINGLE_SELECTION);
        this.setCellSelectionEnabled(false);
        this.getTableHeader().setReorderingAllowed(false);
        this.setSurrendersFocusOnKeystroke(true);
        editor = new QoSTableEditor(this);
        this.getColumnModel().getColumn(2).setCellEditor(editor);
        ((EntityQoSTableModel)this.getModel()).setEditor(editor);
        
        if(status != null){
            editor.setStatusListener(status);
        }
        QoSTableCellRenderer r = new QoSTableCellRenderer();
        this.getColumnModel().getColumn(0).setCellRenderer(r);
        this.getColumnModel().getColumn(1).setCellRenderer(r);
        this.getColumnModel().getColumn(2).setCellRenderer(r);
    }
    
    public void setStatusListener(StatusPanel  _status){
        editor.setStatusListener(_status);
    }
    
    /**
     * Updates the table by resolving the current QoS of the Entity and 
     * display it.
     *      
     * @return If successfully updated; true and false otherwise.
     */
    public boolean update(){
        EntityQoSTableModel tableModel = (EntityQoSTableModel)this.getModel();
        return tableModel.update();
    }
    
    /**
     * Triggers the model to apply the current visible QoS to the Entity.   
     * 
     * @throws CommonException Thrown when the application of the QoS failed.
     */
    public void applyCurrentQoS() throws CommonException{
        EntityQoSTableModel tableModel = (EntityQoSTableModel)this.getModel();
        tableModel.applyQoS();
    }
    
    
    private void init(Participant entity) throws CommonException{
        ParticipantQoSTableModel tableModel = new ParticipantQoSTableModel(entity);
        this.setModel(tableModel);        
    }
    
    private void init(Publisher entity) throws CommonException{
        PublisherQoSTableModel tableModel = new PublisherQoSTableModel(entity);
        this.setModel(tableModel);
    }
    
    private void init(Subscriber entity) throws CommonException{
        SubscriberQoSTableModel tableModel = new SubscriberQoSTableModel(entity);
        this.setModel(tableModel);
    }
    
    private void init(Topic entity) throws CommonException{
        TopicQoSTableModel tableModel = new TopicQoSTableModel(entity);
        this.setModel(tableModel);
    }
    
    private void init(Reader entity) throws CommonException{
        ReaderQoSTableModel tableModel = new ReaderQoSTableModel(entity);
        this.setModel(tableModel);
    }
    
    private void init(Writer entity) throws CommonException{
        WriterQoSTableModel tableModel = new WriterQoSTableModel(entity);
        this.setModel(tableModel);
    }
    
    public void setEnabled(boolean enabled){
        this.editor.cancelCellEditing();
        super.setEnabled(enabled);
    }
}
