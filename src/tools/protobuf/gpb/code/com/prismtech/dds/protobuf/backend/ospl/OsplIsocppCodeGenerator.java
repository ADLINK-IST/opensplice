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

import java.io.IOException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.util.List;
import java.util.logging.Level;
import java.util.regex.Pattern;

import com.google.protobuf.compiler.PluginProtos.CodeGeneratorResponse.File;
import com.prismtech.dds.protobuf.Logger;
import com.prismtech.dds.protobuf.ProtoParseException;
import com.prismtech.dds.protobuf.backend.CodeGenerator;
import com.prismtech.dds.protobuf.backend.Idlpp;
import com.prismtech.dds.protobuf.frontend.MetaField;
import com.prismtech.dds.protobuf.frontend.MetaFile;
import com.prismtech.dds.protobuf.frontend.MetaMessage;

public class OsplIsocppCodeGenerator extends CodeGenerator {

    protected OsplIsocppCodeGenerator(MetaFile metadata, Idlpp idlpp)
            throws ProtoParseException {
        super(metadata, idlpp);
    }

    public OsplIsocppCodeGenerator(MetaFile metadata)
            throws ProtoParseException {
        super(metadata, new OsplIsocppIdlpp());
    }

    @Override
    public List<File> generateCode() throws ProtoParseException {
        List<File> result;

        this.verifyTypeNamesAreOverridden();

        if (this.metadata.hasDDSEnabledMessages()) {
            StringBuffer buffer = new StringBuffer();
            result = super.preProcesIDL();

            String fileTemplate = this.getFileProtobufTemplate();
            String template = this.getTypeSupportProtobufTemplate();

            for (MetaMessage message : this.metadata.getDDSMessages()) {
                if (message.isDDSEnabled()) {
                    buffer.append(this
                            .generateProtobufTypeSupport(template, message));
                }
            }
            File.Builder builder = File.newBuilder();
            builder.setName(getFullFileName());
            String content = this.generateProtobufFile(fileTemplate,
                    buffer.toString());
            builder.setContent(content);
            result.add(builder.build());

            Logger.getInstance().log(
                    "Template expansion: '" + getFullFileName()
                            + "'\n----------\n", Level.FINEST);
            Logger.getInstance().log(content, Level.FINEST);
            Logger.getInstance().log("\n----------\n", Level.FINEST);
        } else {
            result = new ArrayList<File>();
        }
        return result;
    }

    private void verifyTypeNamesAreOverridden() throws ProtoParseException {
        for (MetaMessage message : this.metadata.getDDSMessages()) {
            if (message.isDDSEnabled()) {
                String ddsName = message.getDDSName();
                String protoName = message.getProtobufName();

                if(protoName.equals(ddsName)){
                    throw new ProtoParseException(
                            "DDS type-name for message '"
                            + protoName
                            + "' is equal to proto name, which leads to"
                            + " type clashes in generated C++ code."
                            + " Please use a different DDS type name by"
                            + " means of the .omg.dds.type.name option.");
                }
            }
        }
    }

    private String generateProtobufFile(String template, String types)
            throws ProtoParseException {
        template = template.replaceAll("\\$\\{header-name\\}",
                this.getHeaderName());
        template = template.replaceAll("\\$\\{proto-includes\\}",
                this.getIncludes());
        template = template.replaceAll("\\$\\{proto-topic-traits\\}", types);

        return template;
    }

    private String generateProtobufTypeSupport(String template,
            MetaMessage message) throws ProtoParseException {
        template = template.replaceAll("\\$\\{proto-type-name\\}",
                this.getProtoTypeName(message));
        template = template.replaceAll("\\$\\{dds-type-name\\}",
                this.getDdsTypeName(message));
        template = template.replaceAll("\\$\\{get-protobuf-fields\\}",
                this.getProtobufKeyGetters(message));
        template = template.replaceAll("\\$\\{set-protobuf-fields\\}",
                this.getProtobufKeySetters(message));
        template = template.replaceAll("\\$\\{protobuf-meta-descriptor\\}",
                this.getProtobufMetaDescriptor(this.metadata));
        template = template.replaceAll("\\$\\{protobuf-meta-hash\\}",
                this.getProtobufMetaHash(this.metadata, message));

        template = template.replaceAll("\\$\\{protobuf-extensions\\}",
                this.getProtobufExtensions(this.metadata, message));
        return template;
    }

