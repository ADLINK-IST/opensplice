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
package com.prismtech.dds.protobuf.backend.ospl;

import java.io.IOException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.logging.Level;

import com.google.protobuf.DescriptorProtos.FieldDescriptorProto;
import com.google.protobuf.DescriptorProtos.FileDescriptorProto;
import com.google.protobuf.DescriptorProtos.FileOptions;
import com.google.protobuf.compiler.PluginProtos.CodeGeneratorResponse.File;
import com.prismtech.dds.protobuf.Logger;
import com.prismtech.dds.protobuf.ProtoParseException;
import com.prismtech.dds.protobuf.Util;
import com.prismtech.dds.protobuf.backend.CodeGenerator;
import com.prismtech.dds.protobuf.backend.Idlpp;
import com.prismtech.dds.protobuf.frontend.MetaField;
import com.prismtech.dds.protobuf.frontend.MetaFile;
import com.prismtech.dds.protobuf.frontend.MetaMessage;

public class OsplJava5CodeGenerator extends CodeGenerator {
    protected OsplJava5CodeGenerator(MetaFile metadata, Idlpp idlpp)
            throws ProtoParseException {
        super(metadata, idlpp);
    }

    public OsplJava5CodeGenerator(MetaFile metadata) throws ProtoParseException {
        super(metadata, new OsplJavaIdlpp());
    }

    @Override
    public List<File> generateCode() throws ProtoParseException {
        List<File> result;

        if (this.metadata.hasDDSEnabledMessages()) {
            result = super.preProcesIDL();

            for (MetaMessage message : this.metadata.getDDSMessages()) {
                if (message.isDDSEnabled()) {
                    result.add(this.generateProtobufTypeSupport(message));
                }
            }
            String generateDdsProto = Util
                    .getenv("PROTOBUF_INCLUDE_DESCRIPTOR");

            if ("true".equalsIgnoreCase(generateDdsProto)) {
                result.add(this.generateDdsDescriptorProto());
            }
        } else {
            result = new ArrayList<File>();
        }
        return result;
    }

    private File generateDdsDescriptorProto() throws ProtoParseException {
        File.Builder builder = File.newBuilder();
        Logger.getInstance().log(
                "Adding omg.dds.protobuf.DescriptorProtos.java\n", Level.INFO);

        builder.setContent(this.getDDSDescriptorProto());
        builder.setName("org/omg/dds/protobuf/DescriptorProtos.java");

        return builder.build();
    }

    private File generateProtobufTypeSupport(MetaMessage message)
            throws ProtoParseException {
        File.Builder builder = File.newBuilder();
        String template = this.getTypeSupportProtobufTemplate();
        String fullClassName = this.getMessageFullClassName(message)
                .replaceAll("\\$", ".");

        template = template.replaceAll("\\$\\{package-name\\}",
                this.getFileJavaPackage(this.metadata.getProtoFile()));
        template = template.replaceAll("\\$\\{typesupport-claz-name\\}",
                this.getProtobufTypeSupportClassName(message));
        template = template.replaceAll("\\$\\{proto-type-name\\}",
                fullClassName);
        template = template.replaceAll("\\$\\{dds-type-name\\}",
                this.getMessageFullDDSTypeClassName(message));
        template = template.replaceAll("\\$\\{get-protobuf-fields\\}",
                this.getProtobufFields(message));
        template = template.replaceAll("\\$\\{set-protobuf-key-fields\\}",
                this.setProtobufKeyFields(message));
        template = template.replaceAll("\\$\\{protobuf-meta-descriptor\\}",
                this.getProtobufMetaDescriptor(this.metadata));
        template = template.replaceAll("\\$\\{protobuf-meta-hash\\}",
                this.getProtobufMetaHash(this.metadata, message));

        Logger.getInstance().log(
                "Template expansion: '"
                        + getMessageFullDDSTypeSupportFileName(message)
                        + "'\n----------\n", Level.FINEST);
        Logger.getInstance().log(template, Level.FINEST);
        Logger.getInstance().log("\n----------\n", Level.FINEST);

        builder.setName(getMessageFullDDSTypeSupportFileName(message));
        builder.setContent(template);

        return builder.build();
    }

