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
