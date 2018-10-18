/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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
package org.opensplice.cmdataadapter;

/**
 * CmDataException exception.
 * 
 * The exception provides information about the exception that occurred while
 * adapting data. This information can be accessed by calling the
 * getMessage function.
 */
public class CmDataException extends Exception {

    private static final long serialVersionUID = 3533145785576083180L;
    public static final String PROTOBUF_NOT_INCLUDED = "Support for Google Protocol Buffer data is disabled in this build.";

    /**
     * Initializes the exception.
     *
     * @param message The message that is associated with the exception.
     */
    public CmDataException(String message){
        super(message);
    }

    /**
     * Initializes the exception with cause.
     *
     * @param message The message that is associated with the exception.
     * @param cause The underlying exception that caused this Exception.
     */
    public CmDataException(String message, Throwable cause){
        super(message, cause);
    }
}
