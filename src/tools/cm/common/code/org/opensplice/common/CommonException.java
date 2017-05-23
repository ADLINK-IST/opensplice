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
/**
 * Contains all classes that are common for SPLICE DDS C&M Tooling.
 */
package org.opensplice.common;

import org.opensplice.cm.CMException;

/**
 * Default exception for the common package.
 * 
 * @date Nov 4, 2004 
 */
public class CommonException extends Exception {
    public static final String CONNECTION_LOST = CMException.CONNECTION_LOST;
    /**
     * Creates a new CommonException with the supplied message. 
     *
     * @param message The message to attach to the exception.
     */
    public CommonException(String message){
        super(message);
    }
}
