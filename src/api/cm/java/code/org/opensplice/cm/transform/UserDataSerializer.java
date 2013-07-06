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
package org.opensplice.cm.transform;

import org.opensplice.cm.data.UserData;

/**
 * Interface that must be implemented by each class that offers facilities
 * for serializing UserData and that wants be supported by the 
 * DataTransformerFactory.
 * 
 * @date Jun 2, 2004
 */
public interface UserDataSerializer {
    /**
     * Serializes a UserData object into a serialized representation.
     * 
     * @param data The UserData object that needs to be serialized.
     * @return The serialized UserData representation.
     */
    public String serializeUserData(UserData data) throws TransformationException;
}

