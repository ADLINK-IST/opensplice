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
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.logging.Level;

import org.omg.dds.protobuf.DescriptorProtos;
import org.omg.dds.protobuf.DescriptorProtos.MessageOptions;

import com.google.protobuf.DescriptorProtos.DescriptorProto;
import com.google.protobuf.DescriptorProtos.FieldDescriptorProto;
import com.google.protobuf.DescriptorProtos.FieldDescriptorProto.Type;
import com.google.protobuf.DescriptorProtos.FileDescriptorProto;
import com.google.protobuf.InvalidProtocolBufferException;
import com.google.protobuf.UnknownFieldSet.Field;
import com.prismtech.dds.protobuf.Logger;
import com.prismtech.dds.protobuf.ProtoParseException;

public class MetaMessage {
    private DescriptorProto protobufMessage;
    private MessageOptions ddsMessageOptions;
    private ArrayList<MetaField> ddsFields;
    private MetaMessage parent;
    private MetaFile file;
    private Set<DescriptorProto> descriptorCache;

    public MetaMessage(MetaFile file, MetaMessage parent,
            DescriptorProto message, Field ddsMessageField)
            throws InvalidProtocolBufferException, ProtoParseException {
        this.file = file;
        this.parent = parent;
        this.protobufMessage = message;
        this.ddsFields = new ArrayList<MetaField>();
        this.descriptorCache = new HashSet<DescriptorProto>();

        if (ddsMessageField.getLengthDelimitedList().size() != 0) {
            this.ddsMessageOptions = org.omg.dds.protobuf.DescriptorProtos.MessageOptions
                    .parseFrom(ddsMessageField.getLengthDelimitedList().get(0));

            if (this.ddsMessageOptions.hasName()) {
                String ddsName = this.ddsMessageOptions.getName();

                /*
                 * Type names need to start with a dot to indicate absolute
                 * scope. The scope is assumed relative to the scope of the
                 * message otherwise.
                 *
                 * Names have to start with a through z or A through Z, followed
                 * by a word character [a-zA-Z_0-9].
                 *
                 * Examples: are "MyType", ".MyType", ".some.scope2.MyType",
                 * "scope.MyType4"
                 */
                if (!ddsName.matches(".?([a-zA-Z]\\w*)(.[a-zA-Z]\\w*)*")) {
                    throw new ProtoParseException(
                            "Found unsupported .omg.dds.type.name = '"
                                    + ddsName + "' for message '"
                                    + this.protobufMessage.getName());
                }
            }

            this.setupDDSFields(this.file.getProtoFile(), message,
                    new ArrayList<MetaField.Scope>(),
                    new HashMap<String, MetaField>(),null);
        } else {
            this.ddsMessageOptions = null;
        }
    }

    private void setupDDSFields(FileDescriptorProto protoFile,
            DescriptorProto message, List<MetaField.Scope> scope,
            HashMap<String, MetaField> fieldNames, FieldDescriptorProto dfproto)
            throws InvalidProtocolBufferException, ProtoParseException {

        if (this.descriptorCache.contains(message) && dfproto != null && dfproto.getLabel() !=
                com.google.protobuf.DescriptorProtos.FieldDescriptorProto.Label.LABEL_REQUIRED) {
            /* only add required messages for a second time as they might contain key fields*/
            Logger.getInstance().log(
                    "Message '" + message.getName() + "' already in cache.\n",
                    Level.FINEST);
            return;
        }
        Logger.getInstance().log(
                "Adding message '" + message.getName() + "' to cache.\n",
                Level.FINEST);
        this.descriptorCache.add(message);

        List<FieldDescriptorProto> fieldList = new ArrayList<FieldDescriptorProto>(
                message.getFieldCount() + message.getExtensionCount());
        fieldList.addAll(message.getFieldList());
        fieldList.addAll(message.getExtensionList());

        for (FieldDescriptorProto field : fieldList) {
            Field ddsFieldOption = field.getOptions().getUnknownFields()
                    .getField(DescriptorProtos.MEMBER_FIELD_NUMBER);
            Logger.getInstance().log(
                    "Adding field '" + message.getName() + " " + field.getLabel() +"' to cache.\n",
                    Level.FINEST);

            if (ddsFieldOption.getLengthDelimitedList().size() != 0) {
                MetaField mf = new MetaField(scope, protoFile, message, field,
                        ddsFieldOption);
                this.ddsFields.add(mf);

                MetaField existing = fieldNames.put(mf.getDDSName(), mf);
                Logger.getInstance().log(
                        "- DDS name: '" + mf.getDDSName() + "': ", Level.FINER);

                if (existing != null) {
                    Logger.getInstance().log("KNOWN\n", Level.FINER);

                    throw new ProtoParseException(".omg.dds.member.name of '"
                            + existing.getFullyScopedName() + "' and '"
                            + mf.getFullyScopedName() + "' are both '"
                            + existing.getDDSName()
                            + "' but have to be unique.");
                }
                Logger.getInstance().log("UNKNOWN\n", Level.FINER);
            } else if (field.getType() == Type.TYPE_MESSAGE) {
                Logger.getInstance().log(
                        "Found MESSAGE member '" + field.getName()
                                + "' with type '" + field.getTypeName()
                                + "', parsing contents.\n", Level.FINEST);
                MetaField.Scope s = new MetaField.Scope(protoFile, message,
                        field);
                scope.add(s);

                MetaField.Scope messageType = this.findMessageType(
                        this.file.getProtoFile(), this.protobufMessage,
                        field.getTypeName());

                if (messageType == null) {
                    throw new ProtoParseException(
                            "Unable to find type for message member '"
                                    + field.getName() + "'.");
                }
                this.setupDDSFields(messageType.getFile(),
                        messageType.getMessage(), scope, fieldNames,field);
                scope.remove(s);
            }
        }
    }