    private String getFileFullClassName(FileDescriptorProto file) {
        String result = getFileJavaPackage(file);

        if (!result.isEmpty()) {
            result += '.';
        }
        result += getFileSimpleClassName(file);

        return result;
    }

    private String getFileJavaPackage(FileDescriptorProto file) {
        String result;
        FileOptions options = file.getOptions();

        if (options.hasJavaPackage()) {
            result = options.getJavaPackage();
        } else {
            result = "";

            if (!file.getPackage().isEmpty()) {
                if (!result.isEmpty()) {
                    result += '.';
                }
                result += file.getPackage();
            }
        }
        return result;
    }

    private String getFileSimpleClassName(FileDescriptorProto file) {
        FileOptions options = file.getOptions();

        if (options.hasJavaOuterClassname()) {
            return options.getJavaOuterClassname();
        }
        String basename;
        int lastSlash = file.getName().lastIndexOf('/');

        if (lastSlash == -1) {
            basename = file.getName();
        } else {
            basename = file.getName().substring(lastSlash + 1);
        }
        return underscoresToCamelCaseImpl(stripProto(basename), true);
    }

    private String getMessageFullClassName(MetaMessage message) {
        String result;
        FileDescriptorProto file = this.metadata.getProtoFile();
        FileOptions options = file.getOptions();
        MetaMessage parent = message.getParent();

        if (parent != null) {
            result = this.getMessageFullClassName(parent);
            result += "$";
        } else {
            if (options.hasJavaMultipleFiles()) {
                result = getFileJavaPackage(file);
            } else {
                result = getFileFullClassName(file);
            }
            if (!result.isEmpty()) {
                if (this.metadata.getProtoFile().getOptions()
                        .getJavaMultipleFiles()) {
                    result += '.';
                } else {
                    result += '$';
                }
            }
        }
        result += message.getProtobufName();

        return result;
    }

    private String getMessageFullClassNameNoInner(MetaMessage message) {
        return this.getMessageFullClassName(message).replaceAll("\\$", ".");
    }

    private String getMainBuilderName(MetaMessage message) {
        return "builderFor_"
                + this.getMessageFullClassNameNoInner(message).replaceAll(
                        "\\.", "_");
    }

    private String getMessageFullDDSTypeClassName(MetaMessage message) {
        String ddsTypeName = message.getDDSName();

        if (ddsTypeName.startsWith(".")) {
            // Absolute name
            ddsTypeName = ddsTypeName.substring(1);
        } else {
            // Relative name
            ddsTypeName = this.getFileJavaPackage(message.getFile()
                    .getProtoFile()) + "." + ddsTypeName;
        }

        return ddsTypeName;
    }

    private String getMessageFullDDSTypeSupportFileName(MetaMessage message) {
        return this.getFileJavaPackage(message.getFile().getProtoFile())
                .replaceAll("\\.", "/")
                + java.io.File.separator
                + this.getProtobufTypeSupportClassName(message) + ".java";
    }

    private String getProtobufFields(MetaMessage message) {
        StringBuffer buffer = new StringBuffer();

        for (MetaField field : message.getDDSFields()) {
            buffer.append("protobufData.get");

            if (field.hasScope()) {
                for (String member : field.getScopeNames()) {
                    buffer.append(this
                            .underscoresToCapitalizedCamelCase(member));
                    buffer.append("().get");
                }
            }
            buffer.append(this.underscoresToCapitalizedCamelCase(field
                    .getProtobufName()));
            buffer.append("(),");
        }
        return buffer.toString();
    }

    private String getProtobufTypeSupportClassName(MetaMessage message) {
        String messageClazName = this.getMessageFullClassName(message)
                + "TypeSupportProtobuf";

        int lastDotIndex = messageClazName.lastIndexOf('.');

        if (lastDotIndex != -1) {
            messageClazName = messageClazName.substring(lastDotIndex + 1);
        }
        return messageClazName.replaceAll("\\$", "");
    }

