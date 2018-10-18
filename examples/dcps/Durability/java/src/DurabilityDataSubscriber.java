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
		boolean isPersistent = (args[0].equals("persistent"));
		DDSEntityManager mgr = new DDSEntityManager(durability_kind);

		// create domain participant
		String partition_name = "Durability example";
		mgr.createParticipant(partition_name);

		// create type
		MsgTypeSupport mt = new MsgTypeSupport();
		mgr.registerType(mt);

		// create Topic
		String topic_name = "JavaDurabilityData_Msg";
		if (isPersistent) {
		    topic_name = "PersistentJavaDurabilityData_Msg";
		}
		mgr.createTopic(topic_name);

		// create Subscriber
		mgr.createSubscriber();

		// create DataReader
		mgr.createReader();

		DataReader dreader = mgr.getReader();
		MsgDataReader DurabilityDataReader = MsgDataReaderHelper
				.narrow(dreader);
		ErrorHandler.checkHandle(DurabilityDataReader, "MsgDataReader.narrow");

        DDS.Duration_t waitTimeout = new DDS.Duration_t(40, 0);
        int status = DurabilityDataReader.wait_for_historical_data(waitTimeout);
        ErrorHandler.checkStatus(status, "DurabilityDataReader.waitForHistoricalData");

		System.out.println("=== [Subscriber] Ready ...");

		boolean closed = false;
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
