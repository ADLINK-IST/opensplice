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
 *  
 * Copyright (c) 2007
 * PrismTech Ltd.
 * All rights Reserved.
 * 
 * LOGICAL_NAME:    WaitSetDataPublisher.java
 * FUNCTION:        OpenSplice example code.
 * MODULE:          Tutorial for the Java programming language.
 * DATE             August 2010.
 ************************************************************************
 * 
 * This file contains the implementation for the 'WaitSetDataPublisher' executable.
 * 
 ***/

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
			Thread.currentThread().sleep(500);// sleep for 10000 Ms
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