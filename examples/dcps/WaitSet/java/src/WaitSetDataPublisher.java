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

import DDS.*;
import WaitSetData.*;

public class WaitSetDataPublisher {

	public static void main(String args[]) {
		DDSEntityManager mgr = new DDSEntityManager();

		// create domain participant
		String partition_name = "WaitSet example";
		mgr.createParticipant(partition_name);

		// create type
		MsgTypeSupport mt = new MsgTypeSupport();
		mgr.registerType(mt);

		// create Topic
		String topic_name = "WaitSetData_Msg";
		mgr.createTopic(topic_name);

		// create Publisher
		mgr.createPublisher();

		// create DataWriter
		mgr.createWriter();

		// Publish Events
		DataWriter dwriter = mgr.getWriter();
		MsgDataWriter WaitSetDataWriter = MsgDataWriterHelper.narrow(dwriter);

		Msg msgInstance = new Msg(); /* Example on Stack */
		msgInstance.userID = 1;
		msgInstance.message = "First Hello";
		System.out.println("=== [Publisher] writing a message containing :");
		System.out.println("    userID  : " + msgInstance.userID);
		System.out.println("    Message : \"" + msgInstance.message + "\"");

		int status = WaitSetDataWriter.write(msgInstance, HANDLE_NIL.value);
		ErrorHandler.checkStatus(status, "MsgDataWriter.write");
		// Sleep(500ms);
		try {
			Thread.currentThread().sleep(500);// sleep for 500 Ms
		} catch (InterruptedException ie) {
			// If this thread was intrrupted by nother thread
		}
		// Write a second message
		msgInstance.message = "Hello again";
		status = WaitSetDataWriter.write(msgInstance, HANDLE_NIL.value);
		ErrorHandler.checkStatus(status, "MsgDataWriter::write");

		System.out.println("=== [Publisher] writing a message containing :");
		System.out.println("    userID  : " + msgInstance.userID);
		System.out.println("    Message : \"" + msgInstance.message + "\"");
		// Sleep(500ms);
		try {
			Thread.currentThread().sleep(500);// sleep for 500 Ms
		} catch (InterruptedException ie) {
			// If this thread was intrrupted by nother thread
		}

		/* Remove the DataWriters */
		mgr.getPublisher().delete_datawriter(WaitSetDataWriter);

		/* Remove the Publisher. */
		mgr.deletePublisher();

		/* Remove the Topics. */
		mgr.deleteTopic();

		/* Remove Participant. */
		mgr.deleteParticipant();

	}
}