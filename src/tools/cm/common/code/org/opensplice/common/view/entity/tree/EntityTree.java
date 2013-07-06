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
package org.opensplice.common.view.entity.tree;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.logging.Logger;

import javax.swing.JTree;
import javax.swing.SwingUtilities;
import javax.swing.Timer;
import javax.swing.event.TreeSelectionEvent;
import javax.swing.event.TreeSelectionListener;
import javax.swing.tree.DefaultTreeModel;

import org.opensplice.cm.Entity;
import org.opensplice.cm.Participant;
import org.opensplice.common.CommonException;
import org.opensplice.common.controller.EntityTreeCellRenderer;
import org.opensplice.common.controller.EntityTreeWillExpandListener;
import org.opensplice.common.model.ModelRegister;

/**
 * Tree that holds and displays Entity objects as well as the mutual 
 * relationships between these entities. The tree consists of EntityTreeNode
 * objects, which all hold a specific Entity. The root node of the tree is a
 * descendant of the abstract RootTreeNode object that holds a Participant. All 
 * other nodes are descendants of the EntityTreeNode object and depend on the 
 * implementation of the tree (see concrete descendants of this object). 
 * 
 * The responsibility of this tree is to offer common facilities that can 
 * be used by concrete descendants of this tree. A listener is attached to the
 * tree, that is listening for node expansions/collapses and updates the nodes
 * when the user demands access to them. 
 * 
 * For optimalization purposes, the tree is not loaded entirely. A node is only
 * loaded when its parent is expanded. That means only nodes that are visible
 * for the user are expanded.
 * 
 * To get access to changes in the structure of the tree it is possible to
 * attach a ModelRegister component that will be notified on tree changes.
 * 
 * @date Aug 31, 2004
 */
public abstract class EntityTree extends JTree implements ActionListener{
    
    /**
     * Creates a new EntityTree.   
     *
     * @param _participant The participant that must become the root node
     *                     of the tree.
     * @param _register Component that will be notified on tree changes.
     *                  This may be null.
     * @throws CommonException
     */
    public EntityTree(Participant _participant, ModelRegister _register, boolean childrenVisible) throws CommonException{
        super();
        this.childrenVisible = childrenVisible;
        this.setCellRenderer(new EntityTreeCellRenderer());
        logger = Logger.getLogger("org.opensplice.common.view");
        register = _register;
        participant = _participant;
        
        if(participant == null){
            throw new CommonException("Participant not available.");
        }
        if(participant.isFreed()){
            throw new CommonException("Participant already freed.");
        }
        this.setShowsRootHandles(true);
        this.setRootVisible(false);
        this.addTreeSelectionListener(new EntityTreeSelectionListener());
    }
    
    /**
     * Provides access to the Participant that is the root node of the tree.
     * 
     * @return The 'root' participant of the tree.
     */
    public Participant getParticipant(){
        return participant;
    }
    
    /**
     * Clears the tree, by removing all children of the root node as
     * well as the tree will expand listener. The root node itself is not
     * removed. After the tree is cleared a notification is sent to the 
     * ModelRegister ('entity_tree_cleared'). The Entity objects of the 
     * nodes in this tree will only be freed when the freeing of the Entity
     * of that node is enabled.
     */
    public void clear(){
        if(rootNode != null){
            this.removeTreeWillExpandListener(willExpandListener);
            
            if(this.updateTimer != null){
                this.updateTimer.stop();
                this.updateTimer = null;
            }
            int childCount = rootNode.getChildCount();
            
            for(int i=0; i<childCount; i++){
                ((EntityTreeNode)(rootNode.getChildAt(0))).remove();
            }
            this.fireTreeChanged("cleared");
        }
    }
    
