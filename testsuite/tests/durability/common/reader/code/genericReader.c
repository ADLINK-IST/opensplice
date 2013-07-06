#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>

#include <os.h>
#include "dds_dcps.h"
#include "dds_dcps_private.h"

#include "genericReader.h"

#define CMDLINE_ARGS "p:h:d:xi:u:t:n:o:P"
#define DELIM ","
#define MAX_LINES 2500

#define errMsg(...) fprintf(stderr, __VA_ARGS__)
#define infoMsg(...) fprintf(stdout, __VA_ARGS__)

void printSample(simpleTopic_Type1 *sample);
DDS_ReturnCode_t topicInit();

static readerConfig                 rc;
static DDS_Topic                    topic;
static DDS_Subscriber               subscriber;
static DDS_DomainParticipant        participant;
static sampleStore                  dataStore;
static sampleStore                  dataRead;

volatile sig_atomic_t readInterrupt = 0;
volatile sig_atomic_t readAbort = 0;

os_time defaultDelay = { 0,0 };

void
printUsage(char *name) {
    printf("Generic DDS DataReader\n");
    printf("Usage: %s [-t topicname] [-p partition(s)] [-u URI]\n", name);
    printf("  -p par1,par2   Comma separated list of partitions the reader should join\n");
    printf("  -h timeout     Wait for historical data (in ms)\n");
    printf("  -x             Take, instead of read samples\n");
    printf("  -i filename    File containing reference samples\n");
    printf("  -n retries     Number of read retrys if no data available\n");
    printf("  -d delay       Retry delay (in ms)\n");
    printf("  -t topicname   Read samples using this topic name.\n");
    printf("  -P             Set the topic persistency to PERSISTENT value\n");
}

char*
ddsError(DDS_ReturnCode_t code) {
    switch (code) {
        case DDS_RETCODE_OK:
            return "result is OK";
            break;
        case DDS_RETCODE_ERROR:
            return "result is ERROR";
            break;
        case DDS_RETCODE_UNSUPPORTED:
            return "result is UNSUPPORTED";
            break;
        case DDS_RETCODE_BAD_PARAMETER:
            return "result is BAD_PARAMETER";
            break;
        case DDS_RETCODE_PRECONDITION_NOT_MET:
            return "result is PRECONDITION_NOT_MET";
            break;
        case DDS_RETCODE_OUT_OF_RESOURCES:
            return "result is OUT_OF_RESOURCES";
            break;
        case DDS_RETCODE_NOT_ENABLED:
            return "result is NOT_ENABLED";
            break;
        case DDS_RETCODE_IMMUTABLE_POLICY:
            return "result is IMMUTABLE_POLICY";
            break;
        case DDS_RETCODE_INCONSISTENT_POLICY:
            return "result is INCONSISTENT_POLICY";
            break;
        case DDS_RETCODE_ALREADY_DELETED:
            return "result is ALREADY_DELETED";
            break;
        case DDS_RETCODE_TIMEOUT:
            return "result is TIMEOUT";
            break;
        case DDS_RETCODE_NO_DATA:
            return "result is NO_DATA";
            break;
        default:
            return "result is UNKNOWN";
            break;
    }
}

DDS_ReturnCode_t
osplInit(int domainId) {
    DDS_ReturnCode_t result;
    DDS_DomainParticipantFactory factory;

    factory = DDS_DomainParticipantFactory_get_instance();
    if (factory != NULL) {
        participant = DDS_DomainParticipantFactory_create_participant(factory, domainId, DDS_PARTICIPANT_QOS_DEFAULT, NULL, 0);
    }

    if (participant) {
        result = DDS_RETCODE_OK;
    } else {
        result = DDS_RETCODE_PRECONDITION_NOT_MET;
    }
    return result;
}

