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
