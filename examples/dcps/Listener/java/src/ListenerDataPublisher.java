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

import DDS.DataWriter;
import DDS.HANDLE_NIL;
import ListenerData.Msg;
import ListenerData.MsgDataWriter;
import ListenerData.MsgDataWriterHelper;
import ListenerData.MsgTypeSupport;

public class ListenerDataPublisher {

	public static void main(String[] args) {
		int status;

		DDSEntityManager mgr = new DDSEntityManager();
		String partitionName = "Listener example";

		// create Domain Participant
		mgr.createParticipant(partitionName);

		// create Type
		MsgTypeSupport msgTS = new MsgTypeSupport();
		mgr.registerType(msgTS);

		// create Topic
		mgr.createTopic("ListenerData_Msg");

		// create Publisher
		mgr.createPublisher();

		// create DataWriter
		mgr.createWriter();

		// Publish Events

		DataWriter dwriter = mgr.getWriter();
		MsgDataWriter listenerWriter = MsgDataWriterHelper.narrow(dwriter);

		Msg msgSample = new Msg();
       
		for (int i = 1; i < 2; i++) {
			msgSample.userID = i;
			msgSample.message = "Hello World";
			status = listenerWriter.write(msgSample, HANDLE_NIL.value);
			ErrorHandler.checkStatus(status, "MsgDataWriter.write");

		}
		// sleep(2000);
		try {
			Thread.sleep(2000);
		} catch (InterruptedException e) {

			e.printStackTrace();

		}
		// clean up
		mgr.getPublisher().delete_datawriter(listenerWriter);
		mgr.deletePublisher();
		mgr.deleteTopic();
		mgr.deleteParticipant();
	}
}
