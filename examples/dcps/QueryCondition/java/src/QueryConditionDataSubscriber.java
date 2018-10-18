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

import DDS.ANY_INSTANCE_STATE;
import DDS.ANY_SAMPLE_STATE;
import DDS.ANY_VIEW_STATE;
import DDS.DataReader;
import DDS.LENGTH_UNLIMITED;
import DDS.QueryCondition;
import DDS.ReadCondition;
import DDS.SampleInfoSeqHolder;
import StockMarket.StockDataReader;
import StockMarket.StockDataReaderHelper;
import StockMarket.StockSeqHolder;
import StockMarket.StockTypeSupport;

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
        stockReader.delete_readcondition(qc);
        mgr.getSubscriber().delete_datareader(stockReader);
        mgr.deleteSubscriber();
        mgr.deleteTopic();
        mgr.deleteParticipant();
    }
}
