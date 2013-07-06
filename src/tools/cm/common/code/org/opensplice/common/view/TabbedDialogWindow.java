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
package org.opensplice.common.view;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.util.HashMap;

import javax.swing.JPanel;

public class TabbedDialogWindow extends DialogWindow {
    public TabbedDialogWindow(ActionListener _controller, TabbedNameValuePanel tabPanel, String _action, String _title) {
        super(_controller, _action, _title);
        field = tabPanel;
        this.setContentPane(getJContentPane());
        this.getOkButton().setDefaultCapable(true);
        this.pack();
    }
    
    /**
     * This method initializes the dialog window. 
     */
    void initialize() {
        this.setSize(1200, 1600);
        this.setResizable(false);
        this.setTitle(title);
        this.addWindowListener(new WindowAdapter() {
            public void windowClosing(WindowEvent e) {
                controller.actionPerformed(
                    new ActionEvent(cancelButton, 0, "cancel"));
            }
        });
    }
    
    /**
     * This method initializes the root content pane
     * 
     * @return The created or already existing content pane.
     */
    JPanel getJContentPane() {
        if (jContentPane == null) {
            jContentPane = new JPanel();
            jContentPane.setLayout(new java.awt.BorderLayout());
            jContentPane.add(field, java.awt.BorderLayout.NORTH);
            jContentPane.add(getButtonPanel(), java.awt.BorderLayout.CENTER);
            jContentPane.add(getStatusPanel(), java.awt.BorderLayout.SOUTH);
        }
        return jContentPane;
    }
    
    /**
     * Provides access to all values in the input fields.
     * 
     * @return The map of values. The key is the name of the field <fieldName, value>.
     */
    public HashMap getValues(){
        HashMap values = new HashMap();
        
        for(int i=0; i<fields.length; i++){
            values.put(fields[i].getName(), fields[i].getValue());
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
        NameValuePanel nv = (NameValuePanel)(field.getField(fieldName));
        
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
        NameValuePanel nvp    = null;
        Object test           = null;
        NameValuePanel[] nvps = field.getFields();
        
        for(int i=0; i<nvps.length; i++){
            nvp = nvps[i];
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
        return (NameValuePanel)(field.getField(name));
    }
    
    public NameValuePanel[] getFields(){
        return field.getFields();
    }
    
    /**
     * The name/value pairs that are shown in the window.
     */
    private TabbedNameValuePanel field       = null;  
    
}
