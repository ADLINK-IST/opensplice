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
 * LOGICAL_NAME:    ListenerPublisher.java
 * FUNCTION:        OpenSplice example code.
 * MODULE:          Tutorial for the Java programming language.
 * DATE             August 2010.
 ************************************************************************
 * 
 * This file contains the implementation for the 'ListenerPublisher' executable.
 * 
 ***/

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
		listenerWriter.register_instance(msgSample);
       
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
		status = listenerWriter.unregister_instance(msgSample,
				DDS.HANDLE_NIL.value);
		ErrorHandler.checkStatus(status, "MsgDataWriter.unregister_instance");
		mgr.getPublisher().delete_datawriter(listenerWriter);
		mgr.deletePublisher();
		mgr.deleteTopic();
		mgr.deleteParticipant();
	}
}
