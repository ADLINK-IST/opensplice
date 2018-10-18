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
package org.opensplice.common.controller;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;

import javax.swing.SwingUtilities;

import org.opensplice.common.model.table.StatisticsTableModel;
import org.opensplice.common.view.table.StatisticsTable;

/**
 * Controller component for the StatisticsTable. This controller takes care
 * of resetting Statistics.
 * 
 * @date May 17, 2005 
 */
public class StatisticsTableController implements MouseListener, ActionListener {
    /**
     * The table component to control.
     */
    private StatisticsTable view = null;
    
    /**
     * Constructs a new controller.
     *
     * @param view The StatisticsTable to control.
     */
    public StatisticsTableController(StatisticsTable view){
        this.view = view;
    }
    
    @Override
    public void actionPerformed(ActionEvent e) {
        String command = e.getActionCommand();
        
        if("reset".equals(command)){
            
            Runnable worker = new Runnable(){
                @Override
                public void run(){
                    boolean result = ((StatisticsTableModel)view.getModel()).reset(view.getSelectedRow(), true);
                    
                    if (result == false) {
                        view.notifyListener("reset_failed");
                    }
                }
            };
            SwingUtilities.invokeLater(worker);
        } else if("reset_all".equals(command)){
            Runnable worker = new Runnable(){
                @Override
                public void run(){
                    boolean result = ((StatisticsTableModel)view.getModel()).reset(true);
                    
                    if (result == false) {
                        view.notifyListener("reset_all_failed");
                    }
                }
            };
            SwingUtilities.invokeLater(worker);
        }
    }

    @Override
    public void mouseClicked(MouseEvent e) {
        if((e.getButton() == MouseEvent.BUTTON3) && (e.getClickCount() == 1)) {
            int row = view.rowAtPoint(e.getPoint());
            view.getSelectionModel().addSelectionInterval(row, row);
            view.getPopupMenu(row).show(e.getComponent(),e.getX(), e.getY());
        }
    }

    @Override
    public void mouseEntered(MouseEvent e) {}

    @Override
    public void mouseExited(MouseEvent e) {}

    @Override
    public void mousePressed(MouseEvent e) {}

    @Override
    public void mouseReleased(MouseEvent e) {}

}
