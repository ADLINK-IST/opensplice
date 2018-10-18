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
package org.opensplice.dds.core.policy;

import org.omg.dds.core.policy.History;
import org.omg.dds.core.policy.QosPolicy;
import org.opensplice.dds.core.IllegalArgumentExceptionImpl;
import org.opensplice.dds.core.OsplServiceEnvironment;

public class HistoryImpl extends QosPolicyImpl implements History{
    private static final long serialVersionUID = 8740123506122702059L;
    private final Kind kind;
    private final int depth;

    public HistoryImpl(OsplServiceEnvironment environment) {
        super(environment);
        this.kind = Kind.KEEP_LAST;
        this.depth = 1;
    }

    public HistoryImpl(OsplServiceEnvironment environment, Kind kind, int depth) {
        super(environment);

        if (kind == null) {
            throw new IllegalArgumentExceptionImpl(environment,
                    "Supplied invalid kind.");
        }
        this.kind = kind;
        this.depth = depth;
    }

    @Override
    public Kind getKind() {
        return this.kind;
    }

    @Override
    public int getDepth() {
        return this.depth;
    }

    @Override
    public History withKind(Kind kind) {
        return new HistoryImpl(this.environment, kind, this.depth);
    }

    @Override
    public History withDepth(int depth) {
        return new HistoryImpl(this.environment, this.kind, depth);

    }

    @Override
    public History withKeepAll() {
        return new HistoryImpl(this.environment, Kind.KEEP_ALL, this.depth);

    }

    @Override
    public History withKeepLast(int depth) {
        return new HistoryImpl(this.environment, Kind.KEEP_LAST, depth);

    }

    @Override
    public Class<? extends QosPolicy> getPolicyClass() {
        return History.class;
    }

    @Override
    public boolean equals(Object other) {
        if (!(other instanceof HistoryImpl)) {
            return false;
        }
        HistoryImpl h = (HistoryImpl) other;

        if (this.kind != h.kind) {
            return false;
        }
        return (this.depth == h.depth);
    }

    @Override
    public int hashCode() {
        final int prime = 31;
        int result = 17;

        result = prime * result + this.kind.hashCode();
        result = prime * result + this.depth;

        return result;
    }
}