    private String stripPackage(String name, FileDescriptorProto fdp) {
        if (fdp.hasPackage()) {
            Logger.getInstance().log("Package '" + fdp.getPackage() + "'.\n",
                    Level.FINEST);
            if (name.startsWith(fdp.getPackage() + ".")) {
                int length = fdp.getPackage().length();
                return name.substring(length + 1);
            }
        }
        return name;
    }

    private MetaField.Scope findMessageType(FileDescriptorProto fileProto,
            DescriptorProto scope, String name) {
        if (name.startsWith(".")) {
            name = name.substring(1);
            for (FileDescriptorProto fdp : this.file.getFilePlusDependencies()
                    .getFileList()) {
                String nameWithoutPackage = stripPackage(name, fdp);

                Logger.getInstance().log(
                        "Name without package '" + nameWithoutPackage + "'.\n",
                        Level.FINEST);

                int index = nameWithoutPackage.indexOf('.');
                String nameToSearch = nameWithoutPackage;

                if (index != -1) {
                    nameToSearch = nameToSearch.substring(0, index);
                }

                Logger.getInstance().log(
                        "Searching for Message '" + nameToSearch + "'.\n",
                        Level.FINEST);

                for (DescriptorProto proto : fdp.getMessageTypeList()) {
                    Logger.getInstance().log(
                            "Found Message '" + proto.getName() + "'.\n",
                            Level.FINEST);

                    if (nameToSearch.equals(proto.getName())) {
                        Logger.getInstance().log("Message matches.\n",
                                Level.FINEST);

                        if (index == -1) {
                            MetaField.Scope ms = new MetaField.Scope(fdp,
                                    proto, null);

                            return ms;
                        }
                        Logger.getInstance()
                                .log("Looking for '"
                                        + nameWithoutPackage.substring(index + 1)
                                        + "' now in '" + proto.getName()
                                        + "'.\n", Level.FINEST);
                        return this.findMessageType(fdp, proto,
                                nameWithoutPackage.substring(index + 1));
                    }
                }
            }
        } else {
            if (fileProto == null) {
                return null;
            }
            int index = name.indexOf('.');
            String nameToSearch = name;

            if (index != -1) {
                nameToSearch = nameToSearch.substring(0, index);
            }

            Logger.getInstance().log(
                    "Searching for Message '" + nameToSearch + "'.\n",
                    Level.FINEST);

            for (DescriptorProto proto : scope.getNestedTypeList()) {
                Logger.getInstance().log(
                        "Found Message '" + proto.getName() + "'.\n",
                        Level.FINEST);

                if (nameToSearch.equals(proto.getName())) {
                    Logger.getInstance()
                            .log("Message matches.\n", Level.FINEST);

                    if (index == -1) {
                        return new MetaField.Scope(fileProto, proto, null);
                    }
                    Logger.getInstance().log(
                            "Looking for '" + name.substring(index + 1)
                                    + "' now.\n", Level.FINEST);
                    return this.findMessageType(fileProto, proto,
                            name.substring(index + 1));
                }
            }
        }
        return null;
    }

    public MetaMessage getParent() {
        return this.parent;
    }

    public boolean isDDSEnabled() {
        return (this.ddsMessageOptions != null);
    }

    public String getDDSName() {
        if (this.ddsMessageOptions != null) {
            if (this.ddsMessageOptions.hasName()) {
                return this.ddsMessageOptions.getName();
            }
        } else {
            return "";
        }
        return this.getProtobufName();
    }

    public String getDDSUnscopedName() {
        String name = this.getDDSName();

        int index = name.lastIndexOf('.');

        if (index != -1) {
            name = name.substring(index + 1);
        }
        return name;
    }

    public DescriptorProto getProtoMessage() {
        return this.protobufMessage;
    }

    public String getProtobufName() {
        return this.protobufMessage.getName();
    }

    public List<MetaField> getDDSFields() {
        return new ArrayList<MetaField>(this.ddsFields);
    }

    public byte[] getProtobufMetaDescriptor() {
        return this.protobufMessage.toByteArray();
    }

    public MetaFile getFile() {
        return this.file;
    }

    @Override
    public String toString() {
        String result = " - Message '" + this.getProtobufName();

        if (this.ddsMessageOptions == null) {
            result += " (not DDS enabled)";
        } else {
            result += "'\n" + "  - .omg.dds.type.name :" + this.getDDSName()
                    + "\n";

            for (MetaField field : this.ddsFields) {
                result += field.toString() + "\n";
            }
        }
        return result;

    }
}
