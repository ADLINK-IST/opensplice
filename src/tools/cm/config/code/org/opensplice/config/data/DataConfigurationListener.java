/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
package org.opensplice.config.data;

public interface DataConfigurationListener {
    public void valueChanged(DataValue data, Object oldValue, Object newValue);
    
    public void nodeAdded(DataElement parent, DataNode nodeAdded);
    
    public void nodeRemoved(DataElement parent, DataNode nodeRemoved);
    
}
