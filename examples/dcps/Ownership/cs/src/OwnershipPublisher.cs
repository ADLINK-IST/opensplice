using System;
using System.Threading;

using DDS;
using DDS.OpenSplice;

using OwnershipData;
using DDSAPIHelper;
using System.IO;

namespace OwnershipPublisher
{
    public class OwnershipPublisher
    {
        DDSEntityManager mgr;
        Stock msft;
        InstanceHandle msftHandle;
        StockDataWriter OwnershipDataWriter;
        IDataWriter dwriter;

        public OwnershipPublisher(String pub, int strength)
	    {
		    initPublisher(pub,strength);
	    }
    	
	    private void initPublisher(String pub, int strength)
	    {
		    mgr = new DDSEntityManager("Ownership");
		    String partitionName = "Ownership example";

		    // create Domain Participant
		    mgr.createParticipant(partitionName);
    		
		    // create Type
		    StockTypeSupport stkTS = new StockTypeSupport();
		    mgr.registerType(stkTS);
    		
		    // create Topic
		    mgr.createTopic("OwnershipStockTracker");
    		
		    // create Publisher
		    mgr.createPublisher();
    		
		    // create DataWriter
		    mgr.createWriterWithStrength(strength);
		    dwriter= mgr.getWriter();
            OwnershipDataWriter = dwriter as StockDataWriter;
    		
		    msft = new Stock();
		    msft.ticker = "MSFT";
            msft.publisher = pub;
            msft.strength = strength;
            msftHandle = OwnershipDataWriter.RegisterInstance(msft);
	    }

        public void publishEvent(float price, String pub)
        {            
            msft.publisher = pub;
            msft.price = price;
            ReturnCode status = OwnershipDataWriter.Write(msft, msftHandle);
            ErrorHandler.checkStatus(status, "DataWriter.Write");
        }

        public void dispose()
        {
            // clean up
            OwnershipDataWriter.Dispose(msft, msftHandle);
            OwnershipDataWriter.UnregisterInstance(msft, msftHandle);

            ReturnCode status = mgr.getPublisher().DeleteDataWriter(OwnershipDataWriter);
            ErrorHandler.checkStatus(status, "Publisher.DeleteDataWriter");
            mgr.deletePublisher();
            mgr.deleteTopic();
            mgr.deleteParticipant();
        }

    }
}
