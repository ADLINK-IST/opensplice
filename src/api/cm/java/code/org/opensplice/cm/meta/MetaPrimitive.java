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
package org.opensplice.cm.meta;

/**
 * Represents a primitive field in a type.
 * 
 * @date May 24, 2004
 */
public class MetaPrimitive extends MetaField{
    
    /**
     * Constructs a new primitive field.
     * 
     * @param _name The name of the primitive.
     * @param _typeName The type of the primitive.
     */
    public MetaPrimitive(String _name, String _typeName) {
        super(_name, _typeName);
    }
}
