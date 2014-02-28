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
                public void windowClosing(WindowEvent e) {
                    returnCode = CANCEL_OPTION;
                    dialog.setVisible(false);
                }
            });
            this.dialog.pack();
        }
        return this.dialog;
    }
    
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
