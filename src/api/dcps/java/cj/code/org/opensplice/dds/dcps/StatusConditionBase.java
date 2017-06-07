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

public abstract class StatusConditionBase extends ConditionImpl {

    private static final long serialVersionUID = -3365224307620167095L;

    @Override
    protected int deinit() {
        return super.deinit();
    }

    public class StatusConditionBaseImpl extends DDS._StatusConditionLocalBase {
        private static final long serialVersionUID = 4982853180917696601L;

        /* statuscondition operations */
        @Override
        public int get_enabled_statuses() {
            return 0;
        }

        @Override
        public int set_enabled_statuses(int mask) {
            return 0;
        }

        @Override
        public DDS.Entity get_entity() {
            return null;
        }

        /* condition operations */
        @Override
        public boolean get_trigger_value() {
            return false;
        }
    }

    private DDS._StatusConditionLocalBase base;

    public StatusConditionBase() {
        base = new StatusConditionBaseImpl();
    }

    @Override
    public String[] _ids() {
        return base._ids();
    }

}
