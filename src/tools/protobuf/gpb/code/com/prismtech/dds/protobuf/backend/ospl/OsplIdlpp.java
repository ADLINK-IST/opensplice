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
 */
package com.prismtech.dds.protobuf.backend.ospl;

import java.io.File;

import com.prismtech.dds.protobuf.ProtoParseException;
import com.prismtech.dds.protobuf.backend.Idlpp;

public abstract class OsplIdlpp implements Idlpp {
    protected String osplHome;
    protected String idlppCommand;

    public OsplIdlpp() throws ProtoParseException {
        osplHome = System.getenv("OSPL_HOME_NORMALIZED");

        if (osplHome == null) {
            osplHome = System.getenv("OSPL_HOME");
        }
        if (osplHome == null) {
            throw new ProtoParseException(
                    "OSPL_HOME not set. Please ensure OpenSplice environment is set up correctly.");
        }
        String spliceTarget = System.getenv("SPLICE_TARGET");

        if (spliceTarget == null) {
            idlppCommand = osplHome + File.separator + "bin" + File.separator
                    + "idlpp";
        } else {
            idlppCommand = osplHome + File.separator + "exec" + File.separator
                    + spliceTarget + File.separator + "idlpp";
        }
    }
}
