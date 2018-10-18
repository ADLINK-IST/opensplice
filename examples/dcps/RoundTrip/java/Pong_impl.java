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

/**
 * The RoundTrip example consists of a Ping and a Pong application. Ping sends sample
 * to Pong by writing to the Ping partition which Pong subscribes to. Pong them sends them
 * back to Ping by writing on the Pong partition which Ping subscribes to. Ping measure
 * the amount of time taken to write and read each sample as well as the total round trip
 * time to send a sample to Pong and receive it back.
 *
 * This class performs the Pong role in this example.
 */
class Pong_impl
{
    /**
     * Performs the Pong role in this example.
     */
    public void run (String args[])
    {
        try
        {
            int status;

            /** Initialise entities */
            final Entities e = new Entities("pong", "ping");

            Runtime.getRuntime().addShutdownHook(new Thread()
            {
                public void run()
                {
                    e.terminated.set_trigger_value(true);
                }
            });

            System.out.println("Waiting for samples from ping to send back...");

            DDS.Duration_t waitTimeout = new DDS.Duration_t(
                DDS.DURATION_INFINITE_SEC.value, DDS.DURATION_INFINITE_NSEC.value);
            DDS.ConditionSeqHolder conditions = new DDS.ConditionSeqHolder();
            RoundTripModule.DataTypeSeqHolder samples = new RoundTripModule.DataTypeSeqHolder();
            DDS.SampleInfoSeqHolder info = new DDS.SampleInfoSeqHolder();

            while(!e.terminated.get_trigger_value())
            {
                /** Wait for a sample from ping */
                status = e.waitSet._wait(conditions, waitTimeout);
                ExampleError.CheckStatus(status, "pong, e.waitSet.wait");

                /** Take samples */
                status = e.reader.take(samples, info, DDS.LENGTH_UNLIMITED.value,
                    DDS.ANY_SAMPLE_STATE.value, DDS.ANY_VIEW_STATE.value, DDS.ANY_INSTANCE_STATE.value);

                for(int i = 0; !e.terminated.get_trigger_value() && i < samples.value.length; i++)
                {
                    /** If writer has been disposed terminate pong */
                    if(info.value[i].instance_state == DDS.NOT_ALIVE_DISPOSED_INSTANCE_STATE.value)
                    {
                        System.out.println("Received termination request. Terminating.");
                        e.terminated.set_trigger_value(true);
                        break;
                    }
                    /** If sample is valid, send it back to ping */
                    else if(info.value[i].valid_data)
                    {
                        status = e.writer.write(samples.value[i], DDS.HANDLE_NIL.value);
                        ExampleError.CheckStatus(status, "pong, e.writer.write");
                    }
                }
                status = e.reader.return_loan(samples, info);
                ExampleError.CheckStatus(status, "pong, e.reader.return_loan");
            }
        }
        catch(Exception e)
        {
            System.out.println(e);
        }
    }
}
