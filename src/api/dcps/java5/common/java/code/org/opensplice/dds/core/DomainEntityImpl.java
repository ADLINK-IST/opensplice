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

import org.omg.dds.core.EntityQos;

public abstract class DomainEntityImpl<
        OLD extends DDS.Entity,
        PARENT extends EntityImpl<?, ?, ?, ?, ?>,
        OLDPARENT extends DDS.Entity,
        QOS extends EntityQos<?>,
        LISTENER extends EventListener,
        LISTENERIMPL extends Listener<LISTENER>>
        extends EntityImpl<
            OLD,
            OLDPARENT,
            QOS, LISTENER,
            LISTENERIMPL>
            implements org.omg.dds.core.DomainEntity<LISTENER, QOS> {
    protected PARENT parent;

    public DomainEntityImpl(OsplServiceEnvironment environment, PARENT parent,
            OLDPARENT oldParent) {
        super(environment, oldParent);
        this.parent = parent;
    }
}
