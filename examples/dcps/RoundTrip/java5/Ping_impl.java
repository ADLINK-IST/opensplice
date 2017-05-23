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

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;
import org.omg.dds.core.Duration;
import org.omg.dds.core.InstanceHandle;
import org.omg.dds.core.status.PublicationMatchedStatus;
import org.omg.dds.sub.Sample;

/**
 * The RoundTrip example consists of a Ping and a Pong application. Ping sends
 * sample to Pong by writing to the Ping partition which Pong subscribes to.
 * Pong them sends them back to Ping by writing on the Pong partition which Ping
 * subscribes to. Ping measure the amount of time taken to write and read each
 * sample as well as the total round trip time to send a sample to Pong and
 * receive it back.
 * 
 * This class performs the Ping role in this example.
 */
class Ping_impl {

    RoundTripModule.DataType data;

    /**
     * Performs the Ping role in this example.
     */
    public void run(String args[]) {
        try {

            /*
             * Initialize entities
             */
            final Entities e = new Entities("ping", "pong");

            Runtime.getRuntime().addShutdownHook(new Thread() {
                @Override
                public void run() {
                    e.terminated.setTriggerValue(true);
                }
            });

            int payloadSize = 0;
            long numSamples = 0;
            long timeOut = 0;

            /*
             * Interpret arguments
             */
            Exception usage = new Exception(
                    "Usage (parameters must be supplied in order):\n"
                            + "./ping [payloadSize (bytes, 0 - 655536)] [numSamples (0 = infinite)] [timeOut (seconds, 0 = infinite)]\n"
                            + "./ping quit - ping sends a quit signal to pong.\n"
                            + "Defaults:\n" + "./ping 0 0 0");
            if (args.length == 1) {
                if (args[0].equals("quit")) {
                    System.out.println("Waiting for pong to run...");

                    PublicationMatchedStatus status = e.writer
                            .getPublicationMatchedStatus();
                    while (status.getCurrentCount() == 0) {
                        status = e.writer.getPublicationMatchedStatus();
                    }
                    System.out.println("Sending termination request");
                    /*
                     * Send quit signal to pong if "quit" is supplied as an
                     * argument to ping
                     */
                    data = new RoundTripModule.DataType();
                    e.writer.dispose(InstanceHandle.nilHandle(e.env), data);
                    Thread.sleep(1000);
                    e.participant.close();
                    return;
                } else if (args[0].equals("-h") || args[0].equals("--help")) {
                    throw usage;
                }
            }
            if (args.length >= 1) {
                payloadSize = Integer.parseInt(args[0]);

                if (payloadSize > 655536) {
                    throw usage;
                }
            }
            if (args.length >= 2) {
                numSamples = Integer.parseInt(args[1]);
            }
            if (args.length >= 3) {
                timeOut = Integer.parseInt(args[2]);
            }

            System.out.println("# payloadSize: " + payloadSize
                    + " | numSamples: " + numSamples + " | timeOut: " + timeOut
                    + "\n");

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

            Duration waitTimeout = e.env.getSPI().newDuration(1,
                    TimeUnit.SECONDS);
            data = new RoundTripModule.DataType(new byte[payloadSize]);
            for (int i = 0; i < payloadSize; i++) {
                data.payload[i] = 'a';
            }

            List<Sample<RoundTripModule.DataType>> samples = new ArrayList<Sample<RoundTripModule.DataType>>(
                    10);

            startTime = ExampleUtilities.GetTime();
            System.out.println("# Warming up...");
            boolean warmUp = true;
            while (!e.terminated.getTriggerValue()
                    && (ExampleUtilities.GetTime() - startTime)
                            / ExampleUtilities.US_IN_ONE_SEC < 5) {
                try {
                    e.writer.write(data);
                    e.waitSet.waitForConditions(waitTimeout);
                    e.reader.take(samples);
                } catch (TimeoutException te) {
                    System.out.println(">>>> TIMED-OUT (" + waitTimeout
                            + "s) on take !!!");
                }
            }

            if (!e.terminated.getTriggerValue()) {
                warmUp = false;
                System.out.println("# Warm up complete.\n");
                System.out.println("# Round trip measurements (in us)");
                System.out
                        .println("#             Round trip time [us]         Write-access time [us]       Read-access time [us]");
                System.out
                        .println("# Seconds     Count   median      min      Count   median      min      Count   median      min");
            }

            startTime = ExampleUtilities.GetTime();
            long elapsed = 0;

            for (long i = 0; !e.terminated.getTriggerValue()
                    && (numSamples == 0 || i < numSamples); i++) {
                /*
                 * Write a sample that pong can send back
                 */
                preWriteTime = ExampleUtilities.GetTime();
                e.writer.write(data);
                postWriteTime = ExampleUtilities.GetTime();

                /*
                 * Wait for response from pong
                 */
                try {
                    e.waitSet.waitForConditions(waitTimeout);

                    /*
                     * Take sample and check that it is valid
                     */

                    preTakeTime = ExampleUtilities.GetTime();
                    e.reader.take(samples);
                    postTakeTime = ExampleUtilities.GetTime();

                    if (!e.terminated.getTriggerValue()) {
                        if (samples.size() != 1) {

                            Exception exception = new Exception(
                                    "ERROR: Ping received "
                                            + samples.size()
                                            + " samples but "
                                            + "was expecting 1. Are multiple pong applications running?");
                            throw exception;
                        } else if (samples.get(0).getData() == null) {
                            Exception exception = new Exception(
                                    "ERROR: Ping received an invalid sample. Has pong terminated already?");
                            throw exception;
                        }
                    }
                    /*
                     * Update stats
                     */
                    writeAccess.AddMicroseconds(postWriteTime - preWriteTime);
                    readAccess.AddMicroseconds(postTakeTime - preTakeTime);
                    roundTrip.AddMicroseconds(postTakeTime - preWriteTime);
                    writeAccessOverall.AddMicroseconds(postWriteTime
                            - preWriteTime);
                    readAccessOverall.AddMicroseconds(postTakeTime
                            - preTakeTime);
                    roundTripOverall.AddMicroseconds(postTakeTime
                            - preWriteTime);

                    /*
                     * Print stats each second
                     */
                    if ((postTakeTime - startTime) > ExampleUtilities.US_IN_ONE_SEC
                            || (i >= 1 && i == numSamples)) {
                        System.out
                                .printf("%9d %9d %8.0f %8d %10d %8.0f %8d %10d %8.0f %8d\n",
                                        elapsed + 1, roundTrip.count,
                                        roundTrip.GetMedian(), roundTrip.min,
                                        writeAccess.count,
                                        writeAccess.GetMedian(),
                                        writeAccess.min, readAccess.count,
                                        readAccess.GetMedian(), readAccess.min);

                        /*
                         * Reset stats for next run
                         */
                        roundTrip.Reset();
                        writeAccess.Reset();
                        readAccess.Reset();

                        /*
                         * Set values for next run
                         */
                        startTime = ExampleUtilities.GetTime();
                        elapsed++;
                    }
                } catch (TimeoutException te) {
                    System.out.println(">>>> TIMED-OUT (" + waitTimeout
                            + "s) on take !!!");
                    elapsed += waitTimeout.getDuration(TimeUnit.SECONDS);
                }

                if (timeOut >= 1 && elapsed == timeOut) {
                    e.terminated.setTriggerValue(true);
                }
            }

            if (!warmUp) {
                /*
                 * Print overall stats
                 */
                System.out.printf(
                        "\n%9s %9d %8.0f %8d %10d %8.0f %8d %10d %8.0f %8d\n",
                        "# Overall", roundTripOverall.count,
                        roundTripOverall.GetMedian(), roundTripOverall.min,
                        writeAccessOverall.count,
                        writeAccessOverall.GetMedian(), writeAccessOverall.min,
                        readAccessOverall.count, readAccessOverall.GetMedian(),
                        readAccessOverall.min);
            }
            e.participant.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
