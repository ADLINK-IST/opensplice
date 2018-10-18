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

import org.omg.dds.core.ServiceEnvironment;
import org.omg.dds.core.policy.TypeConsistencyEnforcement;
import org.opensplice.dds.core.OsplServiceEnvironment;

public class TypeConsistencyEnforcementImpl implements TypeConsistencyEnforcement {
    private static final long serialVersionUID = -5160265825622794754L;
    private final Kind kind;
    private final transient OsplServiceEnvironment environment;

    public TypeConsistencyEnforcementImpl(OsplServiceEnvironment environment, Kind kind){
        this.environment = environment;
        this.kind = kind;
    }

    @Override
    public ServiceEnvironment getEnvironment() {
        return this.environment;
    }

    @Override
    public Comparable<TypeConsistencyEnforcement> requestedOfferedContract() {
        return this;
    }

    @Override
    public int compareTo(TypeConsistencyEnforcement o) {
        Kind other = o.getKind();
        
        if(other.equals(this.kind)){
            return 0;
        } else if(this.kind.equals(Kind.EXACT_TYPE_TYPE_CONSISTENCY)){
            return 1;
        } else if(this.kind.equals(Kind.EXACT_NAME_TYPE_CONSISTENCY)){
            if(other.equals(Kind.EXACT_TYPE_TYPE_CONSISTENCY)){
                return -1;
            }
            return 1;
        } else if(this.kind.equals(Kind.DECLARED_TYPE_CONSISTENCY)){
            if(other.equals(Kind.ASSIGNABLE_TYPE_CONSISTENCY)){
                return 1;
            }
            return -1;
        }
        return -1;
    }

    @Override
    public boolean equals(Object other) {
        if (other instanceof TypeConsistencyEnforcementImpl) {
            if (this.compareTo((TypeConsistencyEnforcementImpl) other) == 0) {
                return true;
            }
        }
        return false;
    }

    @Override
    public int hashCode() {
        return 31 * 17 + this.kind.ordinal();
    }

    @Override
    public Kind getKind() {
        return this.kind;
    }

    @Override
    public TypeConsistencyEnforcement withKind(Kind kind) {
        return new TypeConsistencyEnforcementImpl(this.environment, kind);
    }

    @Override
    public TypeConsistencyEnforcement withExactTypeTypeConsistency() {
        return new TypeConsistencyEnforcementImpl(this.environment, Kind.EXACT_TYPE_TYPE_CONSISTENCY);
    }

    @Override
    public TypeConsistencyEnforcement withExactNameTypeConsistency() {
        return new TypeConsistencyEnforcementImpl(this.environment, Kind.EXACT_NAME_TYPE_CONSISTENCY);
    }

    @Override
    public TypeConsistencyEnforcement withDeclaredTypeConsistency() {
        return new TypeConsistencyEnforcementImpl(this.environment, Kind.DECLARED_TYPE_CONSISTENCY);
    }

    @Override
    public TypeConsistencyEnforcement withAssignableTypeConsistency() {
        return new TypeConsistencyEnforcementImpl(this.environment, Kind.ASSIGNABLE_TYPE_CONSISTENCY);
    }

}
