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

package org.opensplice.common.view.table;

import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;
import javax.swing.JTable;
import javax.swing.ListSelectionModel;
import javax.swing.table.TableColumnModel;

import org.opensplice.cm.Entity;
import org.opensplice.common.CommonException;
import org.opensplice.common.controller.StatisticsTableController;
import org.opensplice.common.model.ModelListener;
import org.opensplice.common.model.table.StatisticsTableModel;

/**
 * Table that is able to display Statistics of a specific Entity and offers
 * facilities for resetting (parts of) it.
 * 
 * @date May 12, 2005 
 */
public class StatisticsTable extends JTable{
    /**
     * Menu item for resetting the complete statistics.
     */
    private JMenuItem resetAllItem = null;
    
    /**
     * Menu item for resetting one field in the statistics.
     */
    private JMenuItem resetItem = null;
    
    /**
     * Controller component that takes care of the actual resetting of the 
     * Statistics.
     */
    private StatisticsTableController controller = null;
    
    /**
     * Listener to notify changes to the creator.
     */
    private ModelListener listener = null;
    
    /**
     * Constructs a new StatisticsTable.
     *  
     *
     * @param entity The Entity, which Statistics to display.
     * @param listener The listener that will be notified of changes. This can be
     *                 null.
     * @throws CommonException Thrown when:
     *                          - C&M API not initialized.
     *                          - Supplied Entity not valid.
     *                          - Communication with SPLICE-DDS failed.
     */
    public StatisticsTable(Entity entity, ModelListener listener) throws CommonException{
        super();
        this.listener = listener;
        this.setModel(new StatisticsTableModel(entity));
        this.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        this.getTableHeader().setReorderingAllowed(false);
        this.setAutoResizeMode(AUTO_RESIZE_LAST_COLUMN);
        this.setSurrendersFocusOnKeystroke(true);
        
        TableColumnModel tcm = this.getColumnModel();
        
        if(tcm.getColumnCount() > 2){
            tcm.getColumn(0).setPreferredWidth(150);
            tcm.getColumn(1).setPreferredWidth(80);
            tcm.getColumn(1).setMaxWidth(80);
            tcm.getColumn(2).setPreferredWidth(244);
            controller = new StatisticsTableController(this);
            this.addMouseListener(controller);
            this.setToolTipText("Right click to view actions.");
        }
    }
    
    /**
     * Updates the Statistics in the table.
     * 
     * @return
     */
    public boolean update(){
        return ((StatisticsTableModel)this.dataModel).update();
    }
    
    @Override
    public boolean isCellEditable(int row, int column){
        return false;
    }
    
    /**
     * Notifies the listener of the table with the supplied message.
     * 
     * @param description The message to notify.
     */
    public void notifyListener(String description){
        if(listener != null){
            listener.update(description);
        }
    }
    
    /**
     * Provides access to the Statistics action menu.
     * 
     * @return
     */
    public JPopupMenu getPopupMenu(int row){
        JPopupMenu popupMenu = new JPopupMenu("Statistics actions");
        JMenuItem item = getResetItem();
        String row0, row1;
        
        row0 = (String)getValueAt(row, 0);
        row1 = (String)getValueAt(row, 1);
        
        if(("".equals(row0)) && ("".equals(row1))){
            item.setText("Reset ''");
        } else if("".equals(row0)){
            item.setText("Reset '" + getValueAt(row, 1) + "'");
        } else if("".equals(row1)){
            item.setText("Reset '" + getValueAt(row, 0) + "'");
        } else {
            item.setText("Reset '" + getValueAt(row, 0) + "." + 
                                    getValueAt(row, 1) + "'");
        }
        popupMenu.add(item);
        popupMenu.add(getResetAllItem());
        
        return popupMenu;
    }
    
    /**
     * Provides access to resetAllItem.
     * 
     * @return Returns the resetAllItem.
     */
    private JMenuItem getResetAllItem() {
        if(resetAllItem == null){
            resetAllItem = new JMenuItem();
            resetAllItem.setText("Reset all");
            resetAllItem.setActionCommand("reset_all");
            resetAllItem.setToolTipText("Resets all statistics for this Entity");
            resetAllItem.addActionListener(controller);
        }
        return resetAllItem;
    }
    
    /**
     * Provides access to resetItem.
     * 
     * @return Returns the resetItem.
     */
    private JMenuItem getResetItem() {
        if(resetItem == null){
            resetItem = new JMenuItem();
            resetItem.setText("Reset ");
            resetItem.setActionCommand("reset");
            resetItem.addActionListener(controller);
        }
        return resetItem;
    }
}
