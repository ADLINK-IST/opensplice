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

import org.omg.CORBA.IntHolder;
import org.omg.CORBA.LongHolder;
import org.vortex.FACE.TS;

import FACE.CONNECTION_DIRECTION_TYPE;
import FACE.CONNECTION_DIRECTION_TYPEHolder;
import FACE.MESSAGING_PATTERN_TYPE;
import FACE.RETURN_CODE_TYPE;
import FACE.RETURN_CODE_TYPEHolder;
import HelloWorldData.Msg;
import HelloWorldData.MsgHolder;
import HelloWorldData.MsgTS;

public class HelloWorldDataPublisher {

    public static void main(String[] args) {
        RETURN_CODE_TYPEHolder  return_code = new RETURN_CODE_TYPEHolder(FACE.RETURN_CODE_TYPE.NO_ERROR);

        String configuration = "dds_face_config.xml";
        TS.Initialize(configuration, return_code);
        if (return_code.value != RETURN_CODE_TYPE.NO_ERROR) {
            System.out.println("Error: " + return_code.value.value());
        } else {
            String connection_name = "HelloWorldPub";
            MESSAGING_PATTERN_TYPE pattern = MESSAGING_PATTERN_TYPE.PUB_SUB;
            LongHolder connection_id = new LongHolder(1);
            CONNECTION_DIRECTION_TYPEHolder connection_direction = new CONNECTION_DIRECTION_TYPEHolder(CONNECTION_DIRECTION_TYPE.SOURCE);
            IntHolder max_message_size = new IntHolder(0);
            long timeout = 0;
            TS.Create_Connection(connection_name, pattern, connection_id, connection_direction, max_message_size, timeout, return_code);
            if (return_code.value != RETURN_CODE_TYPE.NO_ERROR) {
                System.out.println("Create_Connection Error: " + return_code.value.value());
            } else {
                Msg msg = new Msg(0, "Hello World");

                for (; msg.userID < 5 && return_code.value == RETURN_CODE_TYPE.NO_ERROR; msg.userID++) {
                    System.out.println(" ________________________________________________________________");
                    System.out.println("|");
                    System.out.println("| Publish message : " + msg.userID + ", " + msg.message);
                    System.out.println("|________________________________________________________________");
                    System.out.println("");

                    LongHolder transaction_id = new LongHolder(0);
                    MsgHolder message = new MsgHolder(msg);
                    IntHolder message_size = new IntHolder(0);
                    long message_type_id = 0;
                    MsgTS.Send_Message(connection_id.value, timeout, transaction_id, message, message_type_id, message_size, return_code);
                    if (return_code.value != RETURN_CODE_TYPE.NO_ERROR) {
                        System.out.println("Send_Message Error: " + return_code.value.value());
                    }
                    try {
                        Thread.sleep(100);
                    } catch (InterruptedException e1) {
                        // nothing
                    }
                }

                try {
                    // Wait to ensure data is received before we delete writer
                    Thread.sleep(1000);
                } catch (InterruptedException e1) {
                    // nothing
                }

                TS.Destroy_Connection(connection_id.value, return_code);
                if (return_code.value != RETURN_CODE_TYPE.NO_ERROR) {
                    System.out.println("Destroy_Connection Error: " + return_code.value.value());
                }
            }
        }
    }
}