    private String getTypeSupportProtobufTemplate() throws ProtoParseException {
        String templateFileName;
        java.io.File templateFile;

        templateFileName = this.getTemplatesDirectory().getAbsolutePath()
                + java.io.File.separator
                + "tmplTypeSupportProtobuf.java";

        templateFile = new java.io.File(templateFileName);

        if (!templateFile.exists() || !templateFile.canRead()) {
            throw new ProtoParseException(
                    "Unable to locate or read Java Protobuf TypeSupport template (expected at '"
                            + templateFileName + "'.");
        }
        try {
            return this.readFile(templateFile);
        } catch (IOException e) {
            throw new ProtoParseException(
                    "Unable to read contents of template file. "
                            + e.getMessage());
        }
    }

    private String getDDSDescriptorProto() throws ProtoParseException {
        String templateFileName;
        java.io.File descriptorFile;

        templateFileName = this.getTemplatesDirectory().getAbsolutePath()
                + java.io.File.separator + "DescriptorProtos.java";

        descriptorFile = new java.io.File(templateFileName);

        if (!descriptorFile.exists() || !descriptorFile.canRead()) {
            throw new ProtoParseException(
                    "Unable to locate or read Java Protobuf TypeSupport template (expected at '"
                            + templateFileName + "'.");
        }
        try {
            return this.readFile(descriptorFile);
        } catch (IOException e) {
            throw new ProtoParseException(
                    "Unable to read contents of template file. "
                            + e.getMessage());
        }
    }

    private String getTypeNameForFieldDescriptor(MetaField field,
            MetaField.Scope scope) {
        String result = this.getFileFullClassName(field.getProtoFile());

        Logger.getInstance().log("File class name: '" + result + "'\n",
                Level.FINEST);

        String typeName = scope.getField().getTypeName();

        Logger.getInstance().log("Type name: '" + typeName + "'\n",
                Level.FINEST);

        if (typeName.startsWith(".")) {
            String pckg = this.getFileJavaPackage(field.getProtoFile());

            if (!pckg.isEmpty()) {
                String outerName = this.getFileSimpleClassName(field
                        .getProtoFile());
                typeName = typeName.substring(1);

                if (typeName.startsWith(pckg)) {
                    typeName = typeName.substring(pckg.length());
                }
                typeName = pckg + "." + outerName + typeName;
            } else {
                typeName = result + typeName;
            }
            Logger.getInstance().log(
                    "getTypeNameForFieldDescriptor returns '" + typeName
                            + "'\n", Level.FINEST);
            return typeName;
        }
        Logger.getInstance().log(
                "getTypeNameForFieldDescriptor returns '" + result + "."
                        + typeName + "'\n", Level.FINEST);
        return result + "." + typeName;
    }

    private String getBuilderClassNameForFieldDescriptor(MetaField field,
            MetaField.Scope scope) {
        return getTypeNameForFieldDescriptor(field, scope) + ".Builder";
    }

    private void protobufKeyFieldsSetupBuilders(StringBuffer buffer,
            MetaMessage message, HashMap<String, String> builders,
            ArrayList<String> builderList,
            HashMap<String, String> parentBuilders,
            HashMap<String, FieldDescriptorProto> builderMemberNames) {
        String fullClassName = this.getMessageFullClassNameNoInner(message);
        String mainBuilderName = this.getMainBuilderName(message);
        String tab = IDL_TAB + IDL_TAB;

        buffer.append(fullClassName);
        buffer.append(".Builder ");
        buffer.append(mainBuilderName);
        buffer.append(" = ");
        buffer.append(fullClassName);
        buffer.append(".newBuilder();\n");

        for (MetaField field : message.getDDSFields()) {
            if (field.isDDSKey()) {
                List<MetaField.Scope> scope = field.getScope();

                if (scope.size() > 0) {
                    String parent = null;

                    for (MetaField.Scope s : scope) {
                        String typeName = this.getTypeNameForFieldDescriptor(
                                field, s);
                        String builder = builders.get(typeName);

                        if (builder == null) {
                            String builderName = "builderFor"
                                    + typeName
                                            .replaceAll("\\.", "_");
                            buffer.append(tab);
                            buffer.append(this
                                    .getBuilderClassNameForFieldDescriptor(
                                            field, s));
                            buffer.append(" ");
                            buffer.append(builderName);
                            buffer.append(" = ");
                            buffer.append(typeName);
                            buffer.append(".newBuilder();\n");

                            Logger.getInstance().log(
                                    "builders.put(\"" + typeName + "\", \""
                                            + builderName + "\");\n",
                                    Level.FINE);
                            builders.put(typeName, builderName);

                            Logger.getInstance().log(
                                    "builderList.add(0, \"" + typeName
                                            + "\");\n", Level.FINE);
                            builderList.add(0, typeName);

                            String parentBuilder;

                            if (parent == null) {
                                parentBuilder = mainBuilderName;

                            } else {
                                parentBuilder = builders.get(parent);
                            }
                            Logger.getInstance().log(
                                    "parentBuilders.put(\"" + builderName
                                            + "\", \"" + parentBuilder
                                            + "\");\n", Level.FINE);
                            parentBuilders.put(builderName, parentBuilder);

                            Logger.getInstance().log(
                                    "buildMemberNames.put(\"" + builderName
                                            + "\", \"" + s.getField().getName()
                                            + "\");\n", Level.FINE);
                            builderMemberNames.put(builderName, s.getField());
                        }
                        parent = typeName;
                    }
                }
            }
        }
        buffer.append("\n");
    }

