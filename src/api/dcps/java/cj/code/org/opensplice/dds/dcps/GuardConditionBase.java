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

public abstract class GuardConditionBase extends ConditionImpl {
    private static final long serialVersionUID = 7462934311358725264L;

    @Override
    protected int deinit() {
        return super.deinit();
    }

    public class GuardConditionBaseImpl extends
            DDS._GuardConditionInterfaceLocalBase {
        private static final long serialVersionUID = -474617353646821643L;

        /* guardconditioninterface operations */
        @Override
        public int set_trigger_value(boolean value) {
            return 0;
        }

        /* condition operations */
        @Override
        public boolean get_trigger_value() {
            return false;
        }
    }

    private DDS._GuardConditionInterfaceLocalBase base;

    public GuardConditionBase() {
        base = new GuardConditionBaseImpl();
    }

    @Override
    public String[] _ids() {
        return base._ids();
    }

}
