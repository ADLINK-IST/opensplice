#include "dds.h"
#include "HelloWorldData.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* An array of one message (aka sample in dds terms) will be used. */
#define MAX_SAMPLES 1

int main (int argc, char ** argv)
{
    dds_entity_t topic;
    dds_entity_t reader;
    HelloWorldData_Msg *msg;
    void *samples[MAX_SAMPLES];
    dds_sample_info_t infos[MAX_SAMPLES];
    int status;
    dds_qos_t *qos;

    /* Create a Topic. */
    /* Passing NULL as first argument will result in an implicitly created participant */
    status = dds_topic_create (NULL, &topic, &HelloWorldData_Msg_desc, "HelloWorldData_Msg", NULL, NULL);
    DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    /* Create a reliable Reader. */
    qos = dds_qos_create ();
    dds_qset_reliability (qos, DDS_RELIABILITY_RELIABLE, DDS_SECS (10));
    /* Passing NULL as first argument will result in an implicitly created subscriber */
    status = dds_reader_create (NULL, &reader, topic, qos, NULL);
    DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
    dds_qos_delete(qos);

    printf ("\n=== [Subscriber] Waiting for a sample ...\n");

    /* Initialize sample buffer, by pointing the void pointer within
       the buffer array to a valid sample memory location. */
    samples[0] = HelloWorldData_Msg__alloc ();

    /* Poll until data has been read. */
    while (true)
    {
        /* Do the actual read.
           The return value contains the number of read samples. */
        status = dds_read (reader, samples, MAX_SAMPLES, infos, 0);
        DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

        /* Check if we read some data and it is valid. */
        if ((status > 0) && (infos[0].valid_data))
        {
            /* Print Message. */
            msg = (HelloWorldData_Msg*) samples[0];
            printf ("=== [Subscriber] Received : ");
            printf ("Message (%d, %s)\n", msg->userID, msg->message);
            break;
        }
        else
        {
            /* Polling sleep. */
            dds_sleepfor (DDS_MSECS (20));
        }
    }

    /* Free the data location. */
    HelloWorldData_Msg_free (samples[0], DDS_FREE_ALL);

    dds_entity_delete (topic);
    dds_entity_delete (reader);

    return EXIT_SUCCESS;
}
