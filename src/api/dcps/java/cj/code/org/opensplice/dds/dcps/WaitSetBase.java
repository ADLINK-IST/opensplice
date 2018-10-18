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

package org.opensplice.dds.dcps;

public class WaitSetBase extends ObjectImpl {

    @Override
    protected int deinit() {
        return super.deinit();
    }

    public static class WaitSetBaseImpl extends DDS._WaitSetInterfaceLocalBase {
        private static final long serialVersionUID = 4781909398140097734L;

        /* waitsetinterface operations */
        @Override
        public int _wait(DDS.ConditionSeqHolder active_conditions,
                DDS.Duration_t timeout) {
            return 0;
        }

        @Override
        public int attach_condition(DDS.Condition cond) {
            return 0;
        }

        @Override
        public int detach_condition(DDS.Condition cond) {
            return 0;
        }

        @Override
        public int get_conditions(DDS.ConditionSeqHolder attached_conditions) {
            return 0;
        }
    }

    private DDS._WaitSetInterfaceLocalBase base;

    public WaitSetBase() {
        base = new WaitSetBaseImpl();
    }

    @Override
    public String[] _ids() {
        return base._ids();
    }

}
