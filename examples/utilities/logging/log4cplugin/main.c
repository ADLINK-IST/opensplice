/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */

/**
 * @file
 * This file contains a simple demonstration of replacing the
 * OpenSplice default logging with the example log4c plug-in.
 */
#include "dds_dcps.h"
#include "os_stdlib.h"
#include "os_report.h"
#include "os_heap.h"
#include "example_main.h"

int
OSPL_MAIN(int argc, const char *argv[])
{
    FILE *testDataFile;
    char testDataLine[128];
#ifdef _WIN32
    char* oldSearchPath;
    char* newSearchPath;
#endif
    DDS_DomainParticipantFactory domainParticipantFactory;
    DDS_DomainParticipant domainParticipant = DDS_OBJECT_NIL;
    os_result result = os_resultSuccess;

    /** A local configuration file is used : ospllog4cplugin.xml
     * and a DomainParticpant is created with this configuration */
    os_setenv("OSPL_URI", "file://ospllog4cplugin.xml");

#ifdef _WIN32
    /**
     * Iff the patform is windows, the plugin lib directory is added onto the
     * PATH so it can be loaded */
    oldSearchPath = os_getenv(OS_LIB_LOAD_PATH_VAR);
    if (oldSearchPath == NULL)
    {
        oldSearchPath = "";
    }
    newSearchPath = os_malloc(sizeof(".." "lib") + 2 + strlen(oldSearchPath));
    sprintf(newSearchPath, "%s%c%s%c%s", "..", OS_FILESEPCHAR, "lib", OS_PATHSEPCHAR, oldSearchPath);
    os_setenv(OS_LIB_LOAD_PATH_VAR, newSearchPath);
    os_free(newSearchPath);
#endif

    /**
     * A single process DomainParticipant is created. */
    domainParticipantFactory = DDS_DomainParticipantFactory_get_instance();
    if (domainParticipantFactory == NULL)
    {
        printf("Error: can't get the domain participant factory.\n");
        return -1;
    }
    domainParticipant = DDS_DomainParticipantFactory_create_participant(domainParticipantFactory,
                                                                       DDS_DOMAIN_ID_DEFAULT,
                                                                       DDS_PARTICIPANT_QOS_DEFAULT,
                                                                       NULL,
                                                                       DDS_STATUS_MASK_NONE);
    if (domainParticipant == NULL)
    {
        printf("Error: can't create the domain participant. Please check the configuration & the contents of any error logs.\n");
        return -1;
    }

    /** After a DomainParticipant has been created the log plug-in will have
     * been initialised and will be in use. Some test data is now loaded and
     * written as report messages. */
    testDataFile = fopen("testdata.txt", "r");
    if (testDataFile != NULL)
    {
        int i = 0;
        while (i < 1000 && result == os_resultSuccess)
        {
            if (fgets(testDataLine, sizeof(testDataLine), testDataFile) != NULL)
            {
                i++;
                testDataLine[strlen(testDataLine) - 1] = '\0';
                OS_REPORT_3 ((os_reportType) (i % (OS_REPAIRED - OS_DEBUG + 1)),
                             "OpenSplice log4c plug-in example",
                             i,
                             "Test data line %d: %s\t\t%s",
                             i, testDataLine,
                             os_reportTypeText[i % (OS_REPAIRED - OS_DEBUG + 1)]
                             );
            }
            else if (i == 0)
            {
                OS_REPORT(OS_FATAL, "OpenSplice log4c plug-in example", 0,
                                        "Data cannot be read from testsdata.txt.");
                result = os_resultFail;
            }
            else
            {
                fseek(testDataFile, 0, SEEK_SET);
            }
        }
        fclose(testDataFile);
    }

    DDS_DomainParticipantFactory_delete_participant(domainParticipantFactory,
                                                    domainParticipant);
    return result;
}