    private String getProtobufTemplate(String fileName)
            throws ProtoParseException {
        String templateFileName;
        java.io.File templateFile;

        templateFileName = this.getTemplatesDirectory().getAbsolutePath()
                + java.io.File.separator + fileName;

        templateFile = new java.io.File(templateFileName);

        if (!templateFile.exists() || !templateFile.canRead()) {
            throw new ProtoParseException(
                    "Unable to locate or read C++ Protobuf TypeSupport template (expected at '"
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

    private String getFileProtobufTemplate() throws ProtoParseException {
        return this.getProtobufTemplate("ProtoTraits.hpp");
    }

    private String getTypeSupportProtobufTemplate() throws ProtoParseException {
        return this.getProtobufTemplate("ProtoTopicTraits.hpp");
    }

    private String getFullFileName() {
        return stripProto(this.metadata.getProtoFile().getName())
                + ".pbdds.hpp";
    }

    private String getHeaderName() {
        if ("\\".equals(java.io.File.separator)) {
            return stripProto(this.metadata.getProtoFile().getName())
                    .toUpperCase().replaceAll("\\\\", "_");
        }
        return stripProto(this.metadata.getProtoFile().getName()).toUpperCase()
                .replaceAll(Pattern.quote(java.io.File.separator), "_");
    }

    private String getProtoTypeName(MetaMessage message) {
        String name = "";

        if (this.metadata.getProtoFile().hasPackage()) {
            name = this.metadata.getProtoFile().getPackage();
            Logger.getInstance().log("getProtoTypeName: '" + name + "'\n",
                    Level.FINEST);
            name = name.replaceAll("\\.", "::");
            Logger.getInstance().log(
                    "getProtoTypeName: replaced dots '" + name + "'\n",
                    Level.FINEST);
            name = "::" + name + "::";
        }
        name += message.getProtobufName();

        return name;
    }

    private String getDdsTypeName(MetaMessage message) {
        String name = message.getDDSName();

        if (!name.startsWith(".")) {
            String pckg = this.metadata.getProtoFile().getPackage();

            if (!("".equals(pckg))) {
                name = pckg + "." + name;
            }
            name = name.replaceAll("\\.", "::");
            name = "::" + name;
        } else {
            name = name.replaceAll("\\.", "::");
        }

        return name;
    }

    private String getIncludes() {
        String fileName = stripProto(this.metadata.getProtoFile().getName());
        StringBuffer buffer = new StringBuffer();
        buffer.append("#include \"");
        buffer.append(fileName);
        buffer.append(".pb.h\"\n");
        buffer.append("#include \"");
        buffer.append(fileName);
        buffer.append("_DCPS.hpp\"\n");
        buffer.append("#include \"org/opensplice/topic/DataRepresentation.hpp\"");

        return buffer.toString();
    }

    private String getProtobufMetaHash(MetaFile file, MetaMessage message)
            throws ProtoParseException {
        String tab = CodeGenerator.IDL_TAB + CodeGenerator.IDL_TAB;
        String tabPlusOne = tab + CodeGenerator.IDL_TAB;

        try {
            byte[] metaDescriptor = message.getProtobufMetaDescriptor();
            MessageDigest md = MessageDigest.getInstance("MD5");
            byte[] md5hash = md.digest(metaDescriptor);
            StringBuffer buffer = new StringBuffer();
            int size = md5hash.length;
            buffer.append("unsigned int tmp;\n");
            buffer.append(tab);
            buffer.append("const char *hexMetaHash = \"");

            for (final byte b : md5hash) {
                buffer.append(String.format("%02x", b & 0xff));
            }
            buffer.append("\", *pos = hexMetaHash;\n");
            buffer.append(tab);
            buffer.append("::std::vector<os_uchar> val(" + size + ");\n\n");
            buffer.append(tab);
            buffer.append("for(size_t count = 0; count < val.size(); count++) {\n");
            buffer.append(tabPlusOne);
            buffer.append("sscanf(pos, \"%2x\", &tmp);\n");
            buffer.append(tabPlusOne);
            buffer.append("val[count] = (os_uchar)tmp;\n");
            buffer.append(tabPlusOne);
            buffer.append("pos += 2;\n");
            buffer.append(tab);
            buffer.append("}\n");
            buffer.append(tab);
            buffer.append("return val;");

            return buffer.toString();
        } catch (NoSuchAlgorithmException e) {
            throw new ProtoParseException(
                    "Unable to generate MD5 hash for message '"
                            + message.getProtobufName() + "' ("
                            + e.getMessage() + ").");
        }
    }

    private static String[] kKeywordList = { "and", "and_eq", "asm", "auto",
            "bitand", "bitor", "bool", "break", "case", "catch", "char",
            "class", "compl", "const", "const_cast", "continue", "default",
            "delete", "do", "double", "dynamic_cast", "else", "enum",
            "explicit", "extern", "false", "float", "for", "friend", "goto",
            "if", "inline", "int", "long", "mutable", "namespace", "new",
            "not", "not_eq", "operator", "or", "or_eq", "private", "protected",
            "public", "register", "reinterpret_cast", "return", "short",
            "signed", "sizeof", "static", "static_cast", "struct", "switch",
            "template", "this", "throw", "true", "try", "typedef", "typeid",
            "typename", "union", "unsigned", "using", "virtual", "void",
            "volatile", "wchar_t", "while", "xor", "xor_eq" };

    private String getProtobufFieldNameForGetAndSet(MetaField field) {
        return getProtobufFieldNameForGetAndSet(field.getProtobufName());
    }

    private String getProtobufFieldNameForGetAndSet(String field) {
        String result = field.toLowerCase();
        boolean found = false;

        for (int i = 0; i < kKeywordList.length && !found; i++) {
            if (kKeywordList[i].equals(result)) {
                result += "_";
                found = true;
            }
        }
        return result;
    }

    private String getProtobufKeyGetters(MetaMessage message) {
        String tab = CodeGenerator.IDL_TAB + CodeGenerator.IDL_TAB
                + CodeGenerator.IDL_TAB;
        StringBuffer result = new StringBuffer();
        boolean first = true;

        for (MetaField field : message.getDDSFields()) {
            if (!first) {
                result.append("\n");
                result.append(tab);
            } else {
                first = false;
            }
            result.append("dds->");
            result.append(field.getDDSName());
            result.append("(proto->");

            if (field.hasScope()) {
                for (String member : field.getScopeNames()) {
                    result.append(this.getProtobufFieldNameForGetAndSet(member));
                    result.append("().");
                }
            }
            result.append(this.getProtobufFieldNameForGetAndSet(field));
            result.append("());");
        }
        return result.toString();
    }

    private String getProtobufKeySetters(MetaMessage message) {
        String tab = CodeGenerator.IDL_TAB + CodeGenerator.IDL_TAB
                + CodeGenerator.IDL_TAB;
        StringBuffer result = new StringBuffer();
        boolean first = true;

        for (MetaField field : message.getDDSFields()) {
            if (field.isDDSKey()) {
                if (!first) {
                    result.append("\n");
                    result.append(tab);
                } else {
                    first = false;
                }
                result.append("proto->");

                if (field.hasScope()) {
                    List<String> scopeNames = field.getScopeNames();

                    for (int i = 0; i < scopeNames.size(); i++) {
                        result.append("mutable_");
                        result.append(this
                                .getProtobufFieldNameForGetAndSet(scopeNames
                                        .get(i)));
                        result.append("()->");
                    }
                }
                result.append("set_");
                result.append(this.getProtobufFieldNameForGetAndSet(field));
                result.append("(dds->");
                result.append(field.getDDSName());
                result.append("());");
            }
        }
        return result.toString();
    }

    private String getProtobufMetaDescriptor(MetaFile file) {
        String metaDescriptor = file.getMetaDescriptorAsString();
        StringBuffer buffer = new StringBuffer();
        int length = metaDescriptor.length();
        String tab = CodeGenerator.IDL_TAB + CodeGenerator.IDL_TAB;
        String tabPlusOne = tab + CodeGenerator.IDL_TAB;
        String tabPlusTwo = tabPlusOne + CodeGenerator.IDL_TAB;
        int index = 0;
        int charsPerLine = 500;

        buffer.append("unsigned int tmp;\n");
        buffer.append(tab);
        buffer.append("const char *hexMetaDescriptor[] = {");

        while (index + charsPerLine < length) {
            if (index != 0) {
                buffer.append(",");
            }
            buffer.append("\n" + tabPlusOne + "\""
                    + metaDescriptor.substring(index, index + charsPerLine)
                    + "\"");
            index += charsPerLine;
        }
        if (length > (index - 1)) {
            buffer.append(",\n" + tabPlusOne + "\""
                    + metaDescriptor.substring(index)
                    + "\"}, *pos = hexMetaDescriptor[0];\n");
        } else {
            buffer.append("\"}, *pos;\n");
        }
        buffer.append(tab);
        buffer.append("::std::vector<os_uchar> val(" + length / 2 + ");\n");
        buffer.append(tab);
        buffer.append("int element = 0;\n");
        buffer.append(tab);
        buffer.append("size_t strIndex = " + charsPerLine
                + " - 2, index = strIndex;\n\n");
        buffer.append(tab);
        buffer.append("for(size_t count = 0; count < val.size(); count++) {\n");
        buffer.append(tabPlusOne);
        buffer.append("if(index == strIndex) {\n");
        buffer.append(tabPlusTwo);
        buffer.append("pos = hexMetaDescriptor[element++];\n");
        buffer.append(tabPlusTwo);
        buffer.append("index = 0;\n");
        buffer.append(tabPlusOne);
        buffer.append("} else {\n");
        buffer.append(tabPlusTwo);
        buffer.append("pos += 2;\n");
        buffer.append(tabPlusTwo);
        buffer.append("index += 2;\n");
        buffer.append(tabPlusOne);
        buffer.append("}\n");
        buffer.append(tabPlusOne);
        buffer.append("sscanf(pos, \"%2x\", &tmp);\n");
        buffer.append(tabPlusOne);
        buffer.append("val[count] = (os_uchar)tmp;\n");
        buffer.append(tab);
        buffer.append("}\n");
        buffer.append(tab);
        buffer.append("return val;");

        return buffer.toString();
    }

    private String getProtobufExtensions(MetaFile file, MetaMessage message) {
        StringBuffer buffer = new StringBuffer();

        buffer.append("return ::std::vector<os_uchar>();");

        return buffer.toString();
    }

    protected java.io.File getTemplatesDirectory() throws ProtoParseException {
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
                + "ISOCPP2");
    }
}
