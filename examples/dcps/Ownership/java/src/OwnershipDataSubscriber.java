/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
		String topic_name = "OwnershipStockTracker";
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