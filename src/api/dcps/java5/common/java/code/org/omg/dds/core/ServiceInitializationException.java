/* Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Inc.
 * Copyright 2010, Real-Time Innovations, Inc.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.omg.dds.core;


/**
 * Indicates that a DDS implementation could not be initialized due to an
 * error that occurred within that implementation.
 */
public class ServiceInitializationException extends RuntimeException {
    // -----------------------------------------------------------------------
    // Constants
    // -----------------------------------------------------------------------

    private static final long serialVersionUID = 323168673886924537L;



    // -----------------------------------------------------------------------
    // Object Life Cycle
    // -----------------------------------------------------------------------

    public ServiceInitializationException() {
        super();
    }

    public ServiceInitializationException(String message) {
        super(message);
    }

    public ServiceInitializationException(Throwable cause) {
        super(cause);
    }

    public ServiceInitializationException(String message, Throwable cause) {
        super(message, cause);
    }

}