    private void protobufKeyFieldsSetBuilderMembers(StringBuffer buffer,
            MetaMessage message, HashMap<String, String> builders)
            throws ProtoParseException {
        String tab = IDL_TAB + IDL_TAB;
        String mainBuilderName = this.getMainBuilderName(message);

        for (MetaField field : message.getDDSFields()) {
            if (field.isDDSKey()) {
                List<MetaField.Scope> scope = field.getScope();
                String builderName = mainBuilderName;
                String typeName = mainBuilderName;

                if (scope.size() > 0) {
                    typeName = getTypeNameForFieldDescriptor(field,
                            scope.get(scope.size() - 1));
                    builderName = builders.get(typeName);
                }
                if (builderName == null) {
                    throw new ProtoParseException(
                            "Internal error: protobufKeyFieldsSetBuilderMembers(): builders.get(\""
                                    + typeName + "\") failed.");
                }
                buffer.append(tab);
                buffer.append(builderName);
                buffer.append(".set");
                buffer.append(this.underscoresToCapitalizedCamelCase(field
                        .getProtobufName()));
                buffer.append("(ddsData.");
                buffer.append(field.getDDSName());
                buffer.append(");\n");
            }
        }
        buffer.append("\n");
    }

    private void protobufKeyFieldsBuildBuilders(StringBuffer buffer,
            MetaMessage message, HashMap<String, String> builders,
            ArrayList<String> builderList,
            HashMap<String, String> parentBuilders,
            HashMap<String, FieldDescriptorProto> builderMemberNames)
            throws ProtoParseException {
        String tab = IDL_TAB + IDL_TAB;

        for (String typeName : builderList) {
            String builder = builders.get(typeName);

            if (builder == null) {
                throw new ProtoParseException(
                        "Internal error: protobufKeyFieldsBuildBuilders(): builders.get(\""
                                + typeName + "\") failed.");
            }

            String parentBuilder = parentBuilders.get(builder);

            if (parentBuilder == null) {
                throw new ProtoParseException(
                        "Internal error: protobufKeyFieldsBuildBuilders(): parentBuilders.get(\""
                                + builder + "\") failed.");
            }

            buffer.append(tab);
            buffer.append(parentBuilder);
            buffer.append(".set");
            buffer.append(this
                    .underscoresToCapitalizedCamelCase(builderMemberNames.get(
                            builder).getName()));

            buffer.append("(");
            buffer.append(builder);
            buffer.append(".buildPartial());\n");
        }
        buffer.append("\n");
        buffer.append(tab);
        buffer.append("return ");
        buffer.append(this.getMainBuilderName(message));
        buffer.append(".buildPartial();");
    }

