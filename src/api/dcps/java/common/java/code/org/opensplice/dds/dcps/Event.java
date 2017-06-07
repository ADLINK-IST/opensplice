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


package org.opensplice.dds.dcps;



final class Event {

    public static final int INCONSISTENT_TOPIC         = 1;
    public static final int OFFERED_DEADLINE_MISSED    = 2;
    public static final int REQUESTED_DEADLINE_MISSED  = 3;
    public static final int OFFERED_INCOMPATIBLE_QOS   = 4;
    public static final int REQUESTED_INCOMPATIBLE_QOS = 5;
    public static final int SAMPLE_LOST                = 6;
    public static final int SAMPLE_REJECTED            = 7;
    public static final int DATA_ON_READERS            = 8;
    public static final int DATA_AVAILABLE             = 9;
    public static final int LIVELINESS_LOST            = 10;
    public static final int LIVELINESS_CHANGED         = 11;
    public static final int PUBLICATION_MATCHED        = 12;
    public static final int SUBSCRIPTION_MATCHED       = 13;
    public static final int TRIGGER                    = 1 << 19;
    public static final int PREPARE_DELETE             = 1 << 28;
    public static final int OBJECT_DESTROYED           = 1 << 30;
    public static final int ALL_DATA_DISPOSED          = 1 << 31;

    public EntityImpl observable;
    public EntityImpl observer;
    public int kind;
    public Object status;

    public Event(int _kind, EntityImpl _observable, EntityImpl _observer, Object _status) {
        this.observable = _observable;
        this.observer = _observer;
        this.kind = _kind;
        this.status = _status;
    }
}
