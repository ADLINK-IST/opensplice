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
package org.opensplice.config.swing;

import java.util.ArrayList;

import javax.swing.SwingUtilities;
import javax.swing.table.DefaultTableModel;

import org.opensplice.common.util.ConfigModeIntializer;
import org.opensplice.config.data.DataAttribute;
import org.opensplice.config.data.DataConfiguration;
import org.opensplice.config.data.DataConfigurationListener;
import org.opensplice.config.data.DataElement;
import org.opensplice.config.data.DataNode;
import org.opensplice.config.data.DataValue;
import org.opensplice.config.meta.MetaAttribute;
import org.opensplice.config.meta.MetaElement;

public class DataElementTableModel extends DefaultTableModel implements DataConfigurationListener {
    private static final long serialVersionUID = -217503219724923467L;
    private DataElement element;
    private ArrayList<DataNode> nodes;
    private DataConfiguration configuration;
    
    public DataElementTableModel(){
        super();
        this.element = null;
        this.configuration = null;
        this.nodes = new ArrayList<DataNode>();
        this.addColumn("Name");
        this.addColumn("Value");
        
    }

    public void setElement(DataElement element){
        if(this.element != null){
            this.clear();
        }
        if(element != null){
            this.element = element;
            this.initElement();
            
            if(!this.element.getOwner().equals(this.configuration)){
                if(this.configuration != null){
                    this.configuration.removeDataConfigurationListener(this);
                }
                this.configuration = this.element.getOwner();
                this.configuration.addDataConfigurationListener(this);
            }
        }
    }
    
    public void nodeAdded(DataElement parent, DataNode nodeAdded) {
        DataNode parentParent;
        
        if(parent.equals(this.element)){
            this.clear();
            this.initElement();
        } else {
            parentParent = parent.getParent();
            
            if(this.element.equals(parentParent)){
                this.clear();
                this.initElement();
            }
        }
        
        /*else if(this.containsNodeAsParent(parent)){
            this.clear();
            this.initElement();
        }*/
    }

    public void nodeRemoved(DataElement parent, DataNode nodeRemoved) {
       if(nodeRemoved.equals(this.element)){
            this.setElement(null);
        } else if(this.nodes.contains(nodeRemoved)){
            this.clear();
            this.initElement();
        } else if(this.containsNodeAsParent(nodeRemoved)){
            this.clear();
            this.initElement();
        }
    }
    
    private boolean containsNodeAsParent(DataNode node){
        for(DataNode n: this.nodes){
            if(n.getParent().equals(node)){
                return true;
            }
        }
        return false;
    }

    public void valueChanged(DataValue data, Object oldValue, Object newValue) {
        final int index = this.nodes.indexOf(data); 
        final Object v = newValue;
        
        if(index != -1){
            /*System.out.println("Value changed: data: " + data + ", oldValue: " + oldValue + " newValue: " + newValue + "(row=" + index +", col=1)");*/
            
            SwingUtilities.invokeLater(new Runnable(){
                public void run() {
                    setValueAt(v, index, 1);
                }
            });
        }
    }
    
    public DataNode getNodeAt(int index){
        DataNode result;
        
        if(this.nodes.size() >= (index-1)){
            result = this.nodes.get(index);
        } else {
            result = null;
        }
        return result;
    }
    
    public boolean isCellEditable(int row, int column) {
        boolean result;
                
        if(column == 1 && ConfigModeIntializer.CONFIGURATOR_MODE == ConfigModeIntializer.COMMERCIAL_MODE) {
            result = true;
        } else if (column == 1 && !element.getMetadata().getVersion().equals(ConfigModeIntializer.COMMERCIAL)) {
            result = true;
        } else {
            result = false;
        }
        return result;
    }
    
    private void initElement(){
        String elName;
        Object[] values = new Object[2];
        
        for(DataNode dn: this.element.getChildren()){
            if(dn instanceof DataAttribute){
                values[0] = "@" + ((MetaAttribute)dn.getMetadata()).getName();
                values[1] = ((DataAttribute)dn).getValue();
                this.nodes.add(((DataAttribute)dn).getDataValue());
                this.addRow(values);
            }
        }
        for(DataNode dn: this.element.getChildren()){
            if(dn instanceof DataValue){
                values[0] = "";
                values[1] = ((DataValue)dn).getValue();
                this.nodes.add(dn);
                this.addRow(values);
            }
        }
        for(DataNode dn: this.element.getChildren()){
            if(dn instanceof DataElement){
                if(!((MetaElement)dn.getMetadata()).hasElementChildren() &&
                   ((MetaElement)dn.getMetadata()).hasValueChildren())
                {
                    elName = ((MetaElement)dn.getMetadata()).getName();
                    
                    for(DataNode elNode: ((DataElement)dn).getChildren()){
                        if(elNode instanceof DataValue){
                            values[0] = elName;
                            values[1] = ((DataValue)elNode).getValue();
                            this.nodes.add(elNode);
                            this.addRow(values);
                        } else if(elNode instanceof DataAttribute){
                            values[0] = elName + "[@" + ((MetaAttribute)elNode.getMetadata()).getName() + "]";
                            values[1] = ((DataAttribute)elNode).getValue();
                            this.nodes.add(((DataAttribute)elNode).getDataValue());
                            this.addRow(values);
                        }
                    }
                }
            }
        }
    }
    
    private void clear(){
        int rows = super.getRowCount();
        
        for(int i=0; i<rows; i++){
            super.removeRow(0);
        }
        this.nodes.clear();
        
        return;
    }
}
