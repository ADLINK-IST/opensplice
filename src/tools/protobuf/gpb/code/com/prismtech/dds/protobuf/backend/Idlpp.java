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
package com.prismtech.dds.protobuf.backend;

import java.io.File;

import com.prismtech.dds.protobuf.ProtoParseException;
import com.prismtech.dds.protobuf.frontend.MetaFile;

public interface Idlpp {
    /**
     * Returns the IDL pre-processor command including all command line options
     * to generate code for the product and programming language associated with
     * this Idlpp instance using the supplied IDL file and meta-data. Each
     * implementation must assume that the command is ran in the same directory
     * as where the supplied IDL file is located. The code that is generated
     * when the returned command is invoked must be in that directory too.
     * 
     * @param idlFile
     *            The IDL file to generate code for. The file is guaranteed to
     *            be located in the same directory the command will be ran in.
     * @param metadata
     *            The meta data that belongs to the proto file associated with
     *            the IDL file.
     * @return The IDL pre-processing command (first element) including command
     *         line options (consecutive elements) to generate code in the
     *         directory relative to where the idlFile is located.
     * @throws ProtoParseException
     *             Thrown when something goes wrong during creation of the
     *             command or when invalid input is supplied.
     */
    public String[] getIdlCommand(File idlFile, MetaFile metadata)
            throws ProtoParseException;

}
