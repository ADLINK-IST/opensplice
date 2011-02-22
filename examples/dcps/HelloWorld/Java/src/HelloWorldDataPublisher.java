/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
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
