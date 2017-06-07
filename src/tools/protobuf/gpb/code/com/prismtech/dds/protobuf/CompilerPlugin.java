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
package com.prismtech.dds.protobuf;

import java.io.IOException;
import java.util.List;
import java.util.logging.Level;

import com.google.protobuf.DescriptorProtos.FileDescriptorProto;
import com.google.protobuf.DescriptorProtos.FileDescriptorSet;
import com.google.protobuf.compiler.PluginProtos.CodeGeneratorRequest;
import com.google.protobuf.compiler.PluginProtos.CodeGeneratorResponse;
import com.google.protobuf.compiler.PluginProtos.CodeGeneratorResponse.File;
import com.prismtech.dds.protobuf.backend.CodeGenerator;
import com.prismtech.dds.protobuf.backend.cafe.CafeJava5CodeGenerator;
import com.prismtech.dds.protobuf.backend.ospl.OsplIsocppCodeGenerator;
import com.prismtech.dds.protobuf.backend.ospl.OsplJava5CodeGenerator;
import com.prismtech.dds.protobuf.backend.lite.LiteIsocppCodeGenerator;
import com.prismtech.dds.protobuf.frontend.MetaFile;

public class CompilerPlugin {

    public static void main(String[] args) {
        String language;
        CodeGeneratorRequest request;
        List<String> filesToGenerate;
        FileDescriptorSet filesPlusDependencies;
        List<FileDescriptorProto> allProtoFiles;
        FileDescriptorSet.Builder s = FileDescriptorSet.newBuilder();
        Logger logger = Logger.getInstance();
        CodeGeneratorResponse.Builder responseBuilder = CodeGeneratorResponse
                .newBuilder();

        logger.log("Starting DDS-GPB compiler plugin.\n", Level.INFO);

        if (args.length < 1) {
            responseBuilder
                    .setError("Command-line parameter to drive output language is missing.");
            System.err
                    .println("Command-line parameter to drive output language is missing.");
            System.exit(1);
        }
        language = args[0];

        try {
            request = CodeGeneratorRequest.parseFrom(System.in);
            filesToGenerate = request.getFileToGenerateList();
            allProtoFiles = request.getProtoFileList();

            logger.log("Command-line parameter: '" + request.getParameter()
                    + "'\n", Level.INFO);

            for (String arg : args) {
                logger.log("- '" + arg + "'\n", Level.INFO);
            }
            logger.log("All involved proto files:\n", Level.FINER);

            for (FileDescriptorProto fd : allProtoFiles) {
                if(("google/protobuf/descriptor.proto".equals(fd.getName())) ||
                   ("omg/dds/descriptor.proto".equals(fd.getName()))){
                    logger.log("- " + fd.getName() + " (IGNORED)\n", Level.FINER);
                } else {
                    logger.log("- " + fd.getName() + "\n", Level.FINER);
                    s.addFile(fd);
                }
            }
            filesPlusDependencies = s.build();

            logger.log("Files to generate:\n", Level.FINE);

            for (FileDescriptorProto fd : allProtoFiles) {
                if (filesToGenerate.contains(fd.getName())) {
                    logger.log("- " + fd.getName() + "\n", Level.INFO);

                    try {
                        CodeGenerator codegen;
                        MetaFile mf = new MetaFile(fd, filesPlusDependencies);
                        logger.log(mf.toString(), Level.INFO);

                        if ("java".equalsIgnoreCase(language)) {
                            codegen = new OsplJava5CodeGenerator(mf);
                        } else if ("cpp".equalsIgnoreCase(language)) {
                            codegen = new OsplIsocppCodeGenerator(mf);
                        } else if ("lcpp".equalsIgnoreCase(language)) {
                            codegen = new LiteIsocppCodeGenerator(mf);
                        } else if ("cjava".equalsIgnoreCase(language)) {
                            codegen = new CafeJava5CodeGenerator(mf);
                        } else {
                            throw new ProtoParseException(
                                    "Cannot generate code for unknown language '"
                                            + language + "'.");
                        }

                        for (File generated : codegen.generateCode()) {
                            responseBuilder.addFile(generated);
                        }
                    } catch (ProtoParseException e) {
                        logger.exception(e);
                        responseBuilder.setError(e.getMessage());
                    }
                }
            }
            System.out.write(responseBuilder.build().toByteArray());

            logger.log("Terminating DDS-GPB compiler plugin.\n", Level.INFO);
        } catch (IOException e) {
            logger.exception(e);
            System.err
                    .println("Reading CodeGeneratorRequest from standard input failed: "
                            + e.getMessage());
            e.printStackTrace();
            logger.log("Terminating DDS-GPB compiler plugin with error.\n", Level.INFO);
            System.exit(2);
        }
    }

}
