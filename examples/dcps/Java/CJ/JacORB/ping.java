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

/*
 * What it does: 
 *   It send a message on the "PING" partition, which the PONG test is waiting for.
 *   The PONG test will send the same message back on the "PONG" partition, which 
 *   the PING test is waiting for. This sequence is repeated a configurable number 
 *   of times.
 *   The PING tests measures:
 *                write_access-time: time the write() method took.
 *                read_access-time:  time the take() method took.
 *                round_trip-time:   time between the call to the write() method 
 *                                   and the return of the take() method.
 *   PING calculates min/max/average statistics on these values over configurable 
 *   data blocks.
 *
 * Configurable:
 *   - blocksize: number of roundtrips in each statistics calculation
 *   - #blocks:   how many times such a statistics calculation is run
 *   - topic:     for the topic, there's a choice between several preconfigured
 *                topics.
 *   - PING and PONG partition: this enables to use several PING-PONG pairs
 *     simultanious with them interfering with each other. It also enables 
 *     creating larger loops, by chaining several PONG tests to one PING test.
 */

class ping {

    public static void main (String args[]) {

	pinger pinger_instance = new pinger();

        pinger_instance.run (args);

    }

}

