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
package org.opensplice.common.util;

/**
 * Interface that provides routines for validating configuration. 
 * Implementations of this interface can guarantee that the configuration for 
 * an application will be correct.
 * 
 * @date Jan 12, 2005 
 */
public interface ConfigValidator {
    /**
     * Returns the correct value for the supplied key. This function
     * checks if the supplied value is valid. If so, return that value. If not
     * return a valid one. The Config component calls this routine for each
     * key it finds when loading a configuration file.
     * 
     * @param key The key in the configuration.
     * @param value The value as it has been found in the confuration file.
     * @return A correct value for that key. If null is returned, the Config 
     *         component will remove the key from the configuration.
     */
    public String getValidatedValue(String key, String value);
 
    /**
     * Returns the default value for the supplied key. This function is called
     * by the Config component when an application asks for a property that has
     * not been defined.
     * 
     * @param key The key where to resolve the default value of. 
     * @return The default value for the supplied key.
     */
    public String getDefaultValue(String key);
    
    /**
     * Checks whether the supplied key/value combination is valid. The Config
     * component calls this function when an application sets a property in the
     * configuration.
     * 
     * @param key The key of the property.
     * @param value The value of the property.
     * @return If the combination is valid; true and false otherwise.
     */
    public boolean isValueValid(String key, String value);
}
