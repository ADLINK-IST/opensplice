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
