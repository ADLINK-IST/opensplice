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

import java.util.Random;

import DDS.DataWriter;
import ContentFilteredTopicData.Stock;
import ContentFilteredTopicData.StockDataWriter;
import ContentFilteredTopicData.StockDataWriterHelper;
import ContentFilteredTopicData.StockTypeSupport;

public class ContentFilteredTopicDataPublisher {

	public static void main(String[] args) {
		DDSEntityManager mgr = new DDSEntityManager();
		String partitionName = "ContentFilteredTopic example";

		// create Domain Participant
		mgr.createParticipant(partitionName);

		// create Type
		StockTypeSupport stkTS = new StockTypeSupport();
		mgr.registerType(stkTS);

		// create Topic
		mgr.createTopic("StockTrackerExclusive");

		// create Publisher
		mgr.createPublisher();

		// create DataWriter
		mgr.createWriter();

		// Publish Events

		DataWriter dwriter = mgr.getWriter();
		StockDataWriter stockWriter = StockDataWriterHelper.narrow(dwriter);

		Stock msft = new Stock();
		msft.ticker = "MSFT";
		msft.price = 19.95f;

		Stock ge = new Stock();
		ge.ticker = "GE";
		ge.price = 10.00f;

		// register the two stock instances
		long msftHandle = stockWriter.register_instance(msft);
		long geHandle = stockWriter.register_instance(ge);

		// update data every second
		for (int x = 0; x < 20; x++) {
			msft.price = msft.price + (new Random()).nextFloat();
			stockWriter.write(msft, msftHandle);

			ge.price = ge.price + (new Random()).nextFloat();
			stockWriter.write(ge, geHandle);

			try {
				Thread.sleep(1000);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}

		msft.price = -1.0f; // signal to terminate
		ge.price = -1.0f; // signal to terminate

		stockWriter.write(msft, msftHandle);
		stockWriter.write(ge, geHandle);
		try {
			Thread.sleep(1000);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}

		// clean up
		stockWriter.dispose(msft, msftHandle);
		stockWriter.dispose(ge, geHandle);
		stockWriter.unregister_instance(msft, msftHandle);
		stockWriter.unregister_instance(ge, geHandle);

		mgr.getPublisher().delete_datawriter(stockWriter);
		mgr.deletePublisher();
		mgr.deleteTopic();
		mgr.deleteParticipant();

	}
}
