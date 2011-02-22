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
 * LOGICAL_NAME:    QueryConditionSubscriber.java
 * FUNCTION:        OpenSplice example code.
 * MODULE:          Tutorial for the Java programming language.
 * DATE             April 2009.
 ************************************************************************
 * 
 * This file contains the implementation for the 'QueryConditionSubscriber' executable.
 * 
 ***/

import DDS.ANY_INSTANCE_STATE;
import DDS.ANY_SAMPLE_STATE;
import DDS.ANY_VIEW_STATE;
import DDS.DataReader;
import DDS.LENGTH_UNLIMITED;
import DDS.QueryCondition;
import DDS.ReadCondition;
import DDS.SampleInfoSeqHolder;
import QueryConditionData.StockDataReader;
import QueryConditionData.StockDataReaderHelper;
import QueryConditionData.StockSeqHolder;
import QueryConditionData.StockTypeSupport;

public class QueryConditionDataSubscriber {

	public static void main(String[] args) {
		String stkToSubscribe = null;
		if (args.length > 0) {
			stkToSubscribe = args[0];
		} else {
			System.out.println("Invalid Arguments \n");
			System.out.println("Expected argument MSFT or GE");
			System.exit(-1);
		}

		DDSEntityManager mgr = new DDSEntityManager();
		String partitionName = "QueryCondition example";

		// create Domain Participant
		mgr.createParticipant(partitionName);

		// create Type
		StockTypeSupport msgTS = new StockTypeSupport();
		mgr.registerType(msgTS);

		// create Topic
		mgr.createTopic("StockTrackerExclusive");

		// create Subscriber
		mgr.createSubscriber();
		mgr.createReader(false);

		// Read Events
		String[] params = new String[1];
		params[0] = new String(stkToSubscribe);
		DataReader dreader = mgr.getReader();
		StockDataReader stockReader = StockDataReaderHelper.narrow(dreader);
		QueryCondition qc = stockReader.create_querycondition(
				ANY_SAMPLE_STATE.value, ANY_VIEW_STATE.value,
				ANY_INSTANCE_STATE.value, "ticker=%0", params);
		ErrorHandler.checkHandle(qc, "create_querycondition");

		StockSeqHolder msgSeq = new StockSeqHolder();
		SampleInfoSeqHolder infoSeq = new SampleInfoSeqHolder();
		boolean terminate = false;
                int count = 0;
		System.out.println("Ready");
		while (!terminate && count < 1500) { // We dont want the example to run indefinitely
			stockReader.take_w_condition(msgSeq, infoSeq,
					LENGTH_UNLIMITED.value, qc);

			for (int i = 0; i < msgSeq.value.length; i++) {
				if (msgSeq.value[i].price == -1.0f) {
					terminate = true;
					break;
				}
				System.out.println(msgSeq.value[i].ticker + ": "
						+ msgSeq.value[i].price);

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
			
		}
                stockReader.return_loan(msgSeq, infoSeq);
		System.out.println("Market Closed");
		// clean up
		mgr.getSubscriber().delete_datareader(stockReader);
		mgr.deleteSubscriber();
		mgr.deleteParticipant();

	}
}
