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

import java.util.Collection;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;

import org.omg.dds.core.EntityQos;
import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.core.policy.PolicyFactory;
import org.omg.dds.core.policy.QosPolicy;

public abstract class EntityQosImpl<T extends QosPolicy> implements
        EntityQos<T> {
    private static final long serialVersionUID = -5714560284083338546L;
    protected final OsplServiceEnvironment environment;
    protected HashMap<Class<? extends T>, T> policies;

    public EntityQosImpl(OsplServiceEnvironment environment,
            Collection<T> policies) {
        this.environment = environment;
        this.policies = new HashMap<Class<? extends T>, T>();
        this.setupPolicies(policies);
        this.setupMissingPolicies();
    }

    public EntityQosImpl(OsplServiceEnvironment environment, T... policies) {
        this.environment = environment;
        this.policies = new HashMap<Class<? extends T>, T>();
        this.setupPolicies(policies);
        this.setupMissingPolicies();
    }

    protected void setupPolicies(Collection<T> policies) {
        synchronized(this.policies) {
            for (T p : policies) {
                this.policies.put(this.getClassIdForPolicy(p), p);
            }
        }
    }

    protected void setupPolicies(T... policies) {
        synchronized(this.policies) {
            for (T p : policies) {
                this.policies.put(this.getClassIdForPolicy(p), p);
            }
        }
    }

    @SuppressWarnings("unchecked")
    protected Class<? extends T> getClassIdForPolicy(T policy) {
        Class<?>[] interfaces = policy.getClass().getInterfaces();

        if (interfaces.length == 0) {
            throw new IllegalArgumentExceptionImpl(this.environment,
                    "Provided policy not a valid policy.");
        }
        /*
         * Ensure extended policies in OpenSplice are still stored using the OMG
         * class if it exists (as some policies are OpenSplice only, not all of
         * them have a OMG parent).
         */
        if (interfaces[0].getName().startsWith(
                "org.opensplice.dds.core.policy.")) {
            Class<?>[] superInterfaces = interfaces[0].getInterfaces();

            if (superInterfaces.length != 0) {
                if (!superInterfaces[0].getName().startsWith(
                        "org.omg.dds.core.policy.QosPolicy")) {
                    return (Class<? extends T>) (superInterfaces[0]);
                }
            }
        }
        return (Class<? extends T>) interfaces[0];
    }

    @Override
    public ServiceEnvironment getEnvironment() {
        return this.environment;
    }

    @Override
    public void clear() {
        synchronized (this.policies) {
            this.policies.clear();
            this.setupMissingPolicies();
        }
    }

    @Override
    public boolean containsKey(Object arg0) {
        synchronized (this.policies) {
            return this.policies.containsKey(arg0);
        }
    }

    @Override
    public boolean containsValue(Object arg0) {
        synchronized (this.policies) {
            return this.policies.containsValue(arg0);
        }
    }

    @Override
    public Set<java.util.Map.Entry<Class<? extends T>, T>> entrySet() {
        synchronized (this.policies) {
            return this.policies.entrySet();
        }
    }

    @Override
    public T get(Object arg0) {
        synchronized (this.policies) {
            return this.policies.get(arg0);
        }
    }

    @Override
    public boolean isEmpty() {
        return false;
    }

    @Override
    public Set<Class<? extends T>> keySet() {
        synchronized (this.policies) {
            return this.policies.keySet();
        }
    }

    @Override
    public T put(Class<? extends T> arg0, T arg1) {
        synchronized (this.policies) {
            return this.policies.put(arg0, arg1);
        }
    }

    @Override
    public void putAll(Map<? extends Class<? extends T>, ? extends T> arg0) {
        synchronized (this.policies) {
            this.policies.putAll(arg0);
        }
    }

    @Override
    public T remove(Object arg0) {
        T removed = null;

        synchronized (this.policies) {
            removed = this.policies.remove(arg0);
            this.setupMissingPolicies();
        }
        return removed;
    }

    @Override
    public int size() {
        synchronized (this.policies) {
            return this.policies.size();
        }
    }

    @Override
    public Collection<T> values() {
        synchronized (this.policies) {
            return this.policies.values();
        }
    }

    @Override
    public <POLICY extends T> POLICY get(Class<POLICY> id) {
        synchronized (this.policies) {
            return id.cast(this.policies.get(id));
        }
    }

    @Override
    public PolicyFactory getPolicyFactory() {
        return PolicyFactory.getPolicyFactory(this.environment);
    }

    protected abstract void setupMissingPolicies();
}
