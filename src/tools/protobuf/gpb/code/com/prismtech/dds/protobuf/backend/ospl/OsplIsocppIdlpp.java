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
import com.prismtech.dds.protobuf.frontend.MetaFile;

public class OsplIsocppIdlpp extends OsplIdlpp {

    public OsplIsocppIdlpp() throws ProtoParseException {
        super();
    }

    @Override
    public String[] getIdlCommand(File idlFile, MetaFile metadata)
            throws ProtoParseException {
        String[] idlppCommand = new String[10];
        int optionIndex = 0;
        idlppCommand[optionIndex++] = this.idlppCommand;
        idlppCommand[optionIndex++] = "-l";
        idlppCommand[optionIndex++] = "isocpp2";
        idlppCommand[optionIndex++] = "-S";
        idlppCommand[optionIndex++] = "-N";
        idlppCommand[optionIndex++] = "-I";
        idlppCommand[optionIndex++] = osplHome + java.io.File.separator + "etc"
                + java.io.File.separator + "idl";
        idlppCommand[optionIndex++] = "-d";
        idlppCommand[optionIndex] = metadata.getProtoFile().getName();

        int index = idlppCommand[optionIndex].indexOf(java.io.File.separator);

        if (index != -1) {
            idlppCommand[optionIndex] = idlppCommand[optionIndex].substring(0,
                    index);
        } else {
            idlppCommand[optionIndex] = ".";
        }
        idlppCommand[++optionIndex] = idlFile.getName();

        return idlppCommand;
    }
}
