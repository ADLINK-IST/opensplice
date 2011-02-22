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
 * LOGICAL_NAME:    OwnershipDataSubscriber.java
 * FUNCTION:        OpenSplice example code.
 * MODULE:          Tutorial for the Java programming language.
 * DATE             April 2009.
 ************************************************************************
 * 
 * This file contains the implementation for the 'OwnershipDataSubscriber' executable.
 * 
 ***/

import DDS.*;
import OwnershipData.*;

public class OwnershipDataSubscriber {
	public static void main(String args[]) {
		StockSeqHolder msgList = new StockSeqHolder();
		SampleInfoSeqHolder infoSeq = new SampleInfoSeqHolder();

		DDSEntityManager mgr = new DDSEntityManager();

		// create domain participant
		String partition_name = "Ownership example";
		mgr.createParticipant(partition_name);

		// create type
		StockTypeSupport st = new StockTypeSupport();
		mgr.registerType(st);

		// create Topic
		String topic_name = "StockTrackerExclusive";
		mgr.createTopic(topic_name);

		// create Subscriber
		mgr.createSubscriber();

		// create DataReader
		mgr.createReader();

		DataReader dreader = mgr.getReader();
		StockDataReader OwnershipDataReader = StockDataReaderHelper
				.narrow(dreader);
		ErrorHandler.checkHandle(OwnershipDataReader,
				"StockDataReader.narrow");
		System.out.println("===[Subscriber] Ready ...");
		System.out
				.println("   Ticker   Price   Publisher   ownership strength");
		boolean closed = false;
		int status = -1;
		int count = 0;
		while (!closed && count < 1500) {
			status = OwnershipDataReader.take(msgList, infoSeq,
					LENGTH_UNLIMITED.value, ANY_SAMPLE_STATE.value,
					ANY_VIEW_STATE.value, ANY_INSTANCE_STATE.value);
			ErrorHandler.checkStatus(status, "OwnershipDataDataReader::take");
			if (msgList.value.length > 0) {
				for (int i = 0; i < msgList.value.length; i++) {
					if (infoSeq.value[i].valid_data) {
						if (msgList.value[i].price < -0.0f) {
							closed = true;
							break;
						}
						System.out.printf("   %s %8.1f    %s        %d\n",
								msgList.value[i].ticker,
								msgList.value[i].price,
								msgList.value[i].publisher,
								msgList.value[i].strength);
					}
				}

				status = OwnershipDataReader.return_loan(msgList, infoSeq);
				ErrorHandler
						.checkStatus(status, "StockDataReader::return_loan");
				try {
					Thread.currentThread().sleep(2);// sleep for 2 ms
				} catch (InterruptedException ie) {
				}
			}
			try {
				Thread.currentThread().sleep(200);// sleep for 200 ms
			} catch (InterruptedException ie) {
			}
			++count;
		}

		System.out.println("===[Subscriber] Market Closed");

		// cleanup

		mgr.getSubscriber().delete_datareader(OwnershipDataReader);
		mgr.deleteSubscriber();
		mgr.deleteTopic();
		mgr.deleteParticipant();

	}
}