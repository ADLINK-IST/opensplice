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

import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.Container;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;

import javax.swing.JDialog;
import javax.swing.JFileChooser;
import javax.swing.JFrame;

/**
 * 
 * 
 * @date Apr 14, 2005 
 */
public class CommonFileChooser extends JFileChooser {
    private int returnCode;
    private JDialog dialog = null;
    
    public CommonFileChooser(String path){
        super(path);
    }

    private synchronized JDialog getDialog(){
        if(this.dialog == null){
            JFrame f = null;
            this.dialog = new JDialog(f, "", true);
            Container contentPane = this.dialog.getContentPane();
            contentPane.setLayout(new BorderLayout());
            contentPane.add(this, BorderLayout.CENTER);

            this.dialog.addWindowListener(new WindowAdapter() {
                @Override
                public void windowClosing(WindowEvent e) {
                    returnCode = CANCEL_OPTION;
                    dialog.setVisible(false);
                }
            });
            this.dialog.pack();
        }
        return this.dialog;
    }
    
    @Override
    protected void fireActionPerformed(String command) {
        super.fireActionPerformed(command);
        
        JDialog myDialog = this.getDialog();

        if(APPROVE_SELECTION.equals(command)){
            returnCode = APPROVE_OPTION;
            myDialog.setVisible(false);
        } else if(CANCEL_SELECTION.equals(command)){
            returnCode = CANCEL_OPTION;
            myDialog.setVisible(false);
        }
    }
    
    @Override
    public int showDialog(Component parent, String title){
        JDialog myDialog = this.getDialog();

        if(parent != null){
            myDialog.setLocationRelativeTo(parent);
        }
        if(title != null){
            myDialog.setTitle(title);
        }
        myDialog.setVisible(true);
        myDialog.toFront();
        
        return returnCode;
    }
}
