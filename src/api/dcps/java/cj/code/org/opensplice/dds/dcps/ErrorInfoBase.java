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

public class ErrorInfoBase extends ObjectImpl {

    @Override
    protected int deinit() {
        return super.deinit();
    }

    public static class ErrorInfoBaseImpl extends DDS._ErrorInfoInterfaceLocalBase {
        private static final long serialVersionUID = 2148256024023115212L;

        /* operations */
        @Override
        public int update() {
            return 0;
        }

        @Override
        public int get_code(org.omg.CORBA.IntHolder code) {
            return 0;
        }

        @Override
        public int get_message(org.omg.CORBA.StringHolder message) {
            return 0;
        }

        @Override
        public int get_location(org.omg.CORBA.StringHolder location) {
            return 0;
        }

        @Override
        public int get_source_line(org.omg.CORBA.StringHolder source_line) {
            return 0;
        }

        @Override
        public int get_stack_trace(org.omg.CORBA.StringHolder stack_trace) {
            return 0;
        }
    }

    private DDS._ErrorInfoInterfaceLocalBase base;

    public ErrorInfoBase() {
        base = new ErrorInfoBaseImpl();
    }

    @Override
    public String[] _ids() {
        return base._ids();
    }

}