DDS_ReturnCode_t
subscriberInit(char **partitions, int partitionCount) {
    int i;
    DDS_ReturnCode_t retVal;
    DDS_SubscriberQos *qos;

    retVal = DDS_RETCODE_OK;
    if (rc->subscriberQos == NULL) {
        rc->subscriberQos = DDS_SubscriberQos__alloc();
        if (rc->subscriberQos != NULL) {
            retVal = DDS_DomainParticipant_get_default_subscriber_qos(participant, rc->subscriberQos);
        } else {
            retVal = DDS_RETCODE_ERROR;
            errMsg("SubscriberQos allocation failed\n");
        }
    }

    if (retVal == DDS_RETCODE_OK) {
        qos = rc->subscriberQos;
        if (partitionCount != 0) {
            qos->partition.name._maximum = partitionCount;
            qos->partition.name._length = partitionCount;
            qos->partition.name._buffer = DDS_StringSeq_allocbuf(partitionCount + 1);
            for (i = 0; i < partitionCount; i++) {
                qos->partition.name._buffer[i] = DDS_string_dup(partitions[i]);
            }
        }
        subscriber = DDS_DomainParticipant_create_subscriber(participant, qos, NULL, 0);
        if (!(subscriber)) {
            retVal = DDS_RETCODE_ERROR;
        }
    }
    return retVal;
}

DDS_ReturnCode_t
topicInit(DDS_DurabilityQosPolicyKind durabilityKind) {
    DDS_ReturnCode_t retVal;
    DDS_TypeSupport ts;
    DDS_Duration_t delay = {10,0};

    retVal = DDS_RETCODE_OK;
    if (rc->topicQos == NULL) {
        if ((rc->topicQos = DDS_TopicQos__alloc()) != NULL) {
            retVal = DDS_DomainParticipant_get_default_topic_qos(participant, rc->topicQos);
        } else {
            retVal = DDS_RETCODE_ERROR;
            errMsg("TopicQos allocation failed\n");
        }

    }

    if (retVal == DDS_RETCODE_OK) {
        rc->topicQos->durability.kind = durabilityKind;
        rc->topicQos->durability_service.service_cleanup_delay = delay;
        rc->topicQos->reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;

        ts = simpleTopic_Type1TypeSupport__alloc();
        if (ts) {
            if ((retVal = simpleTopic_Type1TypeSupport_register_type(ts, participant, "simpleTopic::Type1")) == DDS_RETCODE_OK) {
                topic = DDS_DomainParticipant_create_topic(participant, rc->topicName, "simpleTopic::Type1", rc->topicQos, NULL, 0);
                if (!topic) {
                    retVal = DDS_RETCODE_ERROR;
                    errMsg("Topic creation failed\n");
                }
            } else {
                errMsg("Type registration failed\n");
                DDS_free(ts);
            }
        } else {
            retVal = DDS_RETCODE_ERROR;
            errMsg("TypeSupport allocation failed\n");
        }
    }
    return retVal;
}

void
toggleReaderInterrupt() {
    if (readInterrupt) {
        readInterrupt = 0;
    } else {
        readInterrupt = 1;
    }
}

void
toggleReaderAbort() {
    if (readAbort) {
        readAbort = 0;
    } else {
        readAbort = 1;
    }
}

DDS_SampleInfo*
copySampleInfo(DDS_SampleInfo *src, DDS_SampleInfo *dest) {
    DDS_SampleInfo *result;
    if (src != NULL) {
        if (dest != NULL) {
            result = dest;
        } else {
            result = os_malloc(sizeof(DDS_SampleInfo));
        }
        result->absolute_generation_rank = src->absolute_generation_rank;
        result->reception_timestamp = src->reception_timestamp;
        result->disposed_generation_count = src->disposed_generation_count;
        result->generation_rank = src->generation_rank;
        result->instance_handle = src->instance_handle;
        result->instance_state = src->instance_state;
        result->no_writers_generation_count = src->no_writers_generation_count;
        result->publication_handle = src->publication_handle;
        result->sample_rank = src->sample_rank;
        result->sample_state = src->sample_state;
        result->source_timestamp = src->source_timestamp;
        result->valid_data = src->valid_data;
        result->view_state = src->view_state;
    } else {
        result = NULL;
    }
    return result;
}

DDS_boolean compareSample(simpleTopic_Type1 *s1, simpleTopic_Type1 *s2) {
    return ((s1->key == s2->key) &&
        (s1->val1 == s2->val1) &&
        (s1->val2 == s2->val2));
}

