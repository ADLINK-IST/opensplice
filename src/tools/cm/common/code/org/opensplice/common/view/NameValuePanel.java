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

import javax.swing.*;

import java.awt.Dimension;
import java.awt.FlowLayout;

/**
 * This class is meant to provide a standard generic input panel that
 * contains a label and a input field. 
 * 
 * Input fields that are currently supported are: 
 * - textfields 
 * - comboboxes.
 */
public abstract class NameValuePanel extends JPanel {
    /**
     * Creates an input panel with a label and a textfield.
     * @param _fieldName The name of the field that is also used as label.
     * @param _emptyInputAllowed Boolean that specifies if empty input is allowed when 
     *                           submitted.
     */
    public NameValuePanel(
            String fieldName, 
            Object defaultValue, 
            boolean emptyInputAllowed, 
            Dimension labelDim, 
            Dimension fieldDim)
    {
        super();
        if(labelDim != null){
            this.labelDim = labelDim;
            
            if(this.labelDim.height != 20){
                this.labelDim.height = 20;
            }
        } else {
            this.labelDim = new Dimension(100, 20);
        }
        if(fieldDim != null){
            this.fieldDim = fieldDim;
            
            if(this.fieldDim.height != 20){
                this.fieldDim.height = 20;
            }
        } else {
            this.fieldDim = new Dimension(230, 20);
        }
        this.fieldName = fieldName;
        this.emptyInputAllowed = emptyInputAllowed;
        this.defaultValue = defaultValue;
        this.initLayout();
        this.initLabel();
    }
    
    /**
     * Initializes the layout (FlowLayout is used).
     */
    protected void initLayout(){
        FlowLayout layFlowLayout = new FlowLayout(FlowLayout.LEFT);
        layFlowLayout.setVgap(0);
        this.setLayout(layFlowLayout);
    }
    
    /**
     * Initializes the label.
     * 
     * The fieldName is also used as label text.
     */
    protected void initLabel(){
        JLabel label =  new JLabel();
        label.setText(fieldName);
        label.setPreferredSize(labelDim);
        this.add(label);
    }
    
    /**
     * Provides access to the name of the field.
     * 
     * @return The name of the field.
     */
    public String getName(){
        return fieldName;
    }
    
    /**
     * Provides access to the emptyInputIsAllowed boolean.
     * 
     * @return true if the field may be empty when submitted, false otherwise.
     */
    public boolean isEmptyInputAllowed(){
        return emptyInputAllowed;
    }
    
    /**
     * Provides access to the editor component for the field value.
     * 
     * @return The editor component for the field value.
     */
    public JComponent getField(){
        return field;
    }
        
    public abstract Object getValue();
    
    public abstract void setEnabled(boolean enabled);
    
    /**
     * The name of the field.
     */
    protected String fieldName    = null;
    
    /**
     * The input field.
     */
    protected JComponent field    = null;
    
    /**
     * The label Dimension.
     */
    protected Dimension labelDim = null;
    
    /**
     * The field Dimension.
     */
    protected Dimension fieldDim = null;
    
    /**
     * Boolean that specifies if empy input is allowed for this field.
     */
    protected boolean emptyInputAllowed;
    
    protected Object defaultValue = null;
}
