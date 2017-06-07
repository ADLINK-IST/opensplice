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
package com.prismtech.dds.protobuf.frontend;

import java.util.ArrayList;
import java.util.List;

import org.omg.dds.protobuf.DescriptorProtos;

import com.google.protobuf.DescriptorProtos.DescriptorProto;
import com.google.protobuf.DescriptorProtos.FileDescriptorProto;
import com.google.protobuf.DescriptorProtos.FileDescriptorSet;
import com.google.protobuf.InvalidProtocolBufferException;
import com.google.protobuf.UnknownFieldSet.Field;
import com.prismtech.dds.protobuf.ProtoParseException;

public class MetaFile {
    private ArrayList<MetaMessage> messages;
    private FileDescriptorProto protoFile;
    private FileDescriptorSet filePlusDependencies;


    public MetaFile(FileDescriptorProto protoFile,
            FileDescriptorSet filePlusDependencies)
            throws InvalidProtocolBufferException, ProtoParseException {
        this.protoFile = protoFile;
        this.filePlusDependencies = filePlusDependencies;
        this.messages = new ArrayList<MetaMessage>();

        for (DescriptorProto message : protoFile.getMessageTypeList()) {
            this.processMessage(null, message);
        }
    }

    public boolean hasDDSEnabledMessages() {
        for (MetaMessage message : this.messages) {
            if (message.isDDSEnabled()) {
                return true;
            }
        }
        return false;
    }

    private void processMessage(MetaMessage parent, DescriptorProto message)
            throws InvalidProtocolBufferException, ProtoParseException {
        MetaMessage metaMessage;
        Field ddsMessageField = message.getOptions().getUnknownFields()
                .getField(DescriptorProtos.TYPE_FIELD_NUMBER);

        metaMessage = new MetaMessage(this, parent, message, ddsMessageField);
        this.messages.add(metaMessage);

        for (DescriptorProto nested : message.getNestedTypeList()) {
            this.processMessage(metaMessage, nested);
        }
    }

    public List<MetaMessage> getDDSMessages() {
        return new ArrayList<MetaMessage>(this.messages);
    }

    public FileDescriptorProto getProtoFile() {
        return this.protoFile;
    }

    public FileDescriptorSet getFilePlusDependencies() {
        return this.filePlusDependencies;
    }

    public String getMetaDescriptorAsString() {
        byte[] descriptor = this.filePlusDependencies.toByteArray();
        StringBuilder sb = new StringBuilder(descriptor.length * 2);

        for (final byte b : descriptor) {
            sb.append(String.format("%02x", b & 0xff));
        }
        return sb.toString();
    }

    public byte[] getMetaDescriptor() {
        return this.filePlusDependencies.toByteArray();
    }

    @Override
    public String toString() {
        String result = "- File: '" + this.protoFile.getName() + "'\n";

        for (MetaMessage message : this.messages) {
            result += message.toString() + "\n";
        }
        return result;
    }
}
