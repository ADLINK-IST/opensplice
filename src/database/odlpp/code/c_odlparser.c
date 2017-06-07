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
#include "c_base.h"
#include "c_metabase.h"
#include "c_module.h"
#include "c_genc.h"
#include "c_gencs.h"
#include "vortex_os.h"
#include "os_if.h"

extern void c_odlinit(c_module schema);
extern void c_odlparse(const char *fname);

const char* ODLPP_CMD_OPTIONS = "l:mh";
static char* execName = NULL;

typedef enum odlpp_mode {
    MODE_UNDEFINED,
    MODE_C,
    MODE_CS
} odlpp_mode;

void
usage()
{
    printf("Usage: %s [-l c|cs] [-m] <filename>\n", execName);
}

int
main(
    int argc,
    char* argv[])
{
    int opt;
    odlpp_mode mode = MODE_C; /* Generate C code by default */
    c_bool scopedNames = FALSE;
    char *odlFile;
    c_base base;

    os_osInit();
    os_serviceSetSingleProcess();
    execName = argv[0];

    if (argc == 1) {
        usage();
        return EXIT_SUCCESS;
    }

    while ((opt = getopt(argc, argv, ODLPP_CMD_OPTIONS)) != -1) {
        switch (opt) {
            case 'l':
                if (strcmp(optarg, "cs") == 0) {
                    mode = MODE_CS;
                } else if (strcmp(optarg, "c") == 0) {
                    mode = MODE_C;
                } else {
                    fprintf(stderr, "%s: Invalid generation mode '%s'\n", execName, optarg);
                    usage();
                    return EXIT_FAILURE;
                }
                break;
            case 'm':
                scopedNames = TRUE;
                break;
            case 'h':
                usage();
                return EXIT_SUCCESS;
            case '?':
                /* getopt returns '?' for unsupported options */
            default:
                usage();
                return EXIT_FAILURE;
        }
    }

    if (optind == argc) {
        fprintf(stderr, "%s: Missing filename option\n", execName);
        usage();
        return EXIT_FAILURE;
    } else {
        int result;
        odlFile = os_fileNormalize(argv[optind]);
        if (odlFile) {
            base = c_create("preprocessor", NULL, 0, 0);
            if (base) {
                c_odlinit(c_module(base));
                c_odlparse(odlFile);
                if (mode == MODE_C) {
                    c_gen_C(c_module(base), scopedNames);
                } else if (mode == MODE_CS) {
                    c_gen_CS(c_module(base), scopedNames);
                }
                result = EXIT_SUCCESS;
            } else {
                fprintf(stderr, "%s: Failed to create the database\n", execName);
                result = EXIT_FAILURE;
            }
        } else {
            fprintf(stderr, "%s: Failed to normalize filename path\n", execName);
            result = EXIT_FAILURE;
        }

        os_osExit();

        return result;
    }
}
