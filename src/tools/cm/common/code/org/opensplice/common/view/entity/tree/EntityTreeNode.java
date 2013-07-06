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

import javax.swing.tree.DefaultMutableTreeNode;

import org.opensplice.cm.Entity;
import org.opensplice.cm.impl.EntityImpl;
import org.opensplice.common.CommonException;

/**
 * Represents a node in a EntityTree. It holds an Entity object and relates
 * to other EntityTreeNode objects. This class has been defined abstract
 * because only descendants of this class may actually exist.
 *  
 * @date Sep 24, 2004
 */
public abstract class EntityTreeNode extends DefaultMutableTreeNode {
    
    /**
     * Creates a new EntityTreeNode, which holds the supplied Entity and is
     * located in the supplied EntityTree.
     *
     * @param e The Entity to attach to the node.
     * @param _tree The EntityTree the node is located in.
     */
    public EntityTreeNode(Entity e, EntityTree _tree, boolean childrenVisible){
        super(e);
        entityFreeEnabled = true;
        expanded = false;
        tree = _tree;
        this.childrenVisible = childrenVisible;
        this.recursiveVisible = childrenVisible;
        assert tree != null: "Supplied tree is a null pointer";
    }
    
    /**
     * Provides access to the Entity of the node.
     * 
     * @return The Entity of the node.
     */
    public Entity getEntity(){
        return (Entity)userObject;
    }
    
    /**
     * Sets the expanded state of this node to the supplied value.
     * 
     * @param expanded Whether or not the node is currently expanded.
     */
    public void setExpanded(boolean expanded){
        this.expanded = expanded;
    }
    
    /**
     * Provides access to the expanded variable.
     * 
     * @return true if the node is currently expanded, false otherwise.
     */
    public boolean isExpanded(){
        return expanded;
    }
    
    /**
     * Removes the node from the tree and notifies the tree that is has been
     * removed. The Entity of the node is only freed when entityFreeEnabled
     * is true. The entityFreeEnabled can be changed by calling 
     * setEntityFreeEnabled. 
     */
    public void remove(){
        int childCount = this.getChildCount();
        
        for(int i=0; i<childCount; i++){
            ((EntityTreeNode)this.getChildAt(0)).remove();
        }
        tree.getTreeModel().removeNodeFromParent(this);
        
        if(entityFreeEnabled){
            ((Entity)userObject).free();
        }
        tree.fireTreeChanged("node_removed");
    }
    
    /**
     * Removes all children of this node.
     */
    public void removeChildren(){
        int childCount = this.getChildCount();
        
        for(int i=0; i<childCount; i++){
            ((EntityTreeNode)this.getChildAt(0)).remove();
        }
    }
    
    /**
     * Determines whether the Entity of this node should be freed when the node
     * is removed. This function is recursive and therefore calls this routine
     * on each of its children.
     * 
     * @param e The Entity where to enable/disable the free of.
     * @param enabled Whether or not to free the Entity when removing the node.
     * @return true if this node or one of its children contains the supplied 
     *         Entity, false otherwise.
     */
    public boolean setEntityFreeEnabled(Entity e, boolean enabled){
        EntityTreeNode etn;
        boolean result = false;
        boolean tmpResult;
        
        if(getEntity() == e){
            entityFreeEnabled = enabled;
            result = true;
        }
        
        for(int i=0; i<this.getChildCount(); i++){
            etn = (EntityTreeNode)this.getChildAt(i);
            tmpResult = etn.setEntityFreeEnabled(e, enabled);
            
            if(tmpResult){
                result = true;
            }
        }
        return result;
    }
    
    /**
     * Makes all Entity objects (of this node and its children) be freed when
     * removing the node.
     */
    public void setAllEntityFreeEnabled(){
        EntityTreeNode etn;
        entityFreeEnabled = true;
        
        for(int i=0; i<this.getChildCount(); i++){
            etn = (EntityTreeNode)this.getChildAt(i);
            etn.setAllEntityFreeEnabled();
        }
    }
    
