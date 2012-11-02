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
 * LOGICAL_NAME:    LifecyclePublisher.java
 * FUNCTION:        OpenSplice Tutorial example code.
 * MODULE:          Tutorial for the Java programming language.
 * DATE             September 2010.
 ************************************************************************
 * 
 * This file contains the implementation for the 'LifecyclePublisher' executable.
 * 
 ***/

import java.io.IOException;

import DDS.*;
import LifecycleData.*;

public class LifecycleDataPublisher {

	public static void usage() {
		System.out.println("*** ERROR");
		System.out.println("*** usage : LifecyclePublisher <autodispose_flag> <writer_action>");
		System.out.println("***         . autodispose_flag = false | true");
		System.out.println("***         . dispose | unregister | stoppub");
	}

	public static void main(String args[]) {
		System.out.println("=== args_length=" + args.length);
		if (args.length < 2) {
			usage();
		}
		if (!(args[0].equals("false")) && !(args[0].equals("true"))
                    && !(args[1].equals("dispose")) && !(args[1].equals("unregister")) && !(args[1].equals("stoppub")))
		{
		  usage();
		}
		boolean autodispose = (args[0].equals("true"));
		// ------------------ Msg topic --------------------//
		DDSEntityManager mgr = new DDSEntityManager(autodispose);
		// autodispose_unregistered_instances
		// create domain participant
		mgr.createParticipant("Lifecycle example");
		// create type
		MsgTypeSupport mt = new MsgTypeSupport();
		mgr.registerType(mt);

		// create Topic
		mgr.createTopic("Lifecycle_Msg");

		// create Publisher
		mgr.createPublisher();

		// create DataWriter
		mgr.createWriters();

		// Publish Samples
		DataWriter dwriter = mgr.getWriter();
		MsgDataWriter LifecycleWriter = MsgDataWriterHelper.narrow(dwriter);
		DataWriter dwriter_stopper = mgr.getWriter_stopper();
		MsgDataWriter LifecycleWriter_stopper = MsgDataWriterHelper.narrow(dwriter_stopper);
		try {
			Thread.sleep(500);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}

		int status;
		if (args[1].equals("dispose")){ 
		    // Send Msg (topic to monitor)
		    Msg msgInstance = new Msg();
		    msgInstance.userID = 1;
		    msgInstance.message = "Lifecycle_1";
		    msgInstance.writerStates = "SAMPLE_SENT -> INSTANCE_DISPOSED -> DATAWRITER_DELETED";
		    System.out.println("=== [Publisher]  :");
		    System.out.println("    userID  : " + msgInstance.userID);
		    System.out.println("    Message  : " + msgInstance.message);
		    System.out.println("    writerStates : " + msgInstance.writerStates);
		    status = LifecycleWriter.write(msgInstance, DDS.HANDLE_NIL.value);
		    ErrorHandler.checkStatus(status, "MsgDataWriter.write");
		    // sleep(1000);
		    try {
			      Thread.sleep(500);
			} catch (InterruptedException e) {
			      e.printStackTrace();
			}
			System.out.println("=== [Publisher]  : SAMPLE_SENT");

			// Dispose instance
			status = LifecycleWriter.dispose(msgInstance, DDS.HANDLE_NIL.value);
			ErrorHandler.checkStatus(status, "MsgDataWriter::dispose");
		       System.out.println("=== [Publisher]  : INSTANCE_DISPOSED");
		}
		else if (args[1].equals("unregister")){ 
		    // Send Msg (topic to monitor)
		    Msg msgInstance = new Msg();
		    msgInstance.userID = 2;
		    msgInstance.message = "Lifecycle_2";
		    msgInstance.writerStates = "SAMPLE_SENT -> INSTANCE_UNREGISTERED -> DATAWRITER_DELETED";
		    System.out.println("=== [Publisher]  :");
		    System.out.println("    userID  : " + msgInstance.userID);
		    System.out.println("    Message  : " + msgInstance.message);
		    System.out.println("    writerStates : " + msgInstance.writerStates);
		    status = LifecycleWriter.write(msgInstance, DDS.HANDLE_NIL.value);
		    ErrorHandler.checkStatus(status, "MsgDataWriter.write");
		    // sleep(1000);
		    try {
			      Thread.sleep(500);
			} catch (InterruptedException e) {
			      e.printStackTrace();
			}
		    System.out.println("=== [Publisher]  : SAMPLE_SENT");
		    // Unregister instance : the auto_dispose_unregisterd_instances flag
		    // is currently ignored and the instance is never disposed
		    // automatically
		    status = LifecycleWriter.unregister_instance(msgInstance,
					DDS.HANDLE_NIL.value);
		    ErrorHandler.checkStatus(status,
					"MsgDataWriter::unregister_instance");
		    System.out.println("=== [Publisher]  : INSTANCE_UNREGISTERED");
		}
		else if (args[1].equals("stoppub")){ 
		    // Send Msg (topic to monitor)
		    Msg msgInstance = new Msg();
		    msgInstance.userID = 3;
		    msgInstance.message = "Lifecycle_3";
		    msgInstance.writerStates = "SAMPLE_SENT -> DATAWRITER_DELETED";
		    System.out.println("=== [Publisher]  :");
		    System.out.println("    userID  : " + msgInstance.userID);
		    System.out.println("    Message  : " + msgInstance.message);
		    System.out.println("    writerStates : " + msgInstance.writerStates);
		    status = LifecycleWriter.write(msgInstance, DDS.HANDLE_NIL.value);
		    ErrorHandler.checkStatus(status, "MsgDataWriter.write");
		    // sleep(1000);
		    try {
			      Thread.sleep(500);
			} catch (InterruptedException e) {
			      e.printStackTrace();
			}
		    System.out.println("=== [Publisher]  : SAMPLE_SENT");
		}

		// let the subscriber treat the previous writer state !!!!
		System.out
				.println("=== [Publisher] waiting 500ms to let the subscriber treat the previous write state ...");
		// sleep(2);
		try {
			Thread.sleep(500);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}

		// Clean-up entities for Msg topic
		/* Remove the DataWriters */
		mgr.getPublisher().delete_datawriter(LifecycleWriter);
		try {
			Thread.sleep(500);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		System.out.println("=== [Publisher]  : DATAWRITER_DELETED");

		// Stop the subscriber
		Msg msgInstance = new Msg();
		msgInstance.userID = 4;
		msgInstance.message = "Lifecycle_4";
		msgInstance.writerStates = "STOPPING_SUBSCRIBER";
		System.out.println("=== [Publisher]  :");
		System.out.println("    userID  : " + msgInstance.userID);
		System.out.println("    Message  : " + msgInstance.message);
		System.out.println("    writerStates : " + msgInstance.writerStates);
		status = LifecycleWriter_stopper.write(msgInstance, DDS.HANDLE_NIL.value);
		 ErrorHandler.checkStatus(status, "MsgDataWriter.write");
		 // sleep(1000);
		 try {
			      Thread.sleep(1000);
			} catch (InterruptedException e) {
			      e.printStackTrace();
		}
 		/* Remove the DataWriters */
		mgr.getPublisher().delete_datawriter(LifecycleWriter_stopper);

		/* Remove the Publisher. */
		mgr.deletePublisher();

		/* Remove the Topics. */
		mgr.deleteTopic();

		/* Remove Participant. */
		mgr.deleteParticipant();

	}
}
