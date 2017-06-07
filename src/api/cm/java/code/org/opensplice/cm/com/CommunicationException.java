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
package org.opensplice.cm.com;

/**
 * Represents an exception that occurred in the communication package. When an
 * exception of this kind occurs, something went wrong in the communication
 * layer (org.opensplice.api.cm.com).
 * 
 * @date Jan 17, 2005
 */
public class CommunicationException extends Exception{

    private static final long serialVersionUID = -9217918239490461941L;

    /**
     * Constructs a new CommunicationException.
     *
     * @param message The exception error message.
     */
    public CommunicationException(String message){
        super(message);
    }
}
