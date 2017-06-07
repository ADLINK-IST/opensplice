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
 * The target object was previously closed and therefore cannot process
 * the operation.
 */
public abstract class AlreadyClosedException extends DDSException
{
    // -----------------------------------------------------------------------
    // Private Fields
    // -----------------------------------------------------------------------

    private static final long serialVersionUID = 8818392556145975401L;



    // -----------------------------------------------------------------------
    // Object Life Cycle
    // -----------------------------------------------------------------------

    protected AlreadyClosedException() {
        super();
    }

    protected AlreadyClosedException(String message) {
        super(message);
    }

    protected AlreadyClosedException(Throwable cause) {
        super(cause);
    }

    protected AlreadyClosedException(String message, Throwable cause) {
        super(message, cause);
    }

}
