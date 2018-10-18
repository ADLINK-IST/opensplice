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
 */
package com.prismtech.dds.protobuf.backend.cafe;

import java.util.List;

import com.google.protobuf.compiler.PluginProtos.CodeGeneratorResponse.File;
import com.prismtech.dds.protobuf.ProtoParseException;
import com.prismtech.dds.protobuf.backend.CodeGenerator;
import com.prismtech.dds.protobuf.frontend.MetaFile;

// NOTE: this class is overwritten with a complete implementation (from cafe reop) during Cafe build.
public class CafeJava5CodeGenerator extends CodeGenerator {

    public CafeJava5CodeGenerator(MetaFile metadata) throws ProtoParseException {
        super(metadata, new CafeJava5Idlpp());
    }

    @Override
    public List<File> generateCode() throws ProtoParseException {
        // Overwritten in Cafe code
        throw new ProtoParseException(
                "Code generation for Cafe Java5 PSM not implemented yet.");
    }

}
