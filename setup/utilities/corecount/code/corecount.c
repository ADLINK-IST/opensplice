#if defined (unix) || defined (__unix) || defined (__unix__) || \
    defined (__linux) || defined (__linux__) || defined (__sun) || defined (__APPLE__) || \
    defined (AIX) || defined (INTEGRITY) || defined (OS_RTEMS_DEFS_H) || \
    defined (VXWORKS_RTP) || defined (__Lynx__) || defined (_WRS_KERNEL) || \
    defined (OS_QNX_DEFS_H)

#include <stdio.h>
#include <unistd.h>

int main ()
{
#ifdef _SC_NPROCESSORS_ONLN
    long n = sysconf (_SC_NPROCESSORS_ONLN);
#else
    long n = 1;
#endif
    if (n == -1) {
        printf ("1\n");
        return 1;
    }
    printf ("%ld\n", n);
    return 0;
}

#elif defined _WIN32

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

typedef BOOL (WINAPI *LPFN_GLPI)(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, PDWORD);

int countbits(ULONG_PTR mask)
{
    int n = 0;
    while (mask) {
        if (mask & 1) {
            n++;
        }
        mask >>= 1;
    }
    return n;
}

int main ()
{
    LPFN_GLPI glpi;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = NULL;
    DWORD returnLength = 0;
    DWORD byteOffset = 0;
    int n = 0;

    glpi = (LPFN_GLPI) GetProcAddress(GetModuleHandle("kernel32"), "GetLogicalProcessorInformation");
    if (glpi == 0) {
        printf ("1\n");
        return 0;
    }

    while (!glpi(buffer, &returnLength)) {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            buffer = realloc(buffer, returnLength);
        } else {
            fprintf(stderr, "corecount: error %d\n", GetLastError());
            printf ("1\n");
            return 1;
        }
    }

    ptr = buffer;
    while (byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= returnLength) {
        if (ptr->Relationship == RelationProcessorCore) {
            n += countbits(ptr->ProcessorMask);
        }
        byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
        ptr++;
    }

    printf ("%d\n", n);
    return 0;
}

#else

int main ()
{
    printf ("1\n");
    return 0;
}

#endif
