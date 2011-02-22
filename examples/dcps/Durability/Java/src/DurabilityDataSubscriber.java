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
 * LOGICAL_NAME:    DurabilityDataSubscriber.java
 * FUNCTION:        OpenSplice example code.
 * MODULE:          Tutorial for the Java programming language.
 * DATE             April 2009.
 ************************************************************************
 * 
 * This file contains the implementation for the 'DurabilityDataSubscriber' executable.
 * 
 ***/

import DDS.*;
import DurabilityData.*;

public class DurabilityDataSubscriber {
	public static void usage() {
		
		System.err.println("*** ERROR ***");
		System.err
				.println("*** usage : DurabilityDataSubscriber <durability_kind>");
		System.err
				.println("***         . durability_kind = transient | persistent");
	}

	public static void main(String args[]) {
		MsgSeqHolder msgList = new MsgSeqHolder();
		SampleInfoSeqHolder infoSeq = new SampleInfoSeqHolder();

		if (args.length < 1) {
			usage();
		}
		if ((!args[0].equals("transient")) && (!args[0].equals("persistent"))) {
			usage();
		}
		String durability_kind = args[0];
		DDSEntityManager mgr = new DDSEntityManager(durability_kind);

		// create domain participant
		String partition_name = "Durability example";
		mgr.createParticipant(partition_name);

		// create type
		MsgTypeSupport mt = new MsgTypeSupport();
		mgr.registerType(mt);

		// create Topic
		String topic_name = "DurabilityData_Msg";
		mgr.createTopic(topic_name);

		// create Subscriber
		mgr.createSubscriber();

		// create DataReader
		mgr.createReader();

		DataReader dreader = mgr.getReader();
		MsgDataReader DurabilityDataReader = MsgDataReaderHelper
				.narrow(dreader);
		ErrorHandler.checkHandle(DurabilityDataReader, "MsgDataReader.narrow");

		System.out.println("=== [Subscriber] Ready ...");

		boolean closed = false;
		int status = -1;
                int count = 0;
		do {
			status = DurabilityDataReader.take(msgList, infoSeq,
					LENGTH_UNLIMITED.value, ANY_SAMPLE_STATE.value,
					ANY_VIEW_STATE.value, ANY_INSTANCE_STATE.value);
			ErrorHandler.checkStatus(status, "msgDataReader.take");
			if (msgList.value.length > 0) {
				int j = 0;
				do {
					if (infoSeq.value[j].valid_data) {
						System.out.println(msgList.value[j].content);
						if (msgList.value[j].content.compareTo("9") == 0) {
							closed = true;
						}
					}
				} while (++j < msgList.value.length);
				
			}
                        try
			{
				Thread.sleep(200);
			}
			catch(InterruptedException ie)
			{
				// nothing to do
			}
			++count;
		} while (!closed && count < 1500) ; // We dont want the example to run indefinitely

                status = DurabilityDataReader.return_loan(msgList, infoSeq);
                ErrorHandler.checkStatus(status, "MsgDataReader.return_loan");
		// cleanup
		mgr.getSubscriber().delete_datareader(DurabilityDataReader);
		mgr.deleteSubscriber();
		mgr.deleteTopic();
		mgr.deleteParticipant();

	}
}
