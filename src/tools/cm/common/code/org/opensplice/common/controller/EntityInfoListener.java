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
 * Contains all common controller components for SPLICE DDS C&M Tooling.
 */ 
package org.opensplice.common.controller;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowEvent;
import java.awt.event.WindowListener;

import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import org.opensplice.common.view.entity.EntityInfoFrame;

/**
 * Listener that triggers the attached ModelListener when the state
 * of a component has changed.
 * 
 * @date Nov 16, 2004 
 */
public class EntityInfoListener implements ChangeListener, WindowListener, ActionListener{
    private EntityInfoFrame view;
    
    /**
     * Constructs a new EntityInfoListener.
     *
     * @param _view The ModelListener that will be notified on state changes.
     */
    public EntityInfoListener(EntityInfoFrame _view){
        view = _view;
    }
    
    /**
     * The state of an attached component has changed.
     *
     * @param e The event that occurred.
     */
    public void stateChanged(ChangeEvent e) {
        view.update("tabbed_pane_update");
    }

    public void windowActivated(WindowEvent e) {
    }

    public void windowClosed(WindowEvent e) {
        view.update("window_closed");
    }
    
    public void windowDeiconified(WindowEvent e) {
        view.update("window_deiconified");
    }

    public void windowIconified(WindowEvent e) {
        view.update("window_iconified");
    }

    public void windowClosing(WindowEvent e) {
    }

    public void windowDeactivated(WindowEvent e) {
    }    

    public void windowOpened(WindowEvent e) {
    }

    public void actionPerformed(ActionEvent e) {
        String command = e.getActionCommand();
        
        if(command != null){
            if("close".equals(command)){
                view.update("close_window");
            } else if("get_qos".equals(command)){
                view.update("get_qos");
            } else if("set_qos".equals(command)){
                view.update("set_qos");
            } else {
                view.update(command);
            }
        }
    }
}
