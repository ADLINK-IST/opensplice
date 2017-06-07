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
package com.prismtech.dds.protobuf.backend;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.List;
import java.util.UUID;
import java.util.logging.Level;

import com.prismtech.dds.protobuf.Logger;
import com.prismtech.dds.protobuf.ProtoParseException;
import com.prismtech.dds.protobuf.Util;
import com.prismtech.dds.protobuf.frontend.MetaField;
import com.prismtech.dds.protobuf.frontend.MetaFile;
import com.prismtech.dds.protobuf.frontend.MetaMessage;

public abstract class CodeGenerator {
    protected MetaFile metadata;
    protected String idlString;
    protected static String IDL_INCLUDE_FILE_NAME = "ospl_protobuf_common.idl";
    protected String idlIncludeString;
    protected Idlpp idlpp;
    protected static final String LINE_SEPERATOR = System
            .getProperty("line.separator");
    public static final String IDL_TAB = "    ";
    public static final String IDL_PROTOBUF_FIELD_NAME = "ospl_protobuf_data";

    public CodeGenerator(MetaFile metadata, Idlpp idlpp)
            throws ProtoParseException {
        if (idlpp == null) {
            throw new ProtoParseException("Invalid IDL pre-processor supplied.");
        }
        this.idlpp = idlpp;
        this.metadata = metadata;
        this.setupIDL();
    }

    public abstract List<com.google.protobuf.compiler.PluginProtos.CodeGeneratorResponse.File> generateCode()
            throws ProtoParseException;

    protected void invokeIdlpp(File idlFile, File tempDir)
            throws ProtoParseException {
        String line;
        String[] cmdarray = this.idlpp.getIdlCommand(idlFile, this.metadata);

        Logger.getInstance().log("Invoking: ", Level.INFO);

        for (int i = 0; i < cmdarray.length; i++) {
            Logger.getInstance().log(" " + cmdarray[i], Level.INFO);
        }
        Logger.getInstance().log("\n", Level.INFO);
        ProcessBuilder builder = new ProcessBuilder(cmdarray);
        builder.directory(tempDir);

        try {
            Process idlppProcess = builder.start();
            BufferedReader bre = new BufferedReader(new InputStreamReader(
                    idlppProcess.getErrorStream()));
            String errorOutput = "";

            while ((line = bre.readLine()) != null) {
                errorOutput += line;
            }
            bre.close();

            bre = new BufferedReader(new InputStreamReader(
                    idlppProcess.getInputStream()));

            while ((line = bre.readLine()) != null) {
                errorOutput += line;
            }
            bre.close();

            int retValue = idlppProcess.waitFor();

            if (retValue != 0) {
                throw new ProtoParseException(
                        "Internal error while pre-processing IDL; "
                                + errorOutput);
            }
        } catch (IOException e) {
            Logger.getInstance().exception(e);
            throw new ProtoParseException("Running idlpp failed ("
                    + e.getMessage() + ").");

        } catch (InterruptedException e) {
            Logger.getInstance().exception(e);
            throw new ProtoParseException("Running idlpp failed ("
                    + e.getMessage() + ").");
        }
    }

    private void generateIdlFile(File idlFile, String contents) throws ProtoParseException {
        try {
            Logger.getInstance().log(
                    "Storing IDL in file '" + idlFile.getAbsolutePath()
                            + "'.\n", Level.INFO);

            if (!idlFile.createNewFile()) {
                throw new ProtoParseException(
                        "Cannot store temporary IDL file '"
                                + idlFile.getAbsolutePath() + "'");
            }
            PrintWriter idlWriter = new PrintWriter(idlFile);
            idlWriter.print(contents);
            idlWriter.close();
        } catch (IOException e) {
            Logger.getInstance().exception(e);
            throw new ProtoParseException("Generating IDL failed ("
                    + e.getMessage() + ").");

        }
    }

