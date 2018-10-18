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
package com.prismtech.dds.protobuf.frontend;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.omg.dds.protobuf.DescriptorProtos.FieldOptions;

import com.google.protobuf.DescriptorProtos.DescriptorProto;
import com.google.protobuf.DescriptorProtos.FieldDescriptorProto;
import com.google.protobuf.DescriptorProtos.FileDescriptorProto;
import com.google.protobuf.InvalidProtocolBufferException;
import com.google.protobuf.UnknownFieldSet.Field;
import com.prismtech.dds.protobuf.ProtoParseException;

public class MetaField {
    private FileDescriptorProto protoFile;
    private DescriptorProto message;
    private FieldDescriptorProto protobufField;
    private FieldOptions ddsFieldOptions;
    private String ddsTypeName;
    private List<Scope> scope;

    public static final class Scope {
        private FileDescriptorProto file;
        private DescriptorProto message;
        private FieldDescriptorProto field;

        public Scope(FileDescriptorProto file, DescriptorProto message,
                FieldDescriptorProto field) {
            this.file = file;
            this.message = message;
            this.field = field;
        }

        public FileDescriptorProto getFile() {
            return this.file;
        }

        public DescriptorProto getMessage() {
            return this.message;
        }

        public FieldDescriptorProto getField() {
            return this.field;
        }

        public void setFile(FileDescriptorProto file) {
            this.file = file;
        }

        public void setMessage(DescriptorProto message) {
            this.message = message;
        }

        public void setField(FieldDescriptorProto field) {
            this.field = field;
        }

        @Override
        public String toString(){
            return
                    "- File    : " + file.getName() + "\n" +
                    "- Package : " + file.getPackage() + "\n" +
                    "- Message : " + message.getName() + "\n" +
                    "- Type    : " + field.getTypeName() + "\n" +
                    "- Name    : " + field.getName();
        }
    }

    public MetaField(List<Scope> scope, FileDescriptorProto protoFile,
            DescriptorProto protoMessage,
            FieldDescriptorProto protobufField, Field ddsFieldOption)
            throws InvalidProtocolBufferException, ProtoParseException {
        this.scope = new ArrayList<Scope>();

        if (scope != null) {
            this.scope.addAll(scope);
        }
        this.protoFile = protoFile;
        this.message = protoMessage;
        this.protobufField = protobufField;
        this.ddsFieldOptions = org.omg.dds.protobuf.DescriptorProtos.FieldOptions
                .parseFrom(ddsFieldOption.getLengthDelimitedList().get(0));

        if (this.ddsFieldOptions.hasName()) {
            String ddsName = this.ddsFieldOptions.getName();

            /*
             * Field names have to start with a through z or A through Z,
             * followed by a word character [a-zA-Z_0-9].
             */
            if (!ddsName.matches("[a-zA-Z]\\w*")) {
                throw new ProtoParseException(
                        "Found unsupported .omg.dds.member.name = '" + ddsName
                                + "' for field '"
                                + this.protobufField.getName()
                                + "' in message '" + this.message.getName()
                                + "'.");
            }
        }
        String fieldSoFar = "";
        for (Scope s : this.scope) {
            FieldDescriptorProto fdp = s.getField();

            fieldSoFar += fdp.getName() + ".";
            if (fdp.getLabel() != com.google.protobuf.DescriptorProtos.FieldDescriptorProto.Label.LABEL_REQUIRED) {
                throw new ProtoParseException(
                        "Field with name '"
                                + fieldSoFar.substring(0,
                                        fieldSoFar.length() - 1)
                                + "' is not marked as required. Therefore '"
                                + this.getFullyScopedName()
                                + "' cannot be annotated with the '.omg.dds.member' option.");
            }
        }
        if (this.protobufField.getLabel() != com.google.protobuf.DescriptorProtos.FieldDescriptorProto.Label.LABEL_REQUIRED) {
            throw new ProtoParseException(
                    "Field with name '"
                            + this.getProtobufName()
                            + "' in message '"
                            + this.message.getName()
                            + "' is not marked as required. Therefore it cannot be annotated with the '.omg.dds.member' option.");
        }
        initDDSTypeName();
    }

    public String getFullyScopedName() {
        StringBuilder result = new StringBuilder();

        for (Scope s : this.scope) {
            result.append(s.getField().getName());
            result.append(".");
        }
        return result.append(this.getProtobufName()).toString();
    }

