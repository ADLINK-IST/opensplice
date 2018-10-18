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
package org.opensplice.dds.core;

import java.io.Serializable;
import java.util.EventListener;
import java.util.concurrent.CountDownLatch;

import org.eclipse.cyclone.dds.core.AbstractDDSObject;
import org.omg.dds.core.ServiceEnvironment;

public abstract class Listener<T> extends AbstractDDSObject implements EventListener, Serializable {
    private static final long serialVersionUID = 5928369585907075474L;
    private final CountDownLatch initialised;
    protected OsplServiceEnvironment environment;
    protected T listener;

    public Listener(OsplServiceEnvironment environment, T listener,
            boolean waitUntilInitialised) {
        this.environment = environment;
        this.listener = listener;

        if (waitUntilInitialised) {
            this.initialised = new CountDownLatch(1);
        } else {
            this.initialised = new CountDownLatch(0);
        }
    }

    public ServiceEnvironment getEnvironment() {
        return this.environment;
    }

    public T getRealListener(){
        return this.listener;
    }

    public void setInitialised() {
        this.initialised.countDown();
    }

    public void waitUntilInitialised() {
        try {
            this.initialised.await();
        } catch (InterruptedException e) {
            throw new AlreadyClosedExceptionImpl(this.environment,
                    "Listener interrupted.");
        }
    }
}
