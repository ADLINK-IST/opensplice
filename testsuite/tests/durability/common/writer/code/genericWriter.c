#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>

#include "os.h"
#include "dds_dcps.h"
#include "dds_dcps_private.h"

#include "genericWriter.h"

#define CMDLINE_ARGS "p:u:t:n:i:d:PD"
#define DELIM ","
#define MAX_LINES 2500

#define errMsg(...) fprintf(stderr, __VA_ARGS__)
#define infoMsg(...) fprintf(stdout, __VA_ARGS__)

void printSample(simpleTopic_Type1 *sample);
DDS_ReturnCode_t topicInit();

static writerConfig                 wc;
static DDS_Topic                    topic;
static DDS_Publisher                publisher;
static DDS_DomainParticipant        participant;
static simpleTopic_Type1            **sampleData;

volatile sig_atomic_t writeInterrupt = 0;
volatile sig_atomic_t writeAbort = 0;

os_time defaultDelay = { 0,0 };

void
printUsage(char *name) {
    printf("Generic DDS DataWriter\n");
    printf("Usage: %s -n samples [-t topicname] [-p partition(s)] [-u URI]\n", name);
    printf("  -p par1,par2   Comma separated list of partitions the writer should join\n");
    printf("  -n samples     Number of samples to write\n");
    printf("  -u URI         Use this URI instead of the OSPL_URI environment variable\n");
    printf("  -t topicname   Write samples using this topic name. If omitted the pid will be used\n");
    printf("  -i filename    Read samples from input file\n");
    printf("  -d delay       Sample write delay (in ms)\n");
    printf("  -P             Set the topic persistency to PERSISTENT value\n");
    printf("  -D             Do not auto dispose an instance if its already unregistered\n");
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
publisherInit(char **partitions, int partitionCount) {
    int i;
    DDS_ReturnCode_t retVal;
    DDS_PublisherQos *qos;

    retVal = DDS_RETCODE_OK;
    if (wc->publisherQos == NULL) {
        wc->publisherQos = DDS_PublisherQos__alloc();
        if (wc->publisherQos != NULL) {
            retVal = DDS_DomainParticipant_get_default_publisher_qos(participant, wc->publisherQos);
        } else {
            retVal = DDS_RETCODE_ERROR;
            errMsg("PublisherQos allocation failed\n");
        }
    }

    if (retVal == DDS_RETCODE_OK) {
        qos = wc->publisherQos;
        if (partitionCount != 0) {
            qos->partition.name._maximum = partitionCount;
            qos->partition.name._length = partitionCount;
            qos->partition.name._buffer = DDS_StringSeq_allocbuf(partitionCount + 1);
            for (i = 0; i < partitionCount; i++) {
                qos->partition.name._buffer[i] = DDS_string_dup(partitions[i]);
                infoMsg("Added partition %s to publisher\n", qos->partition.name._buffer[i]);
            }
        }
        publisher = DDS_DomainParticipant_create_publisher(participant, qos, NULL, 0);
        if (!(publisher)) {
            retVal = DDS_RETCODE_ERROR;
        }
    }
    return retVal;
}

DDS_ReturnCode_t
topicInit(DDS_DurabilityQosPolicyKind durabilityKind) {
    DDS_ReturnCode_t retVal;
    DDS_TypeSupport ts;
    char *typeName;
    DDS_Duration_t delay = {10, 0};

    retVal = DDS_RETCODE_OK;
    if (wc->topicQos == NULL) {
        if ((wc->topicQos = DDS_TopicQos__alloc()) != NULL) {
            retVal = DDS_DomainParticipant_get_default_topic_qos(participant, wc->topicQos);
        } else {
            retVal = DDS_RETCODE_ERROR;
            errMsg("TopicQos allocation failed\n");
        }
    }

    if (retVal == DDS_RETCODE_OK) {
        wc->topicQos->durability.kind = durabilityKind;
        if (durabilityKind == DDS_PERSISTENT_DURABILITY_QOS) {
            typeName = "simpleTopic::Type1Persistent";
        } else {
            typeName = "simpleTopic::Type1";
        }

        wc->topicQos->durability_service.service_cleanup_delay = delay;
        wc->topicQos->reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;

        ts = simpleTopic_Type1TypeSupport__alloc();
        if (ts) {
            if ((retVal = simpleTopic_Type1TypeSupport_register_type(ts, participant, typeName)) == DDS_RETCODE_OK) {
                topic = DDS_DomainParticipant_create_topic(participant, wc->topicName, typeName, wc->topicQos, NULL, 0);
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
toggleWriterInterrupt() {
    if (writeInterrupt) {
        writeInterrupt = 0;
    } else {
        writeInterrupt = 1;
    }
}

void
toggleWriterAbort() {
    if (writeAbort) {
        writeAbort = 0;
    } else {
        writeAbort = 1;
    }
}

#include "stdint.h"

DDS_ReturnCode_t
runWriter(const int autoDisposeUnregisteredInstance) {
    DDS_DataWriter writer;
    DDS_ReturnCode_t writeResult;
    DDS_ReturnCode_t result;
    DDS_DataWriterQos wqos;
    int sampleCount;
    sampleCount = 0;

    result = DDS_RETCODE_OK;
    writeResult = DDS_RETCODE_OK;

    memset(&wqos, 0, sizeof(DDS_DataWriterQos));

    /* Init writer QoS */
    if ((result = DDS_Publisher_get_default_datawriter_qos(publisher, &wqos)) != DDS_RETCODE_OK) {
        return result;
    }

    if ((result = DDS_Publisher_copy_from_topic_qos(publisher, &wqos, wc->topicQos)) != DDS_RETCODE_OK) {
        return result;
    }

    wqos.writer_data_lifecycle.autodispose_unregistered_instances = autoDisposeUnregisteredInstance;

    writer = DDS_Publisher_create_datawriter(publisher, topic, &wqos, NULL, 0);
    if (writer) {
        infoMsg("Will write %d samples...\n", wc->sampleCount);
        while (!writeAbort && (sampleCount < wc->sampleCount)) {
            while (!writeInterrupt && (sampleCount < wc->sampleCount)) {
                infoMsg("Writing sample %d/%d...\n", sampleCount+1, wc->sampleCount);
                printSample(sampleData[sampleCount]);
                writeResult = simpleTopic_Type1DataWriter_write(writer,
                    sampleData[sampleCount], DDS_HANDLE_NIL);
                if (writeResult != DDS_RETCODE_OK) {
                    errMsg("Writing sample %d failed!\n", sampleCount+1);
                    toggleWriterAbort();
                    break;
                }
                os_nanoSleep(wc->writeDelay);
                sampleCount++;
            }
        }
    } else {
        errMsg("Failed to initialize writer\n");
        writeResult = DDS_RETCODE_ERROR;
    }
    return writeResult;
}

void
printSample(simpleTopic_Type1 *sample) {
    if (sample != NULL) {
        printf("Sample: (%d, %d, %d)\n", sample->key, sample->val1, sample->val2);
    } else {
        printf("Sample: (null)\n");
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
                errMsg("Error reading key from input");
                result = DDS_RETCODE_ERROR;
            } else {
                fieldCount++;
            }
        }

        /* val1 */
        ptr = strtok(NULL, DELIM);
        if (*ptr) {
            if (!(sscanf(ptr, "%d", &(sample->val1)) > 0)) {
                errMsg("Error reading val1 from input");
                result = DDS_RETCODE_ERROR;
            } else {
                fieldCount++;
            }
        }

        /* val2 */
        ptr = strtok(NULL, DELIM);
        if (*ptr) {
            if (!(sscanf(ptr, "%d", &(sample->val2)) > 0)) {
                errMsg("Error reading val1 from input");
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

void
loadSamples(char *fileName) {
    FILE *file;
    char line[128];
    int idx, maxLines;
    simpleTopic_Type1 *sample;
    DDS_ReturnCode_t result;

    idx = 0;
    file = NULL;

    /* Read the requested number of samples, default to MAX_LINES */
    (wc->sampleCount > 0) ? (maxLines = wc->sampleCount) : (maxLines = MAX_LINES);

    sampleData = os_malloc(sizeof(sampleData));

    if ((file = fopen(fileName, "r")) != NULL) {
        /* Keep reading till end of file or maxLines is reached */
        while((fgets(line, sizeof line, file) != NULL) && (idx < maxLines)) {
            sample = os_malloc(sizeof(simpleTopic_Type1));
            result = buildSample(line, sample);
            if (result == DDS_RETCODE_OK) {
                sampleData = os_realloc(sampleData, (idx + 1) * sizeof(sampleData));
                sampleData[idx] = sample;
                idx++;
            } else {
                errMsg("Failure parsing input file: %s\n", ddsError(result));
                break;
            }
        }
        if (idx < wc->sampleCount) {
            infoMsg("Loaded %d samples, which is less than requested (%d)\n", idx, wc->sampleCount);
            wc->sampleCount = idx;
        } else {
            infoMsg("Loaded %d samples\n", idx);
            if (wc->sampleCount == 0) {
                wc->sampleCount = idx;
            }
        }
        fclose(file);

    } else {
        errMsg("Can't open file\n");
        perror(fileName);
    }
}

void
exitHandler(DDS_ReturnCode_t code) {
    if (wc->topicQos != NULL) {
        DDS_free(wc->topicQos);
    }
    if (wc->publisherQos != NULL) {
        DDS_free(wc->publisherQos);
    }
    if (participant != NULL) {
        DDS_DomainParticipant_delete_contained_entities(participant);
    }
    exit(code);
}

int main(int argc, char **argv) {
    int arg, idx, partitionCount, msDelay;
    char *token, *topicName, *inputFile;
    char *tmpPath, *fullPath;
    struct sigaction pauseAction;
    int domainId;
    sigset_t blockMask;
    DDS_ReturnCode_t retVal;
    DDS_DurabilityQosPolicyKind durabilityKind;
    int autoDisposeUnregisteredInstance;
    char **partitions;
    FILE *tmpFile;

    publisher = NULL;
    participant = NULL;
    topic = NULL;

    wc = writerConfig(os_malloc(C_SIZEOF(writerConfig)));
    wc->sampleCount = 0;
    wc->topicQos = NULL;
    wc->publisherQos = NULL;
    wc->topicName = NULL;

    participant = NULL;
    partitions = NULL;
    inputFile = NULL;
    durabilityKind = NULL;
    topicName = NULL;
    domainId = NULL;
    token = NULL;
    partitionCount = 0;
    msDelay = 0;
    autoDisposeUnregisteredInstance = FALSE;

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
            case 'u':
                domainId = atoi(optarg);
                break;
            case 't':
                topicName = os_strdup(optarg);
                break;
            case 'n':
                if (!(sscanf (optarg, "%d", &(wc->sampleCount))) > 0) {
                    errMsg("Not a valid number of samples\n");
                    printUsage(argv[0]);
                    exitHandler(0);
                }
                break;
            case 'i':
                inputFile = os_strdup(optarg);
                break;
            case 'd':
                if (!(sscanf(optarg, "%d", &msDelay)) > 0) {
                    errMsg("Not a valid delay\n");
                    printUsage(argv[0]);
                    exitHandler(1);
                }
                break;
            case 'P':
                durabilityKind = DDS_PERSISTENT_DURABILITY_QOS;
                infoMsg("Persistency enabled.\n");
                break;
            case 'D':
                autoDisposeUnregisteredInstance = 0;
            break;
            default:
                printUsage(argv[0]);
                exitHandler(0);
                break;
        }
    }

    /* Try to open inputfile */
    fullPath = NULL;
    tmpPath = NULL;
    if ((inputFile != NULL) && (strlen(inputFile) > 0)) {
        if ((tmpFile = fopen(inputFile, "r"))) {
            fclose(tmpFile);
        } else {
            tmpPath = os_getenv("MPTC_BASE");
            if (tmpPath != NULL) {
                fullPath = os_malloc((strlen(inputFile) + strlen(tmpPath) + strlen("%s/etc/%s")) * sizeof(char));
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

    /* If samplecount == 0, the number of samples found in the input file will be written so it is a mandatory option */
    if ((wc->sampleCount <= 0) && ((inputFile == NULL) || (strlen(inputFile) == 0))) {
        errMsg("Please use at least one of the -n or -i options\n");
        printUsage(argv[0]);
        exitHandler(1);
    }

    /* Set default delay of 1 second if not specified */
    if (msDelay > 0) {
        wc->writeDelay.tv_sec = msDelay/1000;
        wc->writeDelay.tv_nsec = (msDelay%1000)*1000000;
    } else {
        wc->writeDelay.tv_sec = 1;
        wc->writeDelay.tv_nsec = 0;
    }

    /* Set default transient durabilitykind if not specified */
    if (durabilityKind == NULL) {
        durabilityKind = DDS_TRANSIENT_DURABILITY_QOS;
    }

    /* Set a default topic name if not specified */
    if (topicName == NULL) {
        wc->topicName = (durabilityKind == DDS_PERSISTENT_DURABILITY_QOS) ? "TestTopicPersistent" : "TestTopic";
    } else {
        wc->topicName= topicName;
    }

    if (inputFile != NULL) {
        loadSamples(inputFile);
    } else {
        sampleData = NULL;
    }

    /* Get a participant */
    if ((retVal = osplInit(domainId)) != DDS_RETCODE_OK) {
        errMsg("Failed to initialize. Is OpenSplice running? (%s)\n", ddsError(retVal));
        exitHandler(retVal);
    }

    /* Get a publisher */
    if ((retVal = publisherInit(partitions, partitionCount)) != DDS_RETCODE_OK) {
        errMsg("Failed to initialize publisher: %s\n", ddsError(retVal));
        exitHandler(retVal);
    }

    /* Get a topic */
    if ((retVal = topicInit(durabilityKind)) != DDS_RETCODE_OK) {
        errMsg("Failed to initialize topic: %s\n", ddsError(retVal));
        exitHandler(retVal);
    }

    sigfillset(&blockMask);
    pauseAction.sa_handler = toggleWriterInterrupt;
    pauseAction.sa_mask = blockMask;
    pauseAction.sa_flags = 0;
    sigaction(SIGUSR1, &pauseAction, NULL);

    retVal = runWriter(autoDisposeUnregisteredInstance);

    exitHandler(retVal);
    exit(1);
}
