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
package org.opensplice.cm.transform.xml;

import java.io.StringWriter;

import org.opensplice.cm.Time;
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
 * @date Jul 12, 2016
 */
public class SampleSerializerXMLTime64 extends SampleSerializerXML {

    @Override
    protected void writeTime(StringWriter writer, String name, long sec, long nsec){
        long _sec = sec * Time.NANOSECONDS + nsec;
        writer.write("<" + name + "><wt>" + sec + "</wt></" + name + ">");
    }

}
