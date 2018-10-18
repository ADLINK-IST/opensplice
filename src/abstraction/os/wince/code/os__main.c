/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "os_heap.h"
#include "os_stdlib.h"

/** \file os/wince/code/os__main.c
 *  \brief parameter parsing for WinCE executables
 *
 *  Processes the command line into C-style argument parameters
 *
 */

/* This is what you would have in your executable source :

int WINAPI WinMain
    (HINSTANCE hInst, HINSTANCE hPrevInst, LPTSTR lpCmdLine, int nCmdShow)
{
    int argc;
    char *argv[64];
    argc = os_mainparams (lpCmdLine, argv);
    return main (argc, argv);
}

int main ...

*/

int os_mainparams (LPTSTR lpCmdLine, char *argv[])
{
    int argc = 1;
    char *c;
    char delim;
    assert(lpCmdLine);

    argv[0] = "unknown";
    c = wce_wctomb (lpCmdLine);

    while (*c)
    {
        while (*c == ' ')
        {
            c++;
        }
        if (*c)
        {
           delim = *c;
           switch (delim)
           {
              case '\'':
                 while (*++c == '\'');
                 break;
              case '\"':
                 while (*++c == '\"');
                 break;
              default:
                 delim = ' ';
           }
           if (*c)
           {
              argv[argc++] = c;
           }
        }
        while (*c && *c != delim)
        {
            c++;
        }
        if (*c)
        {
            *c = '\0';
            while (*++c == delim);
        }
    }

    argv[argc] = NULL;

    return argc;
}

