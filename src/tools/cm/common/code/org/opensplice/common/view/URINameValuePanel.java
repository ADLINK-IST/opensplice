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
