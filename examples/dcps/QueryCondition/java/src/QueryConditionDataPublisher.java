/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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

import java.util.Random;

import DDS.DataWriter;
import StockMarket.Stock;
import StockMarket.StockDataWriter;
import StockMarket.StockDataWriterHelper;
import StockMarket.StockTypeSupport;

public class QueryConditionDataPublisher {

	public static void main(String[] args) {
		DDSEntityManager mgr = new DDSEntityManager();
		String partitionName = "QueryCondition example";

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
		        System.out.format("GE : %.1f MSFT : %.1f\n", ge.price, msft.price);
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
	        System.out.println("Market Closed.");

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