    protected List<com.google.protobuf.compiler.PluginProtos.CodeGeneratorResponse.File> preProcesIDL()
            throws ProtoParseException {
        File tempDir;
        List<com.google.protobuf.compiler.PluginProtos.CodeGeneratorResponse.File> files = new ArrayList<com.google.protobuf.compiler.PluginProtos.CodeGeneratorResponse.File>();
        if (this.idlString != null) {
            tempDir = new File(System.getProperty("java.io.tmpdir", null),
                    "ddsprotobuf." + UUID.randomUUID().toString());

            Logger.getInstance().log(
                    "Using temporary directory: '" + tempDir.getAbsolutePath()
                            + "'.\n", Level.INFO);

            try {
                if (!tempDir.exists() && !tempDir.mkdirs()) {
                    throw new ProtoParseException(
                            "Unable to create temporary directory '"
                                    + tempDir.getAbsolutePath()
                                    + "' for code generation.");
                }
                String idlFileName = this.metadata.getProtoFile().getName();

                Logger.getInstance().log(
                        "Proto file name '" + idlFileName + "'.\n", Level.FINE);

                idlFileName = this.stripProto(idlFileName) + ".idl";

                Logger.getInstance().log(
                        "IDL file name '" + idlFileName + "'.\n", Level.FINE);
                File idlFile = new File(tempDir, idlFileName);
                File idlIncludeFile = new File(tempDir, IDL_INCLUDE_FILE_NAME);

                int index = idlFileName.lastIndexOf(File.separator);

                if (index != -1) {
                    File parent = idlFile.getParentFile();

                    Logger.getInstance().log(
                            "IDL file directory '" + parent.getAbsolutePath()
                                    + "'.\n", Level.FINE);

                    if (!parent.exists() && !parent.mkdirs()) {
                        throw new ProtoParseException(
                                "Unable to create temporary directory '"
                                        + parent.getAbsolutePath()
                                        + "' for code generation.");
                    }
                }
                this.generateIdlFile(idlFile, this.idlString);
                this.generateIdlFile(idlIncludeFile, this.idlIncludeString);
                this.invokeIdlpp(idlFile, tempDir);
                this.invokeIdlpp(idlIncludeFile, tempDir);

                Logger.getInstance().log(
                        "Collecting generated files from '"
                                + tempDir.getAbsolutePath() + "':\n",
                        Level.INFO);

                boolean includeIdl = Boolean.parseBoolean(Util
                        .getenv("PROTOBUF_INCLUDE_IDL"));

                if (!includeIdl && idlFile.exists()) {
                    idlFile.delete();
                }
                if (!includeIdl && idlIncludeFile.exists()) {
                    idlIncludeFile.delete();
                }
                collectGeneratedFiles(tempDir, "", files);
            } catch (IOException e) {
                Logger.getInstance().exception(e);
                throw new ProtoParseException("Running idlpp failed ("
                        + e.getMessage() + ").");

            } catch (ProtoParseException e) {
                Logger.getInstance().exception(e);
                throw e;
            } finally {
                Logger.getInstance().log(
                        "Deleting temporary dir: '" + tempDir.getAbsolutePath()
                                + "'.\n", Level.INFO);
                deleteDir(tempDir);
            }
        } else {
            Logger.getInstance()
                    .log("Not generating any code as no DDS annotated messages were found.\n",
                            Level.INFO);
        }
        return files;
    }

    private void deleteDir(File aFile) {
        if (aFile.exists()) {
            if (aFile.isDirectory()) {
                for (File f : aFile.listFiles()) {
                    this.deleteDir(f);
                }
            }
            aFile.delete();
        }
    }

