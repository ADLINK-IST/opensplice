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
package org.opensplice.dds.core;

import java.util.EventListener;
import java.util.Set;
import java.util.concurrent.atomic.AtomicInteger;

import org.omg.dds.core.EntityQos;
import org.omg.dds.core.InstanceHandle;
import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.core.status.Status;
import org.opensplice.dds.core.status.StatusConverter;

public abstract class EntityImpl<OLD extends DDS.Entity, OLDPARENT, QOS extends EntityQos<?>, LISTENER extends EventListener, LISTENERIMPL extends Listener<LISTENER>>
        extends AbstractDDSObject implements org.omg.dds.core.Entity<LISTENER, QOS>, Properties {
    protected final transient OsplServiceEnvironment environment;
    private final OLDPARENT oldParent;
    private OLD old;
    protected LISTENERIMPL listener;
    private AtomicInteger refCount;
    private static java.util.HashMap<String, Integer> references = new java.util.HashMap<String, Integer>();

    private static boolean addReference(String name) {
        synchronized (references) {
            Integer refs = references.get(name);

            if (refs == null) {
                references.put(name, 1);
            } else {
                references.put(name, refs.intValue() + 1);
            }
        }
        return true;
    }

    private static boolean removeReference(String name) {
        synchronized (references) {
            Integer refs = references.get(name);

            assert (refs != null);
            references.put(name, refs.intValue() - 1);
        }
        return true;
    }

    public static boolean printReferences() {
        boolean noMoreRefs = true;

        synchronized (references) {
            for (String name : references.keySet()) {
                int count = references.get(name);
                if (count != 0) {
                    noMoreRefs = false;
                }
                System.out.println(name + "=" + count);
            }
            if (!noMoreRefs) {
                System.out.println("----------------");

                for (String name : references.keySet()) {
                    int count = references.get(name);
                    if (count != 0) {
                        noMoreRefs = false;
                    }
                    System.out.println(name + "=" + count);
                }
                System.out.println("----------------");
            }
        }
        return noMoreRefs;
    }

    public EntityImpl(OsplServiceEnvironment environment, OLDPARENT oldParent) {
        this.environment = environment;
        this.oldParent = oldParent;
        this.listener = null;
        this.refCount = new AtomicInteger(1);

        assert (addReference(this.getClass().getSimpleName()));
    }

    public OLD getOld() {
        final OLD old = this.old;

        if (old == null) {
            throw new AlreadyClosedExceptionImpl(this.environment,
                    "Entity already closed.");
        }
        return old;
    }

    protected void setOld(OLD old) {
        this.old = old;
    }

    public OLDPARENT getOldParent() {
        return this.oldParent;
    }

    @Override
    public ServiceEnvironment getEnvironment() {
        return this.environment;
    }

    @Override
    public final void retain() {
        int newValue = this.refCount.incrementAndGet();

        if (newValue <= 0) {
            this.refCount.decrementAndGet();
            throw new AlreadyClosedExceptionImpl(this.environment,
                    "Entity already closed.");
        }
        assert (newValue > newValue - 1);
        assert (newValue > 1);
    }

    @Override
    public final void close() {
        int newValue = this.refCount.decrementAndGet();

        if (newValue == 0) {
            this.destroy();
            this.old = null;
            assert (removeReference(this.getClass().getSimpleName()));
        } else if (newValue < 0) {
            throw new AlreadyClosedExceptionImpl(this.environment,
                    "Entity already closed.");
        }
    }

    @Override
    public final void enable() {
        int rc = this.old.enable();
        Utilities.checkReturnCode(rc, this.environment,
                "Entity.enable() failed.");
    }

    @Override
    public final Set<Class<? extends Status>> getStatusChanges() {
        return StatusConverter.convertMask(this.environment,
                this.old.get_status_changes());
    }

    @Override
    public final InstanceHandle getInstanceHandle() {
        return new InstanceHandleImpl(this.environment,
                old.get_instance_handle());
    }

    protected abstract void destroy();

    @Override
    public final LISTENER getListener(){
        if (this.listener != null) {
            return this.listener.getRealListener();
        }
        return null;
    }

    @Override
    public void setProperty(String key, String value) {
        int rc = this.old.set_property(new DDS.Property(key, value));
        Utilities.checkReturnCode(rc, this.environment,
                "Properties.setProperty() failed.");
    }

    @Override
    public String getProperty(String key) {
        DDS.PropertyHolder holder = new DDS.PropertyHolder();
        holder.value = new DDS.Property(key, null);
        int rc = this.old.get_property(holder);
        Utilities.checkReturnCode(rc, this.environment,
                "Properties.getProperty() failed.");
        return holder.value.value;
    }
}
