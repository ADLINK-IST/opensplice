#include "c_base.h"
#include "c_metabase.h"
#include "c_module.h"
#include "c_genc.h"
#include "os.h"
#include "os_if.h"

#define SERVICE_NAME_MAX 256
#define SERVICE_NAME_PREFIX "osplODLpp"

extern void c_odlinit(c_module schema);
extern void c_odlparse(const char *fname);

int
main(
    int argc,
    char* argv[])
{
    c_base base;
    int fileIndex = 1;
    c_bool scopedNames = FALSE;
    char osServiceName[SERVICE_NAME_MAX];

    if (argc < 2) {
        printf("Usage: %s [-m] <filename>\n", argv[0]);
        return -1;
    }

    snprintf(osServiceName, SERVICE_NAME_MAX, "%s%d", SERVICE_NAME_PREFIX, (int)os_procIdSelf());
    if (os_serviceStart(osServiceName) != os_resultSuccess) {
        printf("Failed to start mutex service\n");
        return -2;
    }
    os_osInit();

    if (strcmp(argv[1], "-m") == 0) {
        scopedNames =TRUE;
        fileIndex++;
    }

    base = c_create("preprocessor",NULL,0);
    c_odlinit(c_module(base));
    c_odlparse(argv[fileIndex]);
    c_gen_C(c_module(base), scopedNames);

    if (os_serviceStop() != os_resultSuccess) {
        printf("Failed to stop mutex service\n");
        return -3;
    }
    return 0;
}