    /**
     * Clears the tree, by removing all children of the root node as
     * well as the tree will expand listener. The root node itself is not
     * removed. After the tree is cleared a notification is sent to the 
     * ModelRegister ('entity_tree_cleared'). The Entity objects of the 
     * nodes in this tree will always be freed. 
     */
    public void clearForced(){
        if(rootNode != null){
            this.removeTreeWillExpandListener(willExpandListener);
            
            if(this.updateTimer != null){
                this.updateTimer.stop();
                this.updateTimer = null;
            }
            rootNode.setAllEntityFreeEnabled();
            int childCount = rootNode.getChildCount();
            
            for(int i=0; i<childCount; i++){
                ((EntityTreeNode)(rootNode.getChildAt(0))).removeForced();
            }
            this.fireTreeChanged("cleared");
        }
    }
    
    /**
     * Refreshes the supplied node by calling the refresh function on the node
     * itself. Children of the supplied node are recursively refreshed depending
     * on whether their parent is expanded or not. During the refresh the tree 
     * is disabled so the user cannot make a gesture on the tree while
     * refreshing.
     * 
     * @param node The node to refresh.
     * @throws CommonException Thrown when the node could not be refreshed.
     */
    public synchronized void refresh(EntityTreeNode node) throws CommonException{
        if((participant == null) || (node == null)){
            return;
        }
        this.setEnabled(false);
        node.refresh();
        this.setEnabled(true);
        this.fireTreeChanged("refreshed");
    }
    
    /**
     * Refreshes the root node of the tree. Convenience function for 
     * refresh(rootNode). 
     * @throws CommonException Thrown when the tree could not be refreshed.
     */
    public synchronized void refresh() throws CommonException{
        if(rootNode != null){
            this.refresh(rootNode);
        }
        if(updateTimer != null){
            if(updateTimer.isRunning()){
                updateTimer.restart();
            }
        }
    }
    
    /**
     * Inserts a node in the tree. The child node is inserted as a child of the
     * parent node in alpabethical order.
     * 
     * @param child The child node to insert in the tree.
     * @param parent The parent node where to attach the node to.
     */
    public void addNode(EntityTreeNode child, EntityTreeNode parent){
        if((child != null) && (parent != null)){
            this.getTreeModel().insertNodeInto(child, parent, this.getInsertIndex(parent, child));
        }
    }
    
    /**
     * Sets the update delay for the automatic updating of the tree.
     * 
     * @param millis The time between updates of the tree (in milliseconds). 
     *               When a negative number is supplied, no automatic updates 
     *               will occur anymore.
     */
    public synchronized void setUpdateDelay(int millis){
        if(millis > 0){
            if(updateTimer == null){
                updateTimer = new Timer(millis, this);
                updateTimer.setRepeats(false);
            } else if(updateTimer.isRunning()){
                updateTimer.stop();
                updateTimer.setDelay(millis);
            } else {
                updateTimer.setDelay(millis);
            }
            updateTimer.start();
            this.fireTreeChanged("update_delay");
        } else if(updateTimer != null){
            if(updateTimer.isRunning()){
                updateTimer.stop();
            }
            updateTimer = null;
            this.fireTreeChanged("update_delay");
        }
    }
    
    /**
     * Called when the Timer ended. The function will refresh the tree and 
     * restart the timer.
     * 
     * @param e The event that occurred.
     */
    @Override
    public void actionPerformed(ActionEvent e){
        if(e.getSource().equals(updateTimer)){
            Runnable worker = new Runnable(){
                @Override
                public void run(){
                    try {
                        refresh();
                        
                        if(updateTimer != null){
                            updateTimer.restart();
                        }
                    } catch (CommonException ce) {
                    }
                }
            };
            SwingUtilities.invokeLater(worker);
        }
    }
    
    /**
     * Triggers all nodes in the tree that hold the supplied Entity that the
     * Entity may or may not be freed when the tree is cleared. The supplied
     * entity must be exactly the same as the one in a node (same address). 
     * 
     * @param entity The entity that may not be freed.
     * @param enabled true, if the Entity may be freed, false otherwise.
     * @return true if the Entity is currently in the tree, false otherwise.
     */
    public synchronized boolean setEntityFreeEnabled(Entity entity, boolean enabled){
        return rootNode.setEntityFreeEnabled(entity, enabled);
    }
    
