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
/**
 * Contains all SPLICE DDS C&M Tooling common view components. 
 */
package org.opensplice.common.view;

import javax.swing.JComponent;
import javax.swing.JFrame;
import javax.swing.JTextField;
import javax.swing.JPanel;
import javax.swing.JButton;

import java.awt.FlowLayout;
import java.awt.event.*;
import java.util.*;

/**
 * This class represents a generic dialog window and is meant to provide a fast and
 * simple way of constructing dialog windows. 
 * 
 * The window contains a status bar and
 * 'OK' and 'Cancel' buttons. By providing NameValuePanel objects, the user can
 * fill the dialog window with as many fields as he wishes. 
 */
public class DialogWindow extends JFrame {    
    /**
     * The constructor of this object. It initializes the window.
     * 
     * @param _controller The controller where to sent actions to. 'cancel' 
     * for the Cancel button and _action for the OK button
     * @param _fields Array of name/value pairs that must be showed in the dialog window.
     * @param _action The action that the controller expects when the user clicks th OK button.
     * @param _title The title of the dialog window that will be shown in the top.
     */
    public DialogWindow(ActionListener _controller, NameValuePanel[] _fields, String _action, String _title) {
        super();
        controller = _controller;
        action = _action;
        fields = _fields;
        title = _title;
        fieldNameMapping = new HashMap(_fields.length);
        initialize();
    }
    
    DialogWindow(ActionListener _controller, String _action, String _title){
        super();
        controller = _controller;
        action = _action;
        title = _title;
        fields = null;
        fieldNameMapping = new HashMap();
        initialize();
    }
    
    /**
     * This method initializes the dialog window. 
     */
    void initialize() {
        this.setSize(1200, 1600);
        this.setContentPane(getJContentPane());
        this.setResizable(false);
        this.setTitle(title);
        this.addWindowListener(new WindowAdapter() {
            public void windowClosing(WindowEvent e) {
                controller.actionPerformed(
                    new ActionEvent(cancelButton, 0, "cancel"));
            }
        });
        this.pack();
    }
    
    /**
     * This method initializes the root content pane
     * 
     * @return The created or already existing content pane.
     */
    JPanel getJContentPane() {
        if (jContentPane == null) {
            jContentPane = new JPanel();
            java.awt.GridLayout layGridLayout15 = new java.awt.GridLayout();
            
            if(fields != null){
                layGridLayout15.setRows(fields.length);    
            }
            
            layGridLayout15.setColumns(1);
            layGridLayout15.setHgap(0);
            layGridLayout15.setVgap(5);
            JPanel inputPanel = new JPanel();
            inputPanel.setLayout(layGridLayout15);
            jContentPane.setLayout(new java.awt.BorderLayout());
            
            if(fields != null){
                for(int i=0; i<fields.length; i++){
                    String label = fields[i].getName();
                    JComponent editor = fields[i].getField();
                     
                    if(editor instanceof JTextField){
                        ((JTextField)editor).setActionCommand(action);
                        ((JTextField)editor).addActionListener(controller);
                        editor.addFocusListener(new FocusListener() {

                            public void focusGained(FocusEvent e) {
                                JTextField source = (JTextField)(e.getSource());
                                source.selectAll();
                            }
                            public void focusLost(FocusEvent e) {
                                JTextField source = (JTextField)(e.getSource());
                                source.select(0, 0);
                            }
                        });
                    }
                    fieldNameMapping.put(label, fields[i]);
                    inputPanel.add(fields[i], null);  
                }
            }
            jContentPane.add(inputPanel, java.awt.BorderLayout.NORTH);
            jContentPane.add(getButtonPanel(), java.awt.BorderLayout.CENTER);
            jContentPane.add(getStatusPanel(), java.awt.BorderLayout.SOUTH);
        }
        return jContentPane;
    }
    
	/**
     * This method initializes the status panel
	 * 
	 * @return The created or already existing status panel.
	 */
	StatusPanel getStatusPanel() {
		if(statusPanel == null) {
			statusPanel = new StatusPanel(250, "Please provide input.", false, false);
		}
		return statusPanel;
	}
	
	/**
     * This method initializes the buttonPanel that contains the OK and Cancel button.
	 * 
	 * @return The created or already existing buttonPanel.
	 */
	JPanel getButtonPanel() {
		if(buttonPanel == null) {
			buttonPanel = new JPanel();
            FlowLayout layFlowLayout = new FlowLayout(FlowLayout.RIGHT);
            buttonPanel.setLayout(layFlowLayout);
            buttonPanel.add(getOkButton(), null);
			buttonPanel.add(getCancelButton(), null);
		}
		return buttonPanel;
	}
    
	/**
     * This method initializes the cancelButton
	 * 
	 * @return The created or already existing Cancel button.
	 */
	JButton getCancelButton() {
		if(cancelButton == null) {
			cancelButton = new JButton();
			cancelButton.setText("Cancel");
            cancelButton.setMnemonic('C');
            cancelButton.setActionCommand("cancel");
            cancelButton.addActionListener(controller);
            cancelButton.setPreferredSize(new java.awt.Dimension(100, 20));
		}
		return cancelButton;
	}
    
	/**
     * This method initializes the OK button.
	 * 
	 * @return The created or already existing OK button.
	 */
	JButton getOkButton() {
		if(okButton == null) {
			okButton = new JButton();
			okButton.setText("Ok");
            okButton.setMnemonic('O');
            okButton.setActionCommand(action);
            okButton.addActionListener(controller);
            okButton.setPreferredSize(new java.awt.Dimension(100, 20));
            this.getRootPane().setDefaultButton(okButton);
		}
		return okButton;
	}
    
    /**
     * Sets the status message in the status panel of the dialog window.
     * 
     * @param message The message to show in the status panel.
     * @param persistent true if the message should be shown until the next
     * call to this operation, false if it should disappear after a few seconds.
     */
    public void setStatus(String message, boolean persistent){
        statusPanel.setStatus(message, persistent);
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
        NameValuePanel nv = (NameValuePanel)(fieldNameMapping.get(fieldName));
        
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
        NameValuePanel nvp  = null;
        Object test         = null;
        Collection f        = fieldNameMapping.values();
        Iterator fIter      = f.iterator();
        
        while(fIter.hasNext()){
            nvp = (NameValuePanel)fIter.next();
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
        return (NameValuePanel)fieldNameMapping.get(name);
    }
    
    public NameValuePanel[] getFields(){
        return fields;
    }
    
    /**
     * The root pane of the dialog window.
     */
    javax.swing.JPanel jContentPane     = null;
    
    /**
     * The status panel.
     */
    StatusPanel statusPanel      = null;
    
    /**
     * The panel where the 'OK' and 'Cancel' buttons are placed on.
     */
    JPanel buttonPanel      = null;
    
    /**
     * The 'Cancel' button.
     */
    JButton cancelButton    = null;
    
    /**
     * The 'OK' button.
     */
    JButton okButton        = null;
    
    /**
     * The controller (mvC), where the action command will be sent to when the
     * user clicks the OK button.
     */
    ActionListener controller           = null;
    
    /**
     * The title of the dialog window.
     */
    String title                        = null;
    
    /**
     * The action string that must be sent to the controller 
     * when the 'OK' button is clicked.
     */
    String action                       = null;
    
    /**
     * The name/value pairs that are shown in the window.
     */
    NameValuePanel[] fields             = null;  
    
    /**
     * Fields in the window <String, NameValuePanel>
     */
    HashMap fieldNameMapping            = null;
}
