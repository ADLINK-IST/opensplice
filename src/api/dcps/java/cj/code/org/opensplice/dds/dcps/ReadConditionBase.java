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

public abstract class ReadConditionBase extends ConditionImpl {
    private static final long serialVersionUID = 3818811868747494840L;

    @Override
    protected int deinit() {
        return super.deinit();
    }

    public class ReadConditionBaseImpl extends DDS._ReadConditionLocalBase {
        private static final long serialVersionUID = 1076540784430246687L;

        /* readcondition operations */
        @Override
        public int get_sample_state_mask() {
            return 0;
        }

        @Override
        public int get_view_state_mask() {
            return 0;
        }

        @Override
        public int get_instance_state_mask() {
            return 0;
        }

        @Override
        public DDS.DataReader get_datareader() {
            return null;
        }

        @Override
        public DDS.DataReaderView get_datareaderview() {
            return null;
        }

        /* condition operations */
        @Override
        public boolean get_trigger_value() {
            return false;
        }
    }

    private DDS._ReadConditionLocalBase base;

    public ReadConditionBase() {
        base = new ReadConditionBaseImpl();
    }

    @Override
    public String[] _ids() {
        return base._ids();
    }

}
