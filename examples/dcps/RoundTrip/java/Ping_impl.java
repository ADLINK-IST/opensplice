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

/**
 * The RoundTrip example consists of a Ping and a Pong application. Ping sends sample
 * to Pong by writing to the Ping partition which Pong subscribes to. Pong them sends them
 * back to Ping by writing on the Pong partition which Ping subscribes to. Ping measure
 * the amount of time taken to write and read each sample as well as the total round trip
 * time to send a sample to Pong and receive it back.
 *
 * This class performs the Ping role in this example.
 */
class Ping_impl
{
    /**
     * Performs the Ping role in this example.
     */
    public void run (String args[])
    {
        try
        {
            int status;

            /** Initialise entities */
            final Entities e = new Entities("ping", "pong");

            Runtime.getRuntime().addShutdownHook(new Thread()
            {
                public void run()
                {
                    e.terminated.set_trigger_value(true);
                }
            });

            int payloadSize = 0;
            long numSamples = 0;
            long timeOut = 0;

            /** Interpret arguments */
            Exception usage = new Exception(
                "Usage (parameters must be supplied in order):\n" +
                "./ping [payloadSize (bytes, 0 - 65536)] [numSamples (0 = infinite)] [timeOut (seconds, 0 = infinite)]\n" +
                "./ping quit - ping sends a quit signal to pong.\n" +
                "Defaults:\n" +
                "./ping 0 0 0");
            if(args.length == 1)
            {
                if(args[0].equals("quit"))
                {
                    /** Wait for pong to run */
                    System.out.println("Waiting for pong to run...");
                    DDS.PublicationMatchedStatusHolder pms = new DDS.PublicationMatchedStatusHolder();
                    status = e.writer.get_publication_matched_status(pms);
                    ExampleError.CheckStatus(status, "ping, e.writer.get_publication_matched_status");
                    while(pms.value.current_count == 0)
                    {
                        status = e.writer.get_publication_matched_status(pms);
                        ExampleError.CheckStatus(status, "ping, e.writer.get_publication_matched_status");
                        Thread.sleep(500);
                    }
                    System.out.println("Sending termination request");
                    /** Dispose an instance to signify that pong should quit if "quit" is supplied as an argument to ping */
                    RoundTripModule.DataType data = new RoundTripModule.DataType();

                    status = e.writer.dispose(data, DDS.HANDLE_NIL.value);
                    ExampleError.CheckStatus(status, "ping, e.writer.dispose");
                    Thread.sleep(1000);
                    return;
                }
                else if(args[0].equals("-h") || args[0].equals("--help"))
                {
                    throw usage;
                }
            }
            if(args.length >= 1)
            {
                payloadSize = Integer.parseInt(args[0]);

                if(payloadSize > 65536)
                {
                    throw usage;
                }
            }
            if(args.length >= 2)
            {
                numSamples = Integer.parseInt(args[1]);
            }
            if(args.length >= 3)
            {
                timeOut = Integer.parseInt(args[2]);
            }

            System.out.println("# payloadSize: " + payloadSize + " | numSamples: " + numSamples +
                " | timeOut: " + timeOut + "\n");

            long startTime;
            long preWriteTime;
            long postWriteTime;
            long preTakeTime;
            long postTakeTime;
            ExampleUtilities.TimeStats writeAccess = new ExampleUtilities.TimeStats();
            ExampleUtilities.TimeStats readAccess = new ExampleUtilities.TimeStats();
            ExampleUtilities.TimeStats roundTrip = new ExampleUtilities.TimeStats();
            ExampleUtilities.TimeStats writeAccessOverall = new ExampleUtilities.TimeStats();
            ExampleUtilities.TimeStats readAccessOverall = new ExampleUtilities.TimeStats();
            ExampleUtilities.TimeStats roundTripOverall = new ExampleUtilities.TimeStats();
            DDS.Duration_t waitTimeout = new DDS.Duration_t(1, 0);
            RoundTripModule.DataType data = new RoundTripModule.DataType(new byte[payloadSize]);
            for(int i = 0; i < payloadSize; i++)
            {
                data.payload[i] = (byte)1;
            }
            DDS.ConditionSeqHolder conditions = new DDS.ConditionSeqHolder();
            RoundTripModule.DataTypeSeqHolder samples = new RoundTripModule.DataTypeSeqHolder();
            DDS.SampleInfoSeqHolder info = new DDS.SampleInfoSeqHolder();

            startTime = ExampleUtilities.GetTime();
            System.out.println("# Warming up to stabilise performance...");
            boolean warmUp = true;
            while(!e.terminated.get_trigger_value() && (ExampleUtilities.GetTime() -
                startTime) / ExampleUtilities.US_IN_ONE_SEC < 5)
            {
                status = e.writer.write(data, DDS.HANDLE_NIL.value);
                ExampleError.CheckStatus(status, "ping, e.writer.write");
                status = e.waitSet._wait(conditions, waitTimeout);
                if(status != DDS.RETCODE_TIMEOUT.value)
                {
                    ExampleError.CheckStatus(status, "ping, e.waitSet.wait");
                    status = e.reader.take(samples, info, DDS.LENGTH_UNLIMITED.value,
                        DDS.ANY_SAMPLE_STATE.value, DDS.ANY_VIEW_STATE.value, DDS.ANY_INSTANCE_STATE.value);
                    ExampleError.CheckStatus(status, "ping, e.reader.take");
                    status = e.reader.return_loan(samples, info);
                    ExampleError.CheckStatus(status, "ping, e.reader.return_loan");
                }
            }
            if(!e.terminated.get_trigger_value())
            {
                warmUp = false;
                System.out.println("# Warm up complete.\n");

                System.out.println("# Round trip measurements (in us)");
                System.out.println("#             Round trip time [us]         Write-access time [us]       Read-access time [us]");
                System.out.println("# Seconds     Count   median      min      Count   median      min      Count   median      min");
            }

            startTime = ExampleUtilities.GetTime();
            long elapsed = 0;
            for(long i = 0; !e.terminated.get_trigger_value() && (numSamples == 0 || i < numSamples); i++)
            {
                /** Write a sample that pong can send back */
                preWriteTime = ExampleUtilities.GetTime();
                status = e.writer.write(data, DDS.HANDLE_NIL.value);
                postWriteTime = ExampleUtilities.GetTime();
                ExampleError.CheckStatus(status, "ping, e.writer.write");

                /** Wait for response from pong */
                status = e.waitSet._wait(conditions, waitTimeout);
                if(status != DDS.RETCODE_TIMEOUT.value)
                {
                    ExampleError.CheckStatus(status, "ping, e.waitSet._wait");

                    /** Take sample and check that it is valid */
                    preTakeTime = ExampleUtilities.GetTime();
                    status = e.reader.take(samples, info, DDS.LENGTH_UNLIMITED.value,
                        DDS.ANY_SAMPLE_STATE.value, DDS.ANY_VIEW_STATE.value, DDS.ANY_INSTANCE_STATE.value);
                    postTakeTime = ExampleUtilities.GetTime();
                    ExampleError.CheckStatus(status, "ping, e.reader.take");

                    if(!e.terminated.get_trigger_value())
                    {
                        if(samples.value.length != 1)
                        {
                            Exception exception = new Exception(
                                "ERROR: Ping received " + samples.value.length + " samples but " +
                                "was expecting 1. Are multiple pong applications running?");
                            throw exception;
                        }
                        else if(!info.value[0].valid_data)
                        {
                            Exception exception = new Exception(
                                "ERROR: Ping received an invalid sample. Has pong terminated already?");
                            throw exception;
                        }
                    }
                    status = e.reader.return_loan(samples, info);
                    ExampleError.CheckStatus(status, "ping, e.reader.return_loan");

                    /** Update stats */
                    writeAccess.AddMicroseconds(postWriteTime - preWriteTime);
                    readAccess.AddMicroseconds(postTakeTime - preTakeTime);
                    roundTrip.AddMicroseconds(postTakeTime - preWriteTime);
                    writeAccessOverall.AddMicroseconds(postWriteTime - preWriteTime);
                    readAccessOverall.AddMicroseconds(postTakeTime - preTakeTime);
                    roundTripOverall.AddMicroseconds(postTakeTime - preWriteTime);

                    /** Print stats each second */
                    if((postTakeTime - startTime) > ExampleUtilities.US_IN_ONE_SEC || (i >= 1 && i == numSamples))
                    {
                        System.out.printf("%9d %9d %8.0f %8d %10d %8.0f %8d %10d %8.0f %8d\n",
                                elapsed + 1,
                                roundTrip.count,
                                roundTrip.GetMedian(),
                                roundTrip.min,
                                writeAccess.count,
                                writeAccess.GetMedian(),
                                writeAccess.min,
                                readAccess.count,
                                readAccess.GetMedian(),
                                readAccess.min);

                        /** Reset stats for next run */
                        roundTrip.Reset();
                        writeAccess.Reset();
                        readAccess.Reset();

                        /** Set values for next run */
                        startTime = ExampleUtilities.GetTime();
                        elapsed++;
                    }
                }
                else
                {
                    elapsed += waitTimeout.sec;
                }
                if(timeOut >= 1 && elapsed == timeOut)
                {
                    status = e.terminated.set_trigger_value(true);
                    ExampleError.CheckStatus(status, "ping, e.terminated.set_trigger_value");
                }
            }

            if(!warmUp)
            {
                /** Print overall stats */
                System.out.printf("\n%9s %9d %8.0f %8d %10d %8.0f %8d %10d %8.0f %8d\n",
                        "# Overall",
                        roundTripOverall.count,
                        roundTripOverall.GetMedian(),
                        roundTripOverall.min,
                        writeAccessOverall.count,
                        writeAccessOverall.GetMedian(),
                        writeAccessOverall.min,
                        readAccessOverall.count,
                        readAccessOverall.GetMedian(),
                        readAccessOverall.min);
            }
        }
        catch(Exception e)
        {
            System.out.println(e);
        }
    }
}
