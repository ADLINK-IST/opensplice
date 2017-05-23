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
    @Override
    public void stateChanged(ChangeEvent e) {
        view.update("tabbed_pane_update");
    }

    @Override
    public void windowActivated(WindowEvent e) {
    }

    @Override
    public void windowClosed(WindowEvent e) {
        view.update("window_closed");
    }
    
    @Override
    public void windowDeiconified(WindowEvent e) {
        view.update("window_deiconified");
    }

    @Override
    public void windowIconified(WindowEvent e) {
        view.update("window_iconified");
    }

    @Override
    public void windowClosing(WindowEvent e) {
    }

    @Override
    public void windowDeactivated(WindowEvent e) {
    }    

    @Override
    public void windowOpened(WindowEvent e) {
    }

    @Override
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
