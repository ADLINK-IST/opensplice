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
package org.opensplice.cm;

/**
 * Represents a waitset in SPLICE-DDS.
 * 
 * @date Oct 28, 2004 
 */
public interface Waitset extends Entity {
    public void attach(Entity entity) throws CMException;
    
    public void detach(Entity entity) throws CMException;
    
    public Entity[] _wait() throws CMException;
    
    public Entity[] timedWait(Time time) throws CMException;
    
    public int getEventMask() throws CMException;
    
    public void setEventMask(int mask) throws CMException;
}
