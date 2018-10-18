#include "Throughput.h"
#ifdef _WIN32
#include <windows.h>
#endif
/*
 * The Throughput example measures data throughput in bytes per second. The publisher
 * allows you to specify a payload size in bytes as well as allowing you to specify
 * whether to send data in bursts. The publisher will continue to send data forever
 * unless a time out is specified. The subscriber will receive data and output the
 * total amount received and the data rate in bytes per second. It will also indicate
 * if any samples were received out of order. A maximum number of cycles can be
 * specified and once this has been reached the subscriber will terminate and output
 * totals and averages.
 */

#define BYTES_PER_SEC_TO_MEGABITS_PER_SEC 125000
#define MAX_SAMPLES 100

typedef struct HandleEntry
{
  dds_instance_handle_t handle;
  unsigned long long count;
  struct HandleEntry * next;
} HandleEntry;

typedef struct HandleMap
{
  HandleEntry *entries;
} HandleMap;

static unsigned long cycles = 0;
static unsigned long pollingDelay = 0;
static dds_instance_handle_t ph = 0;

static HandleMap * imap;
static HandleEntry * current = NULL;
static unsigned long long outOfOrder = 0;
static unsigned long long total_bytes = 0;
static unsigned long long total_samples = 0;

static dds_time_t startTime = 0;
static dds_time_t time_now = 0;
static dds_time_t prev_time = 0;

static unsigned long payloadSize = 0;
static double deltaTime = 0;

static ThroughputModule_DataType data [MAX_SAMPLES];
static void * samples[MAX_SAMPLES];

static dds_condition_t terminated;
static dds_condition_t gCond;

static bool done = false;
static bool first_batch = true;

/* Functions to handle Ctrl-C presses. */

#ifdef _WIN32
static int CtrlHandler (DWORD fdwCtrlType)
{
  dds_guard_trigger (terminated);
  done = true;
  return true; /* Don't let other handlers handle this key */
}
#else
struct sigaction oldAction;
static void CtrlHandler (int fdwCtrlType)
{
  dds_guard_trigger (terminated);
  done = true;
}
#endif
/*
 * This struct contains all of the entities used in the publisher and subscriber.
 */

static HandleMap * HandleMap__alloc (void)
{
  HandleMap * map = malloc (sizeof (*map));
  memset (map, 0, sizeof (*map));
  return map;
}

static void HandleMap__free (HandleMap *map)
{
  HandleEntry * entry;

  while (map->entries)
  {
    entry = map->entries;
    map->entries = entry->next;
    free (entry);
  }
  free (map);
}

static HandleEntry * store_handle (HandleMap *map, dds_instance_handle_t key)
{
  HandleEntry * entry = malloc (sizeof (*entry));
  memset (entry, 0, sizeof (*entry));

  entry->handle = key;
  entry->next = map->entries;
  map->entries = entry;

  return entry;
}

static HandleEntry * retrieve_handle (HandleMap *map, dds_instance_handle_t key)
{
  HandleEntry * entry = map->entries;

  while (entry)
  {
    if (entry->handle == key)
    {
      break;
    }
    entry = entry->next;
  }
  return entry;
}

