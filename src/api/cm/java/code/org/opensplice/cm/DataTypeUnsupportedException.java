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
package org.opensplice.cm;

/**
 * Exception that is used for notifying that a specific MetaType is not
 * supported by this API.
 * 
 * @date Mar 4, 2005
 */
public class DataTypeUnsupportedException extends Exception {

    private static final long serialVersionUID = 4527068637497945627L;

    /**
     * Constructs a new exception.
     * 
     * @param message
     *            The message that provides information about why the data type
     *            is not supported.
     */
    public DataTypeUnsupportedException(String message){
        super(message);
    }
}
