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


public abstract class PreconditionNotMetException extends DDSException
{
    // -----------------------------------------------------------------------
    // Private Fields
    // -----------------------------------------------------------------------

    private static final long serialVersionUID = -6204457556635480597L;



    // -----------------------------------------------------------------------
    // Object Life Cycle
    // -----------------------------------------------------------------------

    protected PreconditionNotMetException() {
        super();
    }

    protected PreconditionNotMetException(String message) {
        super(message);
    }

    protected PreconditionNotMetException(Throwable cause) {
        super(cause);
    }

    protected PreconditionNotMetException(String message, Throwable cause) {
        super(message, cause);
    }

}
