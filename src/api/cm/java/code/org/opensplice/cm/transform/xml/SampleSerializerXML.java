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
package org.opensplice.cm.transform.xml;

import java.io.StringWriter;

import org.opensplice.cm.data.GID;
import org.opensplice.cm.data.Message;
import org.opensplice.cm.data.Sample;
import org.opensplice.cm.data.UserData;
import org.opensplice.cm.transform.SampleSerializer;
import org.opensplice.cm.transform.TransformationException;

/**
 * The XML implementation of a SampleDeserializer. It is capable of
 * transforming a Sample into its XML representation. For the handling of the
 * UserData in the Sample, the UserDataSerializerXML is used.
 *
 * @date Mar 31, 2005
 */
public class SampleSerializerXML implements SampleSerializer {

    @Override
    public String serializeSample(Sample sample) throws TransformationException {
        if(sample == null){
            throw new TransformationException("Provided sample invalid.");
        }
        StringWriter writer = new StringWriter();

        writer.write("<object>");
        this.writeSample(writer, sample);
        writer.write("</object>");
        writer.flush();

        return writer.toString();
    }

    private void writeSample(StringWriter writer, Sample sample) throws TransformationException{
        Sample previous;
        Message message;
        UserData userData;
        UserDataSerializerXML uds;
        String xmlUserData;

        this.writeTime(writer, "insertTime", sample.getInsertTimeSec(), sample.getInsertTimeNanoSec());
        writer.write("<sampleState>" + sample.getState().getValue() + "</sampleState>");

        previous = sample.getPrevious();

        if(previous != null){
            writer.write("<previous>");
            this.writeSample(writer, previous);
            writer.write("</previous>");
        } else {
            writer.write("<previous>&lt;NULL&gt;</previous>");
        }
        writer.write("<instance>(an-object-value)</instance>");
        writer.write("<disposeCount>" + sample.getDisposeCount() + "</disposeCount>");
        writer.write("<noWritersCount>" + sample.getNoWritersCount() + "</noWritersCount>");
        message = sample.getMessage();

        if(message != null){
            writer.write("<message><nodeState>" + message.getNodeState().getValue() + "</nodeState>");
            this.writeTime(writer, "writeTime", message.getWriteTimeSec(), message.getWriteTimeNanoSec());
            this.writeGID(writer, "writerGID", message.getWriterGid());
            this.writeGID(writer, "instanceGID", message.getInstanceGid());
            writer.write("<sampleSequenceNumber>" + message.getSampleSequenceNumber() + "</sampleSequenceNumber>");

            int[] value = message.getQos().getValue();


            writer.write("<qos><size>");
            writer.write(Integer.toString(value.length));
            writer.write("</size>");

            for(int i=0; i<value.length; i++){
                writer.write("<element>" + value[i] + "</element>");
            }
            writer.write("</qos>");

            userData = message.getUserData();

            if(userData != null){
                uds = new UserDataSerializerXML();
                xmlUserData = uds.serializeUserData(userData);
                writer.write("<userData>");
                writer.write(xmlUserData.substring(8, xmlUserData.length()-9));
                writer.write("</userData>");
            } else {
                writer.write("<userData>&lt;NULL&gt;</userData>");
            }
            writer.write("</message>");
        } else {
            writer.write("<message>&lt;NULL&gt;</message>");
        }
    }

    protected void writeTime(StringWriter writer, String name, long sec, long nsec){
        writer.write("<" + name + "><seconds>" + sec);
        writer.write("</seconds><nanoseconds>" + nsec);
        writer.write("</nanoseconds></" + name + ">");
    }

    private void writeGID(StringWriter writer, String name, GID gid){
        writer.write("<" + name + ">");
        writer.write("<localId>" + gid.getLocalId());
        writer.write("</localId><systemId>" + gid.getSystemId());
        writer.write("</systemId></" + name +">");
    }
}