    public String getFullyScopedDDSName() {
        StringBuilder result = new StringBuilder();

        for (Scope s : this.scope) {
            result.append(s.getField().getName());
            result.append("_");
        }
        return result.append(this.getProtobufName()).toString();
    }

    public String getDDSName() {
        if (this.ddsFieldOptions.hasName()) {
            return this.ddsFieldOptions.getName();
        }
        if (this.scope.size() > 0) {
            return this.getFullyScopedDDSName();
        }
        return this.getProtobufName();
    }

    public String getDDSTypeName() {
        return this.ddsTypeName;
    }

    private void initDDSTypeName() throws ProtoParseException {
        switch (this.protobufField.getType()) {
        case TYPE_BOOL:
            this.ddsTypeName = "boolean";
            break;
        case TYPE_DOUBLE:
            this.ddsTypeName = "double";
            break;
        case TYPE_FIXED32:
        case TYPE_UINT32:
            this.ddsTypeName = "unsigned long";
            break;
        case TYPE_FIXED64:
        case TYPE_UINT64:
            this.ddsTypeName = "unsigned long long";
            break;
        case TYPE_FLOAT:
            this.ddsTypeName = "float";
            break;
        case TYPE_INT32:
        case TYPE_SINT32:
        case TYPE_SFIXED32:
            this.ddsTypeName = "long";
            break;
        case TYPE_INT64:
        case TYPE_SFIXED64:
        case TYPE_SINT64:
            this.ddsTypeName = "long long";
            break;
        case TYPE_STRING:
            this.ddsTypeName = "string";
            break;
        case TYPE_MESSAGE:
        case TYPE_GROUP:
        case TYPE_ENUM:
        case TYPE_BYTES:
        default:
            throw new ProtoParseException("Field '"
                    + this.protobufField.getName() + "' in Message '"
                            + this.message.getName()
                            + "' has type '"
                    + this.protobufField.getType()
                    + "' which is not supported for the .omg.dds.member option.");
        }
        return;
    }

    public String getProtobufName() {
        return this.protobufField.getName();
    }

    public boolean isDDSKey() {
        if (this.ddsFieldOptions.hasKey()) {
            return this.ddsFieldOptions.getKey();
        }
        return false;
    }

    public boolean isDDSFilterable() {
        if (this.ddsFieldOptions.hasFilterable()) {
            return this.ddsFieldOptions.getFilterable();
        }
        return false;
    }

    public boolean hasScope() {
        return (this.scope.size() == 0) ? false : true;
    }

    public List<String> getScopeNames() {
        List<String> scope = new ArrayList<String>();

        for (Scope s : this.scope) {
            scope.add(s.getField().getName());
        }
        return scope;
    }

    public List<Scope> getScope() {
        return Collections.unmodifiableList(this.scope);
    }

    public FileDescriptorProto getProtoFile() {
        return this.protoFile;
    }

    public DescriptorProto getProtoMessage() {
        return this.message;
    }

    public FieldDescriptorProto getProtoField() {
        return this.protobufField;
    }

    @Override
    public String toString() {
        StringBuffer result = new StringBuffer();

        result.append("  - Field '");
        result.append(this.getFullyScopedName());
        result.append("' (");
        result.append(this.protobufField.getType());
        result.append(")\n    - .omg.dds.member.name       : ");
        result.append(this.getDDSName());
        result.append("\n    - .omg.dds.member.key        : ");
        result.append(this.isDDSKey());
        result.append("\n    - .omg.dds.member.filterable : ");
        result.append(this.isDDSFilterable());
        result.append("\n    - File                       : ");
        result.append(this.protoFile.getName());
        result.append("\n    - Package                    : ");
        result.append(this.protoFile.getPackage());
        result.append("\n    - Message                    : ");
        result.append(this.message.getName());
        result.append("\n    - Name                       : ");
        result.append(this.protobufField.getName());
        result.append("\n    - Scope");

        if(this.scope.size() == 0){
            result.append("                      : -");
        } else {
            String tab = "";
            result.append("                      :");

            for (Scope s : this.scope) {
                result.append("\n      ");
                result.append(tab);
                result.append(s.toString().replaceAll("\\\n",
                        "\\\n      " + tab));
                tab += "  ";
            }
        }
        return result.toString();
    }
}