    /**
     * Determines whether this node should resolve its children. By default a
     * EntityTreeNode takes over the value of its EntityTree, by explicitly
     * calling this method on a specific node, the children visibility can
     * be overruled for that node and its children. 
     * 
     * Overriding the visibility by calling this routine, makes the color of
     * the node change.
     * - Tree visibility == false && node visibility == true --> node is colored green. 
     * - Tree visibility == true && node visibility == false --> node is colored red.
     * - Tree visibility == node visibility                  --> node is colored black.
     * @param visible Whether or not the children of the node should be resolved.
     * @param recursive TODO
     * @throws CommonException
     */
    public void setChildrenVisible(boolean visible, boolean recursive) throws CommonException{
        EntityTreeNode etn;
        this.childrenVisible = visible;
        this.recursiveVisible = recursive;
        
        if(((EntityTreeNode)this.getParent()).isExpanded()){
            this.refresh();
        }
        
        if(visible && recursive){
            for(int i=0; i<this.getChildCount(); i++){
                etn = (EntityTreeNode)this.getChildAt(i);
                etn.setChildrenVisible(visible, recursive);
            }
        }
    }
    
    /**
     * Provides access to the visibility of the children of this node.
     * 
     * @return Whether or not the children of this node are visible.
     */
    public boolean getChildrenVisible(){
        return this.childrenVisible;
    }
    
    /**
     * Refreshes the EntityTreeNode. Concrete descendants of this class must
     * implement this function. 
     * 
     * @throws CommonException Thrown when the node could not be refreshed.
     */
    public abstract void refresh() throws CommonException;
    
    /**
     * Removes the node from the tree and notifies the tree that is has been
     * removed. This function will ALWAYS free its Entity. 
     */
    protected void removeForced(){
        int childCount = this.getChildCount();
        
        for(int i=0; i<childCount; i++){
            ((EntityTreeNode)this.getChildAt(0)).removeForced();
        }
        tree.getTreeModel().removeNodeFromParent(this);
        ((Entity)userObject).free();
        tree.fireTreeChanged("node_removed");
    }
    
    /**
     * Removes all child nodes of this node that are not available in the 
     * supplied parameter. That means all child nodes of this node that contain
     * an entity that is NOT in the list, will be removed.
     * 
     * @param entities The list of entities that ARE available.
     */
    protected void removeUnavailableChildren(Entity[] entities){
        EntityTreeNode child;
        Entity childEntity, e;
        boolean found;
        
        for(int i=0; i<this.getChildCount();){
            child = (EntityTreeNode)(this.getChildAt(i));
            childEntity = (Entity)(child.getUserObject());
            found = false;
            
            for(int j=0; j<entities.length; j++){
                e = entities[j];
                
                if( (e.getIndex() == childEntity.getIndex()) &&
                    (e.getSerial() == childEntity.getSerial())
                   ){
                    found = true;
                    /*TODO:@todo EntityImpl dependancy should be removed.*/ 
                    ((EntityImpl)childEntity).setEnabled(e.isEnabled());
                    break;
                }
            }
            if(!found){
                child.removeForced();
            } else {
                i++; /*only add 1 when the child was not removed.*/
            }
        }
    }
    
    /**
     * Checks whether this node has a child that holds the supplied entity.
     * 
     * @param e The Entity to resolve.
     * @return The node that holds the supplied Entity or null if it is not
     *         available.
     */
    protected EntityTreeNode resolveChildNode(Entity e){
        int childCount = this.getChildCount();
        EntityTreeNode child;
        Entity e2;
        
        for(int i=0; i<childCount; i++){
            child = (EntityTreeNode)(this.getChildAt(i));
            e2 = (Entity)(child.getUserObject());
            
            if( (e2.getIndex() == e.getIndex()) &&
                (e2.getSerial() == e.getSerial())
               ){
                return child;
            }
        }
        return null;
    }
    
    /**
     * Whether or not the node currently is expanded.
     */
    protected boolean expanded;
    
    /**
     * The tree the node is located in.
     */
    protected EntityTree tree;
    
    /**
     * Whether or not to free the Entity of this node when it is removed.
     */
    protected boolean entityFreeEnabled;
    
    /**
     * Whether or not children of this node are visible.
     */
    protected boolean childrenVisible;
    
    protected boolean recursiveVisible;
}
