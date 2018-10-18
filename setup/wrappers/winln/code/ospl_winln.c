/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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
