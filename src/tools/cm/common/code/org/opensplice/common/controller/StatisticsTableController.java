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
