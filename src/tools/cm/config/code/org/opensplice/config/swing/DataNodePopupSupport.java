/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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
package org.opensplice.config.swing;

import java.awt.event.MouseListener;

import javax.swing.JPopupMenu;
import org.opensplice.config.data.DataNode;

public interface DataNodePopupSupport {
    public DataNode getDataNodeAt(int x, int y);
    
    public void showPopup(JPopupMenu popup, int x, int y);
    
    public void setStatus(String message, boolean persistent, boolean busy);
    
    public void addMouseListener(MouseListener listener);
    
    public void removeMouseListener(MouseListener listener);
}