    private void collectGeneratedFiles(
            File root,
            String pathToPrepend,
            List<com.google.protobuf.compiler.PluginProtos.CodeGeneratorResponse.File> files)
            throws IOException {
        String fileName;
        assert (pathToPrepend != null);

        for (File f : root.listFiles()) {
            if (f.isDirectory()) {
                if ("".equals(pathToPrepend)) {
                    collectGeneratedFiles(f, f.getName(), files);
                } else {
                    collectGeneratedFiles(f, pathToPrepend + "/" + f.getName(),
                            files);
                }
            } else {
                com.google.protobuf.compiler.PluginProtos.CodeGeneratorResponse.File.Builder builder = com.google.protobuf.compiler.PluginProtos.CodeGeneratorResponse.File
                        .newBuilder();

                if ("".equals(pathToPrepend)) {
                    fileName = f.getName();
                } else {
                    fileName = pathToPrepend + "/" + f.getName();
                }
                Logger.getInstance().log("- " + fileName + "\n", Level.FINER);
                builder.setName(fileName);
                builder.setContent(this.readFile(f));
                files.add(builder.build());
            }
        }
    }

    protected String readFile(File file) throws IOException {
        BufferedReader reader = new BufferedReader(new FileReader(file));
        String line = null;
        StringBuilder stringBuilder = new StringBuilder();

        try {
            while ((line = reader.readLine()) != null) {
                stringBuilder.append(line);
                stringBuilder.append(CodeGenerator.LINE_SEPERATOR);
            }
        } catch (IOException ioe) {
            throw ioe;
        } finally {
            reader.close();
        }
        return stringBuilder.toString();
    }

    private void insertTabs(StringBuffer buffer, int depth) {
        for (int i = 0; i < depth; i++) {
            buffer.append(CodeGenerator.IDL_TAB);
        }
    }

    /*
     * Return new depth
     */
    private int openOwnScope(StringBuffer buffer, int depth) {
        int result = depth;

        if (!"".equals(this.metadata.getProtoFile().getPackage())) {
            String[] scoped = this.metadata.getProtoFile().getPackage()
                    .split("\\.");

            for (int i = 0; i < scoped.length; i++) {
                if (!"".equals(scoped[i])) {
                    this.insertTabs(buffer, result);
                    buffer.append("module ");
                    buffer.append(scoped[i]);
                    buffer.append(" {");
                    buffer.append(CodeGenerator.LINE_SEPERATOR);
                    result++;
                }
            }
        }
        return result;
    }

    /*
     * Return new depth
     */
    private int closeOwnScope(StringBuffer buffer, int depth) {
        int result = depth;

        if (!"".equals(this.metadata.getProtoFile().getPackage())) {
            String[] scoped = this.metadata.getProtoFile().getPackage()
                    .split("\\.");

            for (int i = 0; i < scoped.length; i++) {
                if (!"".equals(scoped[i])) {
                    result--;
                    this.insertTabs(buffer, result);
                    buffer.append("};");
                }
            }
        }
        return result;
    }

    private void setupIDLInclude() {
        StringBuffer buffer = new StringBuffer();
        buffer.append("module org {" + CodeGenerator.LINE_SEPERATOR);
        buffer.append("    module omg {" + CodeGenerator.LINE_SEPERATOR);
        buffer.append("       module dds {" + CodeGenerator.LINE_SEPERATOR);
        buffer.append("            module protobuf {"
                + CodeGenerator.LINE_SEPERATOR);
        buffer.append("               typedef sequence<octet> gpb_payload_t;"
                + CodeGenerator.LINE_SEPERATOR);
        buffer.append("            };" + CodeGenerator.LINE_SEPERATOR);
        buffer.append("        };" + CodeGenerator.LINE_SEPERATOR);
        buffer.append("    };" + CodeGenerator.LINE_SEPERATOR);
        buffer.append("};" + CodeGenerator.LINE_SEPERATOR
                + CodeGenerator.LINE_SEPERATOR);
        this.idlIncludeString = buffer.toString();

        Logger.getInstance().log(
                "IDL common contents:\n----------\n" + this.idlIncludeString
                        + "\n----------\n", Level.FINEST);
    }

