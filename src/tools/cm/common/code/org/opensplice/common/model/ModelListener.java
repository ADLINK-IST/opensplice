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
/**
 * Contains all common SPLICE DDS C&M Tooling model components. 
 */
package org.opensplice.common.model;

/**
 * Interface that offers the possibility to be notified on model changes.
 * 
 * @date Apr 26, 2004
 */
public interface ModelListener {
    /**
     * Notifies model events. When a ModelListener is registered with a 
     * ModelRegister component, the ModelRegister will notify the ModelListener
     * when something in the ModelRegister changed. A ModelListener is typically
     * a view component that needs to update itself when a model change occurs.
     * 
     * @param description Description of the event that occurred.
     */
    public void update(String description);
}

