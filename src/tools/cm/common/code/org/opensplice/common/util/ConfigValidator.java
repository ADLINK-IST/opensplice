/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
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