    private String setProtobufKeyFields(MetaMessage message)
            throws ProtoParseException {
        StringBuffer buffer = new StringBuffer();
        HashMap<String, String> builders = new HashMap<String, String>();
        HashMap<String, String> parentBuilders = new HashMap<String, String>();
        HashMap<String, FieldDescriptorProto> builderMemberNames = new HashMap<String, FieldDescriptorProto>();
        ArrayList<String> builderList = new ArrayList<String>();

        this.protobufKeyFieldsSetupBuilders(buffer, message, builders,
                builderList, parentBuilders, builderMemberNames);

        this.protobufKeyFieldsSetBuilderMembers(buffer, message, builders);

        this.protobufKeyFieldsBuildBuilders(buffer, message, builders,
                builderList, parentBuilders, builderMemberNames);

        return buffer.toString();
    }

    private String underscoresToCamelCaseImpl(String input,
            boolean capNextLetter) {
        String result = "";

        for (int i = 0; i < input.length(); i++) {
            if (Character.isLowerCase(input.charAt(i))) {
                if (capNextLetter) {
                    result += Character.toUpperCase(input.charAt(i));
                } else {
                    result += input.charAt(i);
                }
                capNextLetter = false;
            } else if (Character.isUpperCase(input.charAt(i))) {
                if (i == 0 && !capNextLetter) {
                    // Force first letter to lower-case unless explicitly told
                    // to capitalize it.
                    result += Character.toUpperCase(input.charAt(i));
                } else {
                    // Capital letters after the first are left as-is.
                    result += input.charAt(i);
                }
                capNextLetter = false;
            } else if (Character.isDigit(input.charAt(i))) {
                result += input.charAt(i);
                capNextLetter = true;
            } else {
                capNextLetter = true;
            }
        }
        return result;
    }

    private String underscoresToCapitalizedCamelCase(String str) {
        return underscoresToCamelCaseImpl(str, true);
    }

    private String getProtobufMetaDescriptor(MetaFile file) {
        String metaDescriptor = file.getMetaDescriptorAsString();
        StringBuffer buffer = new StringBuffer();
        int length = metaDescriptor.length();
        String tab = CodeGenerator.IDL_TAB + CodeGenerator.IDL_TAB
                + CodeGenerator.IDL_TAB + CodeGenerator.IDL_TAB
                + CodeGenerator.IDL_TAB;
        int index = 0;
        int charsPerLine = 500;

        buffer.append("hexStringToByteArray(new java.lang.StringBuilder(");
        buffer.append(metaDescriptor.length());
        buffer.append(")");

        while (index + charsPerLine < length) {
            buffer.append("\n" + tab + ".append(\""
                    + metaDescriptor.substring(index, index + charsPerLine)
                    + "\")");
            index += charsPerLine;
        }

        buffer.append("\n" + tab + ".append(\""
                + metaDescriptor.substring(index) + "\").toString())");

        return buffer.toString();
    }

    private String getProtobufMetaHash(MetaFile file, MetaMessage message)
            throws ProtoParseException {
        try {
            byte[] metaDescriptor = message.getProtobufMetaDescriptor();
            MessageDigest md = MessageDigest.getInstance("MD5");
            byte[] md5hash = md.digest(metaDescriptor);
            StringBuffer buffer = new StringBuffer();

            buffer.append("hexStringToByteArray(\"");

            for (final byte b : md5hash) {
                buffer.append(String.format("%02x", b & 0xff));
            }
            buffer.append("\")");

            return buffer.toString();
        } catch (NoSuchAlgorithmException e) {
            throw new ProtoParseException(
                    "Unable to generate MD5 hash for message '"
                            + message.getProtobufName() + "' ("
                            + e.getMessage() + ").");
        }
    }

    public java.io.File getTemplatesDirectory() throws ProtoParseException {
        String osplHome = System.getenv("OSPL_HOME_NORMALIZED");

        if (osplHome == null) {
            osplHome = System.getenv("OSPL_HOME");
        }
        if (osplHome == null) {
            throw new ProtoParseException(
                    "OSPL_HOME not set. Please ensure OpenSplice environment is set up correctly.");
        }
        return new java.io.File(osplHome + java.io.File.separator + "etc"
                + java.io.File.separator + "protobuf" + java.io.File.separator
                + "SAJ5");
    }
}