    /**
     * Provides access to the model of the tree.
     *  
     * @return The tree model of this tree.
     */
    public DefaultTreeModel getTreeModel(){
        return (DefaultTreeModel)treeModel;
    }
    
    /**
     * Sets the childrenVisible variable of the tree to the supplied value.
     * It determines the entities that will be displayed in the tree. If true is
     * supplied; all relations between entities will be resolved and displayed
     * in the tree. If false is supplied, only the children of the RootTreeNode
     * will be resolved and displayed. Children of the RootTreeNode will not be
     * resolved and displayed. This reduces the intrusiveness on the SPLICE-DDS
     * domain/node it is resolving the relations from. 
     * 
     * @param visible Wether or not children of the RootTreeNode of the tree 
     *                should be resolved.
     * @throws CommonException@throws CommonException Thrown when visibility
     *                                                could not be changed.
     */
    public void setChildrenVisible(boolean visible) throws CommonException{
        this.childrenVisible = visible;
        rootNode.setChildrenVisible(visible, true);
        this.fireTreeChanged("children_visibility");
    }
    
    /**
     * Provides access to the childrenVisible variable of the tree.
     * 
     * @return Whether or not children of the RootTreeNode of the tree will
     *         resolve their relations.
     */
    public boolean getChildrenVisible(){
        return this.childrenVisible;
    }
    
    /**
     * Determines the location to insert the child node in the parent. The 
     * location is determined using the String representation of the object.
     * 
     * @param parent The node where the child must be inserted in.
     * @param child The node where to find the insert index of.
     * @return The insert location of the child in the parent in a way that
     *         children will be sorted alphabetically after the child has been
     *         inserted.
     */
    protected int getInsertIndex(EntityTreeNode parent, EntityTreeNode child){
        EntityTreeNode node;
        String val;
        String s = ((Entity)child.getUserObject()).toStringExtended();
        
        int i = 0;
        int childCount = parent.getChildCount();
        
        for(i=0; i<childCount; i++){
            node = (EntityTreeNode)(parent.getChildAt(i));
            val = ((Entity)node.getUserObject()).toStringExtended();
            
            if(s.compareTo(val) <= 0){
                return i;
            }
        }
        return i;
    }
    
    /**
     * Notifies the ModelRegister when it is available.
     * 
     * @param description The description that will be sent to the ModelRegister
     *                    prefixed by 'entity_tree_'.
     */
    protected void fireTreeChanged(String description){
        if((register != null) && (description != null)){
            register.pushUpdate("entity_tree_" + description);
        }
    }
    
    private class EntityTreeSelectionListener implements TreeSelectionListener {
        private boolean selected = false;

        @Override
        public void valueChanged(TreeSelectionEvent e) {
            if(e.isAddedPath()){
                if(!selected){
                    selected = true;
                    fireTreeChanged("selection_change");
                }
            } else if(selected){
                selected = false;
                fireTreeChanged("selection_change");
                
            }
        }
    }
    
    /**
     * The root node of the tree, that holds a Participant and actually is
     * a concrete descendant of the RootTreeNode object.
     */
    protected RootTreeNode rootNode;
    
    /**
     * The Participant that is associated with the root node and supplies the
     * entrance to all other entities in the system.
     */
    protected Participant participant;
    
    /**
     * Listener that listens for node expansion/collapse events
     */
    protected EntityTreeWillExpandListener willExpandListener;
    
    /**
     * Model component that will be notified when the tree changes.
     */
    protected ModelRegister register;
    
    /**
     * Logging facilities.
     */
    protected Logger logger;
    
    /**
     * The automatic update timer.
     */
    protected Timer updateTimer;
    
    /**
     * Whether or not children of the RootTreeNode will resolve their relations.
     */
    protected boolean childrenVisible;
}