    private void setupIDL() {
        int depth = 0;
        StringBuffer idl = new StringBuffer();
        String name, keys;
        boolean anyDDSEnabled = false;

        idl.append("#include \"");
        idl.append(IDL_INCLUDE_FILE_NAME);
        idl.append("\"");
        idl.append(CodeGenerator.LINE_SEPERATOR);
        idl.append(CodeGenerator.LINE_SEPERATOR);

        for (MetaMessage message : this.metadata.getDDSMessages()) {
            if (message.isDDSEnabled()) {
                anyDDSEnabled = true;
                name = message.getDDSName();

                // Open file scope if DDS name is relative
                if (!name.startsWith(".")) {
                    depth = this.openOwnScope(idl, depth);
                }
                // Open message scope
                String[] scoped = name.split("\\.");

                for (int i = 0; i < (scoped.length - 1); i++) {
                    if (!"".equals(scoped[i])) {
                        this.insertTabs(idl, depth);
                        idl.append("module ");
                        idl.append(scoped[i]);
                        idl.append(" {");
                        idl.append(CodeGenerator.LINE_SEPERATOR);
                        depth++;
                    }
                }

                // Open Message
                this.insertTabs(idl, depth);
                idl.append("struct ");
                idl.append(scoped[scoped.length - 1]);
                idl.append(" {");
                idl.append(CodeGenerator.LINE_SEPERATOR);
                depth++;

                keys = "";
                // Attributes
                for (MetaField field : message.getDDSFields()) {
                    this.insertTabs(idl, depth);
                    idl.append(field.getDDSTypeName());
                    idl.append(" ");
                    idl.append(field.getDDSName());
                    idl.append(";");
                    idl.append(CodeGenerator.LINE_SEPERATOR);

                    if (field.isDDSKey()) {
                        keys += " " + field.getDDSName();
                    }
                }

                // Attributes protobuf sequence of bytes
                this.insertTabs(idl, depth);
                idl.append("::org::omg::dds::protobuf::gpb_payload_t ");
                idl.append(CodeGenerator.IDL_PROTOBUF_FIELD_NAME);
                idl.append(";");
                idl.append(CodeGenerator.LINE_SEPERATOR);

                // Close message
                depth--;
                this.insertTabs(idl, depth);
                idl.append("};");
                idl.append(CodeGenerator.LINE_SEPERATOR);

                // Insert key-list
                this.insertTabs(idl, depth);
                idl.append("#pragma keylist ");
                idl.append(message.getDDSUnscopedName());
                idl.append(keys);
                idl.append(CodeGenerator.LINE_SEPERATOR);

                // Close message scope
                Logger.getInstance().log(
                        "Closing scope for: '" + name + "':\n", Level.FINE);

                for (int i = 0; i < (scoped.length - 1); i++) {
                    if (!"".equals(scoped[i])) {
                        Logger.getInstance().log("- '" + scoped[i] + "'\n",
                                Level.FINE);
                        depth--;
                        this.insertTabs(idl, depth);
                        idl.append("};");
                        idl.append(CodeGenerator.LINE_SEPERATOR);
                    }
                }

                // Close file scope if DDS name is relative
                if (!name.startsWith(".")) {
                    depth = this.closeOwnScope(idl, depth);
                }
                idl.append(CodeGenerator.LINE_SEPERATOR);
            }
        }
        if (anyDDSEnabled) {
            this.setupIDLInclude();
            this.idlString = idl.toString();
            Logger.getInstance().log(
                    "IDL contents:\n----------\n" + this.idlString
                            + "\n----------\n", Level.FINEST);
        } else {
            this.idlString = null;
            Logger.getInstance().log("No DDS enabled messages found,\n",
                    Level.INFO);
        }

    }

    protected String stripProto(String fileName) {
        if (fileName.endsWith(".protodevel")) {
            return fileName.substring(0, fileName.length() - 11);
        } else if (fileName.endsWith(".proto")) {
            return fileName.substring(0, fileName.length() - 6);
        }
        return fileName;
    }
}
