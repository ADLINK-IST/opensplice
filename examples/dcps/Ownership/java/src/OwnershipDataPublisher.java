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
 * LOGICAL_NAME:    OwnershipDataPublisher.java
 * FUNCTION:        OpenSplice example code.
 * MODULE:          Tutorial for the Java programming language.
 * DATE             April 2009.
 ************************************************************************
 *
 * This file contains the implementation for the 'OwnershipDataPublisher' executable.
 *
 ***/

import DDS.*;
import OwnershipData.*;

public class OwnershipDataPublisher {
	DDSEntityManager mgr = new DDSEntityManager();
	DataWriter dWriter;
	StockDataWriter OwnershipDataDataWriter;
	long userHandle;
	Stock m_instance;
	int status;

	void initPublisher(String pub, int strength) {
		mgr = new DDSEntityManager();
		String partition_name = "Ownership example";
		mgr.createParticipant(partition_name);
		// create type
		StockTypeSupport st = new StockTypeSupport();
		mgr.registerType(st);
		// create Topic
		String topic_name = "OwnershipStockTracker";
		mgr.createTopic(topic_name);
		// create Publisher
		mgr.createPublisher();

		// create DataWriter
		mgr.createWriter(strength);

		dWriter = mgr.getWriter();
		OwnershipDataDataWriter = StockDataWriterHelper.narrow(dWriter);

		m_instance = new Stock();

		m_instance.ticker = "MSFT";
		m_instance.price = 0;
		m_instance.publisher = pub;
		m_instance.strength = strength;
		userHandle = OwnershipDataDataWriter.register_instance(m_instance);

	}

	OwnershipDataPublisher(String pub, int strength) {
		initPublisher(pub, strength);
	}

	void publishEvent(float price, String pub) {
		m_instance.price = price;
		m_instance.publisher = pub;
		OwnershipDataDataWriter.write(m_instance, userHandle);
	}

	void dispose() {

		mgr.getPublisher().delete_datawriter(OwnershipDataDataWriter);

		/* Remove the Publisher. */
		mgr.deletePublisher();

		/* Remove the Topics. */
		mgr.deleteTopic();

		/* Remove Participant. */
		mgr.deleteParticipant();

	}

	public static void main(String args[]) {

		System.out.println(args.length);
		if (args.length < 4) {
			System.err
					.println("*** [Publisher] usage : Publisher <publisher_name> <ownership_strength> <nb_iterations> <stop_subscriber_flag>");
			System.exit(-1);
		}

		OwnershipDataPublisher pub;
		String publisher_name = args[0];
		int ownership_strength = Integer.parseInt(args[1]);
		int nb_iteration = Integer.parseInt(args[2]);
		pub = new OwnershipDataPublisher(publisher_name, ownership_strength);
		boolean stop_subscriber = (Integer.parseInt(args[3]) == 1);

		// Publisher publishes the prices in dollars
		System.out.println("=== [Publisher] Publisher " + publisher_name
				+ " with strength : " + ownership_strength);
		System.out.println(" / sending " + nb_iteration + " prices ..."
				+ " stop_subscriber flag=" + args[3]);
		// The subscriber should display the prices sent by the publisher with
		// the highest ownership strength
		float price = 10.0f;
		for (int x = 0; x < nb_iteration; x++) {
			pub.publishEvent(price, publisher_name);
			// Sleep(delay_200ms);
			try {
				Thread.currentThread().sleep(200);// sleep for 200 ms
			} catch (InterruptedException ie) {
			}
			price = price + 0.5f;
		}
		// Sleep(delay_2s);
		try {
			Thread.currentThread().sleep(2000);// sleep for 2000 ms
		} catch (InterruptedException ie) {
		}
		if (stop_subscriber) {
			// send a price = -1 to stop subscriber
			price = -1.0f;
			System.out.println("=== Stopping the subscriber");
			pub.publishEvent(price, publisher_name);
		}
		pub.dispose();
	}
}
