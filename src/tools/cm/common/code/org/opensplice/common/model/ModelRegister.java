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
package org.opensplice.common.model;

import java.util.Collections;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;

/** 
 * Provides facilities for (de)registering and notifying ModelListener
 * components. Descendants of this class typically are model components that 
 * need to notify registered components, because the data model changed. 
 * This class has been defiend abstract, because only descendants of this class
 * may exist.
 * 
 * @date Sep 21, 2004
 */
public abstract class ModelRegister {
    
    /**
     * Set of registered listeners that will be notified on changes.
     */
    protected Set listeners = null;
    
    /**
     * Creates a new ModelRegister. ModelListener components can register with
     * this component and will be notified on changes. 
     */
    public ModelRegister(){
        listeners   = Collections.synchronizedSet(new HashSet());
    }
    /**
     * Registers the supplied entity in its administration.
     * 
     * The listener will now receive a notification an every event.
     * If it is already in its administration, nothing happens.
     * 
     * @param listener The listener to add to the administration.
     */
    public void addListener(ModelListener listener){
        synchronized(listeners){
            listeners.add(listener);
        }
    }
    
    /**
     * Removes the supplied listener from its administration.
     * 
     * The listener will no longer receive any notifications from this component. 
     * When the listener is not in its administration, nothing will happen.
     * 
     * @param listener The listener to remove from the registration.
     */
    public void removeListener(ModelListener listener){
        synchronized(listeners){
            listeners.remove(listener);
        }
    }
    
    /**
     * Notifies all registered listeners.
     * 
     * @param description The message to send to the listeners.
     */
    protected void notifyListeners(String description){
        Iterator listenerIter = null;
        ModelListener listener = null;
        
        synchronized(listeners){
            listenerIter = listeners.iterator();
            
            while(listenerIter.hasNext()){
                listener = (ModelListener)listenerIter.next();
                listener.update(description);                    
            }
        }
    }
    
    /**
     * Allows external components to send a notification to registered
     * listeners. This function is typically being used by other model
     * components then the descendant of ModelRegister itself.
     * 
     * @param description The message to send to the listeners.
     */
    public void pushUpdate(String description){
        this.notifyListeners(description);
    }
    
    
}
