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
#define _WIN32_WINNT 0x0500

#include <Windows.h>
#include <stdio.h>

BOOL CreateHardLinkOtherwiseCopy(const char *targetName, const char *linkName)
{
    BOOL result = CreateHardLink(linkName, targetName, NULL);
    if (!result) {
        DWORD errNr = GetLastError();
        if (errNr != ERROR_INVALID_FUNCTION) { /* In case a hardlink is not supported, e.g. FAT32. */
            printf("Windows System Error %d occurred during creation of link.\n", errNr);
        } else {
            BOOL FailIfExists = TRUE;
            result = CopyFile(targetName, linkName, FailIfExists);
            if (!result) {
                printf("Windows System Error %d occurred during copying of file.\n", GetLastError());
            }
        }
    }
    return result;
}

int main( int argc, char *argv[])
{
    BOOL result = TRUE;

    if ( argc == 3 ) {
        result = CreateHardLinkOtherwiseCopy(argv[1], argv[2]);
    } else if (argc == 4 && strcmp(argv[1], "-s") == 0) {
        result = CreateHardLinkOtherwiseCopy(argv[2], argv[3]);
    } else if (argc == 2 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "--h") == 0) ) {
        printf("Usage: ospl_winln [-s] TARGET LINK_NAME\n");
        printf("create a link to TARGET with the name LINK_NAME.\n");
        printf("Create hard links by default, but delete any previous link with -s.\n");
        printf("When creating hard links, each TARGET must exist.\n");
    } else {
        printf("ospl_winln: missing file operand\n");
        printf("Try `ospl_winln --help' for more information.\n");
    }

    return !result;
}