static void data_available_handler (dds_entity_t reader)
{
  int samples_received;
  dds_sample_info_t info [MAX_SAMPLES];
  int i;

  if (startTime == 0)
  {
    startTime = dds_time ();
  }

  /* Take samples and iterate through them */

  samples_received = dds_take (reader, samples, MAX_SAMPLES, info, 0);
  DDS_ERR_CHECK (samples_received, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

  for (i = 0; !done && i < samples_received; i++)
  {
    if (info[i].valid_data)
    {
      ph = info[i].publication_handle;
      current = retrieve_handle (imap, ph);
      ThroughputModule_DataType * this_sample = &data[i];

      if (current == NULL)
      {
        current = store_handle (imap, ph);
        current->count = this_sample->count;
      }

      if (this_sample->count != current->count)
      {
        outOfOrder++;
      }
      current->count = this_sample->count + 1;

      /* Add the sample payload size to the total received */

      payloadSize = this_sample->payload._length;
      total_bytes += payloadSize + 8;
      total_samples++;
    }
  }

  time_now = dds_time ();
  if ((pollingDelay == 0) && (time_now > (prev_time + DDS_SECS (1))))
  {
     dds_guard_trigger (gCond);
  }
}

int main (int argc, char **argv)
{
  int status;
  unsigned long i;
  int result = EXIT_SUCCESS;
  unsigned long long maxCycles = 0;
  char *partitionName = "Throughput example";

  dds_entity_t participant;
  dds_entity_t topic;
  dds_entity_t subscriber;
  dds_entity_t reader;
  dds_waitset_t waitSet;
  dds_readerlistener_t rd_listener;

  unsigned long long prev_samples = 0;
  unsigned long long prev_bytes = 0;
  dds_time_t deltaTv;

  status = dds_init (argc, argv);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

  time_now = dds_time ();
  prev_time = time_now;

  /* Register handler for Ctrl-C */
#ifdef _WIN32
  SetConsoleCtrlHandler((PHANDLER_ROUTINE) CtrlHandler, true);
#else
  struct sigaction sat;
  sat.sa_handler = CtrlHandler;
  sigemptyset(&sat.sa_mask);
  sat.sa_flags = 0;
  sigaction (SIGINT, &sat, &oldAction);
#endif

  /*
   * Get the program parameters
   * Parameters: subscriber [maxCycles] [pollingDelay] [partitionName]
   */
  if (argc == 2 && (strcmp (argv[1], "-h") == 0 || strcmp (argv[1], "--help") == 0))
  {
    printf ("Usage (parameters must be supplied in order):\n");
    printf ("./subscriber [maxCycles (0 = infinite)] [pollingDelay (ms, 0 = event based)] [partitionName]\n");
    printf ("Defaults:\n");
    printf ("./subscriber 0 0 \"Throughput example\"\n");
    return result;
  }

  if (argc > 1)
  {
    maxCycles = atoi (argv[1]); /* The number of times to output statistics before terminating */
  }
  if (argc > 2)
  {
    pollingDelay = atoi (argv[2]); /* The number of ms to wait between reads (0 = event based) */
  }
  if (argc > 3)
  {
    partitionName = argv[3]; /* The name of the partition */
  }

  printf ("maxCycles: %llu | pollingDelay: %lu | partitionName: %s\n",
    maxCycles, pollingDelay, partitionName);

  /* Initialise entities */

  {
    const char *subParts[1];
    dds_qos_t *subQos = dds_qos_create ();
    dds_qos_t *drQos = dds_qos_create ();
    uint32_t maxSamples = 400;
    dds_attach_t wsresults[1];
    size_t wsresultsize = 1U;
    dds_duration_t infinite = DDS_INFINITY;
    int i=0;
    /* A Participant is created for the default domain. */

    status = dds_participant_create (&participant, DDS_DOMAIN_DEFAULT, NULL, NULL);
    DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    /* A Topic is created for our sample type on the domain participant. */

    status = dds_topic_create (participant, &topic, &ThroughputModule_DataType_desc, "Throughput", NULL, NULL);
    DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    /* A Subscriber is created on the domain participant. */

    subParts[0] = partitionName;
    dds_qset_partition (subQos, 1, subParts);
    status = dds_subscriber_create (participant, &subscriber, subQos, NULL);
    DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
    dds_qos_delete (subQos);

    /* A Reader is created on the Subscriber & Topic with a modified Qos. */

    dds_qset_reliability (drQos, DDS_RELIABILITY_RELIABLE, DDS_SECS (10));
    dds_qset_history (drQos, DDS_HISTORY_KEEP_ALL, 0);
    dds_qset_resource_limits (drQos, maxSamples, DDS_LENGTH_UNLIMITED, DDS_LENGTH_UNLIMITED);

    memset (&rd_listener, 0, sizeof (rd_listener));
    rd_listener.on_data_available = data_available_handler;

    /* A Read Condition is created which is triggered when data is available to read */

    waitSet = dds_waitset_create ();

    gCond = dds_guardcondition_create ();
    status = dds_waitset_attach (waitSet, gCond, gCond);
    DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    terminated = dds_guardcondition_create ();
    status = dds_waitset_attach (waitSet, terminated, terminated);
    DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

    /* Read samples until the maxCycles has been reached (0 = infinite) */

    imap = HandleMap__alloc ();

    memset (data, 0, sizeof (data));
    for ( i = 0; i < MAX_SAMPLES; i++)
    {
      samples[i] = &data[i];
    }

    status = dds_reader_create (subscriber, &reader, topic, drQos, &rd_listener);
    DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
    dds_qos_delete (drQos);

    printf ("Waiting for samples...\n");

    while (!done && (maxCycles == 0 || cycles < maxCycles))
    {
      if (pollingDelay)
      {
        dds_sleepfor (DDS_MSECS (pollingDelay));
      }
      else
      {
        status = dds_waitset_wait (waitSet, wsresults, wsresultsize, infinite);
        DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);

        if ((status > 0 ) && (dds_condition_triggered (gCond)))
        {
          dds_guard_reset (gCond);
        }
      }

      if (!first_batch)
      {
        deltaTv = time_now - prev_time;
        deltaTime = (double) deltaTv / DDS_NSECS_IN_SEC;
        prev_time = time_now;

        printf
        (
          "Payload size: %lu | Total received: %llu samples, %llu bytes | Out of order: %llu samples "
          "Transfer rate: %.2lf samples/s, %.2lf Mbit/s\n",
          payloadSize, total_samples, total_bytes, outOfOrder,
          (deltaTime) ? ((total_samples - prev_samples) / deltaTime) : 0,
          (deltaTime) ? (((total_bytes - prev_bytes) / BYTES_PER_SEC_TO_MEGABITS_PER_SEC) / deltaTime) : 0
        );
        cycles++;
      }
      else
      {
        prev_time = time_now;
        first_batch = false;
      }

      /* Update the previous values for next iteration */

      prev_bytes = total_bytes;
      prev_samples = total_samples;
    }

    /* Disable callbacks */

    dds_status_set_enabled (reader, 0);

    /* Output totals and averages */

    deltaTv = time_now - startTime;
    deltaTime = (double) (deltaTv / DDS_NSECS_IN_SEC);
    printf ("\nTotal received: %llu samples, %llu bytes\n", total_samples, total_bytes);
    printf ("Out of order: %llu samples\n", outOfOrder);
    printf ("Average transfer rate: %.2lf samples/s, ", total_samples / deltaTime);
    printf ("%.2lf Mbit/s\n", (total_bytes / BYTES_PER_SEC_TO_MEGABITS_PER_SEC) / deltaTime);

    HandleMap__free (imap);
  }

#ifdef _WIN32
  SetConsoleCtrlHandler (0, FALSE);
#else
  sigaction (SIGINT, &oldAction, 0);
#endif

  /* Clean up */

  for (i = 0; i < MAX_SAMPLES; i++)
  {
    ThroughputModule_DataType_free (&data[i], DDS_FREE_CONTENTS);
  }
  
  status = dds_waitset_detach (waitSet, terminated);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
  status = dds_waitset_detach (waitSet, gCond);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
  dds_condition_delete (terminated);
  dds_condition_delete (gCond);
  status = dds_waitset_delete (waitSet);
  DDS_ERR_CHECK (status, DDS_CHECK_REPORT | DDS_CHECK_EXIT);
  dds_entity_delete (participant);
  dds_fini ();

  return result;
}
