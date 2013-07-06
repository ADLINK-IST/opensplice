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

import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;

import javax.swing.JButton;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JTextField;
import javax.swing.filechooser.FileFilter;

import org.opensplice.common.util.Config;
import org.opensplice.common.util.Report;

/**
 *
 *
 * @date Mar 31, 2005
 */
public class FileNameValuePanel extends NameValuePanel implements ActionListener {
    public FileNameValuePanel(
            String fieldName,
            String defaultValue,
            boolean emptyInputAllowed,
            String browseText,
            JFrame parent)
    {
        this(fieldName, defaultValue, emptyInputAllowed, browseText, parent, new Dimension(50, 20), new Dimension(100, 20));
    }

    /*
    public FileNameValuePanel(
            String fieldName,
            String defaultValue,
            boolean emptyInputAllowed,
            String browseText,
            JFrame parent,
            Dimension labelDim,
            Dimension fieldDim)
    {
        super(fieldName, defaultValue, emptyInputAllowed, labelDim, fieldDim);

        this.parent = parent;
        field = new JTextField();

        if(defaultValue != null){
            ((JTextField)field).setText(defaultValue);
        }
        field.setMinimumSize(this.fieldDim);
        field.setPreferredSize(this.fieldDim);
        field.setMaximumSize(this.fieldDim);

        this.add(field, null);
    }
    */

    public FileNameValuePanel(
            String fieldName,
            String defaultValue,
            boolean emptyInputAllowed,
            String browseText,
            JFrame parent,
            Dimension labelDim,
            Dimension fieldDim)
    {
        super(fieldName, defaultValue, emptyInputAllowed, labelDim, fieldDim);
        this.parent = parent;
        this.browseText = browseText;

        if(this.browseText == null){
            this.browseText = "Browse";
        }
        field = new JTextField();

        if(defaultValue != null){
            ((JTextField)field).setText(defaultValue);
        } else {
            this.defaultValue = "";
        }
        if(fieldDim.width < 135){
            this.fieldDim = new Dimension(130, fieldDim.height);
        }
        Dimension dim = new Dimension(fieldDim.width - 105, fieldDim.height);
        field.setMinimumSize(dim);
        field.setPreferredSize(dim);
        field.setMaximumSize(dim);
        this.add(field);

        browseButton = new JButton();
        browseButton.setText("Browse");
        browseButton.setPreferredSize(new Dimension(100, 20));
        browseButton.addActionListener(this);
        this.add(browseButton);
    }

    @Override
    public void actionPerformed(ActionEvent e) {
        CommonFileChooser chooser = Config.getInstance().getFileChooser();
        int returnVal;

        chooser.setSelectedFile(new File((String)defaultValue));
        chooser.setMultiSelectionEnabled(false);

        if(filter != null){
            chooser.setFileFilter(filter);
            chooser.setAcceptAllFileFilterUsed(false);
        }
        try{
            returnVal = chooser.showDialog(parent, browseText);
        } catch(Exception exc){
             Report.getInstance().writeErrorLog(exc.getMessage());
            returnVal = JFileChooser.CANCEL_OPTION;
        }

        if (returnVal == JFileChooser.APPROVE_OPTION) {

            String value = chooser.getSelectedFile().toURI().toString();
            ((JTextField)field).setText(value);
        }

    }

    public void setFilter(String extension, String description){
        filter = new ChooseFilter(extension, description);
    }

    @Override
    public Object getValue(){
        String envVarValue;
        String value = ((JTextField)field).getText();
        StringBuffer result = new StringBuffer();
        int index, end;

        index = value.indexOf("${");

        if(index != -1){
            result.append(value.substring(0, index));
        } else {
            result.append(value);
        }

        while(index != -1){
            end = value.indexOf('}', index);

            if(end != -1){
                envVarValue = System.getenv(value.substring(index+2, end));

                if(envVarValue != null){
                    result.append(envVarValue);
                }
                index = value.indexOf("${", end);

                if(index == -1){
                    result.append(value.substring(end+1));
                }
            } else {
                result.append(value.substring(index));
                index = -1;
            }
        }
        return result.toString();
    }

    @Override
    public void setEnabled(boolean enabled) {
        ((JTextField)field).setEditable(false);
        ((JTextField)field).setEnabled(false);
        browseButton.setEnabled(false);
    }

    private static class ChooseFilter extends FileFilter {
        private String extension = null;
        private String description = null;

        public ChooseFilter(String extension, String description){
            this.extension = extension;
            this.description = description;
        }

        @Override
        public boolean accept(File f) {
            if (f.isDirectory()) {
                return true;
            }

            if(f.getName().endsWith(extension)){
                return true;
            }
            return false;
        }

        @Override
        public String getDescription() {
            return this.description;

        }
    }

    protected ChooseFilter filter = null;
    protected String browseText = null;
    protected JButton browseButton = null;
    protected JFrame parent = null;
}
