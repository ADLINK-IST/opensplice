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
import java.io.File;

import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JTextField;

import org.opensplice.common.util.Config;
import org.opensplice.common.util.Report;

public class URINameValuePanel extends FileNameValuePanel {
    public URINameValuePanel(
            String fieldName,
            String defaultValue,
            boolean emptyInputAllowed,
            String browseText,
            JFrame parent)
    {
        super(fieldName, defaultValue, emptyInputAllowed, browseText, parent, null, null);
    }

    public URINameValuePanel(
            String fieldName,
            String defaultValue,
            boolean emptyInputAllowed,
            String browseText,
            JFrame parent,
            Dimension labelDim,
            Dimension fieldDim)
    {
        super(fieldName, defaultValue, emptyInputAllowed, browseText, parent, labelDim, fieldDim);
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

            if(value != null){
                if((value.startsWith("file:/")) && (!value.startsWith("file://"))){
                    if((System.getProperty("os.name").indexOf("Windows")) == -1){
                        value = value.replaceFirst("file:", "file://");
                    } else {
                        value = value.replaceAll("/", "\\\\");
                        value = value.replaceFirst("file:\\\\", "file://");
                    }
                }
                value = value.replaceAll("%20", " ");
            }
            ((JTextField)field).setText(value);
        }

    }
}