DDS_boolean compareRefSample(refSample s1, refSample s2) {
    return (compareSample(&(s1->sample), &(s2->sample)));
}

DDS_boolean
readSample(simpleTopic_Type1 *sample, DDS_SampleInfo *info) {
    DDS_boolean result;
    int i;
    refSample ptr;

    result = FALSE;

    /* ignore dummy samples */
    if (info->valid_data && (info->view_state == DDS_NEW_VIEW_STATE)) {

        /* duplicate check */
        for (i = 0; i < dataRead->size; i++) {
            ptr = dataRead->samples[i];
            if (ptr != NULL) {
                if (ptr->sample.key == sample->key) {
                    if (compareSample(&(ptr->sample), sample)) {
                        infoMsg("Matched duplicate sample\n");
                        ptr->count++;
                        /* store updated sampleinfo */
                        ptr->info = *(copySampleInfo(info, NULL));
                        result = TRUE;
                        break;
                    }
                }
            } else {
                errMsg("Stored samples corruption\n");
                result = TRUE;
                break;
            }
        }

        /* unseen sample */
        if (result == FALSE) {
            infoMsg("New sample: %d,%d,%d\n", sample->key, sample->val1, sample->val2);
            /* Reserve room for additional refSample pointer */
            dataRead->samples = os_realloc(dataRead->samples, (dataRead->size
                + 1) * sizeof(ptr));
            /* Reserve room for the object itself */
            dataRead->samples[dataRead->size] = os_malloc(C_SIZEOF(refSample));
            ptr = dataRead->samples[dataRead->size];

            ptr->sample.key = sample->key;
            ptr->sample.val1 = sample->val1;
            ptr->sample.val2 = sample->val2;
            ptr->count = 1;

            dataRead->size++;
            result = TRUE;
            infoMsg("Done adding\n");
        }

    }
    return result;
}

DDS_ReturnCode_t
runReader() {
    DDS_DataWriter reader;
    DDS_ReturnCode_t readResult;
    DDS_sequence_simpleTopic_Type1 *sampleData;
    DDS_SampleInfoSeq *sampleInfo;
    int retryCount, bufIdx, bufSize, sampleCount;

    if (dataRead == NULL) {
        dataRead = os_malloc(C_SIZEOF(sampleStore));
        dataRead->size = 0;
        dataRead->samples = os_malloc(sizeof(dataRead->samples));
    }

    sampleData = DDS_sequence_simpleTopic_Type1__alloc();
    sampleInfo = DDS_SampleInfoSeq__alloc();
    sampleCount = 0;
    retryCount = 0;
    reader = DDS_Subscriber_create_datareader(subscriber, topic, DDS_DATAREADER_QOS_USE_TOPIC_QOS, NULL, 0);
    readResult = DDS_RETCODE_ERROR;
    if (reader) {
        while (!readAbort && (retryCount < rc->retryCount)) {
            while (!readInterrupt && (retryCount < rc->retryCount)) {
                if (rc->waitHistoricalData) {
                    readResult = simpleTopic_Type1DataReader_wait_for_historical_data(reader, &(rc->historyTimeout));
                    if (readResult != DDS_RETCODE_OK) {
                        errMsg("Wait for historical data: %s\n", ddsError(readResult));
                        toggleReaderAbort();
                        break;
                    }
                }

                /* do read / take */
                if (rc->takeSamples) {
                    readResult = simpleTopic_Type1DataReader_take(reader,
                        sampleData, sampleInfo, DDS_LENGTH_UNLIMITED,
                        DDS_ANY_SAMPLE_STATE, DDS_ANY_VIEW_STATE,
                        DDS_ANY_INSTANCE_STATE);
                } else {
                    readResult = simpleTopic_Type1DataReader_read(reader,
                        sampleData, sampleInfo, DDS_LENGTH_UNLIMITED,
                        DDS_ANY_SAMPLE_STATE, DDS_ANY_VIEW_STATE,
                        DDS_ANY_INSTANCE_STATE);
                }

                /* Process samples */
                if (readResult == DDS_RETCODE_OK) {
                    bufSize = sampleData->_length;
                    infoMsg("Read OK, %d samples\n", bufSize);
                    for(bufIdx = 0; bufIdx < bufSize; bufIdx++) {
                        if (readSample(&(sampleData->_buffer[bufIdx]), &(sampleInfo->_buffer[bufIdx]))) {
                            sampleCount++;
                        }
                    }
                    simpleTopic_Type1DataReader_return_loan(reader, sampleData, sampleInfo);
                } else if (readResult == DDS_RETCODE_NO_DATA) {
                    infoMsg("Read NODATA\n");
                } else {
                    errMsg("Read Error: %s\n", ddsError(readResult));
                    toggleReaderAbort();
                    break;
                }
                os_nanoSleep(rc->retryDelay);
                retryCount++;
            }
        }
    } else {
        errMsg("Failed to initialize reader\n");
    }
    return readResult;
}

