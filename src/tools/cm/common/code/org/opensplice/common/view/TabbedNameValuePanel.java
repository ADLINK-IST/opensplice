/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
package org.opensplice.common.view;

import java.awt.Dimension;
import java.awt.event.ActionListener;
import java.awt.event.FocusEvent;
import java.awt.event.FocusListener;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;

import javax.swing.JComponent;
import javax.swing.JPanel;
import javax.swing.JTabbedPane;
import javax.swing.JTextField;

public class TabbedNameValuePanel extends JTabbedPane {

    public TabbedNameValuePanel(ActionListener controller, String action) {
        super();
        this.controller = controller;
        this.action = action;
        this.fieldDim = new Dimension(0,0);
        this.fieldNameMapping = new HashMap<String, NameValuePanel>();
    }
    
    public void addTab(String tabName, NameValuePanel[] tabFields){
        JPanel tabPanel = new JPanel();
        JPanel inputPanel = new JPanel();
        Dimension dim = new Dimension(10, 5);
        javax.swing.BoxLayout box = new javax.swing.BoxLayout(inputPanel, javax.swing.BoxLayout.Y_AXIS);
        java.awt.GridLayout layGridLayout = new java.awt.GridLayout();
        layGridLayout.setColumns(1);
        layGridLayout.setHgap(0);
        layGridLayout.setVgap(5);
        
        if(tabFields != null){
            layGridLayout.setRows(tabFields.length);
        }
        
        inputPanel.setLayout(box);
        tabPanel.setLayout(new java.awt.BorderLayout());
        
        if(tabFields != null){
            
            for(int i=0; i<tabFields.length ; i++){
                String label = tabFields[i].getName();
                fieldNameMapping.put(tabName + "." + label, tabFields[i]);
                JComponent editor = tabFields[i].getField();
                 
                if(editor instanceof JTextField){
                    ((JTextField)editor).setActionCommand(action);
                    ((JTextField)editor).addActionListener(controller);
                    editor.addFocusListener(new FocusListener() {

                        @Override
                        public void focusGained(FocusEvent e) {
                            JTextField source = (JTextField)(e.getSource());
                            source.selectAll();
                        }

                        @Override
                        public void focusLost(FocusEvent e) {
                            JTextField source = (JTextField)(e.getSource());
                            source.select(0, 0);
                        }
                    });
                }
                inputPanel.add(tabFields[i]);
                
                if(i<(tabFields.length-1)){
                    JPanel gap = new JPanel();
                    gap.setMinimumSize(dim);
                    gap.setPreferredSize(dim);
                    gap.setMaximumSize(dim);
                    inputPanel.add(gap);
                }
            }
        }
        tabPanel.add(inputPanel, java.awt.BorderLayout.NORTH);
        this.addTab(tabName, tabPanel);
        this.repaint();
    }
    
    /**
     * Provides access to all values in the input fields.
     * 
     * @return The map of values. The key is the name of the field <fieldName, value>.
     */
    public HashMap<String, NameValuePanel> getValues() {
        String key;
        HashMap<String, NameValuePanel> values = new HashMap<String, NameValuePanel>();
        
        Iterator<String> ki = fieldNameMapping.keySet().iterator();
        while(ki.hasNext()){
            key = ki.next();
            values.put(key, fieldNameMapping.get(key));
        }
        return values;
    }
    
    /**
     * Provides access to the value of a field.
     * 
     * @param fieldName The fieldName of the field to get the value of.
     * @return The value of the field.
     */
    public Object getValue(String fieldName){
        NameValuePanel nv = fieldNameMapping.get(fieldName);
        
        if(nv == null){
            return "";
        }
        return nv.getValue();
    }
    
    /**
     * Checks if all fields have compatible values in.
     * 
     * @return true if all fields have a compatible value, false otherwise.
     */
    public boolean isInputValid(){
        NameValuePanel nvp = null;
        Object test = null;
        Collection<NameValuePanel> f = fieldNameMapping.values();
        Iterator<NameValuePanel> fIter = f.iterator();
        
        while(fIter.hasNext()){
            nvp = fIter.next();
            test = nvp.getValue();
            if(test == null || test.equals("")){
                if(!(nvp.isEmptyInputAllowed())){
                    return false;
                } 
            }
        }
        return true;
    }

    public NameValuePanel getField(String name){
        return fieldNameMapping.get(name);
    }
    
    public NameValuePanel[] getFields(){
        return fieldNameMapping.values().toArray(
                new NameValuePanel[fieldNameMapping.size()]);
    }
    
    protected Dimension fieldDim = null;
    

    protected ActionListener controller = null;
    
    protected String action = null;
    
    /**
     * Fields in the tabs of window <String, NameValuePanel>
     */
    private HashMap<String, NameValuePanel> fieldNameMapping = null;
}
