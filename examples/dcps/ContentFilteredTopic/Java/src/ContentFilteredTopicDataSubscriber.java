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

import DDS.ANY_INSTANCE_STATE;
import DDS.ANY_SAMPLE_STATE;
import DDS.ANY_VIEW_STATE;
import DDS.DataReader;
import DDS.LENGTH_UNLIMITED;
import DDS.SampleInfoSeqHolder;
import ContentFilteredTopicData.StockDataReader;
import ContentFilteredTopicData.StockDataReaderHelper;
import ContentFilteredTopicData.StockSeqHolder;
import ContentFilteredTopicData.StockTypeSupport;

public class ContentFilteredTopicDataSubscriber {

	public static void main(String[] args) {
		String stkToSubscribe = null;
		if (args.length > 0) {
			stkToSubscribe = args[0];
		}

		DDSEntityManager mgr = new DDSEntityManager();
		String partitionName = "ContentFilteredTopic example";

		// create Domain Participant
		mgr.createParticipant(partitionName);

		// create Type
		StockTypeSupport msgTS = new StockTypeSupport();
		mgr.registerType(msgTS);

		// create Topic
		mgr.createTopic("StockTrackerExclusive");

		// create Subscriber
		mgr.createSubscriber();
		String expr[] = new String[0];
		if (stkToSubscribe == null)
			// Subscribe to all stocks
			mgr.createReader(false);
		else {
			// create Content Filtered Topic
			String sqlExpr = "ticker = '" + stkToSubscribe + "'";
			mgr.createContentFilteredTopic("MyStockTopic", sqlExpr, expr);

			// create Filtered DataReader
			mgr.createReader(true);
		}

		// Read Events

		DataReader dreader = mgr.getReader();
		StockDataReader stockReader = StockDataReaderHelper.narrow(dreader);

		StockSeqHolder msgSeq = new StockSeqHolder();
		SampleInfoSeqHolder infoSeq = new SampleInfoSeqHolder();
		boolean terminate = false;
		int count = 0;
		System.out.println("Ready");
		while (!terminate && count < 1500) { // We dont want the example to run indefinitely
			stockReader.take(msgSeq, infoSeq, LENGTH_UNLIMITED.value,
					ANY_SAMPLE_STATE.value, ANY_VIEW_STATE.value,
					ANY_INSTANCE_STATE.value);
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
		mgr.deleteFilteredTopic();
		mgr.deleteTopic();
		mgr.deleteParticipant();

	}
}
