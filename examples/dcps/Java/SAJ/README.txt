The ping-pong example consists of two executables that will exchange data using
2 partitions.
Running both executables allows to measure roundtrip duration when sending and
receiving back a single message.

What the example does: 
   It send a message on the "PING" partition, which the PONG test is waiting for.
   The PONG test will send the same message back on the "PONG" partition, which 
   the PING test is waiting for. This sequence is repeated a configurable number 
   of times.
   The PING tests measures:
                write_access-time: time the write() method took.
                read_access-time:  time the take() method took.
                round_trip-time:   time between the call to the write() method 
                                   and the return of the take() method.
   PING calculates min/max/average statistics on these values over configurable 
   data blocks.

Configurable:
   - blocks:     number of roundtrips in each statistics calculation
   - nof_cycles: how many times such a statistics calculation is run
   - topic_id:   for the topic, there's a choice between several preconfigured
                 topics.
                 topic_id allows selection of topic used for the test, among those
                  defined by pragma keylist in pinpong.idl, and may be one of : 
                 'm' (PP_min_msg),
                 'q' (PP_seq_msg), 
                 's' (PP_string_msg), 
                 'f' (PP_fixed_msg), 
                 'a' (PP_array_msg), 
                 't' (PP_quit_msg)
                
   - PING and PONG partition: this enables to use several PING-PONG pairs
     simultanious with them interfering with each other. It also enables 
     creating larger loops, by chaining several PONG tests to one PING test.
     
The command line to start ping executable is :
        ping [blocks nof_cycles topic_id WRITE_PARTITION READ_PARTITION]
        >> starting ping with no options is the same as :
        ping    20       100        's'    PING              PONG

The command line to start pong executable is :        
        pong [READ_PARTITION WRITE_PARTITION]
        >> starting pong with no options is the same as :
        pong    PING              PONG
        
        
To build the example, simply use the provided Makefile (tested for GNU make).
The RUN script provides a model of how the ping-pong example can be run
(including OpenSplice daemon start/stop)
