#ifndef _DDS_STRING_H
#define _DDS_STRING_H

#if defined(_WIN32) && defined(_MSC_VER) && _MSC_VER < 1300
/*
 * VC++6.0 compiler has _CRTIMP problems with <string>
 */
#include <string.h>
#else
#include <string>
#endif

#endif
