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

import java.awt.event.ActionListener;
import java.util.ArrayList;

import javax.swing.*;

import org.opensplice.common.model.ModelListener;
import org.opensplice.common.util.Config;

/**  
 * Abstract class that is typically extended from by main windows of an
 * application. It offers a StatusPanel, which is capable of displaying 
 * the current status and implements the ModelListener interface to be able
 * to be attached to ModelRegister components and receive updates from it. This
 * class has been defined abstract because only descendants of this class may
 * exist.
 * 
 * @date Sep 1, 2004
 */
public abstract class MainWindow extends JFrame implements ModelListener{
    protected StatusPanel statusPanel    = null;
    
	/**
	 * This is the default constructor
	 */
	public MainWindow() {
		super();
        ToolTipManager tm = ToolTipManager.sharedInstance();
        tm.setInitialDelay(100);
		this.setDefaultCloseOperation(javax.swing.WindowConstants.DO_NOTHING_ON_CLOSE);
        this.setSize(800, 600);
	}
    
    /**
     * This method initializes statusPanel.  
     *  
     * @return The status panel of the window.
     */    
    protected StatusPanel getStatusPanel() {
        if (statusPanel == null) {
            statusPanel = new StatusPanel(300, "Ready", true, true);
        }
        return statusPanel;
    }
    
    /**
     * Sends a message to the statusPanel and sets the progress.
     * 
     * @param message The message to show in the statusbar.
     * @param persistent true if the message must be shown until a new call to
     *                   this function, or false when it should automatically be
     *                   removed after certain amount of time.
     * @param busy Sets the progress monitor of the statusbar.
     */
    public void setStatus(String message, boolean persistent, boolean busy){
        statusPanel.setStatus(message, persistent, busy);
    }
    
    /**
     * Sends a message to the statusPanel.
     * 
     * @param message The message to show in the statusbar.
     * @param persistent true if the message must be shown until a new call to
     *                   this function, or false when it should automatically be
     *                   removed after certain amount of time.
     */
    public void setStatus(String message, boolean persistent){
        statusPanel.setStatus(message, persistent);
    }
    
    /**
     * Provides access to the current status.
     * 
     * @return The currently shown status.
     */
    public String getStatus(){
        return statusPanel.getStatus();
    }
    
    /**
     * Disables the view component. 
     * 
     * This is done when a dialog is shown and the user must
     * provide input before proceeding.
     */
    public void disableView(){
        this.setEnabled(false);
        this.setFocusable(false);
    }
    
    /**
     * Enables the view component.
     */
    public void enableView(){
        this.setFocusable(true);
        this.setEnabled(true);
    }
    
    /**
     * Called by ModelRegister components where this component has registered,
     * when changes occur.
     */
    public void update(String updateDescription){}
    
    protected JMenuItem[] getConnectionItems(ActionListener al){
        JMenuItem temp;
        String uri;
        String displayUri = null;
        ArrayList items = new ArrayList();
        
        for(int i=0; i<4; i++){
            uri = Config.getInstance().getProperty("connection_" + i);
            
            if(uri != null){
                temp = new JMenuItem();
                temp.setActionCommand("open_connection_" + uri);
                
                if(uri.length() > 30){
                    displayUri = uri.substring(0, 15) + "..." + 
                                    uri.substring(uri.length() - 15);
                } else {
                    displayUri = uri;
                }
                
                temp.setText(displayUri);
                temp.setAccelerator(KeyStroke.getKeyStroke(Integer.toString(i+1).charAt(0), java.awt.Event.SHIFT_MASK, false));
                temp.setToolTipText(uri);
                if(al != null){
                    temp.addActionListener(al);
                }
                items.add(temp);
            }
        }
        return (JMenuItem[])(items.toArray(new JMenuItem[items.size()]));
    }
    
}
