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

/************************************************************************
 * LOGICAL_NAME:    HelloWorldPublisher.java
 * FUNCTION:        Publisher's main for the HelloWorld OpenSplice programming example.
 * MODULE:          OpenSplice HelloWorld example for the java programming language.
 * DATE             September 2010.
 ************************************************************************/

import DDS.DataWriter;
import DDS.HANDLE_NIL;
import HelloWorldData.Msg;
import HelloWorldData.MsgDataWriter;
import HelloWorldData.MsgDataWriterHelper;
import HelloWorldData.MsgTypeSupport;

public class HelloWorldDataPublisher {

	public static void main(String[] args) {
		DDSEntityManager mgr = new DDSEntityManager();
		String partitionName = "HelloWorld example";

		// create Domain Participant
		mgr.createParticipant(partitionName);

		// create Type
		MsgTypeSupport msgTS = new MsgTypeSupport();
		mgr.registerType(msgTS);

		// create Topic
		mgr.createTopic("HelloWorldData_Msg");

		// create Publisher
		mgr.createPublisher();

		// create DataWriter
		mgr.createWriter();

		// Publish Events

		DataWriter dwriter = mgr.getWriter();
		MsgDataWriter HelloWorldWriter = MsgDataWriterHelper.narrow(dwriter);
		Msg msgInstance = new Msg();
		msgInstance.userID = 1;
		msgInstance.message = "Hello World";
		System.out.println("=== [Publisher] writing a message containing :");
		System.out.println("    userID  : " + msgInstance.userID);
		System.out.println("    Message : \"" + msgInstance.message + "\"");
		HelloWorldWriter.register_instance(msgInstance);
		int status = HelloWorldWriter.write(msgInstance, HANDLE_NIL.value);
		ErrorHandler.checkStatus(status, "MsgDataWriter.write");
		try {
			Thread.sleep(4000);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		// clean up
		mgr.getPublisher().delete_datawriter(HelloWorldWriter);
		mgr.deletePublisher();
		mgr.deleteTopic();
		mgr.deleteParticipant();

	}
}