void
printSample(simpleTopic_Type1 *sample) {
    if (sample != NULL) {
        printf("Sample: (%d, %d, %d)\n", sample->key, sample->val1, sample->val2);
    } else {
        printf("Sample: (null)\n");
    }
}

void
printRefSample(refSample sample) {
    if (sample != NULL) {
        fprintf(rc->output, "(%d,%d,%d) : %d\n", sample->sample.key, sample->sample.val1, sample->sample.val2, sample->count);
    } else {
        fprintf(rc->output, "Sample: (null)\n");
    }
}

DDS_ReturnCode_t
buildSample(char *record, simpleTopic_Type1 *sample) {
    char *ptr;
    DDS_ReturnCode_t result;
    int fieldCount;

    fieldCount = 0;
    result = DDS_RETCODE_OK;
    if (sample != NULL) {
        /* Key */
        ptr = strtok(record, DELIM);
        if (*ptr) {
            if (!(sscanf(ptr, "%d", &(sample->key)) > 0)) {
                errMsg("Error reading key from input\n");
                result = DDS_RETCODE_ERROR;
            } else {
                fieldCount++;
            }
        }

        /* val1 */
        ptr = strtok(NULL, DELIM);
        if (*ptr) {
            if (!(sscanf(ptr, "%d", &(sample->val1)) > 0)) {
                errMsg("Error reading val1 from input\n");
                result = DDS_RETCODE_ERROR;
            } else {
                fieldCount++;
            }
        }

        /* val2 */
        ptr = strtok(NULL, DELIM);
        if (*ptr) {
            if (!(sscanf(ptr, "%d", &(sample->val2)) > 0)) {
                errMsg("Error reading val1 from input\n");
                return DDS_RETCODE_ERROR;
            } else {
                fieldCount++;
            }
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
    }
    return result;
}

simpleTopic_Type1*
copySample(simpleTopic_Type1 *src, simpleTopic_Type1 *dest) {
    simpleTopic_Type1 *result;
    if (src != NULL) {
        if (dest != NULL) {
            result = dest;
        } else {
            result = os_malloc(sizeof(simpleTopic_Type1));
        }
        result->key = src->key;
        result->val1 = src->val1;
        result->val2 = src->val2;
    } else {
        result = NULL;
    }
    return result;
}

void
loadRefSamples(char *fileName) {
    FILE *file;
    char line[128];
    refSample ptr;
    simpleTopic_Type1 sample;
    int idx;
    DDS_ReturnCode_t result;

    if (dataStore == NULL) {
        dataStore = os_malloc(C_SIZEOF(sampleStore));
        dataStore->size = 0;
        dataStore->samples = os_malloc(sizeof(dataStore->samples));
    }

    if ((file = fopen(fileName, "r")) != NULL) {
        idx = 0;
        while((fgets(line, sizeof line, file) != NULL)) {
            result = buildSample(line, &sample);
            if (result == DDS_RETCODE_OK) {
                dataStore->samples = os_realloc(dataStore->samples, (dataStore->size+1) * sizeof(ptr));
                dataStore->samples[dataStore->size] = os_malloc(C_SIZEOF(refSample));
                ptr = dataStore->samples[dataStore->size];
                ptr->sample = sample;
                ptr->count = 0;
                dataStore->size++;
            } else {
                errMsg("Can't read input line: %s\n", line);
            }
        }
        fclose(file);
    }
}

DDS_ReturnCode_t
verifySamples() {
    int i,j;
    refSample sampleRead;
    refSample sampleCheck;
    DDS_boolean found;
    DDS_ReturnCode_t result;
    result = DDS_RETCODE_OK;
    infoMsg("Start verify %d samples, read %d\n", dataStore->size, dataRead->size);
    for (i=0; i < dataStore->size; i++) {
        fflush(stdout);
        found = FALSE;
        sampleCheck = dataStore->samples[i];
        for (j=0; j < dataRead->size; j++) {
            fflush(stdout);
            sampleRead = dataRead->samples[j];
            if(compareRefSample(sampleRead, sampleCheck)) {
                sampleCheck->count++;
                found = TRUE;
            }
        }
        if (!found) {
            infoMsg("Can't find sample\n");
            result = DDS_RETCODE_ERROR;
        }
        printRefSample(sampleCheck);
    }
    return result;

}

void
exitHandler(DDS_ReturnCode_t code) {
    if (rc->topicQos != NULL) {
        DDS_free(rc->topicQos);
    }
    if (rc->subscriberQos != NULL) {
        DDS_free(rc->subscriberQos);
    }
    if (participant != NULL) {
        DDS_DomainParticipant_delete_contained_entities(participant);
    }
    exit(code);
}

int main(int argc, char **argv) {
    int arg, idx, partitionCount;
    int msDelay, msHistoryTimeout;
    char *token, *topicName;
    char *inputFile, *outputFile;
    struct sigaction pauseAction;
    sigset_t blockMask;
    DDS_ReturnCode_t retVal;
    DDS_DurabilityQosPolicyKind durabilityKind;
    char **partitions;
    FILE *tmpFile;
    int domainId;

    rc = readerConfig(os_malloc(C_SIZEOF(readerConfig)));

    /* Defaults */
    rc->subscriberQos = NULL;
    rc->topicQos = NULL;
    rc->takeSamples = FALSE;
    rc->waitHistoricalData = FALSE;
    rc->retryCount = 1;

    partitions = NULL;
    inputFile = NULL;
    outputFile = NULL;
    topicName = NULL;
    durabilityKind = NULL;
    domainId = 0;
    token = NULL;
    partitionCount = 0;
    msDelay = 0;
    msHistoryTimeout = 0;

    /* Parse cmdline arguments */
    while ((arg = getopt(argc, argv, CMDLINE_ARGS)) != -1) {
        switch(arg) {
            case 'p':
                token = strtok(optarg, DELIM);
                idx = 0;
                while (token != NULL) {
                    partitions = os_realloc(partitions, (idx + 1) * sizeof(partitions));
                    if (partitions != NULL) {
                        partitions[idx] = os_strdup(token);
                    }
                    token = strtok(NULL, DELIM);
                    idx++;
                }
                partitionCount = idx;
                break;
            case 'h':
                if (!(sscanf(optarg, "%d", &msHistoryTimeout)) > 0) {
                    errMsg("Not a valid timeout");
                    printUsage(argv[0]);
                    exitHandler(1);
                } else {
                    rc->waitHistoricalData = TRUE;
                }
                break;
            case 'd':
                if (!(sscanf(optarg, "%d", &msDelay)) > 0) {
                    errMsg("Not a valid delay\n");
                    printUsage(argv[0]);
                    exitHandler(1);
                }
                break;
            case 'x':
                rc->takeSamples = TRUE;
                break;
            case 'i':
                inputFile = os_strdup(optarg);
                break;
            case 'o':
                outputFile = os_strdup(optarg);
                break;
            case 'u':
                domainId = atoi(optarg);
                break;
            case 't':
                topicName = os_strdup(optarg);
                break;
            case 'n':
                if (!(sscanf (optarg, "%d", &(rc->retryCount))) > 0) {
                    errMsg("Not a valid number of retries\n");
                    printUsage(argv[0]);
                    exitHandler(0);
                }
                break;
            case 'P':
                durabilityKind = DDS_PERSISTENT_DURABILITY_QOS;
                break;
            default:
                printUsage(argv[0]);
                exitHandler(0);
                break;
        }
    }

    if ((inputFile == NULL) || (strlen(inputFile) == 0)) {
        errMsg("Please supply file with reference samples using -i option\n");
        printUsage(argv[0]);
        exitHandler(1);
    } else {
        /* Try to open inputfile */
        char *tmpPath, *fullPath;
        fullPath = NULL;
        tmpPath = NULL;
        if ((inputFile != NULL) && (strlen(inputFile) > 0)) {
            if ((tmpFile = fopen(inputFile, "r"))) {
                fclose(tmpFile);
            } else {
                tmpPath = os_getenv("MPTC_BASE");
                if (tmpPath != NULL) {
                    fullPath = os_malloc((strlen(tmpPath) + strlen(inputFile) + strlen("%s/etc/%s")) * sizeof(char));
                    sprintf(fullPath, "%s/etc/%s", tmpPath, inputFile);
                    if ((tmpFile = fopen(fullPath, "r"))) {
                        inputFile = fullPath;
                    } else {
                        errMsg("Can't open samples file: %s\n", fullPath);
                        exitHandler(1);
                    }

                } else {
                    errMsg("Can't open samples file: %s (MPTC_BASE unset)\n", inputFile);
                    exitHandler(1);
                }
            }
        }
    }

    if ((outputFile == NULL) || (strlen(outputFile) == 0)) {
        rc->output = stdout;
    } else {
        if((rc->output = fopen(outputFile, "w")) == NULL) {
            perror("Can't log to outputFile");
            rc->output = stdout;
        }
    }

    /* Set default delay of 3 seconds if not specified */
    if (msDelay > 0) {
        rc->retryDelay.tv_sec = msDelay/1000;
        rc->retryDelay.tv_nsec = (msDelay%1000)*1000000;
    } else {
        rc->retryDelay.tv_sec = 3;
        rc->retryDelay.tv_nsec = 0;
    }

    /* Set default wait-on-historical-data timeout if not specified */
    if (msHistoryTimeout > 0) {
        rc->historyTimeout.sec = msHistoryTimeout/1000;
        rc->historyTimeout.nanosec = (msHistoryTimeout%1000)*1000000;
    } else {
        rc->historyTimeout.sec = 1;
        rc->historyTimeout.nanosec = 0;
    }

    /* Set default transient durability kind if not specified */
    if (durabilityKind == NULL) {
        durabilityKind = DDS_TRANSIENT_DURABILITY_QOS;
    }

    /* Set a default topic name if not specified */
    if (topicName == NULL) {
        rc->topicName = (durabilityKind == DDS_PERSISTENT_DURABILITY_QOS) ? "TestTopicPersistent" : "TestTopic";
    } else {
        rc->topicName = topicName;
    }

    if (inputFile != NULL) {
        loadRefSamples(inputFile);
    } else {
        /* TODO autogenerate some samples? */
    }

    /* Get a participant */
    if ((retVal = osplInit(domainId)) != DDS_RETCODE_OK) {
        errMsg("Failed to initialize. Is OpenSplice running? (%s)\n", ddsError(retVal));
        exitHandler(retVal);
    }

    /* Get a subscriber */
    if ((retVal = subscriberInit(partitions, partitionCount)) != DDS_RETCODE_OK) {
        errMsg("Failed to initialize subscriber: %s\n", ddsError(retVal));
        exitHandler(retVal);
    }

    /* Get a topic */
    if ((retVal = topicInit(durabilityKind)) != DDS_RETCODE_OK) {
        errMsg("Failed to initialize topic: %s\n", ddsError(retVal));
        exitHandler(retVal);
    }

    sigfillset(&blockMask);
    pauseAction.sa_handler = toggleReaderInterrupt;
    pauseAction.sa_mask = blockMask;
    pauseAction.sa_flags = 0;
    sigaction(SIGUSR1, &pauseAction, NULL);

    retVal = runReader();
    infoMsg("Finished reading\n");

    if (retVal == DDS_RETCODE_OK) {
        retVal = verifySamples();
    }

    exitHandler(retVal);

    exit(1);
}
