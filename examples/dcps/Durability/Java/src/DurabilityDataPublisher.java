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
 * LOGICAL_NAME:    DurabilityDataPublisher.java
 * FUNCTION:        OpenSplice example code.
 * MODULE:          Tutorial for the Java programming language.
 * DATE             April 2009.
 ************************************************************************
 * 
 * This file contains the implementation for the 'DurabilityDataPublisher' executable.
 * 
 ***/

import java.io.IOException;

import DurabilityData.*;
import DDS.*;

public class DurabilityDataPublisher {

	public static void usage() {
		System.err.println("*** ERROR");
		System.err
				.println("*** usage : DurabilityDataPublisher <durability_kind> <autodispose_flag> <automatic_flag>");
		System.err
				.println("***         . durability_kind = transient | persistent");
		System.err.println("***         . autodispose_flag = false | true");
		System.err.println("***         . automatic_flag = false | true");
	}

	public static void main(String args[]) {

		if (args.length < 3) {
			usage();
		}
		if ((!args[0].equals("transient") && !args[0].equals("persistent"))
				|| (!args[1].equals("false") && !args[1].equals("true"))) {
			usage();
		}
		String durability_kind = args[0];
		boolean automatic = true;
		boolean autodispose_unregistered_instances = (args[1].equals("true"));

		DDSEntityManager mgr = new DDSEntityManager(durability_kind,
				autodispose_unregistered_instances);
		automatic = (args[2].equals("true"));
		// create domain participant
		String partition_name = "Durability example";
		mgr.createParticipant(partition_name);

		// create type
		MsgTypeSupport mt = new MsgTypeSupport();
		mgr.registerType(mt);

		// create Topic
		String topic_name = "DurabilityData_Msg";
		mgr.createTopic(topic_name);

		// create Publisher
		mgr.createPublisher();

		// create DataWriter
		mgr.createWriter();

		// Publish Events
		DataWriter dwriter = mgr.getWriter();
		MsgDataWriter DurabilityDataWriter = MsgDataWriterHelper
				.narrow(dwriter);
		int status;

		Msg msgSample = new Msg();
		DurabilityDataWriter.register_instance(msgSample);
		for (int i = 0; i < 10; i++) {
			msgSample.id = i;
			msgSample.content = "" + i;
			status = DurabilityDataWriter.write(msgSample, HANDLE_NIL.value);
			ErrorHandler.checkStatus(status, "MsgDataWriter.write");

		}

		System.out.println("IsAutomatic == " + automatic);
		if (!automatic) {
			char c = 0;
			System.out.println("Enter E to exit");
			while (c != 'E') {
				try {
					c = (char) System.in.read();
					System.in.skip(System.in.available());
					System.out.println(c);
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
		} else {
			try {
				Thread.currentThread().sleep(180000);// sleep for 2 s
			} catch (InterruptedException ie) {
			}
		}
		// clean up

		/*
		 * Do not release the data-samples in order to let persistency test work
		 */

		/* Remove the DataWriters */
		mgr.getPublisher().delete_datawriter(DurabilityDataWriter);

		/* Remove the Publisher. */
		mgr.deletePublisher();

		/* Remove the Topics. */
		mgr.deleteTopic();

		/* Remove Participant. */
		mgr.deleteParticipant();

	}
}
