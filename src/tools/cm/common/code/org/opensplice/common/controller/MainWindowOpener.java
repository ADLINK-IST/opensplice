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
package org.opensplice.common.controller;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JFrame;

/**
 * Controller component that can be used to switch from a child window to its
 * parent.
 * 
 * @date Jan 6, 2005 
 */
public class MainWindowOpener implements ActionListener {
    private JFrame parent = null;
    
    /**
     * Constructs a new MainWindowOpener.
     *
     * @param _view The child window.
     * @param _parent The parent window.
     */
    public MainWindowOpener(JFrame _parent) {
        parent = _parent;
    }
    @Override
    public void actionPerformed(ActionEvent e) {
        String command = e.getActionCommand();
        
        if("view_main".equals(command)){
            parent.setState(JFrame.NORMAL);
            parent.toFront();
        }
    }
}
