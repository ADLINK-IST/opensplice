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

#ifndef OSPL_MAIN
#define OSPL_MAIN main
#ifdef WINCE
#include "os_stdlib.h"
int main (int argc, char* argv[]);
int WINAPI WinMain
    (HINSTANCE hInst, HINSTANCE hPrevInst, LPTSTR lpCmdLine, int nCmdShow)
{
    int argc;
    char *argv[256];
    argc = os_mainparams (lpCmdLine, argv);
    return main (argc, argv);
}
#endif /* WINCE */
#endif /* ! OSPL_MAIN */

#ifdef WINCE
#ifdef __cplusplus
/* The WinCE coredll.lib/dll doesn't have symbols for ostream operator << for
non string/char types. These examples use std::cout/cerr with all sorts so
without the below you would need to link/load onto the target the MSVCRT lib/dlls
to run them.
If you are already already loading these extra libraries then you may wish to
define the below to remove this workaround instead... */
#ifndef OSPL_WINCE_HAS_MSVCRT
std::basic_ostream<char,std::char_traits<char>> &std::basic_ostream<char,std::char_traits<char>>::operator <<(int my_int)
{
    char my_buffer[23];
    _itoa(my_int, my_buffer, 10);
    return *this << my_buffer;
}
std::basic_ostream<char,std::char_traits<char>> &std::basic_ostream<char,std::char_traits<char>>::operator <<(unsigned int my_int)
{
    char my_buffer[23];
    _itoa(my_int, my_buffer, 10);
    return *this << my_buffer;
}
std::basic_ostream<char,std::char_traits<char>> &std::basic_ostream<char,std::char_traits<char>>::operator <<(long my_long)
{
    char my_buffer[23];
    _itoa(my_long, my_buffer, 10);
    return *this << my_buffer;
}
std::basic_ostream<char,std::char_traits<char>> &std::basic_ostream<char,std::char_traits<char>>::operator <<(unsigned long my_long)
{
    char my_buffer[23];
    _itoa(my_long, my_buffer, 10);
    return *this << my_buffer;
}
std::basic_ostream<char,std::char_traits<char>> &std::basic_ostream<char,std::char_traits<char>>::operator <<(float my_float)
{
    char my_buffer[23];
    sprintf(my_buffer, "%f", my_float);
    return *this << my_buffer;
}
std::basic_ostream<char,std::char_traits<char>> &std::basic_ostream<char,std::char_traits<char>>::operator <<(const void* my_pointer)
{
    char my_buffer[23];
    sprintf(my_buffer, "%p", my_pointer);
    return *this << my_buffer;
}
std::basic_ostream<char,std::char_traits<char>> &std::basic_ostream<char,std::char_traits<char>>::operator <<(bool my_bool)
{
    return *this << (my_bool ? "true" : "false");
}
#endif /* ! OSPL_WINCE_HAS_MSVCRT */
#endif /* __cplusplus */
#endif /* WINCE */
